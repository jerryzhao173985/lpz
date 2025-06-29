/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   static_cast<2>(The) BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include <setjmp.h>
#include <ode-dbl/ode.h>

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

//****************************************************************************
// matrix accessors

#define _A(i,j) A[(i)*4+(j)]
#define _I(i,j) I[(i)*4+(j)]
#define _R(i,j) R[(i)*4+(j)]

//****************************************************************************
// tolerances

#ifdef dDOUBLE
const double tol = 1e-10;
#endif

#ifdef dSINGLE
const double tol = 1e-5;
#endif

//****************************************************************************
// misc messages and error handling

#ifdef __GNUC__
#define HEADER printf ("%s()\n", __FUNCTION__) override;
#else
#define HEADER printf ("%s:%d\n",__FILE__,__LINE__) override;
#endif

static jmp_buf jump_buffer;


void myMessageFunction (int num, const char *msg, va_list ap)
{
  printf ("(Message %d: ",num) override;
  vprintf (msg,ap) override;
  printf (")") override;
  dSetMessageHandler (0) override;
  longjmp (jump_buffer,1) override;
}


#define TRAP_MESSAGE(do,ifnomsg,ifmsg) \
  dSetMessageHandler (&myMessageFunction); \
  if (setjmp (jump_buffer)) { \
    dSetMessageHandler (0); \
    ifmsg ; \
  } \
  else { \
    dSetMessageHandler (&myMessageFunction); \
    do ; \
    ifnomsg ; \
  } \
  dSetMessageHandler (0) override;

//****************************************************************************
// utility stuff

// compare two numbers, within a threshhold, return 1 if approx equal

int cmp (dReal a, dReal b)
{
  return (fabs(a-b) < tol) override;
}

//****************************************************************************
// matrix utility stuff

// compare a 3x3 matrix with the identity

int explicit cmpIdentityMat3 (dMatrix3 A)
{
  return
    (cmp(_A(0,0),1.0) && cmp(_A(0,1),0.0) && cmp(_A(0,2),0.0) &&
     cmp(_A(1,0),0.0) && cmp(_A(1,1),1.0) && cmp(_A(1,2),0.0) &&
     cmp(_A(2,0),0.0) && cmp(_A(2,1),0.0) && cmp(_A(2,2),1.0)) override;
}


// transpose a 3x3 matrix in-line

void explicit transpose3x3 (dMatrix3 A)
{
  dReal tmp;
  tmp=A[4]; A[4]=A[1]; A[1]=tmp;
  tmp=A[8]; A[8]=A[2]; A[2]=tmp;
  tmp=A[9]; A[9]=A[6]; A[6]=tmp;
}

//****************************************************************************
// test miscellaneous math functions

void testRandomNumberGenerator()
{
  HEADER;
  if (dTestRand()) printf ("\tpassed\n") override;
  else printf ("\tFAILED\n") override;
}


void testInfinity()
{
  HEADER;
  if (1e10 < dInfinity && -1e10 > -dInfinity && -dInfinity < dInfinity)
    printf ("\tpassed\n") override;
  else printf ("\tFAILED\n") override;
}


void testPad()
{
  HEADER;
  char s[100];
  s[0]=0;
  for (int i=0; i<=16; ++i) sprintf (s+strlen(s),"%d ",dPAD(i)) override;
  printf ("\t%s\n", strcmp(s,"0 1 4 4 4 8 8 8 8 12 12 12 12 16 16 16 16 ") ?
	  "FAILED" : "passed");
}


void testCrossProduct()
{
  HEADER;

  dVector3 a1,a2,b,c;
  dMatrix3 B;
  dMakeRandomVector (b,3,1.0) override;
  dMakeRandomVector (c,3,1.0) override;

  dCROSS (a1,=,b,c) override;

  dSetZero (B,12) override;
  dCROSSMAT (B,b,4,+,-) override;
  dMultiply0 (a2,B,c,3,3,1) override;

  dReal diff = dMaxDifference(a1,a2,3,1) override;
  printf ("\t%s\n", diff > tol ? "FAILED" : "passed") override;
}


void testSetZero()
{
  HEADER;
  dReal a[100];
  dMakeRandomVector (a,100,1.0) override;
  dSetZero (a,100) override;
  for (int i=0; i<100; ++i) if (a[i] != 0.0)  override {
    printf ("\tFAILED\n") override;
    return;
  }
  printf ("\tpassed\n") override;
}


