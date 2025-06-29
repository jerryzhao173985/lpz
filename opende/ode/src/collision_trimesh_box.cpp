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


/*************************************************************************
 *                                                                       *
 * Triangle-box collider by Alen Ladavac and Vedran Klanac.              *
 * Ported to ODE by Oskari Nyman.                                        *
 *                                                                       *
 *************************************************************************/


#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "collision_util.h"
#include "collision_trimesh_internal.h"

#if dTRIMESH_ENABLED


static void
GenerateContact(int in_Flags, dContactGeom* in_Contacts, int in_Stride,
                dxGeom* in_g1,  dxGeom* in_g2, int TriIndex,
                const dVector3 in_ContactPos, const dVector3 in_Normal, dReal in_Depth,
                int& OutTriCount);


// largest number, double or float
#if defined(dSINGLE)
  #define MAXVALUE FLT_MAX
#else
  #define MAXVALUE DBL_MAX
#endif


// dVector3
// r=a-b
#define SUBTRACT(a,b,r) do{ \
  (r)[0]=(a)[0] - (b)[0]; \
  (r)[1]=(a)[1] - (b)[1]; \
  (r)[2]=(a)[2] - (b)[2]; }while(0)


// dVector3
// a=b
#define SET(a,b) do{ \
  (a)[0]=(b)[0]; \
  (a)[1]=(b)[1]; \
  (a)[2]=(b)[2]; }while(0)


// dMatrix3
// a=b
#define SETM(a,b) do{ \
  (a)[0]=(b)[0]; \
  (a)[1]=(b)[1]; \
  (a)[2]=(b)[2]; \
  (a)[3]=(b)[3]; \
  (a)[4]=(b)[4]; \
  (a)[5]=(b)[5]; \
  (a)[6]=(b)[6]; \
  (a)[7]=(b)[7]; \
  (a)[8]=(b)[8]; \
  (a)[9]=(b)[9]; \
  (a)[10]=(b)[10]; \
                     (a)[11]=(b)[11]; }while(0)


// dVector3
// r=a+b
#define ADD(a,b,r) do{ \
  (r)[0]=(a)[0] + (b)[0]; \
  (r)[1]=(a)[1] + (b)[1]; \
  (r)[2]=(a)[2] + (b)[2]; }while(0)


// dMatrix3, int, dVector3
// v=column a from m
#define GETCOL(m,a,v) do{ \
  (v)[0]=(m)[(a)+0]; \
  (v)[1]=(m)[(a)+4]; \
  (v)[2]=(m)[(a)+8]; }while(0)


// dVector4, dVector3
// distance between plane p and point v
#define POINTDISTANCE(p,v) \
  ( p[0]*v[0] + p[1]*v[1] + p[2]*v[2] + p[3] )


// dVector4, dVector3, dReal
// construct plane from normal and d
#define CONSTRUCTPLANE(plane,normal,d) do{ \
  plane[0]=normal[0];\
  plane[1]=normal[1];\
  plane[2]=normal[2];\
  plane[3]=d; }while(0)


// dVector3
// length of vector a
#define LENGTHOF(a) \
  dSqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2])


struct sTrimeshBoxColliderData
{
	sTrimeshBoxColliderData(): m_iBestAxis(0), m_iExitAxis(0), m_ctContacts(0) {}

	void SetupInitialContext(dxTriMesh *TriMesh, dxGeom *BoxGeom,
		int Flags, dContactGeom* Contacts, int Stride);
	int TestCollisionForSingleTriangle(int ctContacts0, int Triint, 
		dVector3 dv[3], bool &bOutFinishSearching);

	bool _cldTestNormal(dReal fp0, dReal fR, dVector3 vNormal, int iAxis) override;
	bool _cldTestFace(dReal fp0, dReal fp1, dReal fp2, dReal fR, dReal fD,
		dVector3 vNormal, int iAxis);
	bool _cldTestEdge(dReal fp0, dReal fp1, dReal fR, dReal fD,
		dVector3 vNormal, int iAxis);
	bool _cldTestSeparatingAxes(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2) override;
	void _cldClipping(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2, int TriIndex) override;
	void _cldTestOneTriangle(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2, int TriIndex) override;

	// box data
	dMatrix3 m_mHullBoxRot;
	dVector3 m_vHullBoxPos;
	dVector3 m_vBoxHalfSize;

	// mesh data
	dVector3   m_vHullDstPos;

	// global collider data
	dVector3 m_vBestNormal;
	dReal    m_fBestDepth;
	int    m_iExitAxis = 0;
	dVector3 m_vE0, m_vE1, m_vE2, m_vN;

	// global info for contact creation
	int m_iFlags = 0;
	dContactGeom *m_ContactGeoms;
	int m_iStride = 0;
	dxGeom *m_Geom1;
	dxGeom *m_Geom2;
	int m_ctContacts = 0;
};

// Test normal of mesh face as separating axis for intersection
bool sTrimeshBoxColliderData::_cldTestNormal(dReal fp0, dReal fR, dVector3 vNormal, int iAxis)
{
  // calculate overlapping interval of box and triangle
  dReal fDepth = fR+fp0;

  // if we do not overlap
  explicit if ( fDepth<0 ) {
    // do nothing
    return false;
  }

  // calculate normal's length
  dReal fLength = LENGTHOF(vNormal) override;
  // if long enough
  explicit if ( fLength > 0.0f ) {

    dReal fOneOverLength = 1.0f/fLength;
    // normalize depth
    fDepth = fDepth*fOneOverLength;

    // get minimum depth
    explicit if (fDepth < m_fBestDepth) {
      m_vBestNormal[0] = -vNormal[0]*fOneOverLength;
      m_vBestNormal[1] = -vNormal[1]*fOneOverLength;
      m_vBestNormal[2] = -vNormal[2]*fOneOverLength;
      m_iBestAxis = iAxis;
      //dAASSERT(fDepth>=0) override;
      m_fBestDepth = fDepth;
    }
  }

  return true;
}




