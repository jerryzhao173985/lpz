# LPZRobots Performance Optimization Guide

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: performance, optimization, SIMD, profiling, benchmarking  
---

## Introduction

This guide provides comprehensive performance optimization strategies for the LPZRobots framework, including SIMD implementations, memory optimization, profiling techniques, and benchmarking approaches.

## 1. SIMD Optimization Strategies

### ARM64 NEON Implementation

**Complete NEON Matrix Multiplication:**
```cpp
// matrix_neon.cpp
#include <arm_neon.h>

void matrix_multiply_neon(double* C, const double* A, const double* B, 
                         int M, int N, int K) {
    // Process 2x2 blocks (NEON processes 2 doubles at a time)
    for (int i = 0; i < M; i += 2) {
        for (int j = 0; j < N; j += 2) {
            float64x2_t c00 = vdupq_n_f64(0.0);
            float64x2_t c01 = vdupq_n_f64(0.0);
            float64x2_t c10 = vdupq_n_f64(0.0);
            float64x2_t c11 = vdupq_n_f64(0.0);
            
            for (int k = 0; k < K; k += 2) {
                // Load A matrix blocks
                float64x2_t a00 = vld1q_f64(&A[i * K + k]);
                float64x2_t a10 = vld1q_f64(&A[(i + 1) * K + k]);
                
                // Load B matrix blocks
                float64x2_t b00 = vld1q_f64(&B[k * N + j]);
                float64x2_t b01 = vld1q_f64(&B[(k + 1) * N + j]);
                
                // Perform multiplication and accumulation
                c00 = vfmaq_laneq_f64(c00, a00, b00, 0);
                c00 = vfmaq_laneq_f64(c00, a00, b01, 0);
                c01 = vfmaq_laneq_f64(c01, a00, b00, 1);
                c01 = vfmaq_laneq_f64(c01, a00, b01, 1);
                
                c10 = vfmaq_laneq_f64(c10, a10, b00, 0);
                c10 = vfmaq_laneq_f64(c10, a10, b01, 0);
                c11 = vfmaq_laneq_f64(c11, a10, b00, 1);
                c11 = vfmaq_laneq_f64(c11, a10, b01, 1);
            }
            
            // Store results
            vst1q_f64(&C[i * N + j], c00);
            vst1q_f64(&C[i * N + j + 2], c01);
            vst1q_f64(&C[(i + 1) * N + j], c10);
            vst1q_f64(&C[(i + 1) * N + j + 2], c11);
        }
    }
}

// Optimized vector operations
class VectorOpsNEON {
public:
    static double dot_product(const double* a, const double* b, int n) {
        float64x2_t sum = vdupq_n_f64(0.0);
        int i = 0;
        
        // Process 4 elements at a time
        for (; i <= n - 4; i += 4) {
            float64x2_t va1 = vld1q_f64(&a[i]);
            float64x2_t vb1 = vld1q_f64(&b[i]);
            float64x2_t va2 = vld1q_f64(&a[i + 2]);
            float64x2_t vb2 = vld1q_f64(&b[i + 2]);
            
            sum = vfmaq_f64(sum, va1, vb1);
            sum = vfmaq_f64(sum, va2, vb2);
        }
        
        // Horizontal sum
        double result = vaddvq_f64(sum);
        
        // Handle remaining elements
        for (; i < n; ++i) {
            result += a[i] * b[i];
        }
        
        return result;
    }
    
    static void add_scaled(double* result, const double* a, 
                          const double* b, double scale, int n) {
        float64x2_t vscale = vdupq_n_f64(scale);
        
        for (int i = 0; i <= n - 2; i += 2) {
            float64x2_t va = vld1q_f64(&a[i]);
            float64x2_t vb = vld1q_f64(&b[i]);
            float64x2_t vresult = vfmaq_f64(va, vb, vscale);
            vst1q_f64(&result[i], vresult);
        }
        
        // Handle remaining element
        if (n & 1) {
            result[n-1] = a[n-1] + b[n-1] * scale;
        }
    }
};
```

### AVX2 Implementation (x86_64)