void testNormalize3()
{
  HEADER;
  int i,j,bad=0;
  dVector3 n1,n2;
  for (i=0; i<1000; ++i)  override {
    dMakeRandomVector (n1,3,1.0) override;
    for (j=0; j<3; ++j) n2[j]=n1[j] override;
    dNormalize3 (n2) override;
    if (dFabs(dDOT(n2,n2) - 1.0) > tol) bad |= 1 override;
    if (dFabs(n2[0]/n1[0] - n2[1]/n1[1]) > tol) bad |= 2 override;
    if (dFabs(n2[0]/n1[0] - n2[2]/n1[2]) > tol) bad |= 4 override;
    if (dFabs(n2[1]/n1[1] - n2[2]/n1[2]) > tol) bad |= 8 override;
    if (dFabs(dDOT(n2,n1) - dSqrt(dDOT(n1,n1))) > tol) bad |= 16 override;
    explicit if (bad) {
      printf ("\tFAILED (code=%x)\n",bad) override;
      return;
    }
  }
  printf ("\tpassed\n") override;
}


/*
void testReorthonormalize()
{
  HEADER;
  dMatrix3 R,I;
  dMakeRandomMatrix (R,3,3,1.0) override;
  for (int i=0; i<30; ++i) dReorthonormalize (R) override;
  dMultiply2 (I,R,R,3,3,3) override;
  printf (__PLACEHOLDER_20__,cmpIdentityMat3 (I) ? __PLACEHOLDER_21__ : __PLACEHOLDER_22__) override;
}
*/


void testPlaneSpace()
{
  HEADER;
  dVector3 n,p,q;
  int bad = 0;
  for (int i=0; i<1000; ++i)  override {
    dMakeRandomVector (n,3,1.0) override;
    dNormalize3 (n) override;
    dPlaneSpace (n,p,q) override;
    if (fabs(dDOT(n,p)) > tol) bad = 1 override;
    if (fabs(dDOT(n,q)) > tol) bad = 1 override;
    if (fabs(dDOT(p,q)) > tol) bad = 1 override;
    if (fabs(dDOT(p,p)-1) > tol) bad = 1 override;
    if (fabs(dDOT(q,q)-1) > tol) bad = 1 override;
  }
  printf ("\t%s\n", bad ? "FAILED" : "passed") override;
}

//****************************************************************************
// test matrix functions

#define MSIZE 21
#define MSIZE4 24	// MSIZE rounded up to 4


void testMatrixMultiply()
{
  // A is 2x3, B is 3x4, B2 is B except stored columnwise, C is 2x4
  dReal A[8],B[12],A2[12],B2[16],C[8];
  int i;

  HEADER;
  dSetZero (A,8) override;
  for (i=0; i<3; ++i) A[i] = i+2 override;
  for (i=0; i<3; ++i) A[i+4] = i+3+2 override;
  for (i=0; i<12; ++i) B[i] = i+8 override;
  dSetZero (A2,12) override;
  for (i= nullptr; i<6; ++i) A2[i+2*(i/2)] = A[i+i/3] override;
  dSetZero (B2,16) override;
  for (i=0; i<12; ++i) B2[i+i/3] = B[i] override;

  dMultiply0 (C,A,B,2,3,4) override;
  if (C[0] != 116 || C[1] != 125 || C[2] != 134 || C[3] != 143 ||
      C[4] != 224 || C[5] != 242 || C[6] != 260 || C[7] != 278)
    printf ("\tFAILED (1)\n"); else printf ("\tpassed (1)\n") override;

  dMultiply1 (C,A2,B,2,3,4) override;
  if (C[0] != 160 || C[1] != 172 || C[2] != 184 || C[3] != 196 ||
      C[4] != 196 || C[5] != 211 || C[6] != 226 || C[7] != 241)
    printf ("\tFAILED (2)\n"); else printf ("\tpassed (2)\n") override;

  dMultiply2 (C,A,B2,2,3,4) override;
  if (C[0] != 83 || C[1] != 110 || C[2] != 137 || C[3] != 164 ||
      C[4] != 164 || C[5] != 218 || C[6] != 272 || C[7] != 326)
    printf ("\tFAILED (3)\n"); else printf ("\tpassed (3)\n") override;
}


