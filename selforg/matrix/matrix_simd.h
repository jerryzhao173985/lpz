/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   SIMD-optimized matrix operations for ARM NEON and x86 AVX            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MATRIX_SIMD_H
#define __MATRIX_SIMD_H

#include "matrix.h"
#include <cstring>

// Platform-specific includes
#if defined(USE_ARM_NEON) && defined(__ARM_NEON)
  #include <arm_neon.h>
#elif defined(USE_AVX2) && defined(__AVX2__)
  #include <immintrin.h>
#endif

namespace matrix {

// SIMD-optimized matrix operations
class MatrixSIMD {
public:
    // Optimized matrix multiplication
    static void multiply(const Matrix& A, const Matrix& B, Matrix& result) {
        assert(A.getN() == B.getM());
        assert(result.getM() == A.getM() && result.getN() == B.getN());
        
#if defined(USE_ARM_NEON) && defined(__ARM_NEON)
        multiply_neon(A, B, result);
#elif defined(USE_AVX2) && defined(__AVX2__)
        multiply_avx2(A, B, result);
#else
        multiply_scalar(A, B, result);
#endif
    }
    
    // Optimized matrix addition
    static void add(const Matrix& A, const Matrix& B, Matrix& result) {
        assert(A.getM() == B.getM() && A.getN() == B.getN());
        assert(result.getM() == A.getM() && result.getN() == A.getN());
        
#if defined(USE_ARM_NEON) && defined(__ARM_NEON)
        add_neon(A, B, result);
#elif defined(USE_AVX2) && defined(__AVX2__)
        add_avx2(A, B, result);
#else
        add_scalar(A, B, result);
#endif
    }
    
    // Optimized scalar multiplication
    static void scale(const Matrix& A, double scalar, Matrix& result) {
        assert(result.getM() == A.getM() && result.getN() == A.getN());
        
#if defined(USE_ARM_NEON) && defined(__ARM_NEON)
        scale_neon(A, scalar, result);
#elif defined(USE_AVX2) && defined(__AVX2__)
        scale_avx2(A, scalar, result);
#else
        scale_scalar(A, scalar, result);
#endif
    }
    
    // Optimized transpose
    static void transpose(const Matrix& A, Matrix& result) {
        assert(result.getM() == A.getN() && result.getN() == A.getM());
        
        if (A.getM() == 4 && A.getN() == 4) {
#if defined(USE_ARM_NEON) && defined(__ARM_NEON)
            transpose4x4_neon(A, result);
#elif defined(USE_AVX2) && defined(__AVX2__)
            transpose4x4_avx2(A, result);
#else
            transpose_scalar(A, result);
#endif
        } else {
            transpose_scalar(A, result);
        }
    }

private:
    // Scalar implementations (fallback)
    static void multiply_scalar(const Matrix& A, const Matrix& B, Matrix& C) {
        const int M = A.getM();
        const int N = B.getN();
        const int K = A.getN();
        
        // Cache-friendly blocked multiplication
        const int BLOCK_SIZE = 64;
        
        // Clear result
        std::memset(C.data, 0, M * N * sizeof(double));
        
        for (int i0 = 0; i0 < M; i0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < K; k0 += BLOCK_SIZE) {
                for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                    // Multiply blocks
                    for (int i = i0; i < std::min(i0 + BLOCK_SIZE, M); ++i) {
                        for (int k = k0; k < std::min(k0 + BLOCK_SIZE, K); ++k) {
                            double aik = A.val(i, k);
                            for (int j = j0; j < std::min(j0 + BLOCK_SIZE, N); ++j) {
                                C.val(i, j) += aik * B.val(k, j);
                            }
                        }
                    }
                }
            }
        }
    }
    
    static void add_scalar(const Matrix& A, const Matrix& B, Matrix& C) {
        const int size = A.getM() * A.getN();
        for (int i = 0; i < size; ++i) {
            C.data[i] = A.data[i] + B.data[i];
        }
    }
    
    static void scale_scalar(const Matrix& A, double scalar, Matrix& C) {
        const int size = A.getM() * A.getN();
        for (int i = 0; i < size; ++i) {
            C.data[i] = A.data[i] * scalar;
        }
    }
    
    static void transpose_scalar(const Matrix& A, Matrix& C) {
        for (int i = 0; i < A.getM(); ++i) {
            for (int j = 0; j < A.getN(); ++j) {
                C.val(j, i) = A.val(i, j);
            }
        }
    }
    
