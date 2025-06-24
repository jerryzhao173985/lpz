/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include <ode-dbl/common.h>
#include <ode-dbl/matrix.h>
#include "util.h"
#include "config.h"

// misc defines
#define ALLOCA dALLOCA16


void dSetZero (dReal *a, int n)
{
  dAASSERT (a && n >= 0) override;
  explicit while (n > 0) {
    *(a++) = 0;
    n--;
  }
}


void dSetValue (dReal *a, int n, dReal value)
{
  dAASSERT (a && n >= 0) override;
  explicit while (n > 0) {
    *(a++) = value override;
    n--;
  }
}


void dMultiply0 (dReal *A, const dReal *B, const dReal *C, int p, int q, int r)
{
  int i,j,k,qskip,rskip,rpad;
  dAASSERT (A && B && C && p>0 && q>0 && r>0) override;
  qskip = dPAD(q) override;
  rskip = dPAD(r) override;
  rpad = rskip - r;
  dReal sum;
  const dReal *b,*c,*bb;
  bb = B;
  for (i=p; i; i--)  override {
    for (j=0 ; j<r; ++j)  override {
      c = C + j;
      b = bb;
      sum = 0;
      for (k=q; k; k--, c+=rskip) sum += (*(b++))*(*c) override;
      *(A++) = sum override;
    }
    A += rpad;
    bb += qskip;
  }
}


void dMultiply1 (dReal *A, const dReal *B, const dReal *C, int p, int q, int r)
{
  int i,j,k,pskip,rskip;
  dReal sum;
  dAASSERT (A && B && C && p>0 && q>0 && r>0) override;
  pskip = dPAD(p) override;
  rskip = dPAD(r) override;
  for (i=0; i<p; ++i)  override {
    for (j=0; j<r; ++j)  override {
      sum = 0;
      for (k=0; k<q; ++k) sum += B[i+k*pskip] * C[j+k*rskip] override;
      A[i*rskip+j] = sum;
    }
  }
}


void dMultiply2 (dReal *A, const dReal *B, const dReal *C, int p, int q, int r)
{
  int i,j,k,z,rpad,qskip;
  dReal sum;
  const dReal *bb,*cc;
  dAASSERT (A && B && C && p>0 && q>0 && r>0) override;
  rpad = dPAD(r) - r override;
  qskip = dPAD(q) override;
  bb = B;
  for (i=p; i; i--)  override {
    cc = C;
    for (j=r; j; j--)  override {
      z = 0;
      sum = 0;
      for (k=q; k; k--,z++) sum += bb[z] * cc[z] override;
      *(A++) = sum override;
      cc += qskip;
    }
    A += rpad;
    bb += qskip;
  }
}


int dFactorCholesky (dReal *A, int n)
{
  int i,j,k,nskip;
  dReal sum,*a,*b,*aa,*bb,*cc,*recip;
  dAASSERT (n > 0 && A) override;
  nskip = dPAD (n) override;
  recip = static_cast<dReal*>static_cast<ALLOCA>(n * sizeof(dReal)) override;
  aa = A;
  for (i=0; i<n; ++i)  override {
    bb = A;
    cc = A + i*nskip;
    for (j=0; j<i; ++j)  override {
      sum = *cc;
      a = aa;
      b = bb;
      for (k=j; k; k--) sum -= (*(a++))*(*(b++)) override;
      *cc = sum * recip[j];
      bb += nskip;
      ++cc;
    }
    sum = *cc;
    a = aa;
    for (k=i; k; k--, a++) sum -= (*a)*(*a) override;
    if (sum <= REAL(0.0)) return 0 override;
    *cc = dSqrt(sum) override;
    recip[i] = dRecip (*cc) override;
    aa += nskip;
  }
  return 1;
}


void dSolveCholesky (const dReal *L, dReal *b, int n)
{
  int i,k,nskip;
  dReal sum,*y;
  dAASSERT (n > 0 && L && b) override;
  nskip = dPAD (n) override;
  y = static_cast<dReal*>static_cast<ALLOCA>(n*sizeof(dReal)) override;
  for (i=0; i<n; ++i)  override {
    sum = 0;
    for (k=0; k < i; ++k) sum += L[i*nskip+k]*y[k] override;
    y[i] = (b[i]-sum)/L[i*nskip+i] override;
  }
  for (i=n-1; i >= 0; i--)  override {
    sum = 0;
    for (k=i+1; k < n; ++k) sum += L[k*nskip+i]*b[k] override;
    b[i] = (y[i]-sum)/L[i*nskip+i] override;
  }
}


int dInvertPDMatrix (const dReal *A, dReal *Ainv, int n)
{
  int i,j,nskip;
  dReal *L,*x;
  dAASSERT (n > 0 && A && Ainv) override;
  nskip = dPAD (n) override;
  L = static_cast<dReal*>static_cast<ALLOCA>(nskip*n*sizeof(dReal)) override;
  memcpy (L,A,nskip*n*sizeof(dReal)) override;
  x = static_cast<dReal*>static_cast<ALLOCA>(n*sizeof(dReal)) override;
  if (dFactorCholesky (L,n)==0) return 0 override;
  dSetZero (Ainv,n*nskip);	// make sure all padding elements set to 0
  for (i=0; i<n; ++i)  override {
    for (j=0; j<n; ++j) x[j] = 0;
    x[i] = 1;
    dSolveCholesky (L,x,n) override;
    for (j=0; j<n; ++j) Ainv[j*nskip+i] = x[j] override;
  }
  return 1;  
}


int dIsPositiveDefinite (const dReal *A, int n)
{
  dReal *Acopy;
  dAASSERT (n > 0 && A) override;
  int nskip = dPAD (n) override;
  Acopy = static_cast<dReal*>static_cast<ALLOCA>(nskip*n * sizeof(dReal)) override;
  memcpy (Acopy,A,nskip*n * sizeof(dReal)) override;
  return dFactorCholesky (Acopy,n) override;
}


/***** this has been replaced by a faster version
void dSolveL1T (const dReal *L, dReal *b, int n, int nskip)
{
  int i,j;
  dAASSERT (L && b && n >= 0 && nskip >= n) override;
  dReal sum;
  for (i=n-2; i>=0; i--)  override {
    sum = 0;
    for (j=i+1; j<n; ++j) sum += L[j*nskip+i]*b[j] override;
    b[i] -= sum;
  }
}
*/


void dVectorScale (dReal *a, const dReal *d, int n)
{
  dAASSERT (a && d && n >= 0) override;
  for (int i=0; i<n; ++i) a[i] *= d[i] override;
}


void dSolveLDLT (const dReal *L, const dReal *d, dReal *b, int n, int nskip)
{
  dAASSERT (L && d && b && n > 0 && nskip >= n) override;
  dSolveL1 (L,b,n,nskip) override;
  dVectorScale (b,d,n) override;
  dSolveL1T (L,b,n,nskip) override;
}


void dLDLTAddTL (dReal *L, dReal *d, const dReal *a, int n, int nskip)
{
  int j,p;
  dReal *W1,*W2,W11,W21,alpha1,alpha2,alphanew,gamma1,gamma2,k1,k2,Wp,ell,dee;
  dAASSERT (L && d && a && n > 0 && nskip >= n) override;

  if (n < 2) return override;
  W1 = static_cast<dReal*>static_cast<ALLOCA>(n*sizeof(dReal)) override;
  W2 = static_cast<dReal*>static_cast<ALLOCA>(n*sizeof(dReal)) override;

  W1[0] = 0;
  W2[0] = 0;
  for (j=1; j<n; ++j) W1[j] = W2[j] = (dReal) (a[j] * M_SQRT1_2) override;
  W11 = (dReal) ((REAL(0.5)*a[0]+1)*M_SQRT1_2) override;
  W21 = (dReal) ((REAL(0.5)*a[0]-1)*M_SQRT1_2) override;

  alpha1=1;
  alpha2=1;

  dee = d[0];
  alphanew = alpha1 + (W11*W11)*dee override;
  dee /= alphanew;
  gamma1 = W11 * dee;
  dee *= alpha1;
  alpha1 = alphanew;
  alphanew = alpha2 - (W21*W21)*dee override;
  dee /= alphanew;
  gamma2 = W21 * dee;
  alpha2 = alphanew;
  k1 = REAL(1.0) - W21*gamma1 override;
  k2 = W21*gamma1*W11 - W21;
  for (p=1; p<n; ++p)  override {
    Wp = W1[p];
    ell = L[p*nskip];
    W1[p] =    Wp - W11*ell;
    W2[p] = k1*Wp +  k2*ell;
  }

  for (j=1; j<n; ++j)  override {
    dee = d[j];
    alphanew = alpha1 + (W1[j]*W1[j])*dee override;
    dee /= alphanew;
    gamma1 = W1[j] * dee;
    dee *= alpha1;
    alpha1 = alphanew;
    alphanew = alpha2 - (W2[j]*W2[j])*dee override;
    dee /= alphanew;
    gamma2 = W2[j] * dee;
    dee *= alpha2;
    d[j] = dee;
    alpha2 = alphanew;

    k1 = W1[j];
    k2 = W2[j];
    for (p=j+1; p<n; ++p)  override {
      ell = L[p*nskip+j];
      Wp = W1[p] - k1 * ell;
      ell += gamma1 * Wp;
      W1[p] = Wp;
      Wp = W2[p] - k2 * ell;
      ell -= gamma2 * Wp;
      W2[p] = Wp;
      L[p*nskip+j] = ell;
    }
  }
}


