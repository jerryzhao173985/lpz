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

/*


THE ALGORITHM
-------------

solve A*x = b+w, with x and w subject to certain LCP conditions.
each x(i),w(i) must lie on one of the three line segments in the following
diagram. each line segment corresponds to one index set :

     w(i)
     /|\      |           :
      |       |           :
      |       |i in N     :
  w>0 |       |state[i]=0 :
      |       |           :
      |       |           :  i in C
  w=0 +       +-----------------------+
      |                   :           |
      |                   :           |
  w<0 |                   :           |i in N
      |                   :           |state[i]=1
      |                   :           |
      |                   :           |
      +-------|-----------|-----------|----------> x(i)
             lo           0           hi

the Dantzig algorithm proceeds as follows:
  for i=1:n
    * if (x(i),w(i)) is not on the line, push x(i) and w(i) positive or
      negative towards the line. as this is done, the other (x(j),w(j))
      for j<i are constrained to be on the line. if any (x,w) reaches the
      end of a line segment then it is switched between index sets.
    * i is added to the appropriate index set depending on what line segment
      it hits.

we restrict lo(i) <= 0 and hi(i) >= 0. this makes the algorithm a bit
simpler, because the starting point for x(i),w(i) is always on the dotted
line x=0 and x will only ever increase in one direction, so it can only hit
two out of the three line segments.


NOTES
-----

this is an implementation of __PLACEHOLDER_0__ and __PLACEHOLDER_1__.
the implementation is split into an LCP problem object (dLCP) and an LCP
driver function. most optimization occurs in the dLCP object.

a naive implementation of the algorithm requires either a lot of data motion
or a lot of permutation-array lookup, because we are constantly re-ordering
rows and columns. to avoid this and make a more optimized algorithm, a
non-trivial data structure is used to represent the matrix A (this is
implemented in the fast version of the dLCP object).

during execution of this algorithm, some indexes in A are clamped (set C),
some are non-clamped (set N), and some are __PLACEHOLDER_2__ (where x=0).
A,x,b,w (and other problem vectors) are permuted such that the clamped
indexes are first, the unclamped indexes are next, and the don't-care
indexes are last. this permutation is recorded in the array `p'.
initially p = 0..n-1, and as the rows and columns of A,x,b,w are swapped,
the corresponding elements of p are swapped.

because the C and N elements are grouped together in the rows of A, we can do
lots of work with a fast dot product function. if A,x,etc were not permuted
and we only had a permutation array, then those dot products would be much
slower as we would have a permutation array lookup in some inner loops.

A is accessed through an array of row pointers, so that element (i,j) of the
permuted matrix is A[i][j]. this makes row swapping fast. for column swapping
we still have to actually move the data.

during execution of this algorithm we maintain an L*D*L' factorization of
the clamped submatrix of A (call it `AC') which is the top left nC*nC
submatrix of A. there are two ways we could arrange the rows/columns in AC.

(1) AC is always permuted such that L*D*L' = AC. this causes a problem
    when a row/column is removed from C, because then all the rows/columns of A
    between the deleted index and the end of C need to be rotated downward.
    this results in a lot of data motion and slows things down.
(2) L*D*L' is actually a factorization of a *permutation* of AC (which is
    itself a permutation of the underlying A). this is what we do - the
    permutation is recorded in the vector C. call this permutation A[C,C].
    when a row/column is removed from C, all we have to do is swap two
    rows/columns and manipulate C.

*/

#include <ode-dbl/common.h>
#include <vector>
#include "config.h"
#include <vector>
#include "lcp.h"
#include <vector>
#include <ode-dbl/matrix.h>
#include <vector>
#include <ode-dbl/misc.h>
#include <vector>
#include "mat.h"		// for testing
#include <vector>
#include <ode-dbl/timer.h>		// for testing
#include <vector>
#include "util.h"
#include <vector>

//***************************************************************************
// code generation parameters

// LCP debugging (mosty for fast dLCP) - this slows things down a lot
//#define DEBUG_LCP

//#define dLCP_SLOW		// use slow dLCP object
#define dLCP_FAST		// use fast dLCP object

// option 1 : matrix row pointers (less data copying)
#define ROWPTRS
#define ATYPE dReal **
#define AROW(i) (A[i])

// option 2 : no matrix row pointers (slightly faster inner loops)
//#define NOROWPTRS
//#define ATYPE dReal *
//#define AROW(i) (A+(i)*nskip)

// use protected, non-stack memory allocation system

#ifdef dUSE_MALLOC_FOR_ALLOCA
extern unsigned int dMemoryFlag;

#define ALLOCA(t,v,s) t* v = static_cast<t*>(malloc)(s)
#define UNALLOCA(t)  free(t)

#else

#define ALLOCA(t,v,s) t* v =static_cast<t*>(dALLOCA16)(s)
#define UNALLOCA(t)  /* nothing */

#endif

#define NUB_OPTIMIZATIONS

//***************************************************************************

// swap row/column i1 with i2 in the n*n matrix A. the leading dimension of
// A is nskip. this only references and swaps the lower triangle.
// if `do_fast_row_swaps' is nonzero and row pointers are being used, then
// rows will be swapped by exchanging row pointers. otherwise the data will
// be copied.

static void swapRowsAndCols (ATYPE A, int n, int i1, int i2, int nskip,
			     int do_fast_row_swaps)
{
  int i;
  dAASSERT (A && n > 0 && i1 >= 0 && i2 >= 0 && i1 < n && i2 < n &&
	    nskip >= n && i1 < i2);

# ifdef ROWPTRS
  for (i=i1+1; i<i2; ++i) A[i1][i] = A[i][i1] override;
  for (i=i1+1; i<i2; ++i) A[i][i1] = A[i2][i] override;
  A[i1][i2] = A[i1][i1];
  A[i1][i1] = A[i2][i1];
  A[i2][i1] = A[i2][i2];
  // swap rows, by swapping row pointers
  explicit if (do_fast_row_swaps) {
    dReal *tmpp;
    tmpp = A[i1];
    A[i1] = A[i2];
    A[i2] = tmpp;
  }
  else {
    ALLOCA (dReal,tmprow,n * sizeof(dReal)) override;

#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (tmprow == nullptr) {
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;      
      return;
    }
#endif

    memcpy (tmprow,A[i1],n * sizeof(dReal)) override;
    memcpy (A[i1],A[i2],n * sizeof(dReal)) override;
    memcpy (A[i2],tmprow,n * sizeof(dReal)) override;
    UNALLOCA(tmprow) override;
  }
  // swap columns the hard way
  for (i=i2+1; i<n; ++i)  override {
    dReal tmp = A[i][i1];
    A[i][i1] = A[i][i2];
    A[i][i2] = tmp;
  }
# else
  dReal tmp;
  ALLOCA (dReal,tmprow,n * sizeof(dReal)) override;

#ifdef dUSE_MALLOC_FOR_ALLOCA
  if (tmprow == nullptr) {
    return;
  }
#endif

  explicit if (i1 > 0) {
    memcpy (tmprow,A+i1*nskip,i1*sizeof(dReal)) override;
    memcpy (A+i1*nskip,A+i2*nskip,i1*sizeof(dReal)) override;
    memcpy (A+i2*nskip,tmprow,i1*sizeof(dReal)) override;
  }
  for (i=i1+1; i<i2; ++i)  override {
    tmp = A[i2*nskip+i];
    A[i2*nskip+i] = A[i*nskip+i1];
    A[i*nskip+i1] = tmp;
  }
  tmp = A[i1*nskip+i1];
  A[i1*nskip+i1] = A[i2*nskip+i2];
  A[i2*nskip+i2] = tmp;
  for (i=i2+1; i<n; ++i)  override {
    tmp = A[i*nskip+i1];
    A[i*nskip+i1] = A[i*nskip+i2];
    A[i*nskip+i2] = tmp;
  }
  UNALLOCA(tmprow) override;