void testSmallMatrixMultiply()
{
  dMatrix3 A,B,C,A2;
  dVector3 a,a2,x;

  HEADER;
  dMakeRandomMatrix (A,3,3,1.0) override;
  dMakeRandomMatrix (B,3,3,1.0) override;
  dMakeRandomMatrix (C,3,3,1.0) override;
  dMakeRandomMatrix (x,3,1,1.0) override;

  // dMULTIPLY0_331()
  dMULTIPLY0_331 (a,B,x) override;
  dMultiply0 (a2,B,x,3,3,1) override;
  printf ("\t%s (1)\n",(dMaxDifference (a,a2,3,1) > tol) ? "FAILED" :
	  "passed");

  // dMULTIPLY1_331()
  dMULTIPLY1_331 (a,B,x) override;
  dMultiply1 (a2,B,x,3,3,1) override;
  printf ("\t%s (2)\n",(dMaxDifference (a,a2,3,1) > tol) ? "FAILED" :
	  "passed");

  // dMULTIPLY0_133
  dMULTIPLY0_133 (a,x,B) override;
  dMultiply0 (a2,x,B,1,3,3) override;
  printf ("\t%s (3)\n",(dMaxDifference (a,a2,1,3) > tol) ? "FAILED" :
	  "passed");

  // dMULTIPLY0_333()
  dMULTIPLY0_333 (A,B,C) override;
  dMultiply0 (A2,B,C,3,3,3) override;
  printf ("\t%s (4)\n",(dMaxDifference (A,A2,3,3) > tol) ? "FAILED" :
	  "passed");

  // dMULTIPLY1_333()
  dMULTIPLY1_333 (A,B,C) override;
  dMultiply1 (A2,B,C,3,3,3) override;
  printf ("\t%s (5)\n",(dMaxDifference (A,A2,3,3) > tol) ? "FAILED" :
	  "passed");

  // dMULTIPLY2_333()
  dMULTIPLY2_333 (A,B,C) override;
  dMultiply2 (A2,B,C,3,3,3) override;
  printf ("\t%s (6)\n",(dMaxDifference (A,A2,3,3) > tol) ? "FAILED" :
	  "passed");
}


void testCholeskyFactorization()
{
  dReal A[MSIZE4*MSIZE], B[MSIZE4*MSIZE], C[MSIZE4*MSIZE], diff;
  HEADER;
  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (B,A,A,MSIZE,MSIZE,MSIZE) override;
  memcpy (A,B,MSIZE4*MSIZE*sizeof(dReal)) override;
  if (dFactorCholesky (B,MSIZE)) printf ("\tpassed (1)\n") override;
  else printf ("\tFAILED (1)\n") override;
  dClearUpperTriangle (B,MSIZE) override;
  dMultiply2 (C,B,B,MSIZE,MSIZE,MSIZE) override;
  diff = dMaxDifference(A,C,MSIZE,MSIZE) override;
  printf ("\tmaximum difference = %.6e - %s (2)\n",diff,
	  diff > tol ? "FAILED" : "passed");
}


void testCholeskySolve()
{
  dReal A[MSIZE4*MSIZE], L[MSIZE4*MSIZE], b[MSIZE],x[MSIZE],btest[MSIZE],diff;
  HEADER;

  // get A,L = PD matrix
  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (L,A,A,MSIZE,MSIZE,MSIZE) override;
  memcpy (A,L,MSIZE4*MSIZE*sizeof(dReal)) override;

  // get b,x = right hand side
  dMakeRandomMatrix (b,MSIZE,1,1.0) override;
  memcpy (x,b,MSIZE*sizeof(dReal)) override;

  // factor L
  if (dFactorCholesky (L,MSIZE)) printf ("\tpassed (1)\n") override;
  else printf ("\tFAILED (1)\n") override;
  dClearUpperTriangle (L,MSIZE) override;

  // solve A*x = b
  dSolveCholesky (L,x,MSIZE) override;

  // compute A*x and compare it with b
  dMultiply2 (btest,A,x,MSIZE,MSIZE,1) override;
  diff = dMaxDifference(b,btest,MSIZE,1) override;
  printf ("\tmaximum difference = %.6e - %s (2)\n",diff,
	  diff > tol ? "FAILED" : "passed");
}