**AVX2 Matrix Operations:**
```cpp
// matrix_avx2.cpp
#include <immintrin.h>

void matrix_multiply_avx2(double* C, const double* A, const double* B,
                         int M, int N, int K) {
    // Process 4x4 blocks (AVX2 processes 4 doubles at a time)
    for (int i = 0; i < M; i += 4) {
        for (int j = 0; j < N; j += 4) {
            __m256d c[4][4];
            
            // Initialize accumulators
            for (int ii = 0; ii < 4; ++ii) {
                for (int jj = 0; jj < 4; ++jj) {
                    c[ii][jj] = _mm256_setzero_pd();
                }
            }
            
            // Compute block multiplication
            for (int k = 0; k < K; ++k) {
                // Load column of A
                __m256d a_col = _mm256_set_pd(
                    A[(i + 3) * K + k],
                    A[(i + 2) * K + k],
                    A[(i + 1) * K + k],
                    A[i * K + k]
                );
                
                // Load row of B and broadcast
                for (int jj = 0; jj < 4; ++jj) {
                    __m256d b_elem = _mm256_broadcast_sd(&B[k * N + j + jj]);
                    
                    // Multiply and accumulate
                    for (int ii = 0; ii < 4; ++ii) {
                        c[ii][jj] = _mm256_fmadd_pd(a_col, b_elem, c[ii][jj]);
                    }
                }
            }
            
            // Store results
            for (int ii = 0; ii < 4; ++ii) {
                for (int jj = 0; jj < 4; ++jj) {
                    _mm256_storeu_pd(&C[(i + ii) * N + j + jj], c[ii][jj]);
                }
            }
        }
    }
}

// Vectorized activation functions
class ActivationFunctionsAVX2 {
public:
    static void tanh_vector(double* result, const double* input, int n) {
        // Constants for fast tanh approximation
        const __m256d alpha = _mm256_set1_pd(0.5);
        const __m256d beta = _mm256_set1_pd(1.0);
        
        for (int i = 0; i <= n - 4; i += 4) {
            __m256d x = _mm256_loadu_pd(&input[i]);
            
            // Fast approximation: tanh(x) ≈ x / (1 + |x|)
            __m256d abs_x = _mm256_andnot_pd(_mm256_set1_pd(-0.0), x);
            __m256d denominator = _mm256_add_pd(beta, abs_x);
            __m256d result_vec = _mm256_div_pd(x, denominator);
            
            _mm256_storeu_pd(&result[i], result_vec);
        }
        
        // Handle remaining elements
        for (int i = n - (n % 4); i < n; ++i) {
            result[i] = std::tanh(input[i]);
        }
    }
};
```

## 2. Memory Optimization Techniques

### Cache-Aware Algorithms

**Blocked Matrix Multiplication:**
```cpp
template<int BlockSize = 64>
class BlockedMatrixOps {
public:
    static void multiply(Matrix& C, const Matrix& A, const Matrix& B) {
        const int M = A.rows();
        const int N = B.cols();
        const int K = A.cols();
        
        C.setZero();
        
        // Loop over blocks
        for (int i = 0; i < M; i += BlockSize) {
            for (int k = 0; k < K; k += BlockSize) {
                for (int j = 0; j < N; j += BlockSize) {
                    // Process block
                    int i_max = std::min(i + BlockSize, M);
                    int k_max = std::min(k + BlockSize, K);
                    int j_max = std::min(j + BlockSize, N);
                    
                    // Multiply blocks
                    for (int ii = i; ii < i_max; ++ii) {
                        for (int kk = k; kk < k_max; ++kk) {
                            double a_ik = A(ii, kk);
                            
                            // Inner loop vectorization
                            #pragma omp simd
                            for (int jj = j; jj < j_max; ++jj) {
                                C(ii, jj) += a_ik * B(kk, jj);
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Prefetching optimization
    static void multiply_with_prefetch(Matrix& C, const Matrix& A, const Matrix& B) {
        const int M = A.rows();
        const int N = B.cols();
        const int K = A.cols();
        
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                double sum = 0.0;
                
                // Prefetch next cache line
                if (j + 8 < N) {
                    __builtin_prefetch(&B(0, j + 8), 0, 3);
                }
                
                for (int k = 0; k < K; ++k) {
                    // Prefetch for next iteration
                    if (k + 1 < K) {
                        __builtin_prefetch(&B(k + 1, j), 0, 3);
                    }
                    
                    sum += A(i, k) * B(k, j);
                }
                
                C(i, j) = sum;
            }
        }
    }
};
```