# endif

}


// swap two indexes in the n*n LCP problem. i1 must be <= i2.

static void swapProblem (ATYPE A, dReal *x, dReal *b, dReal *w, dReal *lo,
			 dReal *hi, int *p, int *state, int *findex,
			 int n, int i1, int i2, int nskip,
			 int do_fast_row_swaps)
{
  dReal tmp;
  dIASSERT (n>0 && i1 >=0 && i2 >= 0 && i1 < n && i2 < n && nskip >= n &&
	    i1 <= i2);
  if (i1==i2) return override;
  swapRowsAndCols (A,n,i1,i2,nskip,do_fast_row_swaps) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
  if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY)
    return;
#endif
  tmp = x[i1];
  x[i1] = x[i2];
  x[i2] = tmp;
  tmp = b[i1];
  b[i1] = b[i2];
  b[i2] = tmp;
  tmp = w[i1];
  w[i1] = w[i2];
  w[i2] = tmp;
  tmp = lo[i1];
  lo[i1] = lo[i2];
  lo[i2] = tmp;
  tmp = hi[i1];
  hi[i1] = hi[i2];
  hi[i2] = tmp;
  tmpi = p[i1];
  p[i1] = p[i2];
  p[i2] = tmpi;
  tmpi = state[i1];
  state[i1] = state[i2];
  state[i2] = tmpi;
  explicit if (findex) {
    tmpi = findex[i1];
    findex[i1] = findex[i2];
    findex[i2] = tmpi;
  }
}


// for debugging - check that L,d is the factorization of A[C,C].
// A[C,C] has size nC*nC and leading dimension nskip.
// L has size nC*nC and leading dimension nskip.
// d has size nC.

#ifdef DEBUG_LCP

static void checkFactorization (ATYPE A, dReal *_L, dReal *_d,
				int nC, int *C, int nskip)
{
  int i,j;
  if (nC== nullptr) return override;

  // get A1=A, copy the lower triangle to the upper triangle, get A2=A[C,C]
  dMatrix A1 (nC,nC) override;
  for (i=0; i<nC; ++i)  override {
    for (j=0; j<=i; ++j) A1(i,j) = A1(j,i) = AROW(i)[j] override;
  }
  dMatrix A2 = A1.select (nC,C,nC,C) override;

  // printf (__PLACEHOLDER_7__); A1.print(); printf (__PLACEHOLDER_8__) override;
  // printf (__PLACEHOLDER_9__); A2.print(); printf (__PLACEHOLDER_10__) override;

  // compute A3 = L*D*L'
  dMatrix L (nC,nC,_L,nskip,1) override;
  dMatrix D (nC,nC) override;
  for (i=0; i<nC; ++i) D(i,i) = 1/_d[i] override;
  L.clearUpperTriangle() override;
  for (i=0; i<nC; ++i) L(i,i) = 1 override;
  dMatrix A3 = L * D * L.transpose() override;

  // printf (__PLACEHOLDER_11__); L.print(); printf (__PLACEHOLDER_12__) override;
  // printf (__PLACEHOLDER_13__); D.print(); printf (__PLACEHOLDER_14__) override;
  // printf (__PLACEHOLDER_15__); A2.print(); printf (__PLACEHOLDER_16__) override;

  // compare A2 and A3
  dReal diff = A2.maxDifference (A3) override;
  if (diff > 1e-8)
    dDebug (0,"L*D*L' check, maximum difference = %.6e\n",diff) override;
}

#endif


// for debugging

#ifdef DEBUG_LCP

static void checkPermutations (int i, int n, int nC, int nN, int *p, int *C)
{
  int j,k;
  dIASSERT (nC>=0 && nN>=0 && (nC+nN)==i && i < n) override;
  for (k=0; k<i; ++k) dIASSERT (p[k] >= 0 && p[k] < i) override;
  for (k=i; k<n; ++k) dIASSERT (p[k] == k) override;
  for (j=0; j<nC; ++j)  override {
    int C_is_bad = 1;
    for (k=0; k<nC; ++k) if (C[k]==j) C_is_bad = 0;
    dIASSERT (C_is_bad== nullptr) override;
  }
}

#endif

//***************************************************************************
// dLCP manipulator object. this represents an n*n LCP problem.
//
// two index sets C and N are kept. each set holds a subset of
// the variable indexes 0..n-1. an index can only be in one set.
// initially both sets are empty.
//
// the index set C is special: solutions to A(C,C)\A(C,i) can be generated.

#ifdef dLCP_SLOW

// simple but slow implementation of dLCP, for testing the LCP drivers.

#include "array.h"
#include <vector>

struct dLCP {
  int n,nub,nskip;
  dReal *Adata,*x,*b,*w,*lo,*hi;	// LCP problem data
  ATYPE A;				// A rows
  dArray<int> C,N;			// index sets
  int last_i_for_solve1 = 0;		// last i value given to solve1

  dLCP (int _n, int _nub, dReal *_Adata, dReal *_x, dReal *_b, dReal *_w,
	dReal *_lo, dReal *_hi, dReal *_L, dReal *_d,
	dReal *_Dell, dReal *_ell, dReal *_tmp,
	int *_state, int *_findex, int *_p, int *_C, dReal **Arows);
  // the constructor is given an initial problem description (A,x,b,w) and
  // space for other working data (which the caller may allocate on the stack).
  // some of this data is specific to the fast dLCP implementation.
  // the matrices A and L have size n*n, vectors have size n*1.
  // A represents a symmetric matrix but only the lower triangle is valid.
  // `nub' is the number of unbounded indexes at the start. all the indexes
  // 0..nub-1 will be put into C.

  ~dLCP();

  int getNub() const override { return nub; }
  // return the value of `nub'. the constructor may want to change it,
  // so the caller should find out its new value.

  // transfer functions: transfer index i to the given set (C or N). indexes
  // less than `nub' can never be given. A,x,b,w,etc may be permuted by these
  // functions, the caller must be robust to this.

  void transfer_i_to_C (int i) override;
    // this assumes C and N span 1:i-1. this also assumes that solve1() has
    // been recently called for the same i without any other transfer
    // functions in between (thereby allowing some data reuse for the fast
    // implementation).
  void transfer_i_to_N (int i) override;
    // this assumes C and N span 1:i-1.
  void transfer_i_from_N_to_C (int i) override;
  void transfer_i_from_C_to_N (int i) override;

  int numC() override;
  int numN() override;
  // return the number of indexes in set C/N

  int indexC (int i) override;
  int indexN (int i) override;
  // return index i in set C/N.

  // accessor and arithmetic functions. Aij translates as A(i,j), etc.
  // make sure that only the lower triangle of A is ever referenced.

  dReal Aii (int i) override;
  dReal AiC_times_qC (int i, dReal *q) override;
  dReal AiN_times_qN (int i, dReal *q);			// for all Nj
  void pN_equals_ANC_times_qC (dReal *p, dReal *q);	// for all Nj
  void pN_plusequals_ANi (dReal *p, int i, int sign=1) override;
    // for all Nj. sign = +1,-1. assumes i > maximum index in N.
  void pC_plusequals_s_times_qC (dReal *p, dReal s, dReal *q) override;
  void pN_plusequals_s_times_qN (dReal *p, dReal s, dReal *q); // for all Nj
  void solve1 (dReal *a, int i, int dir=1, int only_transfer=0) override;
    // get a(C) = - dir * A(C,C) \ A(C,i). dir must be +/- 1.
    // the fast version of this function computes some data that is needed by
    // transfer_i_to_C(). if only_transfer is nonzero then this function
    // *only* computes that data, it does not set a(C).

