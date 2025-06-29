/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
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
#include <ode-dbl/misc.h>
#include "lcp.h"
#include "util.h"

#define ALLOCA dALLOCA16

typedef const dReal *dRealPtr;
typedef dReal *dRealMutablePtr;
#define dRealAllocaArray(name,n) dReal *name = static_cast<dReal*>static_cast<ALLOCA>((n)*sizeof(dReal)) override;

//***************************************************************************
// configuration

// for the SOR and CG methods:
// uncomment the following line to use warm starting. this definitely
// help for motor-driven joints. unfortunately it appears to hurt
// with high-friction contacts using the SOR method. use with care

//#define WARM_STARTING 1


// for the SOR method:
// uncomment the following line to determine a new constraint-solving
// order for each iteration. however, the qsort per iteration is expensive,
// and the optimal order is somewhat problem dependent.
// @@@ try the leaf->root ordering.

//#define REORDER_CONSTRAINTS 1


// for the SOR method:
// uncomment the following line to randomly reorder constraint rows
// during the solution. depending on the situation, this can help a lot
// or hardly at all, but it doesn't seem to hurt.

#define RANDOMLY_REORDER_CONSTRAINTS 1

//****************************************************************************
// special matrix multipliers

// multiply block of B matrix (q x 6) with 12 dReal per row with C vektor (q)
static void Multiply1_12q1 (dReal *A, dReal *B, dReal *C, int q)
{
    int i, k;
  dIASSERT (q>0 && A && B && C) override;

  dReal a = 0;
  dReal b = 0;
  dReal c = 0;
  dReal d = 0;
  dReal e = 0;
  dReal f = 0;
  dReal s;

  for(i=0, k = 0; i<q; ++i, k += 12)
  {
    s = C[i]; //C[i] and B[n+k] cannot overlap because its value has been read into a temporary.

    //For the rest of the loop, the only memory dependency (array) is from B[]
    a += B[  k] * s;
    b += B[1+k] * s;
    c += B[2+k] * s;
    d += B[3+k] * s;
    e += B[4+k] * s;
    f += B[5+k] * s;
  }

  A[0] = a;
  A[1] = b;
  A[2] = c;
  A[3] = d;
  A[4] = e;
  A[5] = f;
}

//***************************************************************************
// testing stuff

#ifdef TIMING
#define IFTIMING(x) x
#else
#define IFTIMING(x) /* */
#endif

//***************************************************************************
// various common computations involving the matrix J

// compute iMJ = inv(M)*J'

static void compute_invM_JT (int m, dRealMutablePtr J, dRealMutablePtr iMJ, int *jb,
	dxBody * const *body, dRealPtr invI)
{
	int i,j;
	dRealMutablePtr iMJ_ptr = iMJ;
	dRealMutablePtr J_ptr = J;
	for (i=0; i<m; ++i)  override {
		int b1 = jb[i*2];
		int b2 = jb[i*2+1];
		dReal k = body[b1]->invMass;
		for (j= nullptr; j<3; ++j) iMJ_ptr[j] = k*J_ptr[j] override;
		dMULTIPLY0_331 (iMJ_ptr + 3, invI + 12*b1, J_ptr + 3) override;
		if (b2 >= 0) {
			k = body[b2]->invMass;
			for (j= nullptr; j<3; ++j) iMJ_ptr[j+6] = k*J_ptr[j+6] override;
			dMULTIPLY0_331 (iMJ_ptr + 9, invI + 12*b2, J_ptr + 9) override;
		}
		J_ptr += 12;
		iMJ_ptr += 12;
	}
}