### Memory Pool Optimization

**High-Performance Memory Pool:**
```cpp
template<typename T, size_t Alignment = 64>
class AlignedMemoryPool {
    struct Block {
        alignas(Alignment) std::array<char, sizeof(T)> data;
        Block* next;
        
        T* get() { return reinterpret_cast<T*>(data.data()); }
    };
    
    struct Chunk {
        static constexpr size_t BlocksPerChunk = 1024;
        std::unique_ptr<Block[]> blocks;
        size_t used = 0;
        
        Chunk() : blocks(std::make_unique<Block[]>(BlocksPerChunk)) {
            // Link blocks
            for (size_t i = 0; i < BlocksPerChunk - 1; ++i) {
                blocks[i].next = &blocks[i + 1];
            }
            blocks[BlocksPerChunk - 1].next = nullptr;
        }
    };
    
    std::vector<std::unique_ptr<Chunk>> chunks_;
    Block* free_list_ = nullptr;
    mutable std::mutex mutex_;
    
    // Statistics
    std::atomic<size_t> allocations_{0};
    std::atomic<size_t> deallocations_{0};
    std::atomic<size_t> chunks_created_{0};
    
    void grow() {
        auto chunk = std::make_unique<Chunk>();
        free_list_ = &chunk->blocks[0];
        chunks_.push_back(std::move(chunk));
        chunks_created_++;
    }
    
public:
    T* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!free_list_) {
            grow();
        }
        
        Block* block = free_list_;
        free_list_ = block->next;
        allocations_++;
        
        return block->get();
    }
    
    void deallocate(T* ptr) {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        Block* block = reinterpret_cast<Block*>(
            reinterpret_cast<char*>(ptr) - offsetof(Block, data));
        
        block->next = free_list_;
        free_list_ = block;
        deallocations_++;
    }
    
    void print_stats() const {
        std::cout << "Memory Pool Statistics:\n"
                  << "  Allocations: " << allocations_ << "\n"
                  << "  Deallocations: " << deallocations_ << "\n"
                  << "  Active objects: " << allocations_ - deallocations_ << "\n"
                  << "  Chunks created: " << chunks_created_ << "\n"
                  << "  Total capacity: " << chunks_created_ * Chunk::BlocksPerChunk << "\n";
    }
};
```

## 3. Profiling and Benchmarking

### Comprehensive Profiling Framework

**Profiler Implementation:**
```cpp
class PerformanceProfiler {
public:
    struct ProfileData {
        std::string name;
        std::chrono::nanoseconds total_time{0};
        std::chrono::nanoseconds min_time{std::numeric_limits<int64_t>::max()};
        std::chrono::nanoseconds max_time{0};
        size_t call_count{0};
    };
    
private:
    std::unordered_map<std::string, ProfileData> profiles_;
    mutable std::mutex mutex_;
    
public:
    class ScopedTimer {
        PerformanceProfiler* profiler_;
        std::string name_;
        std::chrono::high_resolution_clock::time_point start_;
        
    public:
        ScopedTimer(PerformanceProfiler* profiler, const std::string& name)
            : profiler_(profiler), name_(name),
              start_(std::chrono::high_resolution_clock::now()) {}
        
        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = end - start_;
            profiler_->record(name_, duration);
        }
    };
    
    void record(const std::string& name, std::chrono::nanoseconds duration) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& data = profiles_[name];
        data.name = name;
        data.total_time += duration;
        data.min_time = std::min(data.min_time, duration);
        data.max_time = std::max(data.max_time, duration);
        data.call_count++;
    }
    
    void report(std::ostream& out = std::cout) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        out << "\nPerformance Profile Report\n";
        out << std::string(80, '=') << "\n";
        out << std::setw(30) << "Function"
            << std::setw(10) << "Calls"
            << std::setw(12) << "Total (ms)"
            << std::setw(12) << "Avg (µs)"
            << std::setw(12) << "Min (µs)"
            << std::setw(12) << "Max (µs)" << "\n";
        out << std::string(80, '-') << "\n";
        
        std::vector<ProfileData> sorted_data;
        for (const auto& [_, data] : profiles_) {
            sorted_data.push_back(data);
        }
        
        // Sort by total time
        std::sort(sorted_data.begin(), sorted_data.end(),
                  [](const ProfileData& a, const ProfileData& b) {
                      return a.total_time > b.total_time;
                  });
        
        for (const auto& data : sorted_data) {
            double total_ms = data.total_time.count() / 1e6;
            double avg_us = data.total_time.count() / 1e3 / data.call_count;
            double min_us = data.min_time.count() / 1e3;
            double max_us = data.max_time.count() / 1e3;
            
            out << std::setw(30) << data.name
                << std::setw(10) << data.call_count
                << std::setw(12) << std::fixed << std::setprecision(2) << total_ms
                << std::setw(12) << avg_us
                << std::setw(12) << min_us
                << std::setw(12) << max_us << "\n";
        }
    }
};

// Global profiler instance
inline PerformanceProfiler& getProfiler() {
    static PerformanceProfiler profiler;
    return profiler;
}

// Convenience macro
#define PROFILE_SCOPE(name) \
    PerformanceProfiler::ScopedTimer _timer(&getProfiler(), name)
```