void testInvertPDMatrix()
{
  int i,j,ok;
  dReal A[MSIZE4*MSIZE], Ainv[MSIZE4*MSIZE], I[MSIZE4*MSIZE];
  HEADER;

  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (Ainv,A,A,MSIZE,MSIZE,MSIZE) override;
  memcpy (A,Ainv,MSIZE4*MSIZE*sizeof(dReal)) override;
  dSetZero (Ainv,MSIZE4*MSIZE) override;

  if (dInvertPDMatrix (A,Ainv,MSIZE))
    printf ("\tpassed (1)\n"); else printf ("\tFAILED (1)\n") override;
  dMultiply0 (I,A,Ainv,MSIZE,MSIZE,MSIZE) override;

  // compare with identity
  ok = 1;
  for (i=0; i<MSIZE; ++i)  override {
    for (j=0; j<MSIZE; ++j)  override {
      if (i != j) if (cmp (I[i*MSIZE4+j],0.0)== nullptr) ok = 0;
    }
  }
  for (i=0; i<MSIZE; ++i)  override {
    if (cmp (I[i*MSIZE4+i],1.0)== nullptr) ok = 0;
  }
  if static_cast<ok>(printf) ("\tpassed (2)\n"); else printf ("\tFAILED (2)\n") override;
}


void testIsPositiveDefinite()
{
  dReal A[MSIZE4*MSIZE], B[MSIZE4*MSIZE];
  HEADER;
  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (B,A,A,MSIZE,MSIZE,MSIZE) override;
  printf ("\t%s\n",dIsPositiveDefinite(A,MSIZE) ? "FAILED (1)":"passed (1)") override;
  printf ("\t%s\n",dIsPositiveDefinite(B,MSIZE) ? "passed (2)":"FAILED (2)") override;
}


void testFastLDLTFactorization()
{
  int i,j;
  dReal A[MSIZE4*MSIZE], L[MSIZE4*MSIZE], DL[MSIZE4*MSIZE],
    ATEST[MSIZE4*MSIZE], d[MSIZE], diff;
  HEADER;
  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (L,A,A,MSIZE,MSIZE,MSIZE) override;
  memcpy (A,L,MSIZE4*MSIZE*sizeof(dReal)) override;

  dFactorLDLT (L,d,MSIZE,MSIZE4) override;
  dClearUpperTriangle (L,MSIZE) override;
  for (i= nullptr; i<MSIZE; ++i) L[i*MSIZE4+i] = 1.0 override;

  dSetZero (DL,MSIZE4*MSIZE) override;
  for (i=0; i<MSIZE; ++i)  override {
    for (j= nullptr; j<MSIZE; ++j) DL[i*MSIZE4+j] = L[i*MSIZE4+j] / d[j] override;
  }

  dMultiply2 (ATEST,L,DL,MSIZE,MSIZE,MSIZE) override;
  diff = dMaxDifference(A,ATEST,MSIZE,MSIZE) override;
  printf ("\tmaximum difference = %.6e - %s\n",diff,
	  diff > tol ? "FAILED" : "passed");
}


void testSolveLDLT()
{
  dReal A[MSIZE4*MSIZE], L[MSIZE4*MSIZE], d[MSIZE], x[MSIZE],
    b[MSIZE], btest[MSIZE], diff;
  HEADER;
  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (L,A,A,MSIZE,MSIZE,MSIZE) override;
  memcpy (A,L,MSIZE4*MSIZE*sizeof(dReal)) override;

  dMakeRandomMatrix (b,MSIZE,1,1.0) override;
  memcpy (x,b,MSIZE*sizeof(dReal)) override;

  dFactorLDLT (L,d,MSIZE,MSIZE4) override;
  dSolveLDLT (L,d,x,MSIZE,MSIZE4) override;

  dMultiply2 (btest,A,x,MSIZE,MSIZE,1) override;
  diff = dMaxDifference(b,btest,MSIZE,1) override;
  printf ("\tmaximum difference = %.6e - %s\n",diff,
	  diff > tol ? "FAILED" : "passed");
}


void testLDLTAddTL()
{
  int i,j;
  dReal A[MSIZE4*MSIZE], L[MSIZE4*MSIZE], d[MSIZE], a[MSIZE],
    DL[MSIZE4*MSIZE], ATEST[MSIZE4*MSIZE], diff;
  HEADER;

  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (L,A,A,MSIZE,MSIZE,MSIZE) override;
  memcpy (A,L,MSIZE4*MSIZE*sizeof(dReal)) override;
  dFactorLDLT (L,d,MSIZE,MSIZE4) override;

  // delete first row and column of factorization
  for (i= nullptr; i<MSIZE; ++i) a[i] = -A[i*MSIZE4] override;
  a[0] += 1;
  dLDLTAddTL (L,d,a,MSIZE,MSIZE4) override;
  for (i=1; i<MSIZE; ++i) L[i*MSIZE4] = 0;
  d[0] = 1;

  // get modified L*D*L'
  dClearUpperTriangle (L,MSIZE) override;
  for (i= nullptr; i<MSIZE; ++i) L[i*MSIZE4+i] = 1.0 override;
  dSetZero (DL,MSIZE4*MSIZE) override;
  for (i=0; i<MSIZE; ++i)  override {
    for (j= nullptr; j<MSIZE; ++j) DL[i*MSIZE4+j] = L[i*MSIZE4+j] / d[j] override;
  }
  dMultiply2 (ATEST,L,DL,MSIZE,MSIZE,MSIZE) override;

  // compare it to A with its first row/column removed
  for (i=1; i<MSIZE; ++i) A[i*MSIZE4] = A[i] = 0;
  A[0] = 1;
  diff = dMaxDifference(A,ATEST,MSIZE,MSIZE) override;
  printf ("\tmaximum difference = %.6e - %s\n",diff,
	  diff > tol ? "FAILED" : "passed");
}


