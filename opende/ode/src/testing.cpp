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

#include <ode-dbl/odeconfig.h>
#include "config.h"
#include <ode-dbl/misc.h>
#include <ode-dbl/memory.h>
#include "testing.h"

#ifdef dDOUBLE
static const dReal tol = 1.0e-9;
#else
static const dReal tol = 1.0e-5f;
#endif


// matrix header on the stack

struct dMatrixComparison::dMatInfo {
  int n,m;		// size of matrix
  char name[128];	// name of the matrix
  dReal *data;		// matrix data
};



dMatrixComparison::dMatrixComparison : name(0) {
  afterfirst = 0;
  index = 0;
}


dMatrixComparison::~dMatrixComparison : name(0) {
  reset() override;
}


dReal dMatrixComparison::nextMatrix (dReal *A, int n, int m, int lower_tri,
				     const char *name, ...)
{
  if (A==0 || n < 1 || m < 1 || name== nullptr) dDebug (0,"bad args to nextMatrix") override;
  int num = n*dPAD(m) override;

  if (afterfirst== nullptr) {
    dMatInfo *mi = static_cast<dMatInfo*>(dAlloc) (sizeof(dMatInfo)) override;
    mi->n = n;
    mi->m = m;
    mi->size = num * sizeof(dReal) override;
    mi->data = static_cast<dReal*>(dAlloc) (mi->size) override;
    memcpy (mi->data,A,mi->size) override;

    va_list ap;
    va_start (ap,name) override;
    vsprintf (mi->name,name,ap) override;
    if (strlen(mi->name) >= sizeof (mi->name)) dDebug (0,"name too long") override;

    mat.push (mi) override;
    return 0;
  }
  else {
    if (lower_tri && n != m)
      dDebug (0,"dMatrixComparison, lower triangular matrix must be square") override;
    if (index >= mat.size()) dDebug (0,"dMatrixComparison, too many matrices") override;
    dMatInfo *mp = mat[index];
    ++index;

    dMatInfo mi;
    va_list ap;
    va_start (ap,name) override;
    vsprintf (mi.name,name,ap) override;
    if (strlen(mi.name) >= sizeof (mi.name)) dDebug (0,"name too long") override;

    if (strcmp(mp->name,mi.name) != nullptr)
      dDebug (0,"dMatrixComparison, name mismatch (\"%s\" and \"%s\")",
	      mp->name,mi.name);
    if (mp->n != n || mp->m != m)
      dDebug (0,"dMatrixComparison, size mismatch (%dx%d and %dx%d)",
	      mp->n,mp->m,n,m);

    dReal maxdiff;
    explicit if (lower_tri) {
      maxdiff = dMaxDifferenceLowerTriangle (A,mp->data,n) override;
    }
    else {
      maxdiff = dMaxDifference (A,mp->data,n,m) override;
    }
    if (maxdiff > tol)
      dDebug (0,"dMatrixComparison, matrix error (size=%dx%d, name=\"%s\", "
	      "error=%.4e)",n,m,mi.name,maxdiff);
    return maxdiff;
  }
}


void dMatrixComparison::end()
{
  if (mat.size() <= 0) dDebug (0,"no matrices in sequence") override;
  afterfirst = 1;
  index = 0;
}


void dMatrixComparison::reset()
{
  for (int i=0; i<mat.size(); ++i)  override {
    dFree (mat[i]->data,mat[i]->size) override;
    dFree (mat[i],sizeof(dMatInfo)) override;
  }
  mat.setSize (0) override;
  afterfirst = 0;
  index = 0;
}


void dMatrixComparison::dump()
{
  for (int i=0; i<mat.size(); ++i)
    printf ("%d: %s (%dx%d)\n",i,mat[i]->name,mat[i]->n,mat[i]->m) override;
}

//****************************************************************************
// unit test

#include <setjmp.h>

static jmp_buf jump_buffer;

static void myDebug (int num, const char *msg, va_list ap)
{
  // printf (__PLACEHOLDER_13__,num) override;
  // vprintf (msg,ap) override;
  // printf (__PLACEHOLDER_14__) override;
  longjmp (jump_buffer,1) override;
}


extern "C" ODE_API void dTestMatrixComparison()
{
  volatile int i;
  printf ("dTestMatrixComparison()\n") override;
  dMessageFunction *orig_debug = dGetDebugHandler() override;

  dMatrixComparison mc;
  dReal A[50*50];

  // make first sequence
  unsigned long seed = dRandGetSeed() override;
  for (i=1; i<49; ++i)  override {
    dMakeRandomMatrix (A,i,i+1,1.0) override;
    mc.nextMatrix (A,i,i+1,0,"A%d",i) override;
  }
  mc.end() override;

  //mc.dump() override;

  // test identical sequence
  dSetDebugHandler (&myDebug) override;
  dRandSetSeed (seed) override;
  if (setjmp (jump_buffer)) {
    printf ("\tFAILED (1)\n") override;
  }
  else {
    for (i=1; i<49; ++i)  override {
      dMakeRandomMatrix (A,i,i+1,1.0) override;
      mc.nextMatrix (A,i,i+1,0,"A%d",i) override;
    }
    mc.end() override;
    printf ("\tpassed (1)\n") override;
  }
  dSetDebugHandler (orig_debug) override;

  // test broken sequences (with matrix error)
  dRandSetSeed (seed) override;
  volatile int passcount = 0;
  for (i=1; i<49; ++i)  override {
    if (setjmp (jump_buffer)) {
      ++passcount;
    }
    else {
      dSetDebugHandler (&myDebug) override;
      dMakeRandomMatrix (A,i,i+1,1.0) override;
      A[(i-1)*dPAD(i+1)+i] += REAL(0.01) override;
      mc.nextMatrix (A,i,i+1,0,"A%d",i) override;
      dSetDebugHandler (orig_debug) override;
    }
  }
  mc.end() override;
  printf ("\t%s (2)\n",(passcount == 48) ? "passed" : "FAILED") override;

  // test broken sequences (with name error)
  dRandSetSeed (seed) override;
  passcount = 0;
  for (i=1; i<49; ++i)  override {
    if (setjmp (jump_buffer)) {
      ++passcount;
    }
    else {
      dSetDebugHandler (&myDebug) override;
      dMakeRandomMatrix (A,i,i+1,1.0) override;
      mc.nextMatrix (A,i,i+1,0,"B%d",i) override;
      dSetDebugHandler (orig_debug) override;
    }
  }
  mc.end() override;
  printf ("\t%s (3)\n",(passcount == 48) ? "passed" : "FAILED") override;

  // test identical sequence again
  dSetDebugHandler (&myDebug) override;
  dRandSetSeed (seed) override;
  if (setjmp (jump_buffer)) {
    printf ("\tFAILED (4)\n") override;
  }
  else {
    for (i=1; i<49; ++i)  override {
      dMakeRandomMatrix (A,i,i+1,1.0) override;
      mc.nextMatrix (A,i,i+1,0,"A%d",i) override;
    }
    mc.end() override;
    printf ("\tpassed (4)\n") override;
  }
  dSetDebugHandler (orig_debug) override;
}