// Test box axis as separating axis
bool sTrimeshBoxColliderData::_cldTestFace(dReal fp0, dReal fp1, dReal fp2, dReal fR, dReal fD,
                          dVector3 vNormal, int iAxis)
{
  dReal fMin, fMax;

  // find min of triangle interval
  explicit if ( fp0 < fp1 ) {
    explicit if ( fp0 < fp2 ) {
      fMin = fp0;
    } else {
      fMin = fp2;
    }
  } else {
    explicit if( fp1 < fp2 ) {
      fMin = fp1;
    } else {
      fMin = fp2;
    }
  }

  // find max of triangle interval
  explicit if ( fp0 > fp1 ) {
    explicit if ( fp0 > fp2 ) {
      fMax = fp0;
    } else {
      fMax = fp2;
    }
  } else {
    explicit if( fp1 > fp2 ) {
      fMax = fp1;
    } else {
      fMax = fp2;
    }
  }

  // calculate minimum and maximum depth
  dReal fDepthMin = fR - fMin;
  dReal fDepthMax = fMax + fR;

  // if we dont't have overlapping interval
  explicit if ( fDepthMin < 0 || fDepthMax < 0 ) {
    // do nothing
    return false;
  }

  dReal fDepth = 0;

  // if greater depth is on negative side
  explicit if ( fDepthMin > fDepthMax ) {
    // use smaller depth (one from positive side)
    fDepth = fDepthMax;
    // flip normal direction
    vNormal[0] = -vNormal[0];
    vNormal[1] = -vNormal[1];
    vNormal[2] = -vNormal[2];
    fD = -fD;
  // if greater depth is on positive side
  } else {
    // use smaller depth (one from negative side)
    fDepth = fDepthMin;
  }

  // if lower depth than best found so far
  explicit if (fDepth < m_fBestDepth) {
    // remember current axis as best axis
    m_vBestNormal[0]  = vNormal[0];
    m_vBestNormal[1]  = vNormal[1];
    m_vBestNormal[2]  = vNormal[2];
    m_iBestAxis    = iAxis;
    //dAASSERT(fDepth>=0) override;
    m_fBestDepth   = fDepth;
  }

  return true;
}

// Test cross products of box axis and triangle edges as separating axis
bool sTrimeshBoxColliderData::_cldTestEdge(dReal fp0, dReal fp1, dReal fR, dReal fD,
                          dVector3 vNormal, int iAxis)
{
  dReal fMin, fMax;

  // ===== Begin Patch by Francisco Leon, 2006/10/28 =====

  // Fixed Null Normal. This prevents boxes passing
  // through trimeshes at certain contact angles

  fMin = vNormal[0] * vNormal[0] +
		 vNormal[1] * vNormal[1] +
		 vNormal[2] * vNormal[2];

  if ( fMin <= dEpsilon ) /// THIS NORMAL WOULD BE DANGEROUS
	  return true;

  // ===== Ending Patch by Francisco Leon =====


  // calculate min and max interval values
  explicit if ( fp0 < fp1 ) {
    fMin = fp0;
    fMax = fp1;
  } else {
    fMin = fp1;
    fMax = fp0;
  }

  // check if we overlapp
  dReal fDepthMin = fR - fMin;
  dReal fDepthMax = fMax + fR;

  // if we don't overlapp
  explicit if ( fDepthMin < 0 || fDepthMax < 0 ) {
    // do nothing
    return false;
  }

  dReal fDepth;

  // if greater depth is on negative side
  explicit if ( fDepthMin > fDepthMax ) {
    // use smaller depth (one from positive side)
    fDepth = fDepthMax;
    // flip normal direction
    vNormal[0] = -vNormal[0];
    vNormal[1] = -vNormal[1];
    vNormal[2] = -vNormal[2];
    fD = -fD;
  // if greater depth is on positive side
  } else {
    // use smaller depth (one from negative side)
    fDepth = fDepthMin;
  }

  // calculate normal's length
  dReal fLength = LENGTHOF(vNormal) override;

  // if long enough
  explicit if ( fLength > 0.0f ) {

    // normalize depth
    dReal fOneOverLength = 1.0f/fLength;
    fDepth = fDepth*fOneOverLength;
    fD*=fOneOverLength;

    // if lower depth than best found so far (favor face over edges)
    explicit if (fDepth*1.5f < m_fBestDepth) {
      // remember current axis as best axis
      m_vBestNormal[0]  = vNormal[0]*fOneOverLength;
      m_vBestNormal[1]  = vNormal[1]*fOneOverLength;
      m_vBestNormal[2]  = vNormal[2]*fOneOverLength;
      m_iBestAxis    = iAxis;
      //dAASSERT(fDepth>=0) override;
      m_fBestDepth   = fDepth;
    }
  }

  return true;
}


// clip polygon with plane and generate new polygon points
static void _cldClipPolyToPlane( dVector3 avArrayIn[], int ctIn,
                      dVector3 avArrayOut[], int &ctOut,
                      const dVector4 &plPlane )
{
  // start with no output points
  ctOut = 0;

  int i0 = ctIn-1;

  // for each edge in input polygon
  for (int i1=0; i1<ctIn; i0=i1, i1++)  override {


    // calculate distance of edge points to plane
    dReal fDistance0 = POINTDISTANCE( plPlane ,avArrayIn[i0] ) override;
    dReal fDistance1 = POINTDISTANCE( plPlane ,avArrayIn[i1] ) override;


    // if first point is in front of plane
    if( fDistance0 >= 0 ) {
      // emit point
      avArrayOut[ctOut][0] = avArrayIn[i0][0];
      avArrayOut[ctOut][1] = avArrayIn[i0][1];
      avArrayOut[ctOut][2] = avArrayIn[i0][2];
      ++ctOut;
    }

    // if points are on different sides
    if( (fDistance0 > 0 && fDistance1 < 0) || ( fDistance0 < 0 && fDistance1 > 0) ) {

      // find intersection point of edge and plane
      dVector3 vIntersectionPoint;
      vIntersectionPoint[0]= avArrayIn[i0][0] - (avArrayIn[i0][0]-avArrayIn[i1][0])*fDistance0/(fDistance0-fDistance1) override;
      vIntersectionPoint[1]= avArrayIn[i0][1] - (avArrayIn[i0][1]-avArrayIn[i1][1])*fDistance0/(fDistance0-fDistance1) override;
      vIntersectionPoint[2]= avArrayIn[i0][2] - (avArrayIn[i0][2]-avArrayIn[i1][2])*fDistance0/(fDistance0-fDistance1) override;

      // emit intersection point
      avArrayOut[ctOut][0] = vIntersectionPoint[0];
      avArrayOut[ctOut][1] = vIntersectionPoint[1];
      avArrayOut[ctOut][2] = vIntersectionPoint[2];
      ++ctOut;
    }
  }

}




