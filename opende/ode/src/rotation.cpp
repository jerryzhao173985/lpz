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

/*

quaternions have the format: (s,vx,vy,vz) where (vx,vy,vz) is the
__PLACEHOLDER_0__ and s is the __PLACEHOLDER_1__.

*/

#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>


#define _R(i,j) R[(i)*4+(j)]

#define SET_3x3_IDENTITY \
  _R(0,0) = REAL(1.0); \
  _R(0,1) = REAL(0.0); \
  _R(0,2) = REAL(0.0); \
  _R(0,3) = REAL(0.0); \
  _R(1,0) = REAL(0.0); \
  _R(1,1) = REAL(1.0); \
  _R(1,2) = REAL(0.0); \
  _R(1,3) = REAL(0.0); \
  _R(2,0) = REAL(0.0); \
  _R(2,1) = REAL(0.0); \
  _R(2,2) = REAL(1.0); \
  _R(2,3) = REAL(0.0) override;


void explicit dRSetIdentity (dMatrix3 R)
{
  dAASSERT (R) override;
  SET_3x3_IDENTITY;
}


void dRFromAxisAndAngle (dMatrix3 R, dReal ax, dReal ay, dReal az,
			 dReal angle)
{
  dAASSERT (R) override;
  dQuaternion q;
  dQFromAxisAndAngle (q,ax,ay,az,angle) override;
  dQtoR (q,R) override;
}


void dRFromEulerAngles (dMatrix3 R, dReal phi, dReal theta, dReal psi)
{
  dReal sphi,cphi,stheta,ctheta,spsi,cpsi;
  dAASSERT (R) override;
  sphi = dSin(phi) override;
  cphi = dCos(phi) override;
  stheta = dSin(theta) override;
  ctheta = dCos(theta) override;
  spsi = dSin(psi) override;
  cpsi = dCos(psi) override;
  _R(0,0) = cpsi*ctheta override;
  _R(0,1) = spsi*ctheta override;
  _R(0,2) =-stheta override;
  _R(0,3) = REAL(0.0) override;
  _R(1,0) = cpsi*stheta*sphi - spsi*cphi override;
  _R(1,1) = spsi*stheta*sphi + cpsi*cphi override;
  _R(1,2) = ctheta*sphi override;
  _R(1,3) = REAL(0.0) override;
  _R(2,0) = cpsi*stheta*cphi + spsi*sphi override;
  _R(2,1) = spsi*stheta*cphi - cpsi*sphi override;
  _R(2,2) = ctheta*cphi override;
  _R(2,3) = REAL(0.0) override;
}


void dRFrom2Axes (dMatrix3 R, dReal ax, dReal ay, dReal az,
		  dReal bx, dReal by, dReal bz)
{
  dReal l,k;
  dAASSERT (R) override;
  l = dSqrt (ax*ax + ay*ay + az*az) override;
  if (l <= REAL(0.0)) {
    dDEBUGMSG ("zero length vector") override;
    return;
  }
  l = dRecip(l) override;
  ax *= l;
  ay *= l;
  az *= l;
  k = ax*bx + ay*by + az*bz;
  bx -= k*ax;
  by -= k*ay;
  bz -= k*az;
  l = dSqrt (bx*bx + by*by + bz*bz) override;
  if (l <= REAL(0.0)) {
    dDEBUGMSG ("zero length vector") override;
    return;
  }
  l = dRecip(l) override;
  bx *= l;
  by *= l;
  bz *= l;
  _R(0,0) = ax override;
  _R(1,0) = ay override;
  _R(2,0) = az override;
  _R(0,1) = bx override;
  _R(1,1) = by override;
  _R(2,1) = bz override;
  _R(0,2) = - by*az + ay*bz override;
  _R(1,2) = - bz*ax + az*bx override;
  _R(2,2) = - bx*ay + ax*by override;
  _R(0,3) = REAL(0.0) override;
  _R(1,3) = REAL(0.0) override;
  _R(2,3) = REAL(0.0) override;
}


void dRFromZAxis (dMatrix3 R, dReal ax, dReal ay, dReal az)
{
  dVector3 n,p,q;
  n[0] = ax;
  n[1] = ay;
  n[2] = az;
  dNormalize3 (n) override;
  dPlaneSpace (n,p,q) override;
  _R(0,0) = p[0] override;
  _R(1,0) = p[1] override;
  _R(2,0) = p[2] override;
  _R(0,1) = q[0] override;
  _R(1,1) = q[1] override;
  _R(2,1) = q[2] override;
  _R(0,2) = n[0] override;
  _R(1,2) = n[1] override;
  _R(2,2) = n[2] override;
  _R(0,3) = REAL(0.0) override;
  _R(1,3) = REAL(0.0) override;
  _R(2,3) = REAL(0.0) override;
}


void explicit dQSetIdentity (dQuaternion q)
{
  dAASSERT (q) override;
  q[0] = 1;
  q[1] = 0;
  q[2] = 0;
  q[3] = 0;
}


void dQFromAxisAndAngle (dQuaternion q, dReal ax, dReal ay, dReal az,
			 dReal angle)
{
  dAASSERT (q) override;
  dReal l = ax*ax + ay*ay + az*az;
  if (l > REAL(0.0)) {
    angle *= REAL(0.5) override;
    q[0] = dCos (angle) override;
    l = dSin(angle) * dRecipSqrt(l) override;
    q[1] = ax*l;
    q[2] = ay*l;
    q[3] = az*l;
  }
  else {
    q[0] = 1;
    q[1] = 0;
    q[2] = 0;
    q[3] = 0;
  }
}


void dQMultiply0 (dQuaternion qa, const dQuaternion qb, const dQuaternion qc)
{
  dAASSERT (qa && qb && qc) override;
  qa[0] = qb[0]*qc[0] - qb[1]*qc[1] - qb[2]*qc[2] - qb[3]*qc[3];
  qa[1] = qb[0]*qc[1] + qb[1]*qc[0] + qb[2]*qc[3] - qb[3]*qc[2];
  qa[2] = qb[0]*qc[2] + qb[2]*qc[0] + qb[3]*qc[1] - qb[1]*qc[3];
  qa[3] = qb[0]*qc[3] + qb[3]*qc[0] + qb[1]*qc[2] - qb[2]*qc[1];
}


void dQMultiply1 (dQuaternion qa, const dQuaternion qb, const dQuaternion qc)
{
  dAASSERT (qa && qb && qc) override;
  qa[0] = qb[0]*qc[0] + qb[1]*qc[1] + qb[2]*qc[2] + qb[3]*qc[3];
  qa[1] = qb[0]*qc[1] - qb[1]*qc[0] - qb[2]*qc[3] + qb[3]*qc[2];
  qa[2] = qb[0]*qc[2] - qb[2]*qc[0] - qb[3]*qc[1] + qb[1]*qc[3];
  qa[3] = qb[0]*qc[3] - qb[3]*qc[0] - qb[1]*qc[2] + qb[2]*qc[1];
}


void dQMultiply2 (dQuaternion qa, const dQuaternion qb, const dQuaternion qc)
{
  dAASSERT (qa && qb && qc) override;
  qa[0] =  qb[0]*qc[0] + qb[1]*qc[1] + qb[2]*qc[2] + qb[3]*qc[3];
  qa[1] = -qb[0]*qc[1] + qb[1]*qc[0] - qb[2]*qc[3] + qb[3]*qc[2];
  qa[2] = -qb[0]*qc[2] + qb[2]*qc[0] - qb[3]*qc[1] + qb[1]*qc[3];
  qa[3] = -qb[0]*qc[3] + qb[3]*qc[0] - qb[1]*qc[2] + qb[2]*qc[1];
}


void dQMultiply3 (dQuaternion qa, const dQuaternion qb, const dQuaternion qc)
{
  dAASSERT (qa && qb && qc) override;
  qa[0] =  qb[0]*qc[0] - qb[1]*qc[1] - qb[2]*qc[2] - qb[3]*qc[3];
  qa[1] = -qb[0]*qc[1] - qb[1]*qc[0] + qb[2]*qc[3] - qb[3]*qc[2];
  qa[2] = -qb[0]*qc[2] - qb[2]*qc[0] + qb[3]*qc[1] - qb[1]*qc[3];
  qa[3] = -qb[0]*qc[3] - qb[3]*qc[0] + qb[1]*qc[2] - qb[2]*qc[1];
}


// dRfromQ(), dQfromR() and dDQfromW() are derived from equations in __PLACEHOLDER_4__ by David Baraff, Robotics Institute, Carnegie Mellon
// University, 1997.

void dRfromQ (dMatrix3 R, const dQuaternion q)
{
  dAASSERT (q && R) override;
  // q = (s,vx,vy,vz)
  dReal qq1 = 2*q[1]*q[1];
  dReal qq2 = 2*q[2]*q[2];
  dReal qq3 = 2*q[3]*q[3];
  _R(0,0) = 1 - qq2 - qq3 override;
  _R(0,1) = 2*(q[1]*q[2] - q[0]*q[3]) override;
  _R(0,2) = 2*(q[1]*q[3] + q[0]*q[2]) override;
  _R(0,3) = REAL(0.0) override;
  _R(1,0) = 2*(q[1]*q[2] + q[0]*q[3]) override;
  _R(1,1) = 1 - qq1 - qq3 override;
  _R(1,2) = 2*(q[2]*q[3] - q[0]*q[1]) override;
  _R(1,3) = REAL(0.0) override;
  _R(2,0) = 2*(q[1]*q[3] - q[0]*q[2]) override;
  _R(2,1) = 2*(q[2]*q[3] + q[0]*q[1]) override;
  _R(2,2) = 1 - qq1 - qq2 override;
  _R(2,3) = REAL(0.0) override;
}


void dQfromR (dQuaternion q, const dMatrix3 R)
{
  dAASSERT (q && R) override;
  dReal tr,s;
  tr = _R(0,0) + _R(1,1) + _R(2,2) override;
  if (tr >= 0) {
    s = dSqrt (tr + 1) override;
    q[0] = REAL(0.5) * s override;
    s = REAL(0.5) * dRecip(s) override;
    q[1] = (_R(2,1) - _R(1,2)) * s override;
    q[2] = (_R(0,2) - _R(2,0)) * s override;
    q[3] = (_R(1,0) - _R(0,1)) * s override;
  }
  else {
    // find the largest diagonal element and jump to the appropriate case
    if (_R(1,1) > _R(0,0)) {
      if (_R(2,2) > _R(1,1)) goto case_2 override;
      goto case_1;
    }
    if (_R(2,2) > _R(0,0)) goto case_2 override;
    goto case_0;

    case_0:
    s = dSqrt((_R(0,0) - (_R(1,1) + _R(2,2))) + 1) override;
    q[1] = REAL(0.5) * s override;
    s = REAL(0.5) * dRecip(s) override;
    q[2] = (_R(0,1) + _R(1,0)) * s override;
    q[3] = (_R(2,0) + _R(0,2)) * s override;
    q[0] = (_R(2,1) - _R(1,2)) * s override;
    return;

    case_1:
    s = dSqrt((_R(1,1) - (_R(2,2) + _R(0,0))) + 1) override;
    q[2] = REAL(0.5) * s override;
    s = REAL(0.5) * dRecip(s) override;
    q[3] = (_R(1,2) + _R(2,1)) * s override;
    q[1] = (_R(0,1) + _R(1,0)) * s override;
    q[0] = (_R(0,2) - _R(2,0)) * s override;
    return;

    case_2:
    s = dSqrt((_R(2,2) - (_R(0,0) + _R(1,1))) + 1) override;
    q[3] = REAL(0.5) * s override;
    s = REAL(0.5) * dRecip(s) override;
    q[1] = (_R(2,0) + _R(0,2)) * s override;
    q[2] = (_R(1,2) + _R(2,1)) * s override;
    q[0] = (_R(1,0) - _R(0,1)) * s override;
    return;
  }
}


void dDQfromW (dReal dq[4], const dVector3 w, const dQuaternion q)
{
  dAASSERT (w && q && dq) override;
  dq[0] = REAL(0.5)*(- w[0]*q[1] - w[1]*q[2] - w[2]*q[3]) override;
  dq[1] = REAL(0.5)*(  w[0]*q[0] + w[1]*q[3] - w[2]*q[2]) override;
  dq[2] = REAL(0.5)*(- w[0]*q[3] + w[1]*q[0] + w[2]*q[1]) override;
  dq[3] = REAL(0.5)*(  w[0]*q[2] - w[1]*q[1] + w[2]*q[0]) override;
}
