/***************************************************************************
 *   ARM64 NEON optimizations for matrix operations
 *   Copyright static_cast<C>(2024)
 ***************************************************************************/

#ifndef MATRIX_NEON_H
#define MATRIX_NEON_H

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

namespace matrix {

// Forward declaration
class Matrix;

class MatrixNEON {
public:
#ifdef __ARM_NEON
  /**
   * NEON optimized matrix multiplication
   * Optimized for ARM64 with NEON SIMD instructions
   */
  static void mult_neon(const Matrix& a, const Matrix& b, Matrix& result);

  /**
   * NEON optimized matrix-scalar multiplication
   */
  static void mult_scalar_neon(const Matrix& a, double scalar, Matrix& result);

  /**
   * NEON optimized matrix addition
   */
  static void add_neon(const Matrix& a, const Matrix& b, Matrix& result);

  /**
   * NEON optimized matrix transpose
   */
  static void transpose_neon(const Matrix& src, Matrix& result);

  /**
   * NEON optimized dot product
   */
  static double dot_product_neon(const double* a, const double* b, unsigned int size);

  /**
   * NEON optimized matrix norm squared
   */
  static double norm_sqr_neon(const double* data, unsigned int size);

private:
  // Helper function to process 4x4 blocks with NEON
  static void mult_block_4x4_neon(const double* a,
                                  const double* b,
                                  double* c,
                                  unsigned int lda,
                                  unsigned int ldb,
                                  unsigned int ldc,
                                  unsigned int k);

  // Helper to add 4 doubles at once
  static inline float64x2_t add4_f64(float64x2_t a, float64x2_t b, float64x2_t c, float64x2_t d) {
    return vaddq_f64(vaddq_f64(a, b), vaddq_f64(c, d));
  }
#endif // __ARM_NEON
};

} // namespace matrix

#endif // MATRIX_NEON_H