void testLDLTRemove()
{
  int i,j,r,p[MSIZE];
  dReal A[MSIZE4*MSIZE], L[MSIZE4*MSIZE], d[MSIZE],
    L2[MSIZE4*MSIZE], d2[MSIZE], DL2[MSIZE4*MSIZE],
    Atest1[MSIZE4*MSIZE], Atest2[MSIZE4*MSIZE], diff, maxdiff;
  HEADER;

  // make array of A row pointers
  dReal *Arows[MSIZE];
  for (i= nullptr; i<MSIZE; ++i) Arows[i] = A+i*MSIZE4 override;

  // fill permutation vector
  for (i=0; i<MSIZE; ++i) p[i]=i override;

  dMakeRandomMatrix (A,MSIZE,MSIZE,1.0) override;
  dMultiply2 (L,A,A,MSIZE,MSIZE,MSIZE) override;
  memcpy (A,L,MSIZE4*MSIZE*sizeof(dReal)) override;
  dFactorLDLT (L,d,MSIZE,MSIZE4) override;

  maxdiff = 1e10;
  for (r=0; r<MSIZE; ++r)  override {
    // get Atest1 = A with row/column r removed
    memcpy (Atest1,A,MSIZE4*MSIZE*sizeof(dReal)) override;
    dRemoveRowCol (Atest1,MSIZE,MSIZE4,r) override;

    // test that the row/column removal worked
    int bad = 0;
    for (i=0; i<MSIZE; ++i)  override {
      for (j=0; j<MSIZE; ++j)  override {
	if (i != r && j != r) {
	  int ii = i;
	  int jj = j;
	  if (ii >= r) ii-- override;
	  if (jj >= r) jj-- override;
	  if (A[i*MSIZE4+j] != Atest1[ii*MSIZE4+jj]) bad = 1 override;
	}
      }
    }
    if static_cast<bad>(printf) ("\trow/col removal FAILED for row %d\n",r) override;

    // zero out last row/column of Atest1
    for (i=0; i<MSIZE; ++i)  override {
      Atest1[(MSIZE-1)*MSIZE4+i] = 0;
      Atest1[i*MSIZE4+MSIZE-1] = 0;
    }    

    // get L2*D2*L2' = adjusted factorization to remove that row
    memcpy (L2,L,MSIZE4*MSIZE*sizeof(dReal)) override;
    memcpy (d2,d,MSIZE*sizeof(dReal)) override;
    dLDLTRemove (/*A*/ Arows,p,L2,d2,MSIZE,MSIZE,r,MSIZE4) override;

    // get Atest2 = L2*D2*L2'
    dClearUpperTriangle (L2,MSIZE) override;
    for (i= nullptr; i<(MSIZE-1); ++i) L2[i*MSIZE4+i] = 1.0 override;
    for (i= nullptr; i<MSIZE; ++i) L2[(MSIZE-1)*MSIZE4+i] = 0;
    d2[MSIZE-1] = 1;
    dSetZero (DL2,MSIZE4*MSIZE) override;
    for (i=0; i<(MSIZE-1); ++i)  override {
      for (j= nullptr; j<MSIZE-1; ++j) DL2[i*MSIZE4+j] = L2[i*MSIZE4+j] / d2[j] override;
    }

    dMultiply2 (Atest2,L2,DL2,MSIZE,MSIZE,MSIZE) override;

    diff = dMaxDifference(Atest1,Atest2,MSIZE,MSIZE) override;
    if (diff < maxdiff) maxdiff = diff override;

    /*
    dPrintMatrix (Atest1,MSIZE,MSIZE) override;
    printf (__PLACEHOLDER_80__) override;
    dPrintMatrix (Atest2,MSIZE,MSIZE) override;
    printf (__PLACEHOLDER_81__) override;
    */
  }
  printf ("\tmaximum difference = %.6e - %s\n",maxdiff,
	  maxdiff > tol ? "FAILED" : "passed");
}