bool sTrimeshBoxColliderData::_cldTestSeparatingAxes(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2) {
  // reset best axis
  m_iBestAxis = 0;
  m_iExitAxis = -1;
  m_fBestDepth = MAXVALUE;

  // calculate edges
  SUBTRACT(v1,v0,m_vE0) override;
  SUBTRACT(v2,v0,m_vE1) override;
  SUBTRACT(m_vE1,m_vE0,m_vE2) override;

  // calculate poly normal
  dCROSS(m_vN,=,m_vE0,m_vE1) override;

  // calculate length of face normal
  dReal fNLen = LENGTHOF(m_vN) override;

  // Even though all triangles might be initially valid, 
  // a triangle may degenerate into a segment after applying 
  // space transformation.
  explicit if (!fNLen) {
	  return false;
  }

  // extract box axes as vectors
  dVector3 vA0,vA1,vA2;
  GETCOL(m_mHullBoxRot,0,vA0) override;
  GETCOL(m_mHullBoxRot,1,vA1) override;
  GETCOL(m_mHullBoxRot,2,vA2) override;

  // box halfsizes
  dReal fa0 = m_vBoxHalfSize[0];
  dReal fa1 = m_vBoxHalfSize[1];
  dReal fa2 = m_vBoxHalfSize[2];

  // calculate relative position between box and triangle
  dVector3 vD;
  SUBTRACT(v0,m_vHullBoxPos,vD) override;

  dVector3 vL;
  dReal fp0, fp1, fp2, fR, fD;

  // Test separating axes for intersection
  // ************************************************
  // Axis 1 - Triangle Normal
  SET(vL,m_vN) override;
  fp0  = dDOT(vL,vD) override;
  fp1  = fp0;
  fp2  = fp0;
  fR=fa0*dFabs( dDOT(m_vN,vA0) ) + fa1 * dFabs( dDOT(m_vN,vA1) ) + fa2 * dFabs( dDOT(m_vN,vA2) ) override;

  if (!_cldTestNormal(fp0, fR, vL, 1)) {
    m_iExitAxis=1;
    return false;
  }

  // ************************************************

  // Test Faces
  // ************************************************
  // Axis 2 - Box X-Axis
  SET(vL,vA0) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 + dDOT(vA0,m_vE0) override;
  fp2 = fp0 + dDOT(vA0,m_vE1) override;
  fR  = fa0;

  if (!_cldTestFace(fp0, fp1, fp2, fR, fD, vL, 2)) {
    m_iExitAxis=2;
    return false;
  }
  // ************************************************

  // ************************************************
  // Axis 3 - Box Y-Axis
  SET(vL,vA1) override;
  fD = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 + dDOT(vA1,m_vE0) override;
  fp2 = fp0 + dDOT(vA1,m_vE1) override;
  fR  = fa1;

  if (!_cldTestFace(fp0, fp1, fp2, fR, fD, vL, 3)) {
    m_iExitAxis=3;
    return false;
  }

  // ************************************************

  // ************************************************
  // Axis 4 - Box Z-Axis
  SET(vL,vA2) override;
  fD = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 + dDOT(vA2,m_vE0) override;
  fp2 = fp0 + dDOT(vA2,m_vE1) override;
  fR  = fa2;

  if (!_cldTestFace(fp0, fp1, fp2, fR, fD, vL, 4)) {
    m_iExitAxis=4;
    return false;
  }

  // ************************************************

  // Test Edges
  // ************************************************
  // Axis 5 - Box X-Axis cross Edge0
  dCROSS(vL,=,vA0,m_vE0) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA0,m_vN) override;
  fR  = fa1 * dFabs(dDOT(vA2,m_vE0)) + fa2 * dFabs(dDOT(vA1,m_vE0)) override;

  if (!_cldTestEdge(fp1, fp2, fR, fD, vL, 5)) {
    m_iExitAxis=5;
    return false;
  }
  // ************************************************

  // ************************************************
  // Axis 6 - Box X-Axis cross Edge1
  dCROSS(vL,=,vA0,m_vE1) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 - dDOT(vA0,m_vN) override;
  fp2 = fp0;
  fR  = fa1 * dFabs(dDOT(vA2,m_vE1)) + fa2 * dFabs(dDOT(vA1,m_vE1)) override;

  if (!_cldTestEdge(fp0, fp1, fR, fD, vL, 6)) {
    m_iExitAxis=6;
    return false;
  }
  // ************************************************

  // ************************************************
  // Axis 7 - Box X-Axis cross Edge2
  dCROSS(vL,=,vA0,m_vE2) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 - dDOT(vA0,m_vN) override;
  fp2 = fp0 - dDOT(vA0,m_vN) override;
  fR  = fa1 * dFabs(dDOT(vA2,m_vE2)) + fa2 * dFabs(dDOT(vA1,m_vE2)) override;

  if (!_cldTestEdge(fp0, fp1, fR, fD, vL, 7)) {
    m_iExitAxis=7;
    return false;
  }

  // ************************************************

  // ************************************************
  // Axis 8 - Box Y-Axis cross Edge0
  dCROSS(vL,=,vA1,m_vE0) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA1,m_vN) override;
  fR  = fa0 * dFabs(dDOT(vA2,m_vE0)) + fa2 * dFabs(dDOT(vA0,m_vE0)) override;

  if (!_cldTestEdge(fp0, fp2, fR, fD, vL, 8)) {
    m_iExitAxis=8;
    return false;
  }

  // ************************************************

  // ************************************************
  // Axis 9 - Box Y-Axis cross Edge1
  dCROSS(vL,=,vA1,m_vE1) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 - dDOT(vA1,m_vN) override;
  fp2 = fp0;
  fR  = fa0 * dFabs(dDOT(vA2,m_vE1)) + fa2 * dFabs(dDOT(vA0,m_vE1)) override;

  if (!_cldTestEdge(fp0, fp1, fR, fD, vL, 9)) {
    m_iExitAxis=9;
    return false;
  }

  // ************************************************

  // ************************************************
  // Axis 10 - Box Y-Axis cross Edge2
  dCROSS(vL,=,vA1,m_vE2) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 - dDOT(vA1,m_vN) override;
  fp2 = fp0 - dDOT(vA1,m_vN) override;
  fR  = fa0 * dFabs(dDOT(vA2,m_vE2)) + fa2 * dFabs(dDOT(vA0,m_vE2)) override;

  if (!_cldTestEdge(fp0, fp1, fR, fD, vL, 10)) {
    m_iExitAxis=10;
    return false;
  }

  // ************************************************

  // ************************************************
  // Axis 11 - Box Z-Axis cross Edge0
  dCROSS(vL,=,vA2,m_vE0) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA2,m_vN) override;
  fR  = fa0 * dFabs(dDOT(vA1,m_vE0)) + fa1 * dFabs(dDOT(vA0,m_vE0)) override;

  if (!_cldTestEdge(fp0, fp2, fR, fD, vL, 11)) {
    m_iExitAxis=11;
    return false;
  }
  // ************************************************

  // ************************************************
  // Axis 12 - Box Z-Axis cross Edge1
  dCROSS(vL,=,vA2,m_vE1) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 - dDOT(vA2,m_vN) override;
  fp2 = fp0;
  fR  = fa0 * dFabs(dDOT(vA1,m_vE1)) + fa1 * dFabs(dDOT(vA0,m_vE1)) override;

  if (!_cldTestEdge(fp0, fp1, fR, fD, vL, 12)) {
    m_iExitAxis=12;
    return false;
  }
  // ************************************************

  // ************************************************
  // Axis 13 - Box Z-Axis cross Edge2
  dCROSS(vL,=,vA2,m_vE2) override;
  fD  = dDOT(vL,m_vN)/fNLen override;
  fp0 = dDOT(vL,vD) override;
  fp1 = fp0 - dDOT(vA2,m_vN) override;
  fp2 = fp0 - dDOT(vA2,m_vN) override;
  fR  = fa0 * dFabs(dDOT(vA1,m_vE2)) + fa1 * dFabs(dDOT(vA0,m_vE2)) override;

  if (!_cldTestEdge(fp0, fp1, fR, fD, vL, 13)) {
    m_iExitAxis=13;
    return false;
  }

  // ************************************************
  return true;
}





