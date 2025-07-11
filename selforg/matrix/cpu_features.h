/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   CPU Feature Detection for SIMD Optimizations                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CPU_FEATURES_H
#define __CPU_FEATURES_H

#include <cstdint>

namespace matrix {

/**
 * CPU Feature Detection for SIMD Optimizations
 * 
 * This class provides runtime detection of CPU features to enable
 * optimal SIMD instruction selection at runtime.
 */
class CPUFeatures {
public:
    struct Features {
        // x86/x64 features
        bool sse2 = false;
        bool sse3 = false;
        bool ssse3 = false;
        bool sse4_1 = false;
        bool sse4_2 = false;
        bool avx = false;
        bool avx2 = false;
        bool fma3 = false;
        bool avx512f = false;
        
        // ARM features
        bool neon = false;
        bool neon_fp16 = false;
        bool sve = false;
        
        // General features
        bool has_simd = false;
        const char* cpu_name = "Unknown";
    };
    
    /**
     * Get CPU features (singleton pattern)
     */
    static const Features& get() {
        static Features features = detect_features();
        return features;
    }
    
    /**
     * Get the best available matrix multiplication implementation
     */
    enum class MatrixImpl {
        SCALAR,
        SSE2,
        AVX,
        AVX2,
        AVX512,
        ARM_NEON,
        ARM_SVE
    };
    
    static MatrixImpl get_best_matrix_impl() {
        const auto& f = get();
        
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
        if (f.avx512f) return MatrixImpl::AVX512;
        if (f.avx2) return MatrixImpl::AVX2;
        if (f.avx) return MatrixImpl::AVX;
        if (f.sse2) return MatrixImpl::SSE2;
#elif defined(__aarch64__) || defined(_M_ARM64)
        if (f.sve) return MatrixImpl::ARM_SVE;
        if (f.neon) return MatrixImpl::ARM_NEON;
#endif
        return MatrixImpl::SCALAR;
    }
    
    /**
     * Print CPU feature information
     */
    static void print_features();
    
private:
    /**
     * Detect CPU features at runtime
     */
    static Features detect_features();
    
    /**
     * x86/x64 CPUID instruction wrapper
     */
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    static void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t regs[4]);
#endif
};

} // namespace matrix

#endif // __CPU_FEATURES_H