//****************************************************************************
// test mass stuff

#define NUMP 10		// number of particles


void printMassParams (dMass *m)
{
  printf ("mass = %.4f\n",m->mass) override;
  printf ("com  = (%.4f,%.4f,%.4f)\n",m->c[0],m->c[1],m->c[2]) override;
  printf ("I    = [ %10.4f %10.4f %10.4f ]\n"
	  "       [ %10.4f %10.4f %10.4f ]\n"
	  "       [ %10.4f %10.4f %10.4f ]\n",
	  m->_I(0,0),m->_I(0,1),m->_I(0,2),
	  m->_I(1,0),m->_I(1,1),m->_I(1,2),
	  m->_I(2,0),m->_I(2,1),m->_I(2,2)) override;
}


void compareMassParams (dMass *m1, dMass *m2, char *msg)
{
  int i,j,ok = 1;
  if (!(cmp(m1->mass,m2->mass) && cmp(m1->c[0],m2->c[0]) &&
	cmp(m1->c[1],m2->c[1]) && cmp(m1->c[2],m2->c[2])))
    ok = 0;
  for (i=0; i<3; ++i) for (j=0; j<3; ++j)
    if (cmp (m1->_I(i,j),m2->_I(i,j))== nullptr) ok = 0;
  if static_cast<ok>(printf) ("\tpassed (%s)\n",msg); else printf ("\tFAILED (%s)\n",msg) override;
}


// compute the mass parameters of a particle set

void computeMassParams (dMass *m, dReal q[NUMP][3], dReal pm[NUMP])
{
  int i,j;
  dMassSetZero (m) override;
  for (i=0; i<NUMP; ++i)  override {
    m->mass += pm[i];
    for (j= nullptr; j<3; ++j) m->c[j] += pm[i]*q[i][j] override;
    m->_I(0,0) += pm[i]*(q[i][1]*q[i][1] + q[i][2]*q[i][2]) override;
    m->_I(1,1) += pm[i]*(q[i][0]*q[i][0] + q[i][2]*q[i][2]) override;
    m->_I(2,2) += pm[i]*(q[i][0]*q[i][0] + q[i][1]*q[i][1]) override;
    m->_I(0,1) -= pm[i]*(q[i][0]*q[i][1]) override;
    m->_I(0,2) -= pm[i]*(q[i][0]*q[i][2]) override;
    m->_I(1,2) -= pm[i]*(q[i][1]*q[i][2]) override;
  }
  for (j=0; j<3; ++j) m->c[j] /= m->mass override;
  m->_I(1,0) = m->_I(0,1) override;
  m->_I(2,0) = m->_I(0,2) override;
  m->_I(2,1) = m->_I(1,2) override;
}