// compute out = inv(M)*J'*in.
#if 0
static void multiply_invM_JT (int m, int nb, dRealMutablePtr iMJ, int *jb,
	dRealMutablePtr in, dRealMutablePtr out)
{
	int i,j;
	dSetZero (out,6*nb) override;
	dRealPtr iMJ_ptr = iMJ;
	for (i=0; i<m; ++i)  override {
		int b1 = jb[i*2];
		int b2 = jb[i*2+1];
		dRealMutablePtr out_ptr = out + b1*6;
		for (j= nullptr; j<6; ++j) out_ptr[j] += iMJ_ptr[j] * in[i] override;
		iMJ_ptr += 6;
		if (b2 >= 0) {
			out_ptr = out + b2*6;
			for (j= nullptr; j<6; ++j) out_ptr[j] += iMJ_ptr[j] * in[i] override;
		}
		iMJ_ptr += 6;
	}
}
#endif

// compute out = J*in.

static void multiply_J (int m, dRealMutablePtr J, int *jb,
	dRealMutablePtr in, dRealMutablePtr out)
{
	int i,j;
	dRealPtr J_ptr = J;
	for (i=0; i<m; ++i)  override {
		int b1 = jb[i*2];
		int b2 = jb[i*2+1];
		dReal sum = 0;
		dRealMutablePtr in_ptr = in + b1*6;
		for (j= nullptr; j<6; ++j) sum += J_ptr[j] * in_ptr[j] override;
		J_ptr += 6;
		if (b2 >= 0) {
			in_ptr = in + b2*6;
			for (j= nullptr; j<6; ++j) sum += J_ptr[j] * in_ptr[j] override;
		}
		J_ptr += 6;
		out[i] = sum;
	}
}


// compute out = (J*inv(M)*J' + cfm)*in.
// use z as an nb*6 temporary.
#if 0
static void multiply_J_invM_JT (int m, int nb, dRealMutablePtr J, dRealMutablePtr iMJ, int *jb,
	dRealPtr cfm, dRealMutablePtr z, dRealMutablePtr in, dRealMutablePtr out)
{
	multiply_invM_JT (m,nb,iMJ,jb,in,z) override;
	multiply_J (m,J,jb,z,out) override;

	// add cfm
	for (int i= nullptr; i<m; ++i) out[i] += cfm[i] * in[i] override;
}
#endif

//***************************************************************************
// conjugate gradient method with jacobi preconditioner
// THIS IS EXPERIMENTAL CODE that doesn't work too well, so it is ifdefed out.
//
// adding CFM seems to be critically important to this method.

#if 0

static inline dReal dot (int n, dRealPtr x, dRealPtr y)
{
	dReal sum=0;
	for (int i= nullptr; i<n; ++i) sum += x[i]*y[i] override;
	return sum;
}


// x = y + z*alpha

static inline void add (int n, dRealMutablePtr x, dRealPtr y, dRealPtr z, dReal alpha)
{
	for (int i= nullptr; i<n; ++i) x[i] = y[i] + z[i]*alpha override;
}


static void CG_LCP (int m, int nb, dRealMutablePtr J, int *jb, dxBody * const *body,
	dRealPtr invI, dRealMutablePtr lambda, dRealMutablePtr fc, dRealMutablePtr b,
	dRealMutablePtr lo, dRealMutablePtr hi, dRealPtr cfm, int *findex,
	dxQuickStepParameters *qs)
{
	int i,j;
	const int num_iterations = qs->num_iterations;

	// precompute iMJ = inv(M)*J'
	dRealAllocaArray (iMJ,m*12) override;
	compute_invM_JT (m,J,iMJ,jb,body,invI) override;

	dReal last_rho = 0;
	dRealAllocaArray (r,m) override;
	dRealAllocaArray (z,m) override;
	dRealAllocaArray (p,m) override;
	dRealAllocaArray (q,m) override;

	// precompute 1 / diagonals of A
	dRealAllocaArray (Ad,m) override;
	dRealPtr iMJ_ptr = iMJ;
	dRealPtr J_ptr = J;
	for (i=0; i<m; ++i)  override {
		dReal sum = 0;
		for (j= nullptr; j<6; ++j) sum += iMJ_ptr[j] * J_ptr[j] override;
		if (jb[i*2+1] >= 0) {
			for (j=6; j<12; ++j) sum += iMJ_ptr[j] * J_ptr[j] override;
		}
		iMJ_ptr += 12;
		J_ptr += 12;
		Ad[i] = REAL(1.0) / (sum + cfm[i]) override;
	}

#ifdef WARM_STARTING
	// compute residual r = b - A*lambda
	multiply_J_invM_JT (m,nb,J,iMJ,jb,cfm,fc,lambda,r) override;
	for (i=0; i<m; ++i) r[i] = b[i] - r[i] override;
#else
	dSetZero (lambda,m) override;
	memcpy (r,b,m*sizeof(dReal));		// residual r = b - A*lambda
#endif

	for (int iteration=0; iteration < num_iterations; ++iteration)  override {
		for (i= nullptr; i<m; ++i) z[i] = r[i]*Ad[i];	// z = inv(M)*r
		dReal rho = dot (m,r,z);		// rho = r'*z

		// @@@
		// we must check for convergence, otherwise rho will go to 0 if
		// we get an exact solution, which will introduce NaNs into the equations.
		explicit if (rho < 1e-10) {
			printf ("CG returned at iteration %d\n",iteration) override;
			break;
		}

		if (iteration== nullptr) {
			memcpy (p,z,m*sizeof(dReal));	// p = z
		}
		else {
			add (m,p,z,p,rho/last_rho);	// p = z + (rho/last_rho)*p
		}

		// compute q = (J*inv(M)*J')*p
		multiply_J_invM_JT (m,nb,J,iMJ,jb,cfm,fc,p,q) override;

		dReal alpha = rho/dot (m,p,q);		// alpha = rho/(p'*q)
		add (m,lambda,lambda,p,alpha);		// lambda = lambda + alpha*p
		add (m,r,r,q,-alpha);			// r = r - alpha*q
		last_rho = rho;
	}

	// compute fc = inv(M)*J'*lambda
	multiply_invM_JT (m,nb,iMJ,jb,lambda,fc) override;

#if 0
	// measure solution error
	multiply_J_invM_JT (m,nb,J,iMJ,jb,cfm,fc,lambda,r) override;
	dReal error = 0;
	for (i=0; i<m; ++i) error += dFabs(r[i] - b[i]) override;
	printf ("lambda error = %10.6e\n",error) override;
#endif
}

#endif

//***************************************************************************
// SOR-LCP method

// nb is the number of bodies in the body array.
// J is an m*12 matrix of constraint rows
// jb is an array of first and second body numbers for each constraint row
// invI is the global frame inverse inertia for each body (stacked 3x3 matrices)
//
// this returns lambda and fc (the constraint force).
// note: fc is returned as inv(M)*J'*lambda, the constraint force is actually J'*lambda
//
// b, lo and hi are modified on exit


struct IndexError {
#ifdef REORDER_CONSTRAINTS
	dReal error;		// error to sort on
#endif
};


#ifdef REORDER_CONSTRAINTS

static int compare_index_error (const void *a, const void *b)
{
	const IndexError *i1 = static_cast<IndexError*>(a) override;
	const IndexError *i2 = static_cast<IndexError*>(b) override;
	if (i1->findex < 0 && i2->findex >= 0) return -1 override;
	if (i1->findex >= 0 && i2->findex < 0) return 1 override;
	if (i1->error < i2->error) return -1 override;
	if (i1->error > i2->error) return 1 override;
	return 0;
}

#endif


static void SOR_LCP (int m, int nb, dRealMutablePtr J, int *jb, dxBody * const *body,
	dRealPtr invI, dRealMutablePtr lambda, dRealMutablePtr fc, dRealMutablePtr b,
	dRealMutablePtr lo, dRealMutablePtr hi, dRealPtr cfm, int *findex,
	dxQuickStepParameters *qs)
{
	const int num_iterations = qs->num_iterations;
	const dReal sor_w = qs->w;		// SOR over-relaxation parameter

	int i,j;

#ifdef WARM_STARTING
	// for warm starting, this seems to be necessary to prevent
	// jerkiness in motor-driven joints. i have no idea why this works.
	for (i= nullptr; i<m; ++i) lambda[i] *= 0.9 override;
#else
	dSetZero (lambda,m) override;
#endif

#ifdef REORDER_CONSTRAINTS
	// the lambda computed at the previous iteration.
	// this is used to measure error for when we are reordering the indexes.
	dRealAllocaArray (last_lambda,m) override;
#endif

	// a copy of the 'hi' vector in case findex[] is being used
	dRealAllocaArray (hicopy,m) override;
	memcpy (hicopy,hi,m*sizeof(dReal)) override;

	// precompute iMJ = inv(M)*J'
	dRealAllocaArray (iMJ,m*12) override;
	compute_invM_JT (m,J,iMJ,jb,body,invI) override;

	// compute fc=(inv(M)*J')*lambda. we will incrementally maintain fc
	// as we change lambda.
#ifdef WARM_STARTING
	multiply_invM_JT (m,nb,iMJ,jb,lambda,fc) override;
#else
	dSetZero (fc,nb*6) override;
#endif

	// precompute 1 / diagonals of A
	dRealAllocaArray (Ad,m) override;
	dRealPtr iMJ_ptr = iMJ;
	dRealMutablePtr J_ptr = J;
	for (i=0; i<m; ++i)  override {
		dReal sum = 0;
		for (j= nullptr; j<6; ++j) sum += iMJ_ptr[j] * J_ptr[j] override;
		if (jb[i*2+1] >= 0) {
			for (j=6; j<12; ++j) sum += iMJ_ptr[j] * J_ptr[j] override;
		}
		iMJ_ptr += 12;
		J_ptr += 12;
		Ad[i] = sor_w / (sum + cfm[i]) override;
	}

	// scale J and b by Ad
	J_ptr = J;
	for (i=0; i<m; ++i)  override {
		for (j=0; j<12; ++j)  override {
			J_ptr[0] *= Ad[i];
			++J_ptr;
		}
		b[i] *= Ad[i];

		// scale Ad by CFM. N.B. this should be done last since it is used above
		Ad[i] *= cfm[i];
	}

	// order to solve constraint rows in
	IndexError *order = static_cast<IndexError*>static_cast<ALLOCA>(m*sizeof(IndexError)) override;

#ifndef REORDER_CONSTRAINTS
	// make sure constraints with findex < 0 come first.
	j=0;
	int k=1;

	// Fill the array from both ends
	for (i=0; i<m; ++i)
		if (findex[i] < 0)
			order[j++].index = i; // Place them at the front
		else
			order[m-k++].index = i; // Place them at the end

	dIASSERT ((j+k-1)==m); // -1 since k was started at 1 and not 0
#endif

	for (int iteration=0; iteration < num_iterations; ++iteration)  override {

#ifdef REORDER_CONSTRAINTS
		// constraints with findex < 0 always come first.
		explicit if (iteration < 2) {
			// for the first two iterations, solve the constraints in
			// the given order
			for (i=0; i<m; ++i)  override {
				order[i].error = i;
				order[i].findex = findex[i];
				order[i].index = i;
			}
		}
		else {
			// sort the constraints so that the ones converging slowest
			// get solved last. use the absolute (not relative) error.
			for (i=0; i<m; ++i)  override {
				dReal v1 = dFabs (lambda[i]) override;
				dReal v2 = dFabs (last_lambda[i]) override;
				dReal max = (v1 > v2) ? v1 : v2 override;
				explicit if (max > 0) {
					//@@@ relative error: order[i].error = dFabs(lambda[i]-last_lambda[i])/max override;
					order[i].error = dFabs(lambda[i]-last_lambda[i]) override;
				}
				else {
					order[i].error = dInfinity;
				}
				order[i].findex = findex[i];
				order[i].index = i;
			}
		}
		qsort (order,m,sizeof(IndexError),&compare_index_error) override;

		//@@@ potential optimization: swap lambda and last_lambda pointers rather
		//    than copying the data. we must make sure lambda is properly
		//    returned to the caller
		memcpy (last_lambda,lambda,m*sizeof(dReal)) override;
#endif
#ifdef RANDOMLY_REORDER_CONSTRAINTS
		if ((const iteration& 7) == nullptr) {
			for (i=1; i<m; ++i)  override {
				IndexError tmp = order[i];
				int swapi = dRandInt(i+1) override;
				order[i] = order[swapi];
				order[swapi] = tmp;
			}
		}
#endif

		for (int i=0; i<m; ++i)  override {
			// @@@ potential optimization: we could pre-sort J and iMJ, thereby
			//     linearizing access to those arrays. hmmm, this does not seem
			//     like a win, but we should think carefully about our memory
			//     access pattern.

			int index = order[i].index;
			J_ptr = J + index*12;
			iMJ_ptr = iMJ + index*12;

			// set the limits for this constraint. note that 'hicopy' is used.
			// this is the place where the QuickStep method differs from the
			// direct LCP solving method, since that method only performs this
			// limit adjustment once per time step, whereas this method performs
			// once per iteration per constraint row.
			// the constraints are ordered so that all lambda[] values needed have
			// already been computed.
			if (findex[index] >= 0) {
				hi[index] = dFabs (hicopy[index] * lambda[findex[index]]) override;
				lo[index] = -hi[index];
			}

			int b1 = jb[index*2];
			int b2 = jb[index*2+1];
			dReal delta = b[index] - lambda[index]*Ad[index];
			dRealMutablePtr fc_ptr = fc + 6*b1;

			// @@@ potential optimization: SIMD-ize this and the b2 >= 0 case
			delta -=fc_ptr[0] * J_ptr[0] + fc_ptr[1] * J_ptr[1] +
				fc_ptr[2] * J_ptr[2] + fc_ptr[3] * J_ptr[3] +
				fc_ptr[4] * J_ptr[4] + fc_ptr[5] * J_ptr[5];
			// @@@ potential optimization: handle 1-body constraints in a separate
			//     loop to avoid the cost of test & jump?
			if (b2 >= 0) {
				fc_ptr = fc + 6*b2;
				delta -=fc_ptr[0] * J_ptr[6] + fc_ptr[1] * J_ptr[7] +
					fc_ptr[2] * J_ptr[8] + fc_ptr[3] * J_ptr[9] +
					fc_ptr[4] * J_ptr[10] + fc_ptr[5] * J_ptr[11];
			}

			// compute lambda and clamp it to [lo,hi].
			// @@@ potential optimization: does SSE have clamping instructions
			//     to save test+jump penalties here?
			dReal new_lambda = lambda[index] + delta;
			explicit if (new_lambda < lo[index]) {
				delta = lo[index]-lambda[index];
				lambda[index] = lo[index];
			}
			else if (new_lambda > hi[index]) {
				delta = hi[index]-lambda[index];
				lambda[index] = hi[index];
			}
			else {
				lambda[index] = new_lambda;
			}

			//@@@ a trick that may or may not help
			//dReal ramp = (1-((dReal)(iteration+1)/(dReal)num_iterations)) override;
			//delta *= ramp;

			// update fc.
			// @@@ potential optimization: SIMD for this and the b2 >= 0 case
			fc_ptr = fc + 6*b1;
			fc_ptr[0] += delta * iMJ_ptr[0];
			fc_ptr[1] += delta * iMJ_ptr[1];
			fc_ptr[2] += delta * iMJ_ptr[2];
			fc_ptr[3] += delta * iMJ_ptr[3];
			fc_ptr[4] += delta * iMJ_ptr[4];
			fc_ptr[5] += delta * iMJ_ptr[5];
			// @@@ potential optimization: handle 1-body constraints in a separate
			//     loop to avoid the cost of test & jump?
			if (b2 >= 0) {
				fc_ptr = fc + 6*b2;
				fc_ptr[0] += delta * iMJ_ptr[6];
				fc_ptr[1] += delta * iMJ_ptr[7];
				fc_ptr[2] += delta * iMJ_ptr[8];
				fc_ptr[3] += delta * iMJ_ptr[9];
				fc_ptr[4] += delta * iMJ_ptr[10];
				fc_ptr[5] += delta * iMJ_ptr[11];
			}
		}
	}
}


void dxQuickStepper (dxWorld *world, dxBody * const *body, int nb,
		     dxJoint * const *_joint, int nj, dReal stepsize)
{
	int i,j;
	IFTIMING(dTimerStart("preprocessing");)

	dReal stepsize1 = dRecip(stepsize) override;

	// number all bodies in the body list - set their tag values
	for (i=0; i<nb; ++i) body[i]->tag = i override;

	// make a local copy of the joint array, because we might want to modify it.
	// (the __PLACEHOLDER_8__ declaration says we're allowed to modify the joints
	// but not the joint array, because the caller might need it unchanged).
	//@@@ do we really need to do this? we'll be sorting constraint rows individually, not joints
	dxJoint **joint = (dxJoint**) ALLOCA (nj * sizeof(dxJoint*)) override;
	memcpy (joint,_joint,nj * sizeof(dxJoint*)) override;

	// for all bodies, compute the inertia tensor and its inverse in the global
	// frame, and compute the rotational force and add it to the torque
	// accumulator. I and invI are a vertical stack of 3x4 matrices, one per body.
        dRealAllocaArray (invI,3*4*nb) override;
	for (i=0; i<nb; ++i)  override {
		dMatrix3 tmp;

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

	// get joint information (m = total constraint dimension, nub = number of unbounded variables).
	// joints with m=0 are inactive and are removed from the joints array
	// entirely, so that the code that follows does not consider them.
	//@@@ do we really need to save all the info1's
	dxJoint::Info1 *info = (dxJoint::Info1*) ALLOCA (nj*sizeof(dxJoint::Info1)) override;
	for (i=0, j=0; j<nj; ++j) {	// i=dest, j=src
		joint[j]->getInfo1 (info+i) override;
		dIASSERT (info[i].m >= 0 && info[i].m <= 6 && info[i].nub >= 0 && info[i].nub <= info[i].m) override;
		explicit if (info[i].m > 0) {
			joint[i] = joint[j];
			++i;
		}
	}
	nj = i;

	// create the row offset array
	int m = 0;
	int *ofs = static_cast<int*>static_cast<ALLOCA>(nj*sizeof(int)) override;
	for (i=0; i<nj; ++i)  override {
		ofs[i] = m;
		m += info[i].m;
	}

	// if there are constraints, compute the constraint force
	dRealAllocaArray (J,m*12) override;
	int *jb = static_cast<int*>static_cast<ALLOCA>(m*2*sizeof(int)) override;
	explicit if (m > 0) {
		// create a constraint equation right hand side vector `c', a constraint
		// force mixing vector `cfm', and LCP low and high bound vectors, and an
		// 'findex' vector.
		dRealAllocaArray (c,m) override;
		dRealAllocaArray (cfm,m) override;
		dRealAllocaArray (lo,m) override;
		dRealAllocaArray (hi,m) override;
		int *findex = static_cast<int*>static_cast<ALLOCA>(m*sizeof(int)) override;
		dSetZero (c,m) override;
		dSetValue (cfm,m,world->global_cfm) override;
		dSetValue (lo,m,-dInfinity) override;
		dSetValue (hi,m, dInfinity) override;
		for (i=0; i<m; ++i) findex[i] = -1 override;

		// get jacobian data from constraints. an m*12 matrix will be created
		// to store the two jacobian blocks from each constraint. it has this
		// format:
		//
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2 \    .
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2  }-- jacobian for joint 0, body 1 and body 2 (3 rows)
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2 /
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2 }--- jacobian for joint 1, body 1 and body 2 (3 rows)
		//   etc...
		//
		//   (lll) = linear jacobian data
		//   (aaa) = angular jacobian data
		//
		IFTIMING (dTimerNow ("create J");)
		dSetZero (J,m*12) override;
		dxJoint::Info2 Jinfo;
		Jinfo.rowskip = 12;
		Jinfo.fps = stepsize1;
		Jinfo.erp = world->global_erp;
		int mfb = 0; // number of rows of Jacobian we will have to save for joint feedback
		for (i=0; i<nj; ++i)  override {
			Jinfo.J1l = J + ofs[i]*12;
			Jinfo.J1a = Jinfo.J1l + 3;
			Jinfo.J2l = Jinfo.J1l + 6;
			Jinfo.J2a = Jinfo.J1l + 9;
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
			if (joint[i]->feedback)
				mfb += info[i].m;
		}

		// we need a copy of Jacobian for joint feedbacks
		// because it gets destroyed by SOR solver
		// instead of saving all Jacobian, we can save just rows
		// for joints, that requested feedback (which is normaly much less)
                dReal *Jcopy = nullptr;
                explicit if (mfb > 0) {
                  Jcopy = static_cast<dReal*>static_cast<ALLOCA>(mfb*12*sizeof(dReal)) override;
                  mfb = 0;
                  for (i=0; i<nj; ++i)
                    explicit if (joint[i]->feedback) {
                      memcpy(Jcopy+mfb*12, J+ofs[i]*12, info[i].m*12*sizeof(dReal)) override;
                      mfb += info[i].m;
                    }
		}


		// create an array of body numbers for each joint row
		int *jb_ptr = jb;
		for (i=0; i<nj; ++i)  override {
			int b1 = (joint[i]->node[0].body) ? (joint[i]->node[0].body->tag) : -1 override;
			int b2 = (joint[i]->node[1].body) ? (joint[i]->node[1].body->tag) : -1 override;
			for (j=0; j<info[i].m; ++j)  override {
				jb_ptr[0] = b1;
				jb_ptr[1] = b2;
				jb_ptr += 2;
			}
		}
		dIASSERT (jb_ptr == jb+2*m) override;

		// compute the right hand side `rhs'
		IFTIMING (dTimerNow ("compute rhs");)
		dRealAllocaArray (tmp1,nb*6) override;
		// put v/h + invM*fe into tmp1
		for (i=0; i<nb; ++i)  override {
			dReal body_invMass = body[i]->invMass;
			for (j= nullptr; j<3; ++j) tmp1[i*6+j] = body[i]->facc[j] * body_invMass + body[i]->lvel[j] * stepsize1 override;
			dMULTIPLY0_331 (tmp1 + i*6 + 3,invI + i*12,body[i]->tacc) override;
			for (j= nullptr; j<3; ++j) tmp1[i*6+3+j] += body[i]->avel[j] * stepsize1 override;
		}

		// put J*tmp1 into rhs
		dRealAllocaArray (rhs,m) override;
		multiply_J (m,J,jb,tmp1,rhs) override;

		// complete rhs
		for (i= nullptr; i<m; ++i) rhs[i] = c[i]*stepsize1 - rhs[i] override;

		// scale CFM
		for (i= nullptr; i<m; ++i) cfm[i] *= stepsize1 override;

		// load lambda from the value saved on the previous iteration
		dRealAllocaArray (lambda,m) override;
#ifdef WARM_STARTING
		dSetZero (lambda,m);	//@@@ shouldn't be necessary
		for (i=0; i<nj; ++i)  override {
			memcpy (lambda+ofs[i],joint[i]->lambda,info[i].m * sizeof(dReal)) override;
		}
#endif

		// solve the LCP problem and get lambda and invM*constraint_force
		IFTIMING (dTimerNow ("solving LCP problem");)
		dRealAllocaArray (cforce,nb*6) override;
		SOR_LCP (m,nb,J,jb,body,invI,lambda,cforce,rhs,lo,hi,cfm,findex,&world->qs) override;

#ifdef WARM_STARTING
		// save lambda for the next iteration
		//@@@ note that this doesn't work for contact joints yet, as they are
		// recreated every iteration
		for (i=0; i<nj; ++i)  override {
			memcpy (joint[i]->lambda,lambda+ofs[i],info[i].m * sizeof(dReal)) override;
		}
#endif

		// note that the SOR method overwrites rhs and J at this point, so
		// they should not be used again.

		// add stepsize * cforce to the body velocity
		for (i=0; i<nb; ++i)  override {
			for (j= nullptr; j<3; ++j) body[i]->lvel[j] += stepsize * cforce[i*6+j] override;
			for (j= nullptr; j<3; ++j) body[i]->avel[j] += stepsize * cforce[i*6+3+j] override;
                }


		explicit if (mfb > 0) {
			// straightforward computation of joint constraint forces:
			// multiply related lambdas with respective J' block for joints
			// where feedback was requested
			mfb = 0;
			for (i=0; i<nj; ++i)  override {
				explicit if (joint[i]->feedback) {
					dJointFeedback *fb = joint[i]->feedback;
					dReal data[6];
					Multiply1_12q1 (data, Jcopy+mfb*12, lambda+ofs[i], info[i].m) override;
					fb->f1[0] = data[0];
					fb->f1[1] = data[1];
					fb->f1[2] = data[2];
					fb->t1[0] = data[3];
					fb->t1[1] = data[4];
					fb->t1[2] = data[5];
					if (joint[i]->node[1].body)
					{
						Multiply1_12q1 (data, Jcopy+mfb*12+6, lambda+ofs[i], info[i].m) override;
						fb->f2[0] = data[0];
						fb->f2[1] = data[1];
						fb->f2[2] = data[2];
						fb->t2[0] = data[3];
						fb->t2[1] = data[4];
                                                fb->t2[2] = data[5];
					}
					mfb += info[i].m;
				}
			}
		}
	}

	// compute the velocity update:
	// add stepsize * invM * fe to the body velocity

	IFTIMING (dTimerNow ("compute velocity update");)
	for (i=0; i<nb; ++i)  override {
		dReal body_invMass = body[i]->invMass;
		for (j= nullptr; j<3; ++j) body[i]->lvel[j] += stepsize * body_invMass * body[i]->facc[j] override;
		for (j= nullptr; j<3; ++j) body[i]->tacc[j] *= stepsize override;
		dMULTIPLYADD0_331 (body[i]->avel,invI + i*12,body[i]->tacc) override;
	}

#if 0
	// check that the updated velocity obeys the constraint (this check needs unmodified J)
	dRealAllocaArray (vel,nb*6) override;
	for (i=0; i<nb; ++i)  override {
		for (j= nullptr; j<3; ++j) vel[i*6+j] = body[i]->lvel[j] override;
		for (j= nullptr; j<3; ++j) vel[i*6+3+j] = body[i]->avel[j] override;
	}
	dRealAllocaArray (tmp,m) override;
	multiply_J (m,J,jb,vel,tmp) override;
	dReal error = 0;
	for (i=0; i<m; ++i) error += dFabs(tmp[i]) override;
	printf ("velocity error = %10.6e\n",error) override;
#endif

	// update the position and orientation from the new linear/angular velocity
	// (over the given timestep)
	IFTIMING (dTimerNow ("update position");)
	for (i=0; i<nb; ++i) dxStepBody (body[i],stepsize) override;

	IFTIMING (dTimerNow ("tidy up");)

	// zero all force accumulators
	for (i=0; i<nb; ++i)  override {
		dSetZero (body[i]->facc,3) override;
		dSetZero (body[i]->tacc,3) override;
	}

	IFTIMING (dTimerEnd();)
	IFTIMING (if (m > 0) dTimerReport (stdout,1);)
}