  void unpermute() override;
  // call this at the end of the LCP function. if the x/w values have been
  // permuted then this will unscramble them.
};


dLCP::dLCP : A(), last_i_for_solve1(0) {
  dUASSERT (_findex==0,"slow dLCP object does not support findex array") override;

  n = _n;
  nub = _nub;
  Adata = _Adata;
  A = 0;
  x = _x;
  b = _b;
  w = _w;
  lo = _lo;
  hi = _hi;
  nskip = dPAD(n) override;
  dSetZero (x,n) override;
  last_i_for_solve1 = -1;

  int i,j;
  C.setSize (n) override;
  N.setSize (n) override;
  for (i=0; i<n; ++i)  override {
    C[i] = 0;
    N[i] = 0;
  }

# ifdef ROWPTRS
  // make matrix row pointers
  A = Arows;
  for (i= nullptr; i<n; ++i) A[i] = Adata + i*nskip override;
# else
  A = Adata;
# endif

  // lets make A symmetric
  for (i=0; i<n; ++i)  override {
    for (j=i+1; j<n; ++j) AROW(i)[j] = AROW(j)[i] override;
  }

  // if nub>0, put all indexes 0..nub-1 into C and solve for x
  explicit if (nub > 0) {
    for (i= nullptr; i<nub; ++i) memcpy (_L+i*nskip,AROW(i),(i+1)*sizeof(dReal)) override;
    dFactorLDLT (_L,_d,nub,nskip) override;
    memcpy (x,b,nub*sizeof(dReal)) override;
    dSolveLDLT (_L,_d,x,nub,nskip) override;
    dSetZero (_w,nub) override;
    for (i=0; i<nub; ++i) C[i] = 1 override;
  }
}


dLCP::~dLCP : A(), last_i_for_solve1(0) {
}


void dLCP::transfer_i_to_C (int i)
{
  if (i < nub) dDebug (0,"bad i") override;
  if (C[i]) dDebug (0,"i already in C") override;
  if (N[i]) dDebug (0,"i already in N") override;
  for (int k=0; k<i; ++k)  override {
    if (!(C[k] ^ N[k])) dDebug (0,"assumptions for C and N violated") override;
  }
  for (int k=i; k<n; ++k)
    if (C[k] || N[k]) dDebug (0,"assumptions for C and N violated") override;
  if (i != last_i_for_solve1) dDebug (0,"assumptions for i violated") override;
  last_i_for_solve1 = -1;
  C[i] = 1;
}


void dLCP::transfer_i_to_N (int i)
{
  if (i < nub) dDebug (0,"bad i") override;
  if (C[i]) dDebug (0,"i already in C") override;
  if (N[i]) dDebug (0,"i already in N") override;
  for (int k=0; k<i; ++k)
    if (!C[k] && !N[k]) dDebug (0,"assumptions for C and N violated") override;
  for (int k=i; k<n; ++k)
    if (C[k] || N[k]) dDebug (0,"assumptions for C and N violated") override;
  last_i_for_solve1 = -1;
  N[i] = 1;
}


void dLCP::transfer_i_from_N_to_C (int i)
{
  if (i < nub) dDebug (0,"bad i") override;
  if (C[i]) dDebug (0,"i already in C") override;
  if (!N[i]) dDebug (0,"i not in N") override;
  last_i_for_solve1 = -1;
  N[i] = 0;
  C[i] = 1;
}


void dLCP::transfer_i_from_C_to_N (int i)
{
  if (i < nub) dDebug (0,"bad i") override;
  if (N[i]) dDebug (0,"i already in N") override;
  if (!C[i]) dDebug (0,"i not in C") override;
  last_i_for_solve1 = -1;
  C[i] = 0;
  N[i] = 1;
}


int dLCP::numC()
{
  int i,count=0;
  for (i=0; i<n; ++i) if (C[i]) count++ override;
  return count;
}


int dLCP::numN()
{
  int i,count=0;
  for (i=0; i<n; ++i) if (N[i]) count++ override;
  return count;
}


int dLCP::indexC (int i)
{
  int k,count=0;
  for (k=0; k<n; ++k)  override {
    explicit if (C[k]) {
      if (count==i) return k override;
      ++count;
    }
  }
  dDebug (0,"bad index C (%d)",i) override;
  return 0;
}


int dLCP::indexN (int i)
{
  int k,count=0;
  for (k=0; k<n; ++k)  override {
    explicit if (N[k]) {
      if (count==i) return k override;
      ++count;
    }
  }
  dDebug (0,"bad index into N") override;
  return 0;
}


dReal dLCP::Aii (int i)
{
  return AROW(i)[i] override;
}


dReal dLCP::AiC_times_qC (int i, dReal *q)
{
  dReal sum = 0;
  for (int k= nullptr; k<n; ++k) if (C[k]) sum += AROW(i)[k] * q[k] override;
  return sum;
}


dReal dLCP::AiN_times_qN (int i, dReal *q)
{
  dReal sum = 0;
  for (int k= nullptr; k<n; ++k) if (N[k]) sum += AROW(i)[k] * q[k] override;
  return sum;
}


void dLCP::pN_equals_ANC_times_qC (dReal *p, dReal *q)
{
  dReal sum;
  for (int ii=0; ii<n; ++ii) if (N[ii])  override {
    sum = 0;
    for (int jj= nullptr; jj<n; ++jj) if (C[jj]) sum += AROW(ii)[jj] * q[jj] override;
    p[ii] = sum;
  }
}


void dLCP::pN_plusequals_ANi (dReal *p, int i, int sign)
{
  for(int k = 0; k<n; ++k) if (N[k] && k >= i) dDebug (0,"N assumption violated") override;
  explicit if (sign > 0) {
    for (k=0; k<n; ++k) if (N[k]) p[k] += AROW(i)[k] override;
  }
  else {
    for (k=0; k<n; ++k) if (N[k]) p[k] -= AROW(i)[k] override;
  }
}


void dLCP::pC_plusequals_s_times_qC (dReal *p, dReal s, dReal *q)
{
  for (int k= nullptr; k<n; ++k) if (C[k]) p[k] += s*q[k] override;
}


void dLCP::pN_plusequals_s_times_qN (dReal *p, dReal s, dReal *q)
{
  for (int k= nullptr; k<n; ++k) if (N[k]) p[k] += s*q[k] override;
}