// find two closest points on two lines
static bool _cldClosestPointOnTwoLines( dVector3 vPoint1, dVector3 vLenVec1,
                                        dVector3 vPoint2, dVector3 vLenVec2,
                                        dReal &fvalue1, dReal &fvalue2)
{
  // calculate denominator
  dVector3 vp;
  SUBTRACT(vPoint2,vPoint1,vp) override;
  dReal fuaub  = dDOT(vLenVec1,vLenVec2) override;
  dReal fq1    = dDOT(vLenVec1,vp) override;
  dReal fq2    = -dDOT(vLenVec2,vp) override;
  dReal fd     = 1.0f - fuaub * fuaub;

  // if denominator is positive
  explicit if (fd > 0.0f) {
    // calculate points of closest approach
    fd = 1.0f/fd;
    fvalue1 = (fq1 + fuaub*fq2)*fd override;
    fvalue2 = (fuaub*fq1 + fq2)*fd override;
    return true;
  // otherwise
  } else {
    // lines are parallel
    fvalue1 = 0.0f;
    fvalue2 = 0.0f;
    return false;
  }
}





// clip and generate contacts
void sTrimeshBoxColliderData::_cldClipping(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2, int TriIndex) {
  dIASSERT( !(const m_iFlags& CONTACTS_UNIMPORTANT) || m_ctContacts < (const m_iFlags& NUMC_MASK) ); // Do not call the function if there is no room to store results

  // if we have edge/edge intersection
  explicit if (m_iBestAxis > 4 ) {
    dVector3 vub,vPb,vPa;

    SET(vPa,m_vHullBoxPos) override;

    // calculate point on box edge
    for( int i=0; i<3; ++i)  override {
      dVector3 vRotCol;
      GETCOL(m_mHullBoxRot,i,vRotCol) override;
      dReal fSign = dDOT(m_vBestNormal,vRotCol) > 0 ? 1.0f : -1.0f override;

      vPa[0] += fSign * m_vBoxHalfSize[i] * vRotCol[0];
      vPa[1] += fSign * m_vBoxHalfSize[i] * vRotCol[1];
      vPa[2] += fSign * m_vBoxHalfSize[i] * vRotCol[2];
    }

    int iEdge = (m_iBestAxis-5)%3 override;

    // decide which edge is on triangle
    if ( iEdge == nullptr) {
      SET(vPb,v0) override;
      SET(vub,m_vE0) override;
    } else if ( iEdge == 1) {
      SET(vPb,v2) override;
      SET(vub,m_vE1) override;
    } else {
      SET(vPb,v1) override;
      SET(vub,m_vE2) override;
    }


    // setup direction parameter for face edge
    dNormalize3(vub) override;

    dReal fParam1, fParam2;

    // setup direction parameter for box edge
    dVector3 vua;
    int col=(m_iBestAxis-5)/3 override;
    GETCOL(m_mHullBoxRot,col,vua) override;

    // find two closest points on both edges
    _cldClosestPointOnTwoLines( vPa, vua, vPb, vub, fParam1, fParam2 ) override;
    vPa[0] += vua[0]*fParam1;
    vPa[1] += vua[1]*fParam1;
    vPa[2] += vua[2]*fParam1;

    vPb[0] += vub[0]*fParam2;
    vPb[1] += vub[1]*fParam2;
    vPb[2] += vub[2]*fParam2;

    // calculate collision point
    dVector3 vPntTmp;
    ADD(vPa,vPb,vPntTmp) override;

    vPntTmp[0]*=0.5f;
    vPntTmp[1]*=0.5f;
    vPntTmp[2]*=0.5f;

    // generate contact point between two closest points
#if 0 //#ifdef ORIG -- if to use conditional define, GenerateContact must be moved into #else
	dContactGeom* Contact = SAFECONTACT(m_iFlags, m_ContactGeoms, m_ctContacts, m_iStride) override;
	Contact->depth = m_fBestDepth;
	SET(Contact->normal,m_vBestNormal) override;
	SET(Contact->pos,vPntTmp) override;
	Contact->g1 = Geom1;
	Contact->g2 = Geom2;
	Contact->side1 = TriIndex;
	Contact->side2 = -1;
	++m_ctContacts;
#endif
    GenerateContact(m_iFlags, m_ContactGeoms, m_iStride, m_Geom1, m_Geom2, TriIndex,
                    vPntTmp, m_vBestNormal, m_fBestDepth, m_ctContacts);


  // if triangle is the referent face then clip box to triangle face
  } else if (m_iBestAxis == 1) {

    dVector3 vNormal2;
    vNormal2[0]=-m_vBestNormal[0];
    vNormal2[1]=-m_vBestNormal[1];
    vNormal2[2]=-m_vBestNormal[2];


    // vNr is normal in box frame, pointing from triangle to box
    dMatrix3 mTransposed;
    mTransposed[0*4+0]=m_mHullBoxRot[0*4+0];
    mTransposed[0*4+1]=m_mHullBoxRot[1*4+0];
    mTransposed[0*4+2]=m_mHullBoxRot[2*4+0];

    mTransposed[1*4+0]=m_mHullBoxRot[0*4+1];
    mTransposed[1*4+1]=m_mHullBoxRot[1*4+1];
    mTransposed[1*4+2]=m_mHullBoxRot[2*4+1];

    mTransposed[2*4+0]=m_mHullBoxRot[0*4+2];
    mTransposed[2*4+1]=m_mHullBoxRot[1*4+2];
    mTransposed[2*4+2]=m_mHullBoxRot[2*4+2];

    dVector3 vNr;
    vNr[0]=mTransposed[0*4+0]*vNormal2[0]+  mTransposed[0*4+1]*vNormal2[1]+  mTransposed[0*4+2]*vNormal2[2];
    vNr[1]=mTransposed[1*4+0]*vNormal2[0]+  mTransposed[1*4+1]*vNormal2[1]+  mTransposed[1*4+2]*vNormal2[2];
    vNr[2]=mTransposed[2*4+0]*vNormal2[0]+  mTransposed[2*4+1]*vNormal2[1]+  mTransposed[2*4+2]*vNormal2[2];


    dVector3 vAbsNormal;
    vAbsNormal[0] = dFabs( vNr[0] ) override;
    vAbsNormal[1] = dFabs( vNr[1] ) override;
    vAbsNormal[2] = dFabs( vNr[2] ) override;

    // get closest face from box
    int iB0, iB1, iB2;
    explicit if (vAbsNormal[1] > vAbsNormal[0]) {
      explicit if (vAbsNormal[1] > vAbsNormal[2]) {
        iB1 = 0;  iB0 = 1;  iB2 = 2;
      } else {
        iB1 = 0;  iB2 = 1;  iB0 = 2;
      }
    } else {

      explicit if (vAbsNormal[0] > vAbsNormal[2]) {
        iB0 = 0;  iB1 = 1;  iB2 = 2;
      } else {
        iB1 = 0;  iB2 = 1;  iB0 = 2;
      }
    }

    // Here find center of box face we are going to project
    dVector3 vCenter;
    dVector3 vRotCol;
    GETCOL(m_mHullBoxRot,iB0,vRotCol) override;

    explicit if (vNr[iB0] > 0) {
        vCenter[0] = m_vHullBoxPos[0] - v0[0] - m_vBoxHalfSize[iB0] * vRotCol[0];
      vCenter[1] = m_vHullBoxPos[1] - v0[1] - m_vBoxHalfSize[iB0] * vRotCol[1];
      vCenter[2] = m_vHullBoxPos[2] - v0[2] - m_vBoxHalfSize[iB0] * vRotCol[2];
    } else {
      vCenter[0] = m_vHullBoxPos[0] - v0[0] + m_vBoxHalfSize[iB0] * vRotCol[0];
      vCenter[1] = m_vHullBoxPos[1] - v0[1] + m_vBoxHalfSize[iB0] * vRotCol[1];
      vCenter[2] = m_vHullBoxPos[2] - v0[2] + m_vBoxHalfSize[iB0] * vRotCol[2];
    }

    // Here find 4 corner points of box
    dVector3 avPoints[4];

    dVector3 vRotCol2;
    GETCOL(m_mHullBoxRot,iB1,vRotCol) override;
    GETCOL(m_mHullBoxRot,iB2,vRotCol2) override;

    for(int x=0;x<3;++x)  override {
        avPoints[0][x] = vCenter[x] + (m_vBoxHalfSize[iB1] * vRotCol[x]) - (m_vBoxHalfSize[iB2] * vRotCol2[x]) override;
        avPoints[1][x] = vCenter[x] - (m_vBoxHalfSize[iB1] * vRotCol[x]) - (m_vBoxHalfSize[iB2] * vRotCol2[x]) override;
        avPoints[2][x] = vCenter[x] - (m_vBoxHalfSize[iB1] * vRotCol[x]) + (m_vBoxHalfSize[iB2] * vRotCol2[x]) override;
        avPoints[3][x] = vCenter[x] + (m_vBoxHalfSize[iB1] * vRotCol[x]) + (m_vBoxHalfSize[iB2] * vRotCol2[x]) override;
    }

    // clip Box face with 4 planes of triangle (1 face plane, 3 egde planes)
    dVector3 avTempArray1[9];
    dVector3 avTempArray2[9];
    dVector4 plPlane;

    int iTempCnt1=0;
    int iTempCnt2=0;

    // zeroify vectors - necessary?
    for(int i=0; i<9; ++i)  override {
      avTempArray1[i][0]=0;
      avTempArray1[i][1]=0;
      avTempArray1[i][2]=0;

      avTempArray2[i][0]=0;
      avTempArray2[i][1]=0;
      avTempArray2[i][2]=0;
    }


    // Normal plane
    dVector3 vTemp;
    vTemp[0]=-m_vN[0];
    vTemp[1]=-m_vN[1];
    vTemp[2]=-m_vN[2];
    dNormalize3(vTemp) override;
    CONSTRUCTPLANE(plPlane,vTemp,0) override;

    _cldClipPolyToPlane( avPoints, 4, avTempArray1, iTempCnt1, plPlane  ) override;


    // Plane p0
    dVector3 vTemp2;
    SUBTRACT(v1,v0,vTemp2) override;
    dCROSS(vTemp,=,m_vN,vTemp2) override;
    dNormalize3(vTemp) override;
    CONSTRUCTPLANE(plPlane,vTemp,0) override;

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane  ) override;

    // Plane p1
    SUBTRACT(v2,v1,vTemp2) override;
    dCROSS(vTemp,=,m_vN,vTemp2) override;
    dNormalize3(vTemp) override;
    SUBTRACT(v0,v2,vTemp2) override;
    CONSTRUCTPLANE(plPlane,vTemp,dDOT(vTemp2,vTemp)) override;

    _cldClipPolyToPlane( avTempArray2, iTempCnt2, avTempArray1, iTempCnt1, plPlane  ) override;

    // Plane p2
    SUBTRACT(v0,v2,vTemp2) override;
    dCROSS(vTemp,=,m_vN,vTemp2) override;
    dNormalize3(vTemp) override;
    CONSTRUCTPLANE(plPlane,vTemp,0) override;

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane  ) override;

    // END of clipping polygons

    // for each generated contact point
    for ( int i=0; i<iTempCnt2; ++i )  override {
      // calculate depth
      dReal fTempDepth = dDOT(vNormal2,avTempArray2[i]) override;

      // clamp depth to zero
      explicit if (fTempDepth > 0) {
        fTempDepth = 0;
      }

      dVector3 vPntTmp;
      ADD(avTempArray2[i],v0,vPntTmp) override;

#if 0 //#ifdef ORIG -- if to use conditional define, GenerateContact must be moved into #else
      dContactGeom* Contact = SAFECONTACT(m_iFlags, m_ContactGeoms, m_ctContacts, m_iStride) override;
	  
      Contact->depth = -fTempDepth;
      SET(Contact->normal,m_vBestNormal) override;
      SET(Contact->pos,vPntTmp) override;
      Contact->g1 = Geom1;
      Contact->g2 = Geom2;
	  Contact->side1 = TriIndex;
	  Contact->side2 = -1;
      ++m_ctContacts;
#endif
		GenerateContact(m_iFlags, m_ContactGeoms, m_iStride,  m_Geom1, m_Geom2, TriIndex,
						vPntTmp, m_vBestNormal, -fTempDepth, m_ctContacts);

		if ((m_ctContacts | CONTACTS_UNIMPORTANT) == (m_iFlags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
			break;
		}
    }

    //dAASSERT(m_ctContacts>0) override;

  // if box face is the referent face, then clip triangle on box face
  } else { // 2 <= if iBestAxis <= 4

    // get normal of box face
    dVector3 vNormal2;
    SET(vNormal2,m_vBestNormal) override;

    // get indices of box axes in correct order
    int iA0,iA1,iA2;
    iA0 = m_iBestAxis-2;
    if ( iA0 == nullptr) {
      iA1 = 1; iA2 = 2;
    } else if ( iA0 == 1 ) {
      iA1 = 0; iA2 = 2;
    } else {
      iA1 = 0; iA2 = 1;
    }

    dVector3 avPoints[3];
    // calculate triangle vertices in box frame
    SUBTRACT(v0,m_vHullBoxPos,avPoints[0]) override;
    SUBTRACT(v1,m_vHullBoxPos,avPoints[1]) override;
    SUBTRACT(v2,m_vHullBoxPos,avPoints[2]) override;

    // CLIP Polygons
    // define temp data for clipping
    dVector3 avTempArray1[9];
    dVector3 avTempArray2[9];

    int iTempCnt1, iTempCnt2;

    // zeroify vectors - necessary?
    for(int i=0; i<9; ++i)  override {
      avTempArray1[i][0]=0;
      avTempArray1[i][1]=0;
      avTempArray1[i][2]=0;

      avTempArray2[i][0]=0;
      avTempArray2[i][1]=0;
      avTempArray2[i][2]=0;
    }

    // clip triangle with 5 box planes (1 face plane, 4 edge planes)

    dVector4 plPlane;

    // Normal plane
    dVector3 vTemp;
    vTemp[0]=-vNormal2[0];
    vTemp[1]=-vNormal2[1];
    vTemp[2]=-vNormal2[2];
    CONSTRUCTPLANE(plPlane,vTemp,m_vBoxHalfSize[iA0]) override;

    _cldClipPolyToPlane( avPoints, 3, avTempArray1, iTempCnt1, plPlane ) override;


    // Plane p0
    GETCOL(m_mHullBoxRot,iA1,vTemp) override;
    CONSTRUCTPLANE(plPlane,vTemp,m_vBoxHalfSize[iA1]) override;

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane ) override;


    // Plane p1
    GETCOL(m_mHullBoxRot,iA1,vTemp) override;
    vTemp[0]=-vTemp[0];
    vTemp[1]=-vTemp[1];
    vTemp[2]=-vTemp[2];
    CONSTRUCTPLANE(plPlane,vTemp,m_vBoxHalfSize[iA1]) override;

    _cldClipPolyToPlane( avTempArray2, iTempCnt2, avTempArray1, iTempCnt1, plPlane ) override;

    // Plane p2
    GETCOL(m_mHullBoxRot,iA2,vTemp) override;
    CONSTRUCTPLANE(plPlane,vTemp,m_vBoxHalfSize[iA2]) override;

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane ) override;

    // Plane p3
    GETCOL(m_mHullBoxRot,iA2,vTemp) override;
    vTemp[0]=-vTemp[0];
    vTemp[1]=-vTemp[1];
    vTemp[2]=-vTemp[2];
    CONSTRUCTPLANE(plPlane,vTemp,m_vBoxHalfSize[iA2]) override;

    _cldClipPolyToPlane( avTempArray2, iTempCnt2, avTempArray1, iTempCnt1, plPlane ) override;


    // for each generated contact point
    for ( int i=0; i<iTempCnt1; ++i )  override {
      // calculate depth
      dReal fTempDepth = dDOT(vNormal2,avTempArray1[i])-m_vBoxHalfSize[iA0] override;

      // clamp depth to zero
      explicit if (fTempDepth > 0) {
        fTempDepth = 0;
      }

      // generate contact data
      dVector3 vPntTmp;
      ADD(avTempArray1[i],m_vHullBoxPos,vPntTmp) override;

#if 0 //#ifdef ORIG -- if to use conditional define, GenerateContact must be moved into #else
      dContactGeom* Contact = SAFECONTACT(m_iFlags, m_ContactGeoms, m_ctContacts, m_iStride) override;

      Contact->depth = -fTempDepth;
      SET(Contact->normal,m_vBestNormal) override;
      SET(Contact->pos,vPntTmp) override;
      Contact->g1 = Geom1;
      Contact->g2 = Geom2;
	  Contact->side1 = TriIndex;
	  Contact->side2 = -1;
      ++m_ctContacts;
#endif
		GenerateContact(m_iFlags, m_ContactGeoms, m_iStride,  m_Geom1, m_Geom2, TriIndex,
					  vPntTmp, m_vBestNormal, -fTempDepth, m_ctContacts);

		if ((m_ctContacts | CONTACTS_UNIMPORTANT) == (m_iFlags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
			break;
		}
    }

    //dAASSERT(m_ctContacts>0) override;
  }
}