### Benchmarking Suite

**Comprehensive Benchmark Framework:**
```cpp
class BenchmarkSuite {
public:
    struct BenchmarkResult {
        std::string name;
        double ops_per_second;
        double time_per_op_ns;
        double throughput_mbps;
        std::vector<double> samples;
    };
    
    template<typename Func>
    static BenchmarkResult run(const std::string& name, Func&& func,
                              size_t iterations = 1000,
                              size_t warmup_iterations = 100) {
        BenchmarkResult result;
        result.name = name;
        
        // Warmup
        for (size_t i = 0; i < warmup_iterations; ++i) {
            func();
        }
        
        // Actual benchmark
        std::vector<double> times;
        times.reserve(iterations);
        
        for (size_t i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration<double, std::nano>(end - start);
            times.push_back(duration.count());
        }
        
        // Calculate statistics
        double total_time = std::accumulate(times.begin(), times.end(), 0.0);
        result.time_per_op_ns = total_time / iterations;
        result.ops_per_second = 1e9 / result.time_per_op_ns;
        result.samples = std::move(times);
        
        return result;
    }
    
    static void matrix_benchmarks() {
        std::cout << "\nMatrix Operation Benchmarks\n";
        std::cout << std::string(60, '=') << "\n";
        
        const std::vector<int> sizes = {10, 50, 100, 200, 500};
        
        for (int size : sizes) {
            Matrix A(size, size), B(size, size), C(size, size);
            A.randomize(-1.0, 1.0);
            B.randomize(-1.0, 1.0);
            
            // Standard multiplication
            auto result_standard = run(
                "Matrix Multiply " + std::to_string(size) + "x" + std::to_string(size),
                [&]() { C = A * B; },
                100
            );
            
            // SIMD multiplication
            auto result_simd = run(
                "Matrix Multiply SIMD " + std::to_string(size) + "x" + std::to_string(size),
                [&]() { 
                    #ifdef USE_NEON
                    matrix_multiply_neon(C.data(), A.data(), B.data(), size, size, size);
                    #else
                    C = A * B;
                    #endif
                },
                100
            );
            
            double gflops_standard = (2.0 * size * size * size) / result_standard.time_per_op_ns;
            double gflops_simd = (2.0 * size * size * size) / result_simd.time_per_op_ns;
            
            std::cout << "Size " << size << "x" << size << ":\n";
            std::cout << "  Standard: " << result_standard.time_per_op_ns / 1e6 
                      << " ms (" << gflops_standard << " GFLOPS)\n";
            std::cout << "  SIMD:     " << result_simd.time_per_op_ns / 1e6 
                      << " ms (" << gflops_simd << " GFLOPS)\n";
            std::cout << "  Speedup:  " << result_standard.time_per_op_ns / result_simd.time_per_op_ns 
                      << "x\n\n";
        }
    }
};
```

## 4. Controller Performance Optimization

### Optimized Controller Implementation