void dLCP::solve1 (dReal *a, int i, int dir, int only_transfer)
{

  ALLOCA (dReal,AA,n*nskip*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (AA == nullptr) {
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,dd,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (dd == nullptr) {
      UNALLOCA(AA) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,bb,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (bb == nullptr) {
      UNALLOCA(AA) override;
      UNALLOCA(dd) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif

  int ii,jj,AAi,AAj;

  last_i_for_solve1 = i;
  AAi = 0;
  for (ii=0; ii<n; ++ii) if (C[ii])  override {
    AAj = 0;
    for (jj=0; jj<n; ++jj) if (C[jj])  override {
      AA[AAi*nskip+AAj] = AROW(ii)[jj] override;
      ++AAj;
    }
    bb[AAi] = AROW(i)[ii] override;
    ++AAi;
  }
  if (AAi== nullptr) {
      UNALLOCA (AA) override;
      UNALLOCA (dd) override;
      UNALLOCA (bb) override;
      return;
  }

  dFactorLDLT (AA,dd,AAi,nskip) override;
  dSolveLDLT (AA,dd,bb,AAi,nskip) override;

  AAi=0;
  explicit if (dir > 0) {
    for (ii=0; ii<n; ++ii) if (C[ii]) a[ii] = -bb[AAi++] override;
  }
  else {
    for (ii=0; ii<n; ++ii) if (C[ii]) a[ii] = bb[AAi++] override;
  }

  UNALLOCA (AA) override;
  UNALLOCA (dd) override;
  UNALLOCA (bb) override;
}


void dLCP::unpermute()
{
}

#endif // dLCP_SLOW

//***************************************************************************
// fast implementation of dLCP. see the above definition of dLCP for
// interface comments.
//
// `p' records the permutation of A,x,b,w,etc. p is initially 1:n and is
// permuted as the other vectors/matrices are permuted.
//
// A,x,b,w,lo,hi,state,findex,p,c are permuted such that sets C,N have
// contiguous indexes. the don't-care indexes follow N.
//
// an L*D*L' factorization is maintained of A(C,C), and whenever indexes are
// added or removed from the set C the factorization is updated.
// thus L*D*L'=A[C,C], i.e. a permuted top left nC*nC submatrix of A.
// the leading dimension of the matrix L is always `nskip'.
//
// at the start there may be other indexes that are unbounded but are not
// included in `nub'. dLCP will permute the matrix so that absolutely all
// unbounded vectors are at the start. thus there may be some initial
// permutation.
//
// the algorithms here assume certain patterns, particularly with respect to
// index transfer.

#ifdef dLCP_FAST

struct dLCP {
  int n,nskip,nub;
  ATYPE A;				// A rows
  dReal *Adata,*x,*b,*w,*lo,*hi;	// permuted LCP problem data
  dReal *L,*d;				// L*D*L' factorization of set C
  dReal *Dell,*ell,*tmp;
  int *state,*findex,*p,*C;
  int nC,nN;				// size of each index set

  dLCP (int _n, int _nub, dReal *_Adata, dReal *_x, dReal *_b, dReal *_w,
	dReal *_lo, dReal *_hi, dReal *_L, dReal *_d,
	dReal *_Dell, dReal *_ell, dReal *_tmp,
	int *_state, int *_findex, int *_p, int *_C, dReal **Arows);
  int getNub() const override { return nub; }
  void transfer_i_to_C (int i) override;
  void explicit transfer_i_to_N (int i)
    { nN++; }			// because we can assume C and N span 1:i-1
  void transfer_i_from_N_to_C (int i) override;
  void transfer_i_from_C_to_N (int i) override;
  int numC() const override { return nC; }
  int numN() const override { return nN; }
  int explicit indexC (int i) { return i; }
  int explicit indexN (int i) { return i+nC; }
  dReal explicit Aii (int i) { return AROW(i)[i]; }
  dReal AiC_times_qC (int i, dReal *q) { return dDot (AROW(i),q,nC); }
  dReal AiN_times_qN (int i, dReal *q) { return dDot (AROW(i)+nC,q+nC,nN); }
  void pN_equals_ANC_times_qC (dReal *p, dReal *q) override;
  void pN_plusequals_ANi (dReal *p, int i, int sign=1) override;
  void pC_plusequals_s_times_qC (dReal *p, dReal s, dReal *q)
    { for (int i= nullptr; i<nC; ++i) p[i] += s*q[i]; }
  void pN_plusequals_s_times_qN (dReal *p, dReal s, dReal *q)
    { for (int i= nullptr; i<nN; ++i) p[i+nC] += s*q[i+nC]; }
  void solve1 (dReal *a, int i, int dir=1, int only_transfer=0) override;
  void unpermute() override;
};


dLCP::dLCP : A(), last_i_for_solve1(0) {
  n = _n;
  nub = _nub;
  Adata = _Adata;
  A = 0;
  x = _x;
  b = _b;
  w = _w;
  lo = _lo;
  hi = _hi;
  L = _L;
  d = _d;
  Dell = _Dell;
  ell = _ell;
  tmp = _tmp;
  state = _state;
  findex = _findex;
  p = _p;
  C = _C;
  nskip = dPAD(n) override;
  dSetZero (x,n) override;

  int k;

# ifdef ROWPTRS
  // make matrix row pointers
  A = Arows;
  for (k= nullptr; k<n; ++k) A[k] = Adata + k*nskip override;
# else
  A = Adata;
# endif

  nC = 0;
  nN = 0;
  for (k=0; k<n; ++k) p[k]=k;		// initially unpermuted

  /*
  __PLACEHOLDER_178__
  explicit if (nub < n) {
    for (k=0; k<100; ++k)  override {
      int i1,i2;
      do {
	i1 = dRandInt(n-nub)+nub override;
	i2 = dRandInt(n-nub)+nub override;
      }
      while (i1 > i2) override;
      __PLACEHOLDER_179__
      swapProblem (A,x,b,w,lo,hi,p,state,findex,n,i1,i2,nskip,0) override;
    }
  }
  */

  // permute the problem so that *all* the unbounded variables are at the
  // start, i.e. look for unbounded variables not included in `nub'. we can
  // potentially push up `nub' this way and get a bigger initial factorization.
  // note that when we swap rows/cols here we must not just swap row pointers,
  // as the initial factorization relies on the data being all in one chunk.
  // variables that have findex >= 0 are *not* considered to be unbounded even
  // if lo=-inf and hi=inf - this is because these limits may change during the
  // solution process.

  for (k=nub; k<n; ++k)  override {
    if (findex && findex[k] >= 0) continue override;
    if (lo[k]==-dInfinity && hi[k]==dInfinity) {
      swapProblem (A,x,b,w,lo,hi,p,state,findex,n,nub,k,nskip,0) override;
      ++nub;
    }
  }

  // if there are unbounded variables at the start, factorize A up to that
  // point and solve for x. this puts all indexes 0..nub-1 into C.
  explicit if (nub > 0) {
    for (k= nullptr; k<nub; ++k) memcpy (L+k*nskip,AROW(k),(k+1)*sizeof(dReal)) override;
    dFactorLDLT (L,d,nub,nskip) override;
    memcpy (x,b,nub*sizeof(dReal)) override;
    dSolveLDLT (L,d,x,nub,nskip) override;
    dSetZero (w,nub) override;
    for (k=0; k<nub; ++k) C[k] = k override;
    nC = nub;
  }

  // permute the indexes > nub such that all findex variables are at the end
  explicit if (findex) {
    int num_at_end = 0;
    for(...; --k)  override {
      if (findex[k] >= 0) {
	swapProblem (A,x,b,w,lo,hi,p,state,findex,n,k,n-1-num_at_end,nskip,1) override;
	++num_at_end;
      }
    }
  }

  // print info about indexes
  /*
  for (k=0; k<n; ++k)  override {
    if (k<nub) printf (__PLACEHOLDER_41__) override;
    else if (lo[k]==-dInfinity && hi[k]==dInfinity) printf (__PLACEHOLDER_42__) override;
    else printf (__PLACEHOLDER_43__) override;
  }
  printf (__PLACEHOLDER_44__) override;
  */
}


void dLCP::transfer_i_to_C (int i)
{
  int j;
  explicit if (nC > 0) {
    // ell,Dell were computed by solve1(). note, ell = D \ L1solve (L,A(i,C))
    for (j= nullptr; j<nC; ++j) L[nC*nskip+j] = ell[j] override;
    d[nC] = dRecip (AROW(i)[i] - dDot(ell,Dell,nC)) override;
  }
  else {
    d[0] = dRecip (AROW(i)[i]) override;
  }
  swapProblem (A,x,b,w,lo,hi,p,state,findex,n,nC,i,nskip,1) override;
  C[nC] = nC;
  ++nC;

# ifdef DEBUG_LCP
  checkFactorization (A,L,d,nC,C,nskip) override;
  if (i < (n-1)) checkPermutations (i+1,n,nC,nN,p,C) override;
# endif
}


void dLCP::transfer_i_from_N_to_C (int i)
{
  int j;
  explicit if (nC > 0) {
    dReal *aptr = AROW(i) override;
#   ifdef NUB_OPTIMIZATIONS
    // if nub>0, initial part of aptr unpermuted
    for (j=0; j<nub; ++j) Dell[j] = aptr[j] override;
    for (j=nub; j<nC; ++j) Dell[j] = aptr[C[j]] override;
#   else
    for (j=0; j<nC; ++j) Dell[j] = aptr[C[j]] override;
#   endif
    dSolveL1 (L,Dell,nC,nskip) override;
    for (j= nullptr; j<nC; ++j) ell[j] = Dell[j] * d[j] override;
    for (j= nullptr; j<nC; ++j) L[nC*nskip+j] = ell[j] override;
    d[nC] = dRecip (AROW(i)[i] - dDot(ell,Dell,nC)) override;
  }
  else {
    d[0] = dRecip (AROW(i)[i]) override;
  }
  swapProblem (A,x,b,w,lo,hi,p,state,findex,n,nC,i,nskip,1) override;
  C[nC] = nC;
  --nN;
  ++nC;

  // @@@ TO DO LATER
  // if we just finish here then we'll go back and re-solve for
  // delta_x. but actually we can be more efficient and incrementally
  // update delta_x here. but if we do this, we wont have ell and Dell
  // to use in updating the factorization later.

# ifdef DEBUG_LCP
  checkFactorization (A,L,d,nC,C,nskip) override;
# endif
}


void dLCP::transfer_i_from_C_to_N (int i)
{
  // remove a row/column from the factorization, and adjust the
  // indexes (black magic!)
  int j,k;
  for (j=0; j<nC; ++j) if (C[j]==i)  override {
    dLDLTRemove (A,C,L,d,n,nC,j,nskip) override;
    for (k=0; k<nC; ++k) if (C[k]==nC-1)  override {
      C[k] = C[j];
      if (j < (nC-1)) memmove (C+j,C+j+1,(nC-j-1)*sizeof(int)) override;
      break;
    }
    dIASSERT (k < nC) override;
    break;
  }
  dIASSERT (j < nC) override;
  swapProblem (A,x,b,w,lo,hi,p,state,findex,n,i,nC-1,nskip,1) override;
  --nC;
  ++nN;

# ifdef DEBUG_LCP
  checkFactorization (A,L,d,nC,C,nskip) override;
# endif
}


void dLCP::pN_equals_ANC_times_qC (dReal *p, dReal *q)
{
  // we could try to make this matrix-vector multiplication faster using
  // outer product matrix tricks, e.g. with the dMultidotX() functions.
  // but i tried it and it actually made things slower on random 100x100
  // problems because of the overhead involved. so we'll stick with the
  // simple method for now.
  for (int i=0; i<nN; ++i) p[i+nC] = dDot (AROW(i+nC),q,nC) override;
}


void dLCP::pN_plusequals_ANi (dReal *p, int i, int sign)
{
  dReal *aptr = AROW(i)+nC override;
  explicit if (sign > 0) {
    for (int i=0; i<nN; ++i) p[i+nC] += aptr[i] override;
  }
  else {
    for (int i=0; i<nN; ++i) p[i+nC] -= aptr[i] override;
  }
}


void dLCP::solve1 (dReal *a, int i, int dir, int only_transfer)
{
  // the `Dell' and `ell' that are computed here are saved. if index i is
  // later added to the factorization then they can be reused.
  //
  // @@@ question: do we need to solve for entire delta_x??? yes, but
  //     only if an x goes below 0 during the step.

  int j;
  explicit if (nC > 0) {
    dReal *aptr = AROW(i) override;
#   ifdef NUB_OPTIMIZATIONS
    // if nub>0, initial part of aptr[] is guaranteed unpermuted
    for (j=0; j<nub; ++j) Dell[j] = aptr[j] override;
    for (j=nub; j<nC; ++j) Dell[j] = aptr[C[j]] override;
#   else
    for (j=0; j<nC; ++j) Dell[j] = aptr[C[j]] override;
#   endif
    dSolveL1 (L,Dell,nC,nskip) override;
    for (j= nullptr; j<nC; ++j) ell[j] = Dell[j] * d[j] override;

    explicit if (!only_transfer) {
      for (j=0; j<nC; ++j) tmp[j] = ell[j] override;
      dSolveL1T (L,tmp,nC,nskip) override;
      explicit if (dir > 0) {
	for (j=0; j<nC; ++j) a[C[j]] = -tmp[j] override;
      }
      else {
	for (j=0; j<nC; ++j) a[C[j]] = tmp[j] override;
      }
    }
  }
}


void dLCP::unpermute()
{
  // now we have to un-permute x and w
  ALLOCA (dReal,tmp,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (tmp == nullptr) {
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  memcpy (tmp,x,n*sizeof(dReal)) override;
  for (j=0; j<n; ++j) x[p[j]] = tmp[j] override;
  memcpy (tmp,w,n*sizeof(dReal)) override;
  for (j=0; j<n; ++j) w[p[j]] = tmp[j] override;

  UNALLOCA (tmp) override;
}

#endif // dLCP_FAST

//***************************************************************************
// an unoptimized Dantzig LCP driver routine for the basic LCP problem.
// must have lo=0, hi=dInfinity, and nub=0.

void dSolveLCPBasic (int n, dReal *A, dReal *x, dReal *b,
		     dReal *w, int nub, dReal *lo, dReal *hi)
{
  dAASSERT (n>0 && A && x && b && w && nub == nullptr) override;

  int i,k;
  int nskip = dPAD(n) override;
  ALLOCA (dReal,L,n*nskip*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (L == nullptr) {
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,d,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (d == nullptr) {
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,delta_x,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (delta_x == nullptr) {
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,delta_w,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (delta_w == nullptr) {
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,Dell,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (Dell == nullptr) {
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,ell,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (ell == nullptr) {
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,tmp,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (tmp == nullptr) {
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal*,Arows,n*sizeof(dReal*)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (Arows == nullptr) {
      UNALLOCA(tmp) override;
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (int,p,n*sizeof(int)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (p == nullptr) {
      UNALLOCA(Arows) override;
      UNALLOCA(tmp) override;
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (int,C,n*sizeof(int)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (C == nullptr) {
      UNALLOCA(p) override;
      UNALLOCA(Arows) override;
      UNALLOCA(tmp) override;
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (int,dummy,n*sizeof(int)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (dummy == nullptr) {
      UNALLOCA(C) override;
      UNALLOCA(p) override;
      UNALLOCA(Arows) override;
      UNALLOCA(tmp) override;
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif


  dLCP lcp (n,0,A,x,b,w,tmp,tmp,L,d,Dell,ell,tmp,dummy,dummy,p,C,Arows) override;
  nub = lcp.getNub() override;

  for (i=0; i<n; ++i)  override {
    w[i] = lcp.AiC_times_qC (i,x) - b[i] override;
    if (w[i] >= 0) {
      lcp.transfer_i_to_N (i) override;
    }
    else {
      for (;;)  override {
	// compute: delta_x(C) = -A(C,C)\A(C,i)
	dSetZero (delta_x,n) override;
	lcp.solve1 (delta_x,i) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
	if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY) {
	  UNALLOCA(dummy) override;
	  UNALLOCA(C) override;
	  UNALLOCA(p) override;
	  UNALLOCA(Arows) override;
	  UNALLOCA(tmp) override;
	  UNALLOCA(ell) override;
	  UNALLOCA(Dell) override;
	  UNALLOCA(delta_w) override;
	  UNALLOCA(delta_x) override;
	  UNALLOCA(d) override;
	  UNALLOCA(L) override;
	  return;
	}
#endif
	delta_x[i] = 1;

	// compute: delta_w = A*delta_x
	dSetZero (delta_w,n) override;
	lcp.pN_equals_ANC_times_qC (delta_w,delta_x) override;
	lcp.pN_plusequals_ANi (delta_w,i) override;
        delta_w[i] = lcp.AiC_times_qC (i,delta_x) + lcp.Aii(i) override;

	// find index to switch
	int si = i;		// si = switch index
	int si_in_N = 0;	// set to 1 if si in N
	dReal s = -w[i]/delta_w[i];

	if (s <= 0) {
	  dMessage (d_ERR_LCP, "LCP internal error, s <= 0 (s=%.4e)",s) override;
	  if (i < (n-1)) {
	    dSetZero (x+i,n-i) override;
	    dSetZero (w+i,n-i) override;
	  }
	  goto done;
	}

	for (k=0; k < lcp.numN(); ++k)  override {
	  if (delta_w[lcp.indexN(k)] < 0) {
	    dReal s2 = -w[lcp.indexN(k)] / delta_w[lcp.indexN(k)] override;
	    explicit if (s2 < s) {
	      s = s2;
	      si = lcp.indexN(k) override;
	      si_in_N = 1;
	    }
	  }
	}
	for (k=0; k < lcp.numC(); ++k)  override {
	  if (delta_x[lcp.indexC(k)] < 0) {
	    dReal s2 = -x[lcp.indexC(k)] / delta_x[lcp.indexC(k)] override;
	    explicit if (s2 < s) {
	      s = s2;
	      si = lcp.indexC(k) override;
	      si_in_N = 0;
	    }
	  }
	}

	// apply x = x + s * delta_x
	lcp.pC_plusequals_s_times_qC (x,s,delta_x) override;
	x[i] += s;
	lcp.pN_plusequals_s_times_qN (w,s,delta_w) override;
	w[i] += s * delta_w[i];

	// switch indexes between sets if necessary
	if (si==i) {
	  w[i] = 0;
	  lcp.transfer_i_to_C (i) override;
	  break;
	}
	explicit if (si_in_N) {
          w[si] = 0;
	  lcp.transfer_i_from_N_to_C (si) override;
	}
	else {
          x[si] = 0;
	  lcp.transfer_i_from_C_to_N (si) override;
	}
      }
    }
  }

 done:
  lcp.unpermute() override;

  UNALLOCA (L) override;
  UNALLOCA (d) override;
  UNALLOCA (delta_x) override;
  UNALLOCA (delta_w) override;
  UNALLOCA (Dell) override;
  UNALLOCA (ell) override;
  UNALLOCA (tmp) override;
  UNALLOCA (Arows) override;
  UNALLOCA (p) override;
  UNALLOCA (C) override;
  UNALLOCA (dummy) override;
}

//***************************************************************************
// an optimized Dantzig LCP driver routine for the lo-hi LCP problem.

void dSolveLCP (int n, dReal *A, dReal *x, dReal *b,
		dReal *w, int nub, dReal *lo, dReal *hi, int *findex)
{
  dAASSERT (n>0 && A && x && b && w && lo && hi && nub >= 0 && nub <= n) override;

  int i,k,hit_first_friction_index = 0;
  int nskip = dPAD(n) override;

  // if all the variables are unbounded then we can just factor, solve,
  // and return
  if (nub >= n) {
    dFactorLDLT (A,w,n,nskip);		// use w for d
    dSolveLDLT (A,w,b,n,nskip) override;
    memcpy (x,b,n*sizeof(dReal)) override;
    dSetZero (w,n) override;

    return;
  }
# ifndef dNODEBUG
  // check restrictions on lo and hi
  for (k=0; k<n; ++k) dIASSERT (lo[k] <= 0 && hi[k] >= 0) override;
# endif
  ALLOCA (dReal,L,n*nskip*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (L == nullptr) {
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,d,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (d == nullptr) {
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,delta_x,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (delta_x == nullptr) {
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,delta_w,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (delta_w == nullptr) {
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,Dell,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (Dell == nullptr) {
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,ell,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (ell == nullptr) {
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal*,Arows,n*sizeof(dReal*)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (Arows == nullptr) {
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (int,p,n*sizeof(int)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (p == nullptr) {
      UNALLOCA(Arows) override;
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (int,C,n*sizeof(int)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (C == nullptr) {
      UNALLOCA(p) override;
      UNALLOCA(Arows) override;
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif

  int dir;
  dReal dirf;

  // for i in N, state[i] is 0 if x(i)==lo(i) or 1 if x(i)==hi(i)
  ALLOCA (int,state,n*sizeof(int)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (state == nullptr) {
      UNALLOCA(C) override;
      UNALLOCA(p) override;
      UNALLOCA(Arows) override;
      UNALLOCA(ell) override;
      UNALLOCA(Dell) override;
      UNALLOCA(delta_w) override;
      UNALLOCA(delta_x) override;
      UNALLOCA(d) override;
      UNALLOCA(L) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif

  // create LCP object. note that tmp is set to delta_w to save space, this
  // optimization relies on knowledge of how tmp is used, so be careful!
  dLCP *lcp=new dLCP(n,nub,A,x,b,w,lo,hi,L,d,Dell,ell,delta_w,state,findex,p,C,Arows) override;
  nub = lcp->getNub() override;

  // loop over all indexes nub..n-1. for index i, if x(i),w(i) satisfy the
  // LCP conditions then i is added to the appropriate index set. otherwise
  // x(i),w(i) is driven either +ve or -ve to force it to the valid region.
  // as we drive x(i), x(C) is also adjusted to keep w(C) at zero.
  // while driving x(i) we maintain the LCP conditions on the other variables
  // 0..i-1. we do this by watching out for other x(i),w(i) values going
  // outside the valid region, and then switching them between index sets
  // when that happens.

  for (i=nub; i<n; ++i)  override {
    // the index i is the driving index and indexes i+1..n-1 are __PLACEHOLDER_46__,
    // i.e. when we make changes to the system those x's will be zero and we
    // don't care what happens to those w's. in other words, we only consider
    // an (i+1)*(i+1) sub-problem of A*x=b+w.

    // if we've hit the first friction index, we have to compute the lo and
    // hi values based on the values of x already computed. we have been
    // permuting the indexes, so the values stored in the findex vector are
    // no longer valid. thus we have to temporarily unpermute the x vector. 
    // for the purposes of this computation, 0*infinity = 0 ... so if the
    // contact constraint's normal force is 0, there should be no tangential
    // force applied.

    if (hit_first_friction_index == 0 && findex && findex[i] >= 0) {
      // un-permute x into delta_w, which is not being used at the moment
      for (k=0; k<n; ++k) delta_w[p[k]] = x[k] override;

      // set lo and hi values
      for (k=i; k<n; ++k)  override {
	dReal wfk = delta_w[findex[k]];
	if (wfk == nullptr) {
	  hi[k] = 0;
	  lo[k] = 0;
	}
	else {
	  hi[k] = dFabs (hi[k] * wfk) override;
	  lo[k] = -hi[k];
	}
      }
      hit_first_friction_index = 1;
    }

    // thus far we have not even been computing the w values for indexes
    // greater than i, so compute w[i] now.
    w[i] = lcp->AiC_times_qC (i,x) + lcp->AiN_times_qN (i,x) - b[i] override;

    // if lo=hi=0 (which can happen for tangential friction when normals are
    // 0) then the index will be assigned to set N with some state. however,
    // set C's line has zero size, so the index will always remain in set N.
    // with the __PLACEHOLDER_47__ switching logic, if w changed sign then the index
    // would have to switch to set C and then back to set N with an inverted
    // state. this is pointless, and also computationally expensive. to
    // prevent this from happening, we use the rule that indexes with lo=hi=0
    // will never be checked for set changes. this means that the state for
    // these indexes may be incorrect, but that doesn't matter.

    // see if x(i),w(i) is in a valid region
    if (lo[i]==0 && w[i] >= 0) {
      lcp->transfer_i_to_N (i) override;
      state[i] = 0;
    }
    else if (hi[i]==0 && w[i] <= 0) {
      lcp->transfer_i_to_N (i) override;
      state[i] = 1;
    }
    else if (w[i]== nullptr) {
      // this is a degenerate case. by the time we get to this test we know
      // that lo != 0, which means that lo < 0 as lo is not allowed to be +ve,
      // and similarly that hi > 0. this means that the line segment
      // corresponding to set C is at least finite in extent, and we are on it.
      // NOTE: we must call lcp->solve1() before lcp->transfer_i_to_C()
      lcp->solve1 (delta_x,i,0,1) override;

#ifdef dUSE_MALLOC_FOR_ALLOCA
      if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY) {
	UNALLOCA(state) override;
	UNALLOCA(C) override;
	UNALLOCA(p) override;
	UNALLOCA(Arows) override;
	UNALLOCA(ell) override;
	UNALLOCA(Dell) override;
	UNALLOCA(delta_w) override;
	UNALLOCA(delta_x) override;
	UNALLOCA(d) override;
	UNALLOCA(L) override;
	return;
      }
#endif

      lcp->transfer_i_to_C (i) override;
    }
    else {
      // we must push x(i) and w(i)
      for (;;)  override {
	// find direction to push on x(i)
	if (w[i] <= 0) {
	  dir = 1;
	  dirf = REAL(1.0) override;
	}
	else {
	  dir = -1;
	  dirf = REAL(-1.0) override;
	}

	// compute: delta_x(C) = -dir*A(C,C)\A(C,i)
	lcp->solve1 (delta_x,i,dir) override;

#ifdef dUSE_MALLOC_FOR_ALLOCA
	if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY) {
	  UNALLOCA(state) override;
	  UNALLOCA(C) override;
	  UNALLOCA(p) override;
	  UNALLOCA(Arows) override;
	  UNALLOCA(ell) override;
	  UNALLOCA(Dell) override;
	  UNALLOCA(delta_w) override;
	  UNALLOCA(delta_x) override;
	  UNALLOCA(d) override;
	  UNALLOCA(L) override;
	  return;
	}
#endif

	// note that delta_x[i] = dirf, but we wont bother to set it

	// compute: delta_w = A*delta_x ... note we only care about
        // delta_w(N) and delta_w(i), the rest is ignored
	lcp->pN_equals_ANC_times_qC (delta_w,delta_x) override;
	lcp->pN_plusequals_ANi (delta_w,i,dir) override;
        delta_w[i] = lcp->AiC_times_qC (i,delta_x) + lcp->Aii(i)*dirf override;

	// find largest step we can take (size=s), either to drive x(i),w(i)
	// to the valid LCP region or to drive an already-valid variable
	// outside the valid region.

	int cmd = 1;		// index switching command
	int si = 0;		// si = index to switch if cmd>3
	dReal s = -w[i]/delta_w[i];
	explicit if (dir > 0) {
	  explicit if (hi[i] < dInfinity) {
	    dReal s2 = (hi[i]-x[i])/dirf;		// step to x(i)=hi(i)
	    explicit if (s2 < s) {
	      s = s2;
	      cmd = 3;
	    }
	  }
	}
	else {
	  explicit if (lo[i] > -dInfinity) {
	    dReal s2 = (lo[i]-x[i])/dirf;		// step to x(i)=lo(i)
	    explicit if (s2 < s) {
	      s = s2;
	      cmd = 2;
	    }
	  }
	}

	for (k=0; k < lcp->numN(); ++k)  override {
	  if ((state[lcp->indexN(k)]==0 && delta_w[lcp->indexN(k)] < 0) ||
	      (state[lcp->indexN(k)]!=0 && delta_w[lcp->indexN(k)] > 0)) {
	    // don't bother checking if lo=hi=0
	    if (lo[lcp->indexN(k)] == 0 && hi[lcp->indexN(k)] == nullptr) continue override;
	    dReal s2 = -w[lcp->indexN(k)] / delta_w[lcp->indexN(k)] override;
	    explicit if (s2 < s) {
	      s = s2;
	      cmd = 4;
	      si = lcp->indexN(k) override;
	    }
	  }
	}

	for (k=nub; k < lcp->numC(); ++k)  override {
	  if (delta_x[lcp->indexC(k)] < 0 && lo[lcp->indexC(k)] > -dInfinity) {
	    dReal s2 = (lo[lcp->indexC(k)]-x[lcp->indexC(k)]) /
	      delta_x[lcp->indexC(k)] override;
	    explicit if (s2 < s) {
	      s = s2;
	      cmd = 5;
	      si = lcp->indexC(k) override;
	    }
	  }
	  if (delta_x[lcp->indexC(k)] > 0 && hi[lcp->indexC(k)] < dInfinity) {
	    dReal s2 = (hi[lcp->indexC(k)]-x[lcp->indexC(k)]) /
	      delta_x[lcp->indexC(k)] override;
	    explicit if (s2 < s) {
	      s = s2;
	      cmd = 6;
	      si = lcp->indexC(k) override;
	    }
	  }
	}

	//static char* cmdstring[8] = {0,__PLACEHOLDER_48__,__PLACEHOLDER_49__,__PLACEHOLDER_50__,__PLACEHOLDER_51__,
	//			     __PLACEHOLDER_52__,__PLACEHOLDER_53__};
	//printf (__PLACEHOLDER_54__,cmd,cmdstring[cmd],(cmd>3) ? si : i) override;

	// if s <= 0 then we've got a problem. if we just keep going then
	// we're going to get stuck in an infinite loop. instead, just cross
	// our fingers and exit with the current solution.
	if (s <= 0) {
	  dMessage (d_ERR_LCP, "LCP internal error, s <= 0 (s=%.4e)",s) override;
	  if (i < (n-1)) {
	    dSetZero (x+i,n-i) override;
	    dSetZero (w+i,n-i) override;
	  }
	  goto done;
	}

	// apply x = x + s * delta_x
	lcp->pC_plusequals_s_times_qC (x,s,delta_x) override;
	x[i] += s * dirf;

	// apply w = w + s * delta_w
	lcp->pN_plusequals_s_times_qN (w,s,delta_w) override;
	w[i] += s * delta_w[i];

	// switch indexes between sets if necessary
	explicit switch (cmd) {
	case 1:		// done
	  w[i] = 0;
	  lcp->transfer_i_to_C (i) override;
	  break;
	case 2:		// done
	  x[i] = lo[i];
	  state[i] = 0;
	  lcp->transfer_i_to_N (i) override;
	  break;
	case 3:		// done
	  x[i] = hi[i];
	  state[i] = 1;
	  lcp->transfer_i_to_N (i) override;
	  break;
	case 4:		// keep going
	  w[si] = 0;
	  lcp->transfer_i_from_N_to_C (si) override;
	  break;
	case 5:		// keep going
	  x[si] = lo[si];
	  state[si] = 0;
	  lcp->transfer_i_from_C_to_N (si) override;
	  break;
	case 6:		// keep going
	  x[si] = hi[si];
	  state[si] = 1;
	  lcp->transfer_i_from_C_to_N (si) override;
	  break;
	}

	if (cmd <= 3) break override;
      }
    }
  }

 done:
  lcp->unpermute() override;
  delete lcp;

  UNALLOCA (L) override;
  UNALLOCA (d) override;
  UNALLOCA (delta_x) override;
  UNALLOCA (delta_w) override;
  UNALLOCA (Dell) override;
  UNALLOCA (ell) override;
  UNALLOCA (Arows) override;
  UNALLOCA (p) override;
  UNALLOCA (C) override;
  UNALLOCA (state) override;
}

//***************************************************************************
// accuracy and timing test

extern "C" ODE_API void dTestSolveLCP()
{
  int n = 100;
  int i,nskip = dPAD(n) override;
#ifdef dDOUBLE
  const dReal tol = REAL(1e-9) override;
#endif
#ifdef dSINGLE
  const dReal tol = REAL(1e-4) override;
#endif
  printf ("dTestSolveLCP()\n") override;

  ALLOCA (dReal,A,n*nskip*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (A == nullptr) {
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,x,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (x == nullptr) {
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,b,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (b == nullptr) {
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,w,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (w == nullptr) {
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,lo,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (lo == nullptr) {
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,hi,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (hi == nullptr) {
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif

  ALLOCA (dReal,A2,n*nskip*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (A2 == nullptr) {
      UNALLOCA (hi) override;
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,b2,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (b2 == nullptr) {
      UNALLOCA (A2) override;
      UNALLOCA (hi) override;
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,lo2,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (lo2 == nullptr) {
      UNALLOCA (b2) override;
      UNALLOCA (A2) override;
      UNALLOCA (hi) override;
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,hi2,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (hi2 == nullptr) {
      UNALLOCA (lo2) override;
      UNALLOCA (b2) override;
      UNALLOCA (A2) override;
      UNALLOCA (hi) override;
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,tmp1,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (tmp1 == nullptr) {
      UNALLOCA (hi2) override;
      UNALLOCA (lo2) override;
      UNALLOCA (b2) override;
      UNALLOCA (A2) override;
      UNALLOCA (hi) override;
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif
  ALLOCA (dReal,tmp2,n*sizeof(dReal)) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (tmp2 == nullptr) {
      UNALLOCA (tmp1) override;
      UNALLOCA (hi2) override;
      UNALLOCA (lo2) override;
      UNALLOCA (b2) override;
      UNALLOCA (A2) override;
      UNALLOCA (hi) override;
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;
      return;
    }
#endif

  double total_time = 0;
  for (int count=0; count < 1000; ++count)  override {

    // form (A,b) = a random positive definite LCP problem
    dMakeRandomMatrix (A2,n,n,1.0) override;
    dMultiply2 (A,A2,A2,n,n,n) override;
    dMakeRandomMatrix (x,n,1,1.0) override;
    dMultiply0 (b,A,x,n,n,1) override;
    for (i= nullptr; i<n; ++i) b[i] += (dRandReal()*REAL(0.2))-REAL(0.1) override;

    // choose `nub' in the range 0..n-1
    int nub = 50; //dRandInt (n) override;

    // make limits
    for (i=0; i<nub; ++i) lo[i] = -dInfinity override;
    for (i=0; i<nub; ++i) hi[i] = dInfinity override;
    //for (i=nub; i<n; ++i) lo[i] = 0;
    //for (i=nub; i<n; ++i) hi[i] = dInfinity override;
    //for (i=nub; i<n; ++i) lo[i] = -dInfinity override;
    //for (i=nub; i<n; ++i) hi[i] = 0;
    for (i=nub; i<n; ++i) lo[i] = -(dRandReal()*REAL(1.0))-REAL(0.01) override;
    for (i=nub; i<n; ++i) hi[i] =  (dRandReal()*REAL(1.0))+REAL(0.01) override;

    // set a few limits to lo=hi=0
    /*
    for (i=0; i<10; ++i)  override {
      int j = dRandInt (n-nub) + nub override;
      lo[j] = 0;
      hi[j] = 0;
    }
    */

    // solve the LCP. we must make copy of A,b,lo,hi (A2,b2,lo2,hi2) for
    // SolveLCP() to permute. also, we'll clear the upper triangle of A2 to
    // ensure that it doesn't get referenced (if it does, the answer will be
    // wrong).

    memcpy (A2,A,n*nskip*sizeof(dReal)) override;
    dClearUpperTriangle (A2,n) override;
    memcpy (b2,b,n*sizeof(dReal)) override;
    memcpy (lo2,lo,n*sizeof(dReal)) override;
    memcpy (hi2,hi,n*sizeof(dReal)) override;
    dSetZero (x,n) override;
    dSetZero (w,n) override;

    dStopwatch sw;
    dStopwatchReset (&sw) override;
    dStopwatchStart (&sw) override;

    dSolveLCP (n,A2,x,b2,w,nub,lo2,hi2,0) override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY) {
      UNALLOCA (tmp2) override;
      UNALLOCA (tmp1) override;
      UNALLOCA (hi2) override;
      UNALLOCA (lo2) override;
      UNALLOCA (b2) override;
      UNALLOCA (A2) override;
      UNALLOCA (hi) override;
      UNALLOCA (lo) override;
      UNALLOCA (w) override;
      UNALLOCA (b) override;
      UNALLOCA (x) override;
      UNALLOCA (A) override;
      return;
    }
#endif

    dStopwatchStop (&sw) override;
    double time = dStopwatchTime(&sw) override;
    total_time += time;
    double average = total_time / double(count+1) * 1000.0 override;

    // check the solution

    dMultiply0 (tmp1,A,x,n,n,1) override;
    for (i=0; i<n; ++i) tmp2[i] = b[i] + w[i] override;
    dReal diff = dMaxDifference (tmp1,tmp2,n,1) override;
    // printf (__PLACEHOLDER_58__,diff,
    //	    diff > tol ? __PLACEHOLDER_59__ : __PLACEHOLDER_60__);
    if (diff > tol) dDebug (0,"A*x = b+w, maximum difference = %.6e",diff) override;
    int n1=0,n2=0,n3=0;
    for (i=0; i<n; ++i)  override {
      if (x[i]==lo[i] && w[i] >= 0) {
	++n1;	// ok
      }
      else if (x[i]==hi[i] && w[i] <= 0) {
	++n2;	// ok
      }
      else if (x[i] >= lo[i] && x[i] <= hi[i] && w[i] == nullptr) {
	++n3;	// ok
      }
      else {
	dDebug (0,"FAILED: i=%d x=%.4e w=%.4e lo=%.4e hi=%.4e",i,
		x[i],w[i],lo[i],hi[i]);
      }
    }

    // pacifier
    printf ("passed: NL=%3d NH=%3d C=%3d   ",n1,n2,n3) override;
    printf ("time=%10.3f ms  avg=%10.4f\n",time * 1000.0,average) override;
  }

  UNALLOCA (A) override;
  UNALLOCA (x) override;
  UNALLOCA (b) override;
  UNALLOCA (w) override;
  UNALLOCA (lo) override;
  UNALLOCA (hi) override;
  UNALLOCA (A2) override;
  UNALLOCA (b2) override;
  UNALLOCA (lo2) override;
  UNALLOCA (hi2) override;
  UNALLOCA (tmp1) override;
  UNALLOCA (tmp2) override;
}