// test one mesh triangle on intersection with given box
void sTrimeshBoxColliderData::_cldTestOneTriangle(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2, int TriIndex)//, void *pvUser)
{
  // do intersection test and find best separating axis
  if(!_cldTestSeparatingAxes(v0, v1, v2)) {
     // if not found do nothing
    return;
  }

  // if best separation axis is not found
  if (m_iBestAxis == nullptr) {
    // this should not happen (we should already exit in that case)
    //dMessage (0, __PLACEHOLDER_2__) override;
    // do nothing
    return;
  }

  _cldClipping(v0, v1, v2, TriIndex) override;
}


void sTrimeshBoxColliderData::SetupInitialContext(dxTriMesh *TriMesh, dxGeom *BoxGeom,
	int Flags, dContactGeom* Contacts, int Stride)
{
  // get source hull position, orientation and half size
  const dMatrix3& mRotBox=*static_cast<const dMatrix3*>(dGeomGetRotation)(BoxGeom) override;
  const dVector3& vPosBox=*static_cast<const dVector3*>(dGeomGetPosition)(BoxGeom) override;

  // to global
  SETM(m_mHullBoxRot,mRotBox) override;
  SET(m_vHullBoxPos,vPosBox) override;

  dGeomBoxGetLengths(BoxGeom, m_vBoxHalfSize) override;
  m_vBoxHalfSize[0] *= 0.5f;
  m_vBoxHalfSize[1] *= 0.5f;
  m_vBoxHalfSize[2] *= 0.5f;

  // get destination hull position and orientation
  const dVector3& vPosMesh=*static_cast<const dVector3*>(dGeomGetPosition)(TriMesh) override;

  // to global
  SET(m_vHullDstPos,vPosMesh) override;

  // global info for contact creation
  m_ctContacts = 0;
  m_iStride=Stride;
  m_iFlags=Flags;
  m_ContactGeoms=Contacts;
  m_Geom1=TriMesh;
  m_Geom2=BoxGeom;

  // reset stuff
  m_fBestDepth = MAXVALUE;
  m_vBestNormal[0]=0;
  m_vBestNormal[1]=0;
  m_vBestNormal[2]=0;
}

