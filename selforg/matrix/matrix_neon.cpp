/***************************************************************************
 *   ARM64 NEON optimizations for matrix operations
 *   Copyright (C) 2024
 ***************************************************************************/

#include "matrix_neon.h"
#include "matrix.h"

#ifdef __ARM_NEON

namespace matrix {

void
MatrixNEON::mult_neon(const Matrix& a, const Matrix& b, Matrix& result) {
  assert(a.getN() == b.getM());
  assert(result.getM() == a.getM() && result.getN() == b.getN());

  const unsigned int m = a.getM();
  const unsigned int n = b.getN();
  const unsigned int k = a.getN();

  const double* aData = a.data;
  const double* bData = b.data;
  double* cData = result.data;

  // Clear result matrix
  result.toZero();

  // Process 4x4 blocks
  const unsigned int m4 = (m / 4) * 4;
  const unsigned int n4 = (n / 4) * 4;

  for (unsigned int i = 0; i < m4; i += 4) {
    for (unsigned int j = 0; j < n4; j += 4) {
      mult_block_4x4_neon(aData + i * k, bData + j, cData + i * n + j, k, n, n, k);
    }
  }

  // Handle remaining elements with scalar code
  for (unsigned int i = 0; i < m; i++) {
    for (unsigned int j = (i < m4) ? n4 : 0; j < n; j++) {
      double sum = 0.0;
      for (unsigned int l = 0; l < k; l++) {
        sum += aData[i * k + l] * bData[l * n + j];
      }
      cData[i * n + j] = sum;
    }
  }

  // Handle remaining rows
  for (unsigned int i = m4; i < m; i++) {
    for (unsigned int j = 0; j < n; j++) {
      double sum = 0.0;
      for (unsigned int l = 0; l < k; l++) {
        sum += aData[i * k + l] * bData[l * n + j];
      }
      cData[i * n + j] = sum;
    }
  }
}

void
MatrixNEON::mult_scalar_neon(const Matrix& a, double scalar, Matrix& result) {
  assert(result.getM() == a.getM() && result.getN() == a.getN());

  const unsigned int size = a.size();
  const double* aData = a.data;
  double* rData = result.data;

  float64x2_t vscalar = vdupq_n_f64(scalar);

  // Process 4 elements at a time
  unsigned int i = 0;
  for (; i + 4 <= size; i += 4) {
    float64x2_t va1 = vld1q_f64(aData + i);
    float64x2_t va2 = vld1q_f64(aData + i + 2);

    float64x2_t vr1 = vmulq_f64(va1, vscalar);
    float64x2_t vr2 = vmulq_f64(va2, vscalar);

    vst1q_f64(rData + i, vr1);
    vst1q_f64(rData + i + 2, vr2);
  }

  // Handle remaining elements
  for (; i < size; i++) {
    rData[i] = aData[i] * scalar;
  }
}

void
MatrixNEON::add_neon(const Matrix& a, const Matrix& b, Matrix& result) {
  assert(a.getM() == b.getM() && a.getN() == b.getN());
  assert(result.getM() == a.getM() && result.getN() == a.getN());

  const unsigned int size = a.size();
  const double* aData = a.data;
  const double* bData = b.data;
  double* rData = result.data;

  // Process 4 elements at a time
  unsigned int i = 0;
  for (; i + 4 <= size; i += 4) {
    float64x2_t va1 = vld1q_f64(aData + i);
    float64x2_t va2 = vld1q_f64(aData + i + 2);
    float64x2_t vb1 = vld1q_f64(bData + i);
    float64x2_t vb2 = vld1q_f64(bData + i + 2);

    float64x2_t vr1 = vaddq_f64(va1, vb1);
    float64x2_t vr2 = vaddq_f64(va2, vb2);

    vst1q_f64(rData + i, vr1);
    vst1q_f64(rData + i + 2, vr2);
  }

  // Handle remaining elements
  for (; i < size; i++) {
    rData[i] = aData[i] + bData[i];
  }
}

void
MatrixNEON::transpose_neon(const Matrix& src, Matrix& result) {
  assert(result.getM() == src.getN() && result.getN() == src.getM());

  const unsigned int m = src.getM();
  const unsigned int n = src.getN();
  const double* sData = src.data;
  double* rData = result.data;

  // Process 2x2 blocks for double precision
  const unsigned int m2 = (m / 2) * 2;
  const unsigned int n2 = (n / 2) * 2;

  for (unsigned int i = 0; i < m2; i += 2) {
    for (unsigned int j = 0; j < n2; j += 2) {
      // Load 2x2 block
      float64x2_t row0 = vld1q_f64(sData + i * n + j);
      float64x2_t row1 = vld1q_f64(sData + (i + 1) * n + j);

      // Transpose 2x2
      float64x2_t col0 = vzip1q_f64(row0, row1);
      float64x2_t col1 = vzip2q_f64(row0, row1);

      // Store transposed data
      vst1q_f64(rData + j * m + i, col0);
      vst1q_f64(rData + (j + 1) * m + i, col1);
    }
  }

  // Handle remaining elements with scalar code
  for (unsigned int i = 0; i < m; i++) {
    for (unsigned int j = (i < m2) ? n2 : 0; j < n; j++) {
      rData[j * m + i] = sData[i * n + j];
    }
  }

  // Handle remaining rows
  for (unsigned int i = m2; i < m; i++) {
    for (unsigned int j = 0; j < n; j++) {
      rData[j * m + i] = sData[i * n + j];
    }
  }
}

double
MatrixNEON::dot_product_neon(const double* a, const double* b, unsigned int size) {
  float64x2_t sum = vdupq_n_f64(0.0);

  // Process 4 elements at a time
  unsigned int i = 0;
  for (; i + 4 <= size; i += 4) {
    float64x2_t va1 = vld1q_f64(a + i);
    float64x2_t va2 = vld1q_f64(a + i + 2);
    float64x2_t vb1 = vld1q_f64(b + i);
    float64x2_t vb2 = vld1q_f64(b + i + 2);

    sum = vfmaq_f64(sum, va1, vb1);
    sum = vfmaq_f64(sum, va2, vb2);
  }

  // Sum the two elements
  double result = vaddvq_f64(sum);

  // Handle remaining elements
  for (; i < size; i++) {
    result += a[i] * b[i];
  }

  return result;
}

double
MatrixNEON::norm_sqr_neon(const double* data, unsigned int size) {
  return dot_product_neon(data, data, size);
}

void
MatrixNEON::mult_block_4x4_neon(const double* a,
                                const double* b,
                                double* c,
                                unsigned int lda,
                                unsigned int ldb,
                                unsigned int ldc,
                                unsigned int k) {

  // Initialize result registers to zero
  float64x2_t c00 = vdupq_n_f64(0.0);
  float64x2_t c01 = vdupq_n_f64(0.0);
  float64x2_t c10 = vdupq_n_f64(0.0);
  float64x2_t c11 = vdupq_n_f64(0.0);
  float64x2_t c20 = vdupq_n_f64(0.0);
  float64x2_t c21 = vdupq_n_f64(0.0);
  float64x2_t c30 = vdupq_n_f64(0.0);
  float64x2_t c31 = vdupq_n_f64(0.0);

  // Compute 4x4 block multiplication
  for (unsigned int l = 0; l < k; l++) {
    // Load A column
    float64x2_t a0 = vld1q_f64(a + 0 * lda + l);
    float64x2_t a1 = vld1q_f64(a + 2 * lda + l);

    // Load B row
    float64x2_t b0 = vld1q_f64(b + l * ldb + 0);
    float64x2_t b1 = vld1q_f64(b + l * ldb + 2);

    // Multiply and accumulate
    c00 = vfmaq_laneq_f64(c00, b0, a0, 0);
    c01 = vfmaq_laneq_f64(c01, b1, a0, 0);
    c10 = vfmaq_laneq_f64(c10, b0, a0, 1);
    c11 = vfmaq_laneq_f64(c11, b1, a0, 1);
    c20 = vfmaq_laneq_f64(c20, b0, a1, 0);
    c21 = vfmaq_laneq_f64(c21, b1, a1, 0);
    c30 = vfmaq_laneq_f64(c30, b0, a1, 1);
    c31 = vfmaq_laneq_f64(c31, b1, a1, 1);
  }

  // Store results
  vst1q_f64(c + 0 * ldc + 0, c00);
  vst1q_f64(c + 0 * ldc + 2, c01);
  vst1q_f64(c + 1 * ldc + 0, c10);
  vst1q_f64(c + 1 * ldc + 2, c11);
  vst1q_f64(c + 2 * ldc + 0, c20);
  vst1q_f64(c + 2 * ldc + 2, c21);
  vst1q_f64(c + 3 * ldc + 0, c30);
  vst1q_f64(c + 3 * ldc + 2, c31);
}

} // namespace matrix

#endif // __ARM_NEON