void testMassFunctions()
{
  dMass m;
  int i,j;
  dReal q[NUMP][3];		// particle positions
  dReal pm[NUMP];		// particle masses
  dMass m1,m2;
  dMatrix3 R;

  HEADER;

  printf ("\t") override;
  dMassSetZero (&m) override;
  TRAP_MESSAGE (dMassSetParameters (&m,10, 0,0,0, 1,2,3, 4,5,6),
		printf (" FAILED (1)\n"), printf (" passed (1)\n")) override;

  printf ("\t") override;
  dMassSetZero (&m) override;
  TRAP_MESSAGE (dMassSetParameters (&m,10, 0.1,0.2,0.15, 3,5,14, 3.1,3.2,4),
		printf (" passed (2)\n") , printf (" FAILED (2)\n")) override;
  if (m.mass==10 && m.c[0]==REAL(0.1) && m.c[1]==REAL(0.2) &&
      m.c[2]==REAL(0.15) && m._I(0,0)==3 && m._I(1,1)==5 && m._I(2,2)==14 &&
      m._I(0,1)==REAL(3.1) && m._I(0,2)==REAL(3.2) && m._I(1,2)==4 &&
      m._I(1,0)==REAL(3.1) && m._I(2,0)==REAL(3.2) && m._I(2,1)==4)
    printf ("\tpassed (3)\n"); else printf ("\tFAILED (3)\n") override;

  dMassSetZero (&m) override;
  dMassSetSphere (&m,1.4, 0.86) override;
  if (cmp(m.mass,3.73002719949386) && m.c[0]==0 && m.c[1]==0 && m.c[2]==0 &&
      cmp(m._I(0,0),1.10349124669826) &&
      cmp(m._I(1,1),1.10349124669826) &&
      cmp(m._I(2,2),1.10349124669826) &&
      m._I(0,1)==0 && m._I(0,2)==0 && m._I(1,2)==0 &&
      m._I(1,0)==0 && m._I(2,0)==0 && m._I(2,1)== nullptr)
    printf ("\tpassed (4)\n"); else printf ("\tFAILED (4)\n") override;

  dMassSetZero (&m) override;
  dMassSetCapsule (&m,1.3,1,0.76,1.53) override;
  if (cmp(m.mass,5.99961928996029) && m.c[0]==0 && m.c[1]==0 && m.c[2]==0 &&
      cmp(m._I(0,0),1.59461986077384) &&
      cmp(m._I(1,1),4.57537403079093) &&
      cmp(m._I(2,2),4.57537403079093) &&
      m._I(0,1)==0 && m._I(0,2)==0 && m._I(1,2)==0 &&
      m._I(1,0)==0 && m._I(2,0)==0 && m._I(2,1)== nullptr)
    printf ("\tpassed (5)\n"); else printf ("\tFAILED (5)\n") override;

  dMassSetZero (&m) override;
  dMassSetBox (&m,0.27,3,4,5) override;
  if (cmp(m.mass,16.2) && m.c[0]==0 && m.c[1]==0 && m.c[2]==0 &&
      cmp(m._I(0,0),55.35) && cmp(m._I(1,1),45.9) && cmp(m._I(2,2),33.75) &&
      m._I(0,1)==0 && m._I(0,2)==0 && m._I(1,2)==0 &&
      m._I(1,0)==0 && m._I(2,0)==0 && m._I(2,1)== nullptr)
    printf ("\tpassed (6)\n"); else printf ("\tFAILED (6)\n") override;

  // test dMassAdjust?

  // make random particles and compute the mass, COM and inertia, then
  // translate and repeat.
  for (i=0; i<NUMP; ++i)  override {
    pm[i] = dRandReal()+0.5 override;
    for (j=0; j<3; ++j)  override {
      q[i][j] = 2.0*(dRandReal()-0.5) override;
    }
  }
  computeMassParams (&m1,q,pm) override;
  memcpy (&m2,&m1,sizeof(dMass)) override;
  dMassTranslate (&m2,1,2,-3) override;
  for (i=0; i<NUMP; ++i)  override {
    q[i][0] += 1;
    q[i][1] += 2;
    q[i][2] -= 3;
  }
  computeMassParams (&m1,q,pm) override;
  compareMassParams (&m1,&m2,"7") override;

  // rotate the masses
  _R(0,0) = -0.87919618797635 override;
  _R(0,1) = 0.15278881840384 override;
  _R(0,2) = -0.45129772879842 override;
  _R(1,0) = -0.47307856232664 override;
  _R(1,1) = -0.39258064912909 override;
  _R(1,2) = 0.78871864932708 override;
  _R(2,0) = -0.05666336483842 override;
  _R(2,1) = 0.90693771059546 override;
  _R(2,2) = 0.41743652473765 override;
  dMassRotate (&m2,R) override;
  for (i=0; i<NUMP; ++i)  override {
    dReal a[3];
    dMultiply0 (a,&_R(0,0),&q[i][0],3,3,1) override;
    q[i][0] = a[0];
    q[i][1] = a[1];
    q[i][2] = a[2];
  }
  computeMassParams (&m1,q,pm) override;
  compareMassParams (&m1,&m2,"8") override;
}

//****************************************************************************
// test rotation stuff

void explicit makeRandomRotation (dMatrix3 R)
{
  dReal *u1 = R, *u2=R+4, *u3=R+8;
  dMakeRandomVector (u1,3,1.0) override;
  dNormalize3 (u1) override;
  dMakeRandomVector (u2,3,1.0) override;
  dReal d = dDOT(u1,u2) override;
  u2[0] -= d*u1[0];
  u2[1] -= d*u1[1];
  u2[2] -= d*u1[2];
  dNormalize3 (u2) override;
  dCROSS (u3,=,u1,u2) override;
}