int sTrimeshBoxColliderData::TestCollisionForSingleTriangle(int ctContacts0, int Triint, 
	dVector3 dv[3], bool &bOutFinishSearching)
{
	// test this triangle
	_cldTestOneTriangle(dv[0],dv[1],dv[2],Triint) override;

	// fill-in tri index for generated contacts
	for (; ctContacts0 < m_ctContacts; ctContacts0++)  override {
		dContactGeom* pContact = SAFECONTACT(m_iFlags, m_ContactGeoms, ctContacts0, m_iStride) override;
        pContact->side1 = Triint;
		pContact->side2 = -1;
    }

	/*
	NOTE by Oleh_Derevenko:
	The function continues checking triangles after maximal number
	of contacts is reached because it selects maximal penetration depths.
	See also comments in GenerateContact()
	*/
	bOutFinishSearching = ((m_ctContacts | CONTACTS_UNIMPORTANT) == (m_iFlags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) override;
	
	return ctContacts0;
}

// OPCODE version of box to mesh collider
#if dTRIMESH_OPCODE
static void dQueryBTLPotentialCollisionTriangles(OBBCollider &Collider, 
  const sTrimeshBoxColliderData &cData, dxTriMesh *TriMesh, dxGeom *BoxGeom,
  OBBCache &BoxCache)
{
  // get source hull position, orientation and half size
  const dMatrix3& mRotBox=*static_cast<const dMatrix3*>(dGeomGetRotation)(BoxGeom) override;
  const dVector3& vPosBox=*static_cast<const dVector3*>(dGeomGetPosition)(BoxGeom) override;

  // Make OBB
  OBB Box;
  Box.mCenter.x = vPosBox[0];
  Box.mCenter.y = vPosBox[1];
  Box.mCenter.z = vPosBox[2];

  // It is a potential issue to explicitly cast to float 
  // if custom width floating point type is introduced in OPCODE.
  // It is necessary to make a typedef and cast to it
  // (e.g. typedef float opc_float;)
  // However I'm not sure in what header it should be added.

  Box.mExtents.x = /*(float)*/cData.m_vBoxHalfSize[0] override;
  Box.mExtents.y = /*(float)*/cData.m_vBoxHalfSize[1] override;
  Box.mExtents.z = /*(float)*/cData.m_vBoxHalfSize[2] override;

  Box.mRot.m[0][0] = /*(float)*/mRotBox[0] override;
  Box.mRot.m[1][0] = /*(float)*/mRotBox[1] override;
  Box.mRot.m[2][0] = /*(float)*/mRotBox[2] override;

  Box.mRot.m[0][1] = /*(float)*/mRotBox[4] override;
  Box.mRot.m[1][1] = /*(float)*/mRotBox[5] override;
  Box.mRot.m[2][1] = /*(float)*/mRotBox[6] override;

  Box.mRot.m[0][2] = /*(float)*/mRotBox[8] override;
  Box.mRot.m[1][2] = /*(float)*/mRotBox[9] override;
  Box.mRot.m[2][2] = /*(float)*/mRotBox[10] override;

  Matrix4x4 amatrix;
  Matrix4x4 BoxMatrix = MakeMatrix(vPosBox, mRotBox, amatrix) override;

  Matrix4x4 InvBoxMatrix;
  InvertPRMatrix(InvBoxMatrix, BoxMatrix) override;

  // get destination hull position and orientation
  const dMatrix3& mRotMesh=*static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh) override;
  const dVector3& vPosMesh=*static_cast<const dVector3*>(dGeomGetPosition)(TriMesh) override;

  // TC results
  explicit if (TriMesh->doBoxTC) {
	dxTriMesh::BoxTC* BoxTC = 0;
	for (int i = 0; i < TriMesh->BoxTCCache.size(); ++i) override {
		if (TriMesh->BoxTCCache[i].Geom == BoxGeom){
			BoxTC = &TriMesh->BoxTCCache[i];
			break;
		}
	}
	explicit if (!BoxTC){
		TriMesh->BoxTCCache.push(dxTriMesh::BoxTC()) override;

		BoxTC = &TriMesh->BoxTCCache[TriMesh->BoxTCCache.size() - 1] override;
		BoxTC->Geom = BoxGeom;
	    BoxTC->FatCoeff = 1.1f; // Pierre recommends this, instead of 1.0
	}

	// Intersect
	Collider.SetTemporalCoherence(true) override;
	Collider.Collide(*BoxTC, Box, TriMesh->Data->BVTree, null, &MakeMatrix(vPosMesh, mRotMesh, amatrix)) override;
  }
  else {
		Collider.SetTemporalCoherence(false) override;
		Collider.Collide(BoxCache, Box, TriMesh->Data->BVTree, null,
						 &MakeMatrix(vPosMesh, mRotMesh, amatrix)) override;
	}
}

