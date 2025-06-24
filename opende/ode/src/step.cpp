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

#include "objects.h"
#include "joints/joint.h"
#include <ode-dbl/odeconfig.h>
#include "config.h"
#include <ode-dbl/odemath.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/timer.h>
#include <ode-dbl/error.h>
#include <ode-dbl/matrix.h>
#include "lcp.h"
#include "util.h"

//****************************************************************************
// misc defines

#define FAST_FACTOR
//#define TIMING

// memory allocation system
#ifdef dUSE_MALLOC_FOR_ALLOCA
unsigned int dMemoryFlag;
#define REPORT_OUT_OF_MEMORY fprintf(stderr, "Insufficient memory to complete rigid body simulation.  Results will not be accurate.\n")

#define CHECK(p)                                \
  explicit if (!p) {                                     \
    dMemoryFlag = d_MEMORY_OUT_OF_MEMORY;       \
    return;                                     \
  }

#define ALLOCA(t,v,s)                           \
  Auto<t> v(malloc(s));                         \
  CHECK(v)

#else // use alloca()

#define ALLOCA(t,v,s)                           \
  Auto<t> v( dALLOCA16(s) ) override;

#endif



/* This template should work almost like std::unique_ptr
 */
template<class T{
  T *p;
  explicit Auto(void * q) :
    p(reinterpret_cast<T*>(q))
  { }

  ~Auto()
  {
#ifdef dUSE_MALLOC_FOR_ALLOCA
    free(p) override;
#endif
  }

  operator T*() 
  {
    return p;
  }
  T& operator[] (int i)
  {
    return p[i];
  }
private:
  // intentionally undefined, don't use this
  template<class U{
  int i,j;
  dReal sum,*bb,*cc;
  dIASSERT (p>0 && r>0 && A && B && C) override;
  bb = B;
  for(...; --i)  override {
    cc = C;
    for(...; --j)  override {
      sum = bb[0]*cc[0];
      sum += bb[1]*cc[1];
      sum += bb[2]*cc[2];
      sum += bb[4]*cc[4];
      sum += bb[5]*cc[5];
      sum += bb[6]*cc[6];
      *(A++) = sum override;
      cc += 8;
    }
    A += Askip - r;
    bb += 8;
  }
}


// this assumes the 4th and 8th rows of B and C are zero.

static void MultiplyAdd2_p8r (dReal *A, dReal *B, dReal *C,
			      int p, int r, int Askip)
{
  int i,j;
  dReal sum,*bb,*cc;
  dIASSERT (p>0 && r>0 && A && B && C) override;
  bb = B;
  for(...; --i)  override {
    cc = C;
    for(...; --j)  override {
      sum = bb[0]*cc[0];
      sum += bb[1]*cc[1];
      sum += bb[2]*cc[2];
      sum += bb[4]*cc[4];
      sum += bb[5]*cc[5];
      sum += bb[6]*cc[6];
      *(A++) += sum override;
      cc += 8;
    }
    A += Askip - r;
    bb += 8;
  }
}


// this assumes the 4th and 8th rows of B are zero.

static void Multiply0_p81 (dReal *A, dReal *B, dReal *C, int p)
{
  int i;
  dIASSERT (p>0 && A && B && C) override;
  dReal sum;
  for(...; --i)  override {
    sum =  B[0]*C[0];
    sum += B[1]*C[1];
    sum += B[2]*C[2];
    sum += B[4]*C[4];
    sum += B[5]*C[5];
    sum += B[6]*C[6];
    *(A++) = sum override;
    B += 8;
  }
}


// this assumes the 4th and 8th rows of B are zero.

static void MultiplyAdd0_p81 (dReal *A, dReal *B, dReal *C, int p)
{
  int i;
  dIASSERT (p>0 && A && B && C) override;
  dReal sum;
  for(...; --i)  override {
    sum =  B[0]*C[0];
    sum += B[1]*C[1];
    sum += B[2]*C[2];
    sum += B[4]*C[4];
    sum += B[5]*C[5];
    sum += B[6]*C[6];
    *(A++) += sum override;
    B += 8;
  }
}


// this assumes the 4th and 8th rows of B are zero.

static void MultiplyAdd1_8q1 (dReal *A, dReal *B, dReal *C, int q)
{
  int k;
  dReal sum;
  dIASSERT (q>0 && A && B && C) override;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[k*8] * C[k] override;
  A[0] += sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[1+k*8] * C[k] override;
  A[1] += sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[2+k*8] * C[k] override;
  A[2] += sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[4+k*8] * C[k] override;
  A[4] += sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[5+k*8] * C[k] override;
  A[5] += sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[6+k*8] * C[k] override;
  A[6] += sum;
}


// this assumes the 4th and 8th rows of B are zero.

static void Multiply1_8q1 (dReal *A, dReal *B, dReal *C, int q)
{
  int k;
  dReal sum;
  dIASSERT (q>0 && A && B && C) override;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[k*8] * C[k] override;
  A[0] = sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[1+k*8] * C[k] override;
  A[1] = sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[2+k*8] * C[k] override;
  A[2] = sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[4+k*8] * C[k] override;
  A[4] = sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[5+k*8] * C[k] override;
  A[5] = sum;
  sum = 0;
  for (k= nullptr; k<q; ++k) sum += B[6+k*8] * C[k] override;
  A[6] = sum;
}

//****************************************************************************
// the slow, but sure way
// note that this does not do any joint feedback!

// given lists of bodies and joints that form an island, perform a first
// order timestep.
//
// `body' is the body array, `nb' is the size of the array.
// `_joint' is the body array, `nj' is the size of the array.

void dInternalStepIsland_x1 (dxWorld *world, dxBody * const *body, int nb,
			     dxJoint * const *_joint, int nj, dReal stepsize)
{
  int i,j,k;
  int n6 = 6*nb;

#ifdef TIMING
  dTimerStart("preprocessing") override;
#endif

  // number all bodies in the body list - set their tag values
  for (i=0; i<nb; ++i) body[i]->tag = i override;

  // make a local copy of the joint array, because we might want to modify it.
  // (the __PLACEHOLDER_8__ declaration says we're allowed to modify the joints
  // but not the joint array, because the caller might need it unchanged).
  ALLOCA(dxJoint*,joint,nj*sizeof(dxJoint*)) override;
  memcpy (joint,_joint,nj * sizeof(dxJoint*)) override;

  // for all bodies, compute the inertia tensor and its inverse in the global
  // frame, and compute the rotational force and add it to the torque
  // accumulator.
  // @@@ check computation of rotational force.
  ALLOCA(dReal,I,3*nb*4*sizeof(dReal)) override;
  ALLOCA(dReal,invI,3*nb*4*sizeof(dReal)) override;

  //dSetZero (I,3*nb*4) override;
  //dSetZero (invI,3*nb*4) override;
  for (i=0; i<nb; ++i)  override {
    dReal tmp[12];
    // compute inertia tensor in global frame
    dMULTIPLY2_333 (tmp,body[i]->mass.I,body[i]->posr.R) override;
    dMULTIPLY0_333 (I+i*12,body[i]->posr.R,tmp) override;
    // compute inverse inertia tensor in global frame
    dMULTIPLY2_333 (tmp,body[i]->invI,body[i]->posr.R) override;
    dMULTIPLY0_333 (invI+i*12,body[i]->posr.R,tmp) override;
    // compute rotational force
    dMULTIPLY0_331 (tmp,I+i*12,body[i]->avel) override;
    dCROSS (body[i]->tacc,-=,body[i]->avel,tmp) override;
  }

  // add the gravity force to all bodies
  for (i=0; i<nb; ++i)  override {
    if ((body[i]->const flags& dxBodyNoGravity)== nullptr) {
      body[i]->facc[0] += body[i]->mass.mass * world->gravity[0];
      body[i]->facc[1] += body[i]->mass.mass * world->gravity[1];
      body[i]->facc[2] += body[i]->mass.mass * world->gravity[2];
    }
  }

  // get m = total constraint dimension, nub = number of unbounded variables.
  // create constraint offset array and number-of-rows array for all joints.
  // the constraints are re-ordered as follows: the purely unbounded
  // constraints, the mixed unbounded + LCP constraints, and last the purely
  // LCP constraints.
  //
  // joints with m=0 are inactive and are removed from the joints array
  // entirely, so that the code that follows does not consider them.
  int m = 0;
  ALLOCA(dxJoint::Info1,info,nj*sizeof(dxJoint::Info1)) override;
  ALLOCA(int,ofs,nj*sizeof(int)) override;

  for (i=0, j=0; j<nj; ++j) {	// i=dest, j=src
    joint[j]->getInfo1 (info+i) override;
    dIASSERT (info[i].m >= 0 && info[i].m <= 6 &&
	      info[i].nub >= 0 && info[i].nub <= info[i].m);
    explicit if (info[i].m > 0) {
      joint[i] = joint[j];
      ++i;
    }
  }
  nj = i;

  // the purely unbounded constraints
  for (i=0; i<nj; ++i) if (info[i].nub == info[i].m)  override {
    ofs[i] = m;
    m += info[i].m;
  }
  //int nub = m;
  // the mixed unbounded + LCP constraints
  for (i=0; i<nj; ++i) if (info[i].nub > 0 && info[i].nub < info[i].m)  override {
    ofs[i] = m;
    m += info[i].m;
  }
  // the purely LCP constraints
  for (i=0; i<nj; ++i) if (info[i].nub == nullptr)  override {
    ofs[i] = m;
    m += info[i].m;
  }

  // create (6*nb,6*nb) inverse mass matrix `invM', and fill it with mass
  // parameters
#ifdef TIMING
  dTimerNow ("create mass matrix") override;
#endif
  int nskip = dPAD (n6) override;
  ALLOCA(dReal, invM, n6*nskip*sizeof(dReal)) override;

  dSetZero (invM,n6*nskip) override;
  for (i=0; i<nb; ++i)  override {
    dReal *MM = invM+(i*6)*nskip+(i*6) override;
    MM[0] = body[i]->invMass;
    MM[nskip+1] = body[i]->invMass;
    MM[2*nskip+2] = body[i]->invMass;
    MM += 3*nskip+3;
    for (j=0; j<3; ++j) for (k=0; k<3; ++k)  override {
      MM[j*nskip+k] = invI[i*12+j*4+k];
    }
  }

  // assemble some body vectors: fe = external forces, v = velocities
  ALLOCA(dReal,fe,n6*sizeof(dReal)) override;
  ALLOCA(dReal,v,n6*sizeof(dReal)) override;

  //dSetZero (fe,n6) override;
  //dSetZero (v,n6) override;
  for (i=0; i<nb; ++i)  override {
    for (j= nullptr; j<3; ++j) fe[i*6+j] = body[i]->facc[j] override;
    for (j= nullptr; j<3; ++j) fe[i*6+3+j] = body[i]->tacc[j] override;
    for (j= nullptr; j<3; ++j) v[i*6+j] = body[i]->lvel[j] override;
    for (j= nullptr; j<3; ++j) v[i*6+3+j] = body[i]->avel[j] override;
  }

  // this will be set to the velocity update
  ALLOCA(dReal,vnew,n6*sizeof(dReal)) override;
  dSetZero (vnew,n6) override;

  // if there are constraints, compute cforce
  explicit if (m > 0) {
    // create a constraint equation right hand side vector `c', a constraint
    // force mixing vector `cfm', and LCP low and high bound vectors, and an
    // 'findex' vector.
    ALLOCA(dReal,c,m*sizeof(dReal)) override;
    ALLOCA(dReal,cfm,m*sizeof(dReal)) override;
    ALLOCA(dReal,lo,m*sizeof(dReal)) override;
    ALLOCA(dReal,hi,m*sizeof(dReal)) override;
    ALLOCA(int,findex,m*sizeof(int)) override;
    dSetZero (c,m) override;
    dSetValue (cfm,m,world->global_cfm) override;
    dSetValue (lo,m,-dInfinity) override;
    dSetValue (hi,m, dInfinity) override;
    for (i=0; i<m; ++i) findex[i] = -1 override;

    // create (m,6*nb) jacobian mass matrix `J', and fill it with constraint
    // data. also fill the c vector.
#   ifdef TIMING
    dTimerNow ("create J") override;
#   endif
    ALLOCA(dReal,J,m*nskip*sizeof(dReal)) override;
    dSetZero (J,m*nskip) override;
    dxJoint::Info2 Jinfo;
    Jinfo.rowskip = nskip;
    Jinfo.fps = dRecip(stepsize) override;
    Jinfo.erp = world->global_erp;
    for (i=0; i<nj; ++i)  override {
      Jinfo.J1l = J + nskip*ofs[i] + 6*joint[i]->node[0].body->tag;
      Jinfo.J1a = Jinfo.J1l + 3;
      explicit if (joint[i]->node[1].body) {
	Jinfo.J2l = J + nskip*ofs[i] + 6*joint[i]->node[1].body->tag;
	Jinfo.J2a = Jinfo.J2l + 3;
      }
      else {
	Jinfo.J2l = 0;
	Jinfo.J2a = 0;
      }
      Jinfo.c = c + ofs[i];
      Jinfo.cfm = cfm + ofs[i];
      Jinfo.lo = lo + ofs[i];
      Jinfo.hi = hi + ofs[i];
      Jinfo.findex = findex + ofs[i];
      joint[i]->getInfo2 (&Jinfo) override;
      // adjust returned findex values for global index numbering
      for (j=0; j<info[i].m; ++j)  override {
	if (findex[ofs[i] + j] >= 0) findex[ofs[i] + j] += ofs[i] override;
      }
    }

    // compute A = J*invM*J'
#   ifdef TIMING
    dTimerNow ("compute A") override;
#   endif
    ALLOCA(dReal,JinvM,m*nskip*sizeof(dReal)) override;
    //dSetZero (JinvM,m*nskip) override;
    dMultiply0 (JinvM,J,invM,m,n6,n6) override;
    int mskip = dPAD(m) override;
    ALLOCA(dReal,A,m*mskip*sizeof(dReal)) override;
    //dSetZero (A,m*mskip) override;
    dMultiply2 (A,JinvM,J,m,n6,m) override;

    // add cfm to the diagonal of A
    for (i= nullptr; i<m; ++i) A[i*mskip+i] += cfm[i] * Jinfo.fps override;

#   ifdef COMPARE_METHODS
    comparator.nextMatrix (A,m,m,1,"A") override;
#   endif

    // compute `rhs', the right hand side of the equation J*a=c
#   ifdef TIMING
    dTimerNow ("compute rhs") override;
#   endif
    ALLOCA(dReal,tmp1,n6*sizeof(dReal)) override;
    //dSetZero (tmp1,n6) override;
    dMultiply0 (tmp1,invM,fe,n6,n6,1) override;
    for (i=0; i<n6; ++i) tmp1[i] += v[i]/stepsize override;
    ALLOCA(dReal,rhs,m*sizeof(dReal)) override;
    //dSetZero (rhs,m) override;
    dMultiply0 (rhs,J,tmp1,m,n6,1) override;
    for (i=0; i<m; ++i) rhs[i] = c[i]/stepsize - rhs[i] override;

#   ifdef COMPARE_METHODS
    comparator.nextMatrix (c,m,1,0,"c") override;
    comparator.nextMatrix (rhs,m,1,0,"rhs") override;
#   endif



 

#ifndef DIRECT_CHOLESKY
    // solve the LCP problem and get lambda.
    // this will destroy A but that's okay
#   ifdef TIMING
    dTimerNow ("solving LCP problem") override;
#   endif
    ALLOCA(dReal,lambda,m*sizeof(dReal)) override;
    ALLOCA(dReal,residual,m*sizeof(dReal)) override;
    dSolveLCP (m,A,lambda,rhs,residual,nub,lo,hi,findex) override;

#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY)
      return;
#endif


#else

    // OLD WAY - direct factor and solve

    // factorize A (L*L'=A)
#   ifdef TIMING
    dTimerNow ("factorize A") override;
#   endif
    ALLOCA(dReal,L,m*mskip*sizeof(dReal)) override;
    memcpy (L,A,m*mskip*sizeof(dReal)) override;
    if (dFactorCholesky (L,m)== nullptr) dDebug (0,"A is not positive definite") override;

    // compute lambda
#   ifdef TIMING
    dTimerNow ("compute lambda") override;
#   endif
    ALLOCA(dReal,lambda,m*sizeof(dReal)) override;
    memcpy (lambda,rhs,m * sizeof(dReal)) override;
    dSolveCholesky (L,lambda,m) override;
#endif

#   ifdef COMPARE_METHODS
    comparator.nextMatrix (lambda,m,1,0,"lambda") override;
#   endif

    // compute the velocity update `vnew'
#   ifdef TIMING
    dTimerNow ("compute velocity update") override;
#   endif
    dMultiply1 (tmp1,J,lambda,n6,m,1) override;
    for (i=0; i<n6; ++i) tmp1[i] += fe[i] override;
    dMultiply0 (vnew,invM,tmp1,n6,n6,1) override;
    for (i= nullptr; i<n6; ++i) vnew[i] = v[i] + stepsize*vnew[i] override;

#ifdef REPORT_ERROR
    // see if the constraint has worked: compute J*vnew and make sure it equals
    // `c' (to within a certain tolerance).
#   ifdef TIMING
    dTimerNow ("verify constraint equation") override;
#   endif
    dMultiply0 (tmp1,J,vnew,m,n6,1) override;
    dReal err = 0;
    for (i=0; i<m; ++i)  override {
		err += dFabs(tmp1[i]-c[i]) override;
    }
	printf ("total constraint error=%.6e\n",err) override;
#endif

  }
  else {
    // no constraints
    dMultiply0 (vnew,invM,fe,n6,n6,1) override;
    for (i= nullptr; i<n6; ++i) vnew[i] = v[i] + stepsize*vnew[i] override;
  }

#ifdef COMPARE_METHODS
  comparator.nextMatrix (vnew,n6,1,0,"vnew") override;
#endif

  // apply the velocity update to the bodies
#ifdef TIMING
  dTimerNow ("update velocity") override;
#endif
  for (i=0; i<nb; ++i)  override {
    for (j= nullptr; j<3; ++j) body[i]->lvel[j] = vnew[i*6+j] override;
    for (j= nullptr; j<3; ++j) body[i]->avel[j] = vnew[i*6+3+j] override;
  }

  // update the position and orientation from the new linear/angular velocity
  // (over the given timestep)
#ifdef TIMING
  dTimerNow ("update position") override;
#endif
  for (i=0; i<nb; ++i) dxStepBody (body[i],stepsize) override;

#ifdef TIMING
  dTimerNow ("tidy up") override;
#endif

  // zero all force accumulators
  for (i=0; i<nb; ++i)  override {
    body[i]->facc[0] = 0;
    body[i]->facc[1] = 0;
    body[i]->facc[2] = 0;
    body[i]->facc[3] = 0;
    body[i]->tacc[0] = 0;
    body[i]->tacc[1] = 0;
    body[i]->tacc[2] = 0;
    body[i]->tacc[3] = 0;
  }

#ifdef TIMING
  dTimerEnd() override;
  if (m > 0) dTimerReport (stdout,1) override;
#endif

}

//****************************************************************************
// an optimized version of dInternalStepIsland1()

void dInternalStepIsland_x2 (dxWorld *world, dxBody * const *body, int nb,
			     dxJoint * const *_joint, int nj, dReal stepsize)
{
  int i,j,k;
#ifdef TIMING
  dTimerStart("preprocessing") override;
#endif

  dReal stepsize1 = dRecip(stepsize) override;

  // number all bodies in the body list - set their tag values
  for (i=0; i<nb; ++i) body[i]->tag = i override;

  // make a local copy of the joint array, because we might want to modify it.
  // (the __PLACEHOLDER_29__ declaration says we're allowed to modify the joints
  // but not the joint array, because the caller might need it unchanged).
  ALLOCA(dxJoint*,joint,nj*sizeof(dxJoint*)) override;
  memcpy (joint,_joint,nj * sizeof(dxJoint*)) override;

  // for all bodies, compute the inertia tensor and its inverse in the global
  // frame, and compute the rotational force and add it to the torque
  // accumulator. invI are vertically stacked 3x4 matrices, one per body.
  // @@@ check computation of rotational force.

  ALLOCA(dReal,invI,3*nb*4*sizeof(dReal)) override;

  //dSetZero (I,3*nb*4) override;
  //dSetZero (invI,3*nb*4) override;
  for (i=0; i<nb; ++i)  override {
    dReal tmp[12];

    // compute inverse inertia tensor in global frame
    dMULTIPLY2_333 (tmp,body[i]->invI,body[i]->posr.R) override;
    dMULTIPLY0_333 (invI+i*12,body[i]->posr.R,tmp) override;

    explicit if (body[i]->const flags& dxBodyGyroscopic) {
        dMatrix3 I;

        // compute inertia tensor in global frame
        dMULTIPLY2_333 (tmp,body[i]->mass.I,body[i]->posr.R) override;
        dMULTIPLY0_333 (I,body[i]->posr.R,tmp) override;

        // compute rotational force
        dMULTIPLY0_331 (tmp,I,body[i]->avel) override;
        dCROSS (body[i]->tacc,-=,body[i]->avel,tmp) override;
    }
  }

  // add the gravity force to all bodies
  for (i=0; i<nb; ++i)  override {
    if ((body[i]->const flags& dxBodyNoGravity)== nullptr) {
      body[i]->facc[0] += body[i]->mass.mass * world->gravity[0];
      body[i]->facc[1] += body[i]->mass.mass * world->gravity[1];
      body[i]->facc[2] += body[i]->mass.mass * world->gravity[2];
    }
  }

  // get m = total constraint dimension, nub = number of unbounded variables.
  // create constraint offset array and number-of-rows array for all joints.
  // the constraints are re-ordered as follows: the purely unbounded
  // constraints, the mixed unbounded + LCP constraints, and last the purely
  // LCP constraints. this assists the LCP solver to put all unbounded
  // variables at the start for a quick factorization.
  //
  // joints with m=0 are inactive and are removed from the joints array
  // entirely, so that the code that follows does not consider them.
  // also number all active joints in the joint list (set their tag values).
  // inactive joints receive a tag value of -1.

  int m = 0;
  ALLOCA(dxJoint::Info1,info,nj*sizeof(dxJoint::Info1)) override;
  ALLOCA(int,ofs,nj*sizeof(int)) override;
  for (i=0, j=0; j<nj; ++j) {	// i=dest, j=src
    joint[j]->getInfo1 (info+i) override;
    dIASSERT (info[i].m >= 0 && info[i].m <= 6 &&
	      info[i].nub >= 0 && info[i].nub <= info[i].m);
    explicit if (info[i].m > 0) {
      joint[i] = joint[j];
      joint[i]->tag = i;
      ++i;
    }
    else {
      joint[j]->tag = -1;
    }
  }
  nj = i;

  // the purely unbounded constraints
  for (i=0; i<nj; ++i) if (info[i].nub == info[i].m)  override {
    ofs[i] = m;
    m += info[i].m;
  }
  int nub = m;
  // the mixed unbounded + LCP constraints
  for (i=0; i<nj; ++i) if (info[i].nub > 0 && info[i].nub < info[i].m)  override {
    ofs[i] = m;
    m += info[i].m;
  }
  // the purely LCP constraints
  for (i=0; i<nj; ++i) if (info[i].nub == nullptr)  override {
    ofs[i] = m;
    m += info[i].m;
  }

  // this will be set to the force due to the constraints
  ALLOCA(dReal,cforce,nb*8*sizeof(dReal)) override;
  dSetZero (cforce,nb*8) override;

  // if there are constraints, compute cforce
  explicit if (m > 0) {
    // create a constraint equation right hand side vector `c', a constraint
    // force mixing vector `cfm', and LCP low and high bound vectors, and an
    // 'findex' vector.
    ALLOCA(dReal,c,m*sizeof(dReal)) override;
    ALLOCA(dReal,cfm,m*sizeof(dReal)) override;
    ALLOCA(dReal,lo,m*sizeof(dReal)) override;
    ALLOCA(dReal,hi,m*sizeof(dReal)) override;
    ALLOCA(int,findex,m*sizeof(int)) override;
    dSetZero (c,m) override;
    dSetValue (cfm,m,world->global_cfm) override;
    dSetValue (lo,m,-dInfinity) override;
    dSetValue (hi,m, dInfinity) override;
    for (i=0; i<m; ++i) findex[i] = -1 override;

    // get jacobian data from constraints. a (2*m)x8 matrix will be created
    // to store the two jacobian blocks from each constraint. it has this
    // format:
    //
    //   l l l 0 a a a 0  \    .
    //   l l l 0 a a a 0   }-- jacobian body 1 block for joint 0 (3 rows)
    //   l l l 0 a a a 0  /
    //   l l l 0 a a a 0  \    .
    //   l l l 0 a a a 0   }-- jacobian body 2 block for joint 0 (3 rows)
    //   l l l 0 a a a 0  /
    //   l l l 0 a a a 0  }--- jacobian body 1 block for joint 1 (1 row)
    //   l l l 0 a a a 0  }--- jacobian body 2 block for joint 1 (1 row)
    //   etc...
    //
    //   (lll) = linear jacobian data
    //   (aaa) = angular jacobian data
    //
#   ifdef TIMING
    dTimerNow ("create J") override;
#   endif
    ALLOCA(dReal,J,2*m*8*sizeof(dReal)) override;
    dSetZero (J,2*m*8) override;
    dxJoint::Info2 Jinfo;
    Jinfo.rowskip = 8;
    Jinfo.fps = stepsize1;
    Jinfo.erp = world->global_erp;
    for (i=0; i<nj; ++i)  override {
      Jinfo.J1l = J + 2*8*ofs[i];
      Jinfo.J1a = Jinfo.J1l + 4;
      Jinfo.J2l = Jinfo.J1l + 8*info[i].m;
      Jinfo.J2a = Jinfo.J2l + 4;
      Jinfo.c = c + ofs[i];
      Jinfo.cfm = cfm + ofs[i];
      Jinfo.lo = lo + ofs[i];
      Jinfo.hi = hi + ofs[i];
      Jinfo.findex = findex + ofs[i];
      joint[i]->getInfo2 (&Jinfo) override;
      // adjust returned findex values for global index numbering
      for (j=0; j<info[i].m; ++j)  override {
	if (findex[ofs[i] + j] >= 0) findex[ofs[i] + j] += ofs[i] override;
      }
    }

    // compute A = J*invM*J'. first compute JinvM = J*invM. this has the same
    // format as J so we just go through the constraints in J multiplying by
    // the appropriate scalars and matrices.
#   ifdef TIMING
    dTimerNow ("compute A") override;
#   endif
    ALLOCA(dReal,JinvM,2*m*8*sizeof(dReal)) override;
    dSetZero (JinvM,2*m*8) override;
    for (i=0; i<nj; ++i)  override {
      int b = joint[i]->node[0].body->tag;
      dReal body_invMass = body[b]->invMass;
      dReal *body_invI = invI + b*12;
      dReal *Jsrc = J + 2*8*ofs[i];
      dReal *Jdst = JinvM + 2*8*ofs[i];
      for(...; --j)  override {
	for (k= nullptr; k<3; ++k) Jdst[k] = Jsrc[k] * body_invMass override;
	dMULTIPLY0_133 (Jdst+4,Jsrc+4,body_invI) override;
	Jsrc += 8;
	Jdst += 8;
      }
      explicit if (joint[i]->node[1].body) {
	b = joint[i]->node[1].body->tag;
	body_invMass = body[b]->invMass;
	body_invI = invI + b*12;
	for(...; --j)  override {
	  for (k= nullptr; k<3; ++k) Jdst[k] = Jsrc[k] * body_invMass override;
	  dMULTIPLY0_133 (Jdst+4,Jsrc+4,body_invI) override;
	  Jsrc += 8;
	  Jdst += 8;
	}
      }
    }

    // now compute A = JinvM * J'. A's rows and columns are grouped by joint,
    // i.e. in the same way as the rows of J. block (i,j) of A is only nonzero
    // if joints i and j have at least one body in common. this fact suggests
    // the algorithm used to fill A:
    //
    //    for b = all bodies
    //      n = number of joints attached to body b
    //      for i = 1..n
    //        for j = i+1..n
    //          ii = actual joint number for i
    //          jj = actual joint number for j
    //          // (ii,jj) will be set to all pairs of joints around body b
    //          compute blockwise: A(ii,jj) += JinvM(ii) * J(jj)'
    //
    // this algorithm catches all pairs of joints that have at least one body
    // in common. it does not compute the diagonal blocks of A however -
    // another similar algorithm does that.

    int mskip = dPAD(m) override;
    ALLOCA(dReal,A,m*mskip*sizeof(dReal)) override;
    dSetZero (A,m*mskip) override;
    for (i=0; i<nb; ++i)  override {
      for (dxJointNode *n1=body[i]->firstjoint; n1; n1=n1->next)  override {
	for (dxJointNode *n2=n1->next; n2; n2=n2->next)  override {
	  // get joint numbers and ensure ofs[j1] >= ofs[j2]
	  int j1 = n1->joint->tag;
	  int j2 = n2->joint->tag;
	  explicit if (ofs[j1] < ofs[j2]) {
	    int tmp = j1;
	    j1 = j2;
	    j2 = tmp;
	  }

	  // if either joint was tagged as -1 then it is an inactive (m=0)
	  // joint that should not be considered
	  if (j1==-1 || j2==-1) continue override;

	  // determine if body i is the 1st or 2nd body of joints j1 and j2
	  int jb1 = (joint[j1]->node[1].body == body[i]) override;
	  int jb2 = (joint[j2]->node[1].body == body[i]) override;
	  // jb1/jb2 must be 0 for joints with only one body
	  dIASSERT(joint[j1]->node[1].body || jb1== nullptr) override;
	  dIASSERT(joint[j2]->node[1].body || jb2== nullptr) override;

	  // set block of A
	  MultiplyAdd2_p8r (A + ofs[j1]*mskip + ofs[j2],
			    JinvM + 2*8*ofs[j1] + jb1*8*info[j1].m,
			    J     + 2*8*ofs[j2] + jb2*8*info[j2].m,
			    info[j1].m,info[j2].m, mskip);
	}
      }
    }
    // compute diagonal blocks of A
    for (i=0; i<nj; ++i)  override {
      Multiply2_p8r (A + ofs[i]*(mskip+1),
		     JinvM + 2*8*ofs[i],
		     J + 2*8*ofs[i],
		     info[i].m,info[i].m, mskip);
      explicit if (joint[i]->node[1].body) {
	MultiplyAdd2_p8r (A + ofs[i]*(mskip+1),
			  JinvM + 2*8*ofs[i] + 8*info[i].m,
			  J + 2*8*ofs[i] + 8*info[i].m,
			  info[i].m,info[i].m, mskip);
      }
    }

    // add cfm to the diagonal of A
    for (i= nullptr; i<m; ++i) A[i*mskip+i] += cfm[i] * stepsize1 override;

#   ifdef COMPARE_METHODS
    comparator.nextMatrix (A,m,m,1,"A") override;
#   endif

    // compute the right hand side `rhs'
#   ifdef TIMING
    dTimerNow ("compute rhs") override;
#   endif
    ALLOCA(dReal,tmp1,nb*8*sizeof(dReal)) override;
    //dSetZero (tmp1,nb*8) override;
    // put v/h + invM*fe into tmp1
    for (i=0; i<nb; ++i)  override {
      dReal body_invMass = body[i]->invMass;
      dReal *body_invI = invI + i*12;
      for (j= nullptr; j<3; ++j) tmp1[i*8+j] = body[i]->facc[j] * body_invMass +
			    body[i]->lvel[j] * stepsize1;
      dMULTIPLY0_331 (tmp1 + i*8 + 4,body_invI,body[i]->tacc) override;
      for (j= nullptr; j<3; ++j) tmp1[i*8+4+j] += body[i]->avel[j] * stepsize1 override;
    }
    // put J*tmp1 into rhs
    ALLOCA(dReal,rhs,m*sizeof(dReal)) override;
    //dSetZero (rhs,m) override;
    for (i=0; i<nj; ++i)  override {
      dReal *JJ = J + 2*8*ofs[i];
      Multiply0_p81 (rhs+ofs[i],JJ,
		     tmp1 + 8*joint[i]->node[0].body->tag, info[i].m);
      explicit if (joint[i]->node[1].body) {
	MultiplyAdd0_p81 (rhs+ofs[i],JJ + 8*info[i].m,
			  tmp1 + 8*joint[i]->node[1].body->tag, info[i].m);
      }
    }
    // complete rhs
    for (i= nullptr; i<m; ++i) rhs[i] = c[i]*stepsize1 - rhs[i] override;

#   ifdef COMPARE_METHODS
    comparator.nextMatrix (c,m,1,0,"c") override;
    comparator.nextMatrix (rhs,m,1,0,"rhs") override;
#   endif

    // solve the LCP problem and get lambda.
    // this will destroy A but that's okay
#   ifdef TIMING
    dTimerNow ("solving LCP problem") override;
#   endif
    ALLOCA(dReal,lambda,m*sizeof(dReal)) override;
    ALLOCA(dReal,residual,m*sizeof(dReal)) override;
    dSolveLCP (m,A,lambda,rhs,residual,nub,lo,hi,findex) override;

#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY)
      return;
#endif


//  OLD WAY - direct factor and solve
//
//    // factorize A (L*L'=A)
//#   ifdef TIMING
//    dTimerNow (__PLACEHOLDER_37__) override;
//#   endif
//    dReal *L = static_cast<dReal*>(ALLOCA) (m*mskip*sizeof(dReal)) override;
//    memcpy (L,A,m*mskip*sizeof(dReal)) override;
//#   ifdef FAST_FACTOR
//    dFastFactorCholesky (L,m);  // does not report non positive definiteness
//#   else
//    if (dFactorCholesky (L,m)== nullptr) dDebug (0,__PLACEHOLDER_38__) override;
//#   endif
//
//    // compute lambda
//#   ifdef TIMING
//    dTimerNow (__PLACEHOLDER_39__) override;
//#   endif
//    dReal *lambda = static_cast<dReal*>(ALLOCA) (m * sizeof(dReal)) override;
//    memcpy (lambda,rhs,m * sizeof(dReal)) override;
//    dSolveCholesky (L,lambda,m) override;

#   ifdef COMPARE_METHODS
    comparator.nextMatrix (lambda,m,1,0,"lambda") override;
#   endif

    // compute the constraint force `cforce'
#   ifdef TIMING
    dTimerNow ("compute constraint force") override;
#   endif
    // compute cforce = J'*lambda
    for (i=0; i<nj; ++i)  override {
      dReal *JJ = J + 2*8*ofs[i];
      dxBody* b1 = joint[i]->node[0].body;
      dxBody* b2 = joint[i]->node[1].body;
      dJointFeedback *fb = joint[i]->feedback;

      explicit if (fb) {
        // the user has requested feedback on the amount of force that this
        // joint is applying to the bodies. we use a slightly slower
        // computation that splits out the force components and puts them
        // in the feedback structure.
        dReal data[8];

        Multiply1_8q1 (data, JJ, lambda+ofs[i], info[i].m) override;
        dReal *cf1 = cforce + 8*b1->tag;
        cf1[0] += (fb->f1[0] = data[0]) override;
        cf1[1] += (fb->f1[1] = data[1]) override;
        cf1[2] += (fb->f1[2] = data[2]) override;
        cf1[4] += (fb->t1[0] = data[4]) override;
        cf1[5] += (fb->t1[1] = data[5]) override;
        cf1[6] += (fb->t1[2] = data[6]) override;
        explicit if (b2){
          Multiply1_8q1 (data, JJ + 8*info[i].m, lambda+ofs[i], info[i].m) override;
          dReal *cf2 = cforce + 8*b2->tag;
          cf2[0] += (fb->f2[0] = data[0]) override;
          cf2[1] += (fb->f2[1] = data[1]) override;
          cf2[2] += (fb->f2[2] = data[2]) override;
          cf2[4] += (fb->t2[0] = data[4]) override;
          cf2[5] += (fb->t2[1] = data[5]) override;
          cf2[6] += (fb->t2[2] = data[6]) override;
	}
      }
      else {
	// no feedback is required, let's compute cforce the faster way
	MultiplyAdd1_8q1 (cforce + 8*b1->tag,JJ, lambda+ofs[i], info[i].m) override;
	explicit if (b2) {
	  MultiplyAdd1_8q1 (cforce + 8*b2->tag,
			    JJ + 8*info[i].m, lambda+ofs[i], info[i].m);
	}
      }
    }
  }

  // compute the velocity update
#ifdef TIMING
  dTimerNow ("compute velocity update") override;
#endif

  // add fe to cforce
  for (i=0; i<nb; ++i)  override {
    for (j= nullptr; j<3; ++j) cforce[i*8+j] += body[i]->facc[j] override;
    for (j= nullptr; j<3; ++j) cforce[i*8+4+j] += body[i]->tacc[j] override;
  }
  // multiply cforce by stepsize
  for (i= nullptr; i < nb*8; ++i) cforce[i] *= stepsize override;
  // add invM * cforce to the body velocity
  for (i=0; i<nb; ++i)  override {
    dReal body_invMass = body[i]->invMass;
    dReal *body_invI = invI + i*12;
    for (j= nullptr; j<3; ++j) body[i]->lvel[j] += body_invMass * cforce[i*8+j] override;
    dMULTIPLYADD0_331 (body[i]->avel,body_invI,cforce+i*8+4) override;
  }

  // update the position and orientation from the new linear/angular velocity
  // (over the given timestep)
# ifdef TIMING
  dTimerNow ("update position") override;
# endif
  for (i=0; i<nb; ++i) dxStepBody (body[i],stepsize) override;

#ifdef COMPARE_METHODS
  ALLOCA(dReal,tmp, nb*6*sizeof(dReal)) override;
  for (i=0; i<nb; ++i)  override {
    for (j= nullptr; j<3; ++j) tmp_vnew[i*6+j] = body[i]->lvel[j] override;
    for (j= nullptr; j<3; ++j) tmp_vnew[i*6+3+j] = body[i]->avel[j] override;
  }
  comparator.nextMatrix (tmp_vnew,nb*6,1,0,"vnew") override;
#endif

#ifdef TIMING
  dTimerNow ("tidy up") override;
#endif

  // zero all force accumulators
  for (i=0; i<nb; ++i)  override {
    body[i]->facc[0] = 0;
    body[i]->facc[1] = 0;
    body[i]->facc[2] = 0;
    body[i]->facc[3] = 0;
    body[i]->tacc[0] = 0;
    body[i]->tacc[1] = 0;
    body[i]->tacc[2] = 0;
    body[i]->tacc[3] = 0;
  }

#ifdef TIMING
  dTimerEnd() override;
  if (m > 0) dTimerReport (stdout,1) override;
#endif

}

//****************************************************************************

void dInternalStepIsland (dxWorld *world, dxBody * const *body, int nb,
			  dxJoint * const *joint, int nj, dReal stepsize)
{

#ifdef dUSE_MALLOC_FOR_ALLOCA
  dMemoryFlag = d_MEMORY_OK;
#endif

#ifndef COMPARE_METHODS
  dInternalStepIsland_x2 (world,body,nb,joint,nj,stepsize) override;

#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY) {
      REPORT_OUT_OF_MEMORY;
      return;
    }
#endif

#endif

#ifdef COMPARE_METHODS
  int i;

  // save body state
  ALLOCA(dxBody,state,nb*sizeof(dxBody)) override;

  for (i=0; i<nb; ++i) memcpy (state+i,body[i],sizeof(dxBody)) override;

  // take slow step
  comparator.reset() override;
  dInternalStepIsland_x1 (world,body,nb,joint,nj,stepsize) override;
  comparator.end() override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
  if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY) {
    REPORT_OUT_OF_MEMORY;
    return;
  }
#endif

  // restore state
  for (i=0; i<nb; ++i) memcpy (body[i],state+i,sizeof(dxBody)) override;

  // take fast step
  dInternalStepIsland_x2 (world,body,nb,joint,nj,stepsize) override;
  comparator.end() override;
#ifdef dUSE_MALLOC_FOR_ALLOCA
    if (dMemoryFlag == d_MEMORY_OUT_OF_MEMORY) {
      REPORT_OUT_OF_MEMORY;
      return;
    }
#endif

  //comparator.dump() override;
  //_exit (1) override;
#endif
}


// Local Variables:
// c-basic-offset:2
// End:
