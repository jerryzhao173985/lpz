/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Matrix Performance Benchmarking Tool                                  *
 *                                                                         *
 ***************************************************************************/

#include "matrix.h"
#include "matrix_simd.h"
#include "cpu_features.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>

namespace matrix {

class MatrixBenchmark {
public:
    struct BenchmarkResult {
        std::string name;
        double avg_time_ms;
        double min_time_ms;
        double max_time_ms;
        double gflops;
        bool correctness_passed;
    };
    
    static void run_benchmarks() {
        std::cout << "=== Matrix Performance Benchmarking ===" << std::endl;
        
        // Print CPU features
        CPUFeatures::print_features();
        std::cout << std::endl;
        
        // Test different matrix sizes
        std::vector<int> sizes = {64, 128, 256, 512, 1024};
        
        for (int size : sizes) {
            std::cout << "Benchmarking " << size << "x" << size << " matrices:" << std::endl;
            benchmark_matrix_size(size);
            std::cout << std::endl;
        }
    }
    
private:
    static void benchmark_matrix_size(int size) {
        const int num_iterations = std::max(1, 100000 / (size * size / 100));
        
        // Create random matrices
        auto A = create_random_matrix(size, size);
        auto B = create_random_matrix(size, size);
        auto C1 = Matrix(size, size);
        auto C2 = Matrix(size, size);
        
        // Benchmark standard multiplication
        auto result1 = benchmark_multiply(A, B, C1, num_iterations, "Standard", 
                                         [](const Matrix& a, const Matrix& b, Matrix& c) {
                                             c = a * b;
                                         });
        
        // Benchmark SIMD multiplication
        auto result2 = benchmark_multiply(A, B, C2, num_iterations, "SIMD Optimized", 
                                         [](const Matrix& a, const Matrix& b, Matrix& c) {
                                             MatrixSIMD::multiply(a, b, c);
                                         });
        
        // Check correctness
        bool correctness_ok = matrices_equal(C1, C2, 1e-10);
        result1.correctness_passed = true;  // Standard is reference
        result2.correctness_passed = correctness_ok;
        
        // Print results
        print_results({result1, result2});
        
        if (!correctness_ok) {
            std::cout << "⚠️  WARNING: SIMD result differs from standard multiplication!" << std::endl;
        }
        
        // Calculate speedup
        double speedup = result1.avg_time_ms / result2.avg_time_ms;
        std::cout << "Speedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    }
    
    template<typename MultiplyFunc>
    static BenchmarkResult benchmark_multiply(const Matrix& A, const Matrix& B, Matrix& C, 
                                            int iterations, const std::string& name,
                                            MultiplyFunc multiply_func) {
        BenchmarkResult result;
        result.name = name;
        
        std::vector<double> times;
        times.reserve(iterations);
        
        // Warm up
        for (int i = 0; i < 3; ++i) {
            multiply_func(A, B, C);
        }
        
        // Benchmark
        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            multiply_func(A, B, C);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            times.push_back(duration.count() / 1000000.0);  // Convert to milliseconds
        }
        
        // Calculate statistics
        result.min_time_ms = *std::min_element(times.begin(), times.end());
        result.max_time_ms = *std::max_element(times.begin(), times.end());
        result.avg_time_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        
        // Calculate GFLOPS (2 * n^3 operations for n x n matrix multiplication)
        int n = A.getM();
        double operations = 2.0 * n * n * n;
        result.gflops = operations / (result.avg_time_ms * 1e6);  // Convert ms to seconds
        
        return result;
    }
    
    static Matrix create_random_matrix(int rows, int cols) {
        Matrix m(rows, cols);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> dist(0.0, 1.0);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                m.val(i, j) = dist(gen);
            }
        }
        
        return m;
    }
    
    static bool matrices_equal(const Matrix& A, const Matrix& B, double tolerance) {
        if (A.getM() != B.getM() || A.getN() != B.getN()) {
            return false;
        }
        
        for (int i = 0; i < A.getM(); ++i) {
            for (int j = 0; j < A.getN(); ++j) {
                if (std::abs(A.val(i, j) - B.val(i, j)) > tolerance) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    static void print_results(const std::vector<BenchmarkResult>& results) {
        std::cout << std::left << std::setw(20) << "Implementation"
                  << std::setw(12) << "Avg (ms)"
                  << std::setw(12) << "Min (ms)"
                  << std::setw(12) << "Max (ms)"
                  << std::setw(12) << "GFLOPS"
                  << std::setw(12) << "Correct"
                  << std::endl;
        
        std::cout << std::string(80, '-') << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::left << std::setw(20) << result.name
                      << std::setw(12) << std::fixed << std::setprecision(3) << result.avg_time_ms
                      << std::setw(12) << std::fixed << std::setprecision(3) << result.min_time_ms
                      << std::setw(12) << std::fixed << std::setprecision(3) << result.max_time_ms
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.gflops
                      << std::setw(12) << (result.correctness_passed ? "✓" : "✗")
                      << std::endl;
        }
    }
};

} // namespace matrix

int main() {
    std::cout << "LPZRobots Matrix Performance Benchmark" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    matrix::MatrixBenchmark::run_benchmarks();
    
    return 0;
}