**High-Performance Controller Base:**
```cpp
class OptimizedControllerBase : public AbstractController {
protected:
    // Aligned matrices for SIMD
    alignas(64) Matrix A;
    alignas(64) Matrix C;
    alignas(64) Matrix h;
    
    // Pre-allocated workspace
    Matrix workspace1, workspace2, workspace3;
    
    // Memory pool for temporary matrices
    static AlignedMemoryPool<Matrix>& getMatrixPool() {
        static AlignedMemoryPool<Matrix> pool;
        return pool;
    }
    
public:
    void step(const sensor* sensors, int sensor_num,
              motor* motors, int motor_num) override {
        PROFILE_SCOPE("OptimizedController::step");
        
        // Use memory pool for temporaries
        auto x = getMatrixPool().allocate();
        auto y = getMatrixPool().allocate();
        auto y_pred = getMatrixPool().allocate();
        
        // Efficient sensor input
        x->set(sensor_num, 1, sensors);
        
        // Optimized forward pass
        {
            PROFILE_SCOPE("Forward pass");
            
            // y = C * y_buffer + A * x + h
            #ifdef USE_NEON
            VectorOpsNEON::add_scaled(y->data(), h.data(), 
                                     x->data(), 1.0, motor_num);
            // ... NEON matrix operations ...
            #else
            *y = C * y_buffer.get(-1) + A * (*x) + h;
            #endif
            
            // Activation function
            y->map(g);
        }
        
        // Learning update
        {
            PROFILE_SCOPE("Learning update");
            updateWeights(*x, *y, *y_pred);
        }
        
        // Output
        y->convertToBuffer(motors, motor_num);
        
        // Return matrices to pool
        getMatrixPool().deallocate(x);
        getMatrixPool().deallocate(y);
        getMatrixPool().deallocate(y_pred);
    }
    
private:
    void updateWeights(const Matrix& x, const Matrix& y, 
                      const Matrix& y_pred) {
        // Optimized weight update with minimal allocations
        // ... implementation ...
    }
};
```

## 5. Performance Best Practices

### General Guidelines

1. **Profile First**: Always measure before optimizing
2. **Cache Efficiency**: Keep data access patterns linear
3. **SIMD Usage**: Align data and use appropriate intrinsics
4. **Memory Allocation**: Use pools for frequent allocations
5. **Parallelization**: Use OpenMP for independent operations

### Optimization Checklist

- [ ] Profile code to identify hotspots
- [ ] Check memory alignment (64-byte for cache lines)
- [ ] Verify SIMD usage in critical paths
- [ ] Minimize allocations in loops
- [ ] Use appropriate data structures
- [ ] Consider cache blocking for large data
- [ ] Enable compiler optimizations (-O3, -march=native)
- [ ] Test with different input sizes
- [ ] Measure performance improvements
- [ ] Document optimization rationale

## Performance Monitoring

### Runtime Performance Monitoring

```cpp
class PerformanceMonitor {
    struct Metrics {
        std::atomic<uint64_t> step_count{0};
        std::atomic<uint64_t> total_time_ns{0};
        std::atomic<double> avg_step_time_ms{0};
        std::chrono::steady_clock::time_point start_time;
    };
    
    std::unordered_map<std::string, Metrics> controller_metrics_;
    
public:
    void recordStep(const std::string& controller_name, 
                   std::chrono::nanoseconds duration) {
        auto& metrics = controller_metrics_[controller_name];
        metrics.step_count++;
        metrics.total_time_ns += duration.count();
        metrics.avg_step_time_ms = metrics.total_time_ns / 1e6 / metrics.step_count;
    }
    
    void printReport() const {
        std::cout << "\nPerformance Monitoring Report\n";
        std::cout << std::string(60, '=') << "\n";
        
        for (const auto& [name, metrics] : controller_metrics_) {
            std::cout << "Controller: " << name << "\n";
            std::cout << "  Steps: " << metrics.step_count << "\n";
            std::cout << "  Avg step time: " << metrics.avg_step_time_ms << " ms\n";
            std::cout << "  Total time: " << metrics.total_time_ns / 1e9 << " s\n\n";
        }
    }
};
```

This performance optimization guide provides the tools and techniques needed to achieve maximum performance from the LPZRobots framework on modern hardware.