/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 ***************************************************************************/

#ifndef __MATRIX_POOL_H
#define __MATRIX_POOL_H

#include "matrix.h"
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>

namespace matrix {

/**
 * @brief High-performance memory pool for Matrix objects
 * 
 * Addresses the critical memory management bottleneck in LPZRobots control loops
 * where frequent matrix allocations cause:
 * - Memory fragmentation
 * - Cache misses
 * - Real-time performance degradation
 * 
 * The pool provides:
 * - Zero-allocation matrix borrowing for temporary calculations
 * - Automatic lifetime management with RAII
 * - Thread-safe operations for multi-threaded simulations
 * - Size-optimized pools for common matrix dimensions
 * 
 * Example usage:
 * ```cpp
 * // Instead of: Matrix temp = A * B + C;  // 2 allocations
 * auto temp_guard = MatrixPool::borrow(A.getM(), B.getN());
 * Matrix& temp = temp_guard.get();
 * temp.mult(A, B);
 * temp.toSum(C);
 * // Automatic return to pool when temp_guard goes out of scope
 * ```
 */
class MatrixPool {
public:
    /**
     * @brief RAII guard for borrowed matrices
     * 
     * Ensures matrices are automatically returned to the pool
     * when the guard goes out of scope, preventing memory leaks.
     */
    class MatrixGuard {
    public:
        explicit MatrixGuard(Matrix* matrix, MatrixPool* pool)
            : matrix_(matrix), pool_(pool) {}
        
        ~MatrixGuard() {
            if (matrix_ && pool_) {
                pool_->return_matrix(matrix_);
            }
        }
        
        // Move-only semantics for safety
        MatrixGuard(const MatrixGuard&) = delete;
        MatrixGuard& operator=(const MatrixGuard&) = delete;
        
        MatrixGuard(MatrixGuard&& other) noexcept
            : matrix_(other.matrix_), pool_(other.pool_) {
            other.matrix_ = nullptr;
            other.pool_ = nullptr;
        }
        
        MatrixGuard& operator=(MatrixGuard&& other) noexcept {
            if (this != &other) {
                if (matrix_ && pool_) {
                    pool_->return_matrix(matrix_);
                }
                matrix_ = other.matrix_;
                pool_ = other.pool_;
                other.matrix_ = nullptr;
                other.pool_ = nullptr;
            }
            return *this;
        }
        
        Matrix& get() { return *matrix_; }
        const Matrix& get() const { return *matrix_; }
        Matrix& operator*() { return *matrix_; }
        const Matrix& operator*() const { return *matrix_; }
        Matrix* operator->() { return matrix_; }
        const Matrix* operator->() const { return matrix_; }
        
    private:
        Matrix* matrix_;
        MatrixPool* pool_;
    };
    
    /**
     * @brief Get the global matrix pool instance
     * 
     * Thread-safe singleton pattern for global access.
     */
    static MatrixPool& instance() {
        static MatrixPool pool;
        return pool;
    }
    
    /**
     * @brief Borrow a matrix from the pool
     * 
     * @param rows Number of rows
     * @param cols Number of columns
     * @return RAII guard that automatically returns the matrix
     */
    static MatrixGuard borrow(unsigned int rows, unsigned int cols) {
        return instance().borrow_matrix(rows, cols);
    }
    
    /**
     * @brief Borrow a matrix with the same dimensions as another matrix
     * 
     * @param reference Matrix to match dimensions
     * @return RAII guard for the borrowed matrix
     */
    static MatrixGuard borrow_like(const Matrix& reference) {
        return borrow(reference.getM(), reference.getN());
    }
    
    /**
     * @brief Pre-allocate matrices for common sizes
     * 
     * Should be called during initialization to avoid allocations
     * during real-time control loops.
     * 
     * @param sizes Vector of (rows, cols) pairs to pre-allocate
     * @param count Number of matrices per size to pre-allocate
     */
    static void preallocate(const std::vector<std::pair<unsigned int, unsigned int>>& sizes, 
                           unsigned int count = 10) {
        instance().preallocate_matrices(sizes, count);
    }
    
    /**
     * @brief Get pool statistics
     * 
     * @return String with allocation statistics for debugging
     */
    static std::string get_statistics() {
        return instance().get_pool_statistics();
    }
    