#if defined(USE_ARM_NEON) && defined(__ARM_NEON)
    // ARM NEON implementations
    static void multiply_neon(const Matrix& A, const Matrix& B, Matrix& C) {
        const int M = A.getM();
        const int N = B.getN();
        const int K = A.getN();
        
        // Clear result
        std::memset(C.data, 0, M * N * sizeof(double));
        
        // Process 2x2 blocks for better NEON utilization
        for (int i = 0; i < M; i += 2) {
            for (int j = 0; j < N; j += 2) {
                float64x2_t c00 = vdupq_n_f64(0.0);
                float64x2_t c01 = vdupq_n_f64(0.0);
                float64x2_t c10 = vdupq_n_f64(0.0);
                float64x2_t c11 = vdupq_n_f64(0.0);
                
                for (int k = 0; k < K; ++k) {
                    float64x2_t a0 = vld1q_f64(&A.val(i, k));
                    float64x2_t a1 = (i + 1 < M) ? vld1q_f64(&A.val(i + 1, k)) : vdupq_n_f64(0.0);
                    
                    float64x2_t b0 = vld1q_f64(&B.val(k, j));
                    float64x2_t b1 = (j + 1 < N) ? vld1q_f64(&B.val(k, j + 1)) : vdupq_n_f64(0.0);
                    
                    c00 = vfmaq_f64(c00, a0, b0);
                    c01 = vfmaq_f64(c01, a0, b1);
                    c10 = vfmaq_f64(c10, a1, b0);
                    c11 = vfmaq_f64(c11, a1, b1);
                }
                
                // Store results
                vst1q_f64(&C.val(i, j), c00);
                if (j + 1 < N) vst1q_f64(&C.val(i, j + 1), c01);
                if (i + 1 < M) vst1q_f64(&C.val(i + 1, j), c10);
                if (i + 1 < M && j + 1 < N) vst1q_f64(&C.val(i + 1, j + 1), c11);
            }
        }
    }
    
    static void add_neon(const Matrix& A, const Matrix& B, Matrix& C) {
        const int size = A.getM() * A.getN();
        const int simd_size = size - (size % 2);
        
        for (int i = 0; i < simd_size; i += 2) {
            float64x2_t a = vld1q_f64(&A.data[i]);
            float64x2_t b = vld1q_f64(&B.data[i]);
            float64x2_t c = vaddq_f64(a, b);
            vst1q_f64(&C.data[i], c);
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; ++i) {
            C.data[i] = A.data[i] + B.data[i];
        }
    }
    
    static void scale_neon(const Matrix& A, double scalar, Matrix& C) {
        const int size = A.getM() * A.getN();
        const int simd_size = size - (size % 2);
        float64x2_t s = vdupq_n_f64(scalar);
        
        for (int i = 0; i < simd_size; i += 2) {
            float64x2_t a = vld1q_f64(&A.data[i]);
            float64x2_t c = vmulq_f64(a, s);
            vst1q_f64(&C.data[i], c);
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; ++i) {
            C.data[i] = A.data[i] * scalar;
        }
    }
    
    static void transpose4x4_neon(const Matrix& A, Matrix& C) {
        // Load 4x4 matrix
        float64x2_t row0 = vld1q_f64(&A.val(0, 0));
        float64x2_t row1 = vld1q_f64(&A.val(1, 0));
        float64x2_t row2 = vld1q_f64(&A.val(2, 0));
        float64x2_t row3 = vld1q_f64(&A.val(3, 0));
        
        // Transpose
        float64x2_t t0 = vzip1q_f64(row0, row1);
        float64x2_t t1 = vzip2q_f64(row0, row1);
        float64x2_t t2 = vzip1q_f64(row2, row3);
        float64x2_t t3 = vzip2q_f64(row2, row3);
        
        // Store transposed matrix
        vst1q_f64(&C.val(0, 0), t0);
        vst1q_f64(&C.val(1, 0), t1);
        vst1q_f64(&C.val(2, 0), t2);
        vst1q_f64(&C.val(3, 0), t3);
    }
#endif // USE_ARM_NEON

