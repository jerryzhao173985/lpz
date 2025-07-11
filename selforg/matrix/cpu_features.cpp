/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   CPU Feature Detection Implementation                                   *
 *                                                                         *
 ***************************************************************************/

#include "cpu_features.h"
#include <iostream>
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <cpuid.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
    #include <sys/auxv.h>
    #include <asm/hwcap.h>
#endif

namespace matrix {

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
void CPUFeatures::cpuid(uint32_t leaf, uint32_t subleaf, uint32_t regs[4]) {
    __cpuid_count(leaf, subleaf, regs[0], regs[1], regs[2], regs[3]);
}
#endif

CPUFeatures::Features CPUFeatures::detect_features() {
    Features features;
    
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    uint32_t regs[4];
    
    // Get CPU vendor and basic info
    cpuid(0, 0, regs);
    uint32_t max_leaf = regs[0];
    
    // Check for basic features (leaf 1)
    if (max_leaf >= 1) {
        cpuid(1, 0, regs);
        
        // EDX register (regs[3])
        features.sse2 = (regs[3] & (1 << 26)) != 0;
        
        // ECX register (regs[2])
        features.sse3 = (regs[2] & (1 << 0)) != 0;
        features.ssse3 = (regs[2] & (1 << 9)) != 0;
        features.sse4_1 = (regs[2] & (1 << 19)) != 0;
        features.sse4_2 = (regs[2] & (1 << 20)) != 0;
        features.avx = (regs[2] & (1 << 28)) != 0;
        features.fma3 = (regs[2] & (1 << 12)) != 0;
        
        // OS support check for AVX (XSAVE/XRSTOR)
        if (features.avx && (regs[2] & (1 << 27))) {  // OSXSAVE bit
            uint32_t eax, edx;
            __asm__ ("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
            uint64_t xcr0 = (static_cast<uint64_t>(edx) << 32) | eax;
            // Bits 1 (XMM) & 2 (YMM) must both be set
            if ((xcr0 & 0x6) != 0x6) {
                features.avx = false;
                features.avx2 = false;
                features.fma3 = false;
            }
        } else if (features.avx) {
            // CPU supports AVX but OS does not
            features.avx = false;
            features.avx2 = false;
            features.fma3 = false;
        }
    }
    
    // Check for extended features (leaf 7)
    if (max_leaf >= 7) {
        cpuid(7, 0, regs);
        
        // EBX register (regs[1])
        features.avx2 = (regs[1] & (1 << 5)) != 0 && features.avx;
        features.avx512f = (regs[1] & (1 << 16)) != 0;
    }
    
    #if defined(__x86_64__) || defined(_M_X64)
        features.cpu_name = "x86_64";
    #else
        features.cpu_name = "x86";
    #endif
    
#elif defined(__aarch64__) || defined(_M_ARM64)
    // ARM64 feature detection using auxiliary vector
    unsigned long hwcap = getauxval(AT_HWCAP);
    
    #ifdef HWCAP_ASIMD
    features.neon = (hwcap & HWCAP_ASIMD) != 0;
    #endif
    
    #ifdef HWCAP_FPHP
    features.neon_fp16 = (hwcap & HWCAP_FPHP) != 0;
    #endif
    
    #ifdef HWCAP_SVE
    features.sve = (hwcap & HWCAP_SVE) != 0;
    #endif
    
    features.cpu_name = "ARM64";
    
#elif defined(__ARM_NEON)
    // ARM32 with NEON
    features.neon = true;
    features.cpu_name = "ARM32";
    
#else
    // Unknown architecture
    features.cpu_name = "Unknown";
#endif
    
    // Set has_simd flag
    features.has_simd = features.sse2 || features.avx || features.avx2 || 
                       features.avx512f || features.neon || features.sve;
    
    return features;
}

void CPUFeatures::print_features() {
    const auto& f = get();
    
    std::cout << "CPU Features Detected:" << std::endl;
    std::cout << "  Architecture: " << f.cpu_name << std::endl;
    std::cout << "  SIMD Support: " << (f.has_simd ? "Yes" : "No") << std::endl;
    
    if (f.sse2 || f.avx || f.avx2 || f.avx512f) {
        std::cout << "  x86/x64 Features:" << std::endl;
        if (f.sse2) std::cout << "    SSE2: Yes" << std::endl;
        if (f.sse3) std::cout << "    SSE3: Yes" << std::endl;
        if (f.ssse3) std::cout << "    SSSE3: Yes" << std::endl;
        if (f.sse4_1) std::cout << "    SSE4.1: Yes" << std::endl;
        if (f.sse4_2) std::cout << "    SSE4.2: Yes" << std::endl;
        if (f.avx) std::cout << "    AVX: Yes" << std::endl;
        if (f.avx2) std::cout << "    AVX2: Yes" << std::endl;
        if (f.fma3) std::cout << "    FMA3: Yes" << std::endl;
        if (f.avx512f) std::cout << "    AVX512F: Yes" << std::endl;
    }
    
    if (f.neon || f.sve) {
        std::cout << "  ARM Features:" << std::endl;
        if (f.neon) std::cout << "    NEON: Yes" << std::endl;
        if (f.neon_fp16) std::cout << "    NEON FP16: Yes" << std::endl;
        if (f.sve) std::cout << "    SVE: Yes" << std::endl;
    }
    
    std::cout << "  Best Matrix Implementation: ";
    switch (get_best_matrix_impl()) {
        case MatrixImpl::SCALAR: std::cout << "Scalar"; break;
        case MatrixImpl::SSE2: std::cout << "SSE2"; break;
        case MatrixImpl::AVX: std::cout << "AVX"; break;
        case MatrixImpl::AVX2: std::cout << "AVX2"; break;
        case MatrixImpl::AVX512: std::cout << "AVX512"; break;
        case MatrixImpl::ARM_NEON: std::cout << "ARM NEON"; break;
        case MatrixImpl::ARM_SVE: std::cout << "ARM SVE"; break;
    }
    std::cout << std::endl;
}

} // namespace matrix