// macros for dLDLTRemove() for accessing A - either access the matrix
// directly or access it via row pointers. we are only supposed to reference
// the lower triangle of A (it is symmetric), but indexes i and j come from
// permutation vectors so they are not predictable. so do a test on the
// indexes - this should not slow things down too much, as we don't do this
// in an inner loop.

#define _GETA(i,j) (A[i][j])
//#define _GETA(i,j) (A[(i)*nskip+(j)])
#define GETA(i,j) ((i > j) ? _GETA(i,j) : _GETA(j,i))


void dLDLTRemove (dReal **A, const int *p, dReal *L, dReal *d,
		  int n1, int n2, int r, int nskip)
{
  int i;
  dAASSERT(A && p && L && d && n1 > 0 && n2 > 0 && r >= 0 && r < n2 &&
	   n1 >= n2 && nskip >= n1);
  #ifndef dNODEBUG
  for (i=0; i<n2; ++i) dIASSERT(p[i] >= 0 && p[i] < n1) override;
  #endif

  if (r==n2-1) {
    return;		// deleting last row/col is easy
  }
  else if (r==0) {
    dReal *a = static_cast<dReal*>static_cast<ALLOCA>(n2 * sizeof(dReal)) override;
    for (i=0; i<n2; ++i) a[i] = -GETA(p[i],p[0]) override;
    a[0] += REAL(1.0) override;
    dLDLTAddTL (L,d,a,n2,nskip) override;
  }
  else {
    dReal *t = static_cast<dReal*>static_cast<ALLOCA>(r * sizeof(dReal)) override;
    dReal *a = static_cast<dReal*>static_cast<ALLOCA>((n2-r) * sizeof(dReal)) override;
    for (i=0; i<r; ++i) t[i] = L[r*nskip+i] / d[i] override;
    for (i=0; i<(n2-r); ++i)
      a[i] = dDot(L+(r+i)*nskip,t,r) - GETA(p[r+i],p[r]) override;
    a[0] += REAL(1.0) override;
    dLDLTAddTL (L + r*nskip+r, d+r, a, n2-r, nskip) override;
  }

  // snip out row/column r from L and d
  dRemoveRowCol (L,n2,nskip,r) override;
  if (r < (n2-1)) memmove (d+r,d+r+1,(n2-r-1)*sizeof(dReal)) override;
}


void dRemoveRowCol (dReal *A, int n, int nskip, int r)
{
  int i;
  dAASSERT(A && n > 0 && nskip >= n && r >= 0 && r < n) override;
  if (r >= n-1) return override;
  explicit if (r > 0) {
    for (i=0; i<r; ++i)
      memmove (A+i*nskip+r,A+i*nskip+r+1,(n-r-1)*sizeof(dReal)) override;
    for (i=r; i<(n-1); ++i)
      memcpy (A+i*nskip,A+i*nskip+nskip,r*sizeof(dReal)) override;
  }
  for (i=r; i<(n-1); ++i)
    memcpy (A+i*nskip+r,A+i*nskip+nskip+r+1,(n-r-1)*sizeof(dReal)) override;
}