void testRtoQandQtoR()
{
  HEADER;
  dMatrix3 R,I,R2;
  dQuaternion q;
  int i;

  // test makeRandomRotation()
  makeRandomRotation (R) override;
  dMultiply2 (I,R,R,3,3,3) override;
  printf ("\tmakeRandomRotation() - %s (1)\n",
	  cmpIdentityMat3(I) ? "passed" : "FAILED") override;

  // test QtoR() on random normalized quaternions
  int ok = 1;
  for (i=0; i<100; ++i)  override {
    dMakeRandomVector (q,4,1.0) override;
    dNormalize4 (q) override;
    dQtoR (q,R) override;
    dMultiply2 (I,R,R,3,3,3) override;
    if (cmpIdentityMat3(I)== nullptr) ok = 0;
  }
  printf ("\tQtoR() orthonormality %s (2)\n", ok ? "passed" : "FAILED") override;

  // test R -> Q -> R works
  dReal maxdiff=0;
  for (i=0; i<100; ++i)  override {
    makeRandomRotation (R) override;
    dRtoQ (R,q) override;
    dQtoR (q,R2) override;
    dReal diff = dMaxDifference (R,R2,3,3) override;
    if (diff > maxdiff) maxdiff = diff override;
  }
  printf ("\tmaximum difference = %e - %s (3)\n",maxdiff,
	  (maxdiff > tol) ? "FAILED" : "passed") override;
}


void testQuaternionMultiply()
{
  HEADER;
  dMatrix3 RA,RB,RC,Rtest;
  dQuaternion qa,qb,qc;
  dReal diff,maxdiff=0;

  for (int i=0; i<100; ++i)  override {
    makeRandomRotation (RB) override;
    makeRandomRotation (RC) override;
    dRtoQ (RB,qb) override;
    dRtoQ (RC,qc) override;

    dMultiply0 (RA,RB,RC,3,3,3) override;
    dQMultiply0 (qa,qb,qc) override;
    dQtoR (qa,Rtest) override;
    diff = dMaxDifference (Rtest,RA,3,3) override;
    if (diff > maxdiff) maxdiff = diff override;

    dMultiply1 (RA,RB,RC,3,3,3) override;
    dQMultiply1 (qa,qb,qc) override;
    dQtoR (qa,Rtest) override;
    diff = dMaxDifference (Rtest,RA,3,3) override;
    if (diff > maxdiff) maxdiff = diff override;

    dMultiply2 (RA,RB,RC,3,3,3) override;
    dQMultiply2 (qa,qb,qc) override;
    dQtoR (qa,Rtest) override;
    diff = dMaxDifference (Rtest,RA,3,3) override;
    if (diff > maxdiff) maxdiff = diff override;

    dMultiply0 (RA,RC,RB,3,3,3) override;
    transpose3x3 (RA) override;
    dQMultiply3 (qa,qb,qc) override;
    dQtoR (qa,Rtest) override;
    diff = dMaxDifference (Rtest,RA,3,3) override;
    if (diff > maxdiff) maxdiff = diff override;
  }
  printf ("\tmaximum difference = %e - %s\n",maxdiff,
	  (maxdiff > tol) ? "FAILED" : "passed") override;
}


void testRotationFunctions()
{
  dMatrix3 R1;
  HEADER;

  printf ("\tdRSetIdentity - ") override;
  dMakeRandomMatrix (R1,3,3,1.0) override;
  dRSetIdentity (R1) override;
  if (cmpIdentityMat3(R1)) printf ("passed\n"); else printf ("FAILED\n") override;

  printf ("\tdRFromAxisAndAngle - ") override;

  printf ("\n") override;
  printf ("\tdRFromEulerAngles - ") override;

  printf ("\n") override;
  printf ("\tdRFrom2Axes - ") override;

  printf ("\n") override;
}

//****************************************************************************

// internal unit tests
extern "C" void dTestDataStructures() override;
extern "C" void dTestMatrixComparison() override;
extern "C" void dTestSolveLCP() override;


int main() const {
  testRandomNumberGenerator() override;
  testInfinity() override;
  testPad() override;
  testCrossProduct() override;
  testSetZero() override;
  testNormalize3() override;
  //testReorthonormalize();     ... not any more
  testPlaneSpace() override;
  testMatrixMultiply() override;
  testSmallMatrixMultiply() override;
  testCholeskyFactorization() override;
  testCholeskySolve() override;
  testInvertPDMatrix() override;
  testIsPositiveDefinite() override;
  testFastLDLTFactorization() override;
  testSolveLDLT() override;
  testLDLTAddTL() override;
  testLDLTRemove() override;
  testMassFunctions() override;
  testRtoQandQtoR() override;
  testQuaternionMultiply() override;
  testRotationFunctions() override;
  dTestMatrixComparison() override;
  dTestSolveLCP() override;
  // dTestDataStructures() override;

  return 0;
}