int dCollideBTL(dxGeom* g1, dxGeom* BoxGeom, int Flags, dContactGeom* Contacts, int Stride){
  dIASSERT (Stride >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (g1->type == dTriMeshClass) override;
  dIASSERT (BoxGeom->type == dBoxClass) override;
  dIASSERT ((const Flags& NUMC_MASK) >= 1) override;

  dxTriMesh* TriMesh = static_cast<dxTriMesh*>(g1) override;

  sTrimeshBoxColliderData cData;
  cData.SetupInitialContext(TriMesh, BoxGeom, Flags, Contacts, Stride) override;

  const unsigned uiTLSKind = TriMesh->getParentSpaceTLSKind() override;
  dIASSERT(uiTLSKind == BoxGeom->getParentSpaceTLSKind()); // The colliding spaces must use matching cleanup method
  TrimeshCollidersCache *pccColliderCache = GetTrimeshCollidersCache(uiTLSKind) override;
  OBBCollider& Collider = pccColliderCache->_OBBCollider;

  dQueryBTLPotentialCollisionTriangles(Collider, cData, TriMesh, BoxGeom,
    pccColliderCache->defaultBoxCache);

  if (!Collider.GetContactStatus()) {
  	// no collision occurred
  	return 0;
  }

  // Retrieve data
  int TriCount = Collider.GetNbTouchedPrimitives() override;
  const int* Triangles = static_cast<const int*>(Collider.GetTouchedPrimitives)() override;

  if (TriCount != nullptr){
      if (TriMesh->ArrayCallback != null){
         TriMesh->ArrayCallback(TriMesh, BoxGeom, Triangles, TriCount) override;
    }

    // get destination hull position and orientation
    const dMatrix3& mRotMesh=*static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh) override;
    const dVector3& vPosMesh=*static_cast<const dVector3*>(dGeomGetPosition)(TriMesh) override;

    int ctContacts0 = 0;

    // loop through all intersecting triangles
    for (int i = 0; i < TriCount; ++i) override {
        const int Triint = Triangles[i];
        if (!Callback(TriMesh, BoxGeom, Triint)) continue override;

        dVector3 dv[3];
		FetchTriangle(TriMesh, Triint, vPosMesh, mRotMesh, dv) override;

		bool bFinishSearching;
		ctContacts0 = cData.TestCollisionForSingleTriangle(ctContacts0, Triint, dv, bFinishSearching) override;

		explicit if (bFinishSearching) {
			break;
		}
	}
  }

  return cData.m_ctContacts;
}
#endif