    /**
     * @brief Clear all pooled matrices
     * 
     * Used for cleanup or memory pressure situations.
     */
    static void clear() {
        instance().clear_pool();
    }

private:
    MatrixPool() = default;
    ~MatrixPool() = default;
    
    // Non-copyable singleton
    MatrixPool(const MatrixPool&) = delete;
    MatrixPool& operator=(const MatrixPool&) = delete;
    
    MatrixGuard borrow_matrix(unsigned int rows, unsigned int cols) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto key = make_key(rows, cols);
        auto& pool = pools_[key];
        
        Matrix* matrix;
        if (!pool.empty()) {
            matrix = pool.back().release();
            pool.pop_back();
            hits_++;
        } else {
            matrix = new Matrix(rows, cols);
            misses_++;
        }
        
        // Clear matrix data for reuse
        matrix->toZero();
        
        return MatrixGuard(matrix, this);
    }
    
    void return_matrix(Matrix* matrix) {
        if (!matrix) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto key = make_key(matrix->getM(), matrix->getN());
        auto& pool = pools_[key];
        
        // Limit pool size to prevent unbounded growth
        if (pool.size() < max_pool_size_) {
            pool.emplace_back(matrix);
        } else {
            delete matrix;
        }
    }
    
    void preallocate_matrices(const std::vector<std::pair<unsigned int, unsigned int>>& sizes,
                             unsigned int count) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& [rows, cols] : sizes) {
            auto key = make_key(rows, cols);
            auto& pool = pools_[key];
            
            for (unsigned int i = 0; i < count; ++i) {
                pool.emplace_back(std::make_unique<Matrix>(rows, cols));
            }
        }
    }
    
    std::string get_pool_statistics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        size_t total_matrices = 0;
        size_t total_pools = pools_.size();
        
        for (const auto& [key, pool] : pools_) {
            total_matrices += pool.size();
        }
        
        double hit_rate = (hits_ + misses_ > 0) ? 
            (double)hits_ / (hits_ + misses_) * 100.0 : 0.0;
        
        return "MatrixPool Statistics:\n"
               "  Total pools: " + std::to_string(total_pools) + "\n"
               "  Pooled matrices: " + std::to_string(total_matrices) + "\n"
               "  Cache hits: " + std::to_string(hits_) + "\n"
               "  Cache misses: " + std::to_string(misses_) + "\n"
               "  Hit rate: " + std::to_string(hit_rate) + "%\n";
    }
    
    void clear_pool() {
        std::lock_guard<std::mutex> lock(mutex_);
        pools_.clear();
        hits_ = 0;
        misses_ = 0;
    }
    
    // Generate unique key for matrix dimensions
    uint64_t make_key(unsigned int rows, unsigned int cols) const {
        return (static_cast<uint64_t>(rows) << 32) | cols;
    }
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Pools organized by matrix dimensions
    std::unordered_map<uint64_t, std::vector<std::unique_ptr<Matrix>>> pools_;
    
    // Statistics
    mutable size_t hits_ = 0;
    mutable size_t misses_ = 0;
    
    // Configuration
    static constexpr size_t max_pool_size_ = 50;  // Per dimension pool limit
};

/**
 * @brief Convenience function for temporary matrix operations
 * 
 * Use this in expressions where temporary matrices are needed:
 * 
 * ```cpp
 * // Instead of:
 * Matrix temp1 = A * B;
 * Matrix temp2 = temp1 + C;
 * result = temp2 * D;
 * 
 * // Use:
 * auto temp1 = temp_matrix_like(A);
 * temp1->mult(A, B);
 * auto temp2 = temp_matrix_like(*temp1);
 * temp2->add(*temp1, C);
 * result.mult(*temp2, D);
 * ```
 */
inline MatrixPool::MatrixGuard temp_matrix(unsigned int rows, unsigned int cols) {
    return MatrixPool::borrow(rows, cols);
}

inline MatrixPool::MatrixGuard temp_matrix_like(const Matrix& reference) {
    return MatrixPool::borrow_like(reference);
}

} // namespace matrix

#endif // __MATRIX_POOL_H