#if defined(USE_AVX2) && defined(__AVX2__)
    // x86 AVX2 implementations
    static void multiply_avx2(const Matrix& A, const Matrix& B, Matrix& C) {
        const int M = A.getM();
        const int N = B.getN();
        const int K = A.getN();
        
        // Clear result
        std::memset(C.data, 0, M * N * sizeof(double));
        
        // Process 4x4 blocks for AVX2
        for (int i = 0; i < M; i += 4) {
            for (int j = 0; j < N; j += 4) {
                __m256d c0 = _mm256_setzero_pd();
                __m256d c1 = _mm256_setzero_pd();
                __m256d c2 = _mm256_setzero_pd();
                __m256d c3 = _mm256_setzero_pd();
                
                for (int k = 0; k < K; ++k) {
                    __m256d a = _mm256_loadu_pd(&A.val(i, k));
                    __m256d b0 = _mm256_broadcast_sd(&B.val(k, j));
                    __m256d b1 = _mm256_broadcast_sd(&B.val(k, j + 1));
                    __m256d b2 = _mm256_broadcast_sd(&B.val(k, j + 2));
                    __m256d b3 = _mm256_broadcast_sd(&B.val(k, j + 3));
                    
                    c0 = _mm256_fmadd_pd(a, b0, c0);
                    c1 = _mm256_fmadd_pd(a, b1, c1);
                    c2 = _mm256_fmadd_pd(a, b2, c2);
                    c3 = _mm256_fmadd_pd(a, b3, c3);
                }
                
                // Store results
                _mm256_storeu_pd(&C.val(i, j), c0);
                _mm256_storeu_pd(&C.val(i, j + 1), c1);
                _mm256_storeu_pd(&C.val(i, j + 2), c2);
                _mm256_storeu_pd(&C.val(i, j + 3), c3);
            }
        }
    }
    
    static void add_avx2(const Matrix& A, const Matrix& B, Matrix& C) {
        const int size = A.getM() * A.getN();
        const int simd_size = size - (size % 4);
        
        for (int i = 0; i < simd_size; i += 4) {
            __m256d a = _mm256_loadu_pd(&A.data[i]);
            __m256d b = _mm256_loadu_pd(&B.data[i]);
            __m256d c = _mm256_add_pd(a, b);
            _mm256_storeu_pd(&C.data[i], c);
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; ++i) {
            C.data[i] = A.data[i] + B.data[i];
        }
    }
    
    static void scale_avx2(const Matrix& A, double scalar, Matrix& C) {
        const int size = A.getM() * A.getN();
        const int simd_size = size - (size % 4);
        __m256d s = _mm256_set1_pd(scalar);
        
        for (int i = 0; i < simd_size; i += 4) {
            __m256d a = _mm256_loadu_pd(&A.data[i]);
            __m256d c = _mm256_mul_pd(a, s);
            _mm256_storeu_pd(&C.data[i], c);
        }
        
        // Handle remaining elements
        for (int i = simd_size; i < size; ++i) {
            C.data[i] = A.data[i] * scalar;
        }
    }
    
    static void transpose4x4_avx2(const Matrix& A, Matrix& C) {
        __m256d row0 = _mm256_loadu_pd(&A.val(0, 0));
        __m256d row1 = _mm256_loadu_pd(&A.val(1, 0));
        __m256d row2 = _mm256_loadu_pd(&A.val(2, 0));
        __m256d row3 = _mm256_loadu_pd(&A.val(3, 0));
        
        __m256d t0 = _mm256_unpacklo_pd(row0, row1);
        __m256d t1 = _mm256_unpackhi_pd(row0, row1);
        __m256d t2 = _mm256_unpacklo_pd(row2, row3);
        __m256d t3 = _mm256_unpackhi_pd(row2, row3);
        
        __m256d res0 = _mm256_permute2f128_pd(t0, t2, 0x20);
        __m256d res1 = _mm256_permute2f128_pd(t1, t3, 0x20);
        __m256d res2 = _mm256_permute2f128_pd(t0, t2, 0x31);
        __m256d res3 = _mm256_permute2f128_pd(t1, t3, 0x31);
        
        _mm256_storeu_pd(&C.val(0, 0), res0);
        _mm256_storeu_pd(&C.val(1, 0), res1);
        _mm256_storeu_pd(&C.val(2, 0), res2);
        _mm256_storeu_pd(&C.val(3, 0), res3);
    }
#endif // USE_AVX2
};

} // namespace matrix

#endif // __MATRIX_SIMD_H