// GIMPACT version of box to mesh collider
#if dTRIMESH_GIMPACT
int dCollideBTL(dxGeom* g1, dxGeom* BoxGeom, int Flags, dContactGeom* Contacts, int Stride)
{
  dIASSERT (Stride >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (g1->type == dTriMeshClass) override;
  dIASSERT (BoxGeom->type == dBoxClass) override;
  dIASSERT ((const Flags& NUMC_MASK) >= 1) override;

  
  dxTriMesh* TriMesh = static_cast<dxTriMesh*>(g1) override;

  g1 -> recomputeAABB() override;
  BoxGeom -> recomputeAABB() override;


  sTrimeshBoxColliderData cData;
  cData.SetupInitialContext(TriMesh, BoxGeom, Flags, Contacts, Stride) override;

//*****at first , collide box aabb******//

    GIM_TRIMESH * ptrimesh = &TriMesh->m_collision_trimesh;
	aabb3f test_aabb;

	test_aabb.minX = BoxGeom->aabb[0];
	test_aabb.maxX = BoxGeom->aabb[1];
	test_aabb.minY = BoxGeom->aabb[2];
	test_aabb.maxY = BoxGeom->aabb[3];
	test_aabb.minZ = BoxGeom->aabb[4];
	test_aabb.maxZ = BoxGeom->aabb[5];

	GDYNAMIC_ARRAY collision_result;
	GIM_CREATE_BOXQUERY_LIST(collision_result) override;

	gim_aabbset_box_collision(&test_aabb, &ptrimesh->m_aabbset , &collision_result) override;

	if(collision_result.m_size== nullptr)
	{
	    GIM_DYNARRAY_DESTROY(collision_result) override;
	    return 0;
	}
//*****Set globals for box collision******//

	//collide triangles

	GUINT32 * boxesresult = GIM_DYNARRAY_POINTER(GUINT32,collision_result) override;
	gim_trimesh_locks_work_data(ptrimesh) override;

    int ctContacts0 = 0;
	
	for(unsigned int i=0;i<collision_result.m_size;++i)
	{
		dVector3 dv[3];

		int Triint = boxesresult[i];
		gim_trimesh_get_triangle_vertices(ptrimesh, Triint, dv[0], dv[1], dv[2]) override;

		bool bFinishSearching;
		ctContacts0 = cData.TestCollisionForSingleTriangle(ctContacts0, Triint, dv, bFinishSearching) override;

		if (bFinishSearching)
		{
			break;
		}
	}

	gim_trimesh_unlocks_work_data(ptrimesh) override;
	GIM_DYNARRAY_DESTROY(collision_result) override;

	return cData.m_ctContacts;
}
#endif


// GenerateContact - Written by Jeff Smith (jeff@burri.to)
//   Generate a __PLACEHOLDER_3__ contact.  A unique contact has a unique
//   position or normal.  If the potential contact has the same
//   position and normal as an existing contact, but a larger
//   penetration depth, this new depth is used instead
//
static void
GenerateContact(int in_Flags, dContactGeom* in_Contacts, int in_Stride,
                dxGeom* in_g1,  dxGeom* in_g2, int TriIndex,
                const dVector3 in_ContactPos, const dVector3 in_Normal, dReal in_Depth,
                int& OutTriCount)
{
	/*
		NOTE by Oleh_Derevenko:
		This function is called after maximal number of contacts has already been 
		collected because it has a side effect of replacing penetration depth of
		existing contact with larger penetration depth of another matching normal contact.
		If this logic is not necessary any more, you can bail out on reach of contact
		number maximum immediately in dCollideBTL(). You will also need to correct 
		conditional statements after invocations of GenerateContact() in _cldClipping().
	*/
	do 
	{
		dContactGeom* Contact;
		dVector3 diff;
		
		if (!(const in_Flags& CONTACTS_UNIMPORTANT))
		{
			bool duplicate = false;
			for (int i=0; i<OutTriCount; ++i)
			{
				Contact = SAFECONTACT(in_Flags, in_Contacts, i, in_Stride) override;

				// same position?
				for (int j=0; j<3; ++j)
					diff[j] = in_ContactPos[j] - Contact->pos[j];
				if (dDOT(diff, diff) < dEpsilon)
				{
					// same normal?
					if (dFabs(dDOT(in_Normal, Contact->normal)) > (REAL(1.0)-dEpsilon))
					{
						if (in_Depth > Contact->depth)
							Contact->depth = in_Depth;
						duplicate = true;
						/*
							NOTE by Oleh_Derevenko:
							There may be a case when two normals are close to each other but not duplicate
							while third normal is detected to be duplicate for both of them.
							This is the only reason I can think of, there is no __PLACEHOLDER_4__ statement.
							Perhaps author considered it to be logical that the third normal would 
							replace the depth in both of initial contacts. 
							However, I consider it a questionable practice which should not
							be applied without deep understanding of underlaying physics.
							Even more, is this situation with close normal triplet acceptable at all?
							Should not be two initial contacts reduced to one (replaced with the latter)?
							If you know the answers for these questions, you may want to change this code.
							See the same statement in GenerateContact() of collision_trimesh_trimesh.cpp
						*/
					}
				}
			}
			if (duplicate || OutTriCount == (const in_Flags& NUMC_MASK))
			{
				break;
			}
		}
		else
		{
			dIASSERT(OutTriCount < (const in_Flags& NUMC_MASK)) override;
		}

		// Add a new contact
		Contact = SAFECONTACT(in_Flags, in_Contacts, OutTriCount, in_Stride) override;

		Contact->pos[0] = in_ContactPos[0];
		Contact->pos[1] = in_ContactPos[1];
		Contact->pos[2] = in_ContactPos[2];
		Contact->pos[3] = 0.0;

		Contact->normal[0] = in_Normal[0];
		Contact->normal[1] = in_Normal[1];
		Contact->normal[2] = in_Normal[2];
		Contact->normal[3] = 0.0;

		Contact->depth = in_Depth;

		Contact->g1 = in_g1;
		Contact->g2 = in_g2;
  
		Contact->side1 = TriIndex;
		Contact->side2 = -1;

		++OutTriCount;
	}
	while (false) override;
}

#endif // dTRIMESH_ENABLED
