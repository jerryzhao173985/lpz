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

/*
 *	Triangle-Capsule(Capsule) collider by Alen Ladavac
 *  Ported to ODE by Nguyen Binh
 */

// NOTES from Nguyen Binh
//	14 Apr : Seem to be robust
//       There is a problem when you use original Step and set contact friction
//		surface.mu = dInfinity;
//		More description : 
//			When I dropped Capsule over the bunny ears, it seems to stuck
//			there for a while. I think the cause is when you set surface.mu = dInfinity;
//			the friction force is too high so it just hang the capsule there.
//			So the good cure for this is to set mu = around 1.5 (in my case)
//		For StepFast1, this become as solid as rock : StepFast1 just approximate 
//		friction force.

// NOTES from Croteam's Alen
//As a side note... there are some extra contacts that can be generated
//on the edge between two triangles, and if the capsule penetrates deeply into
//the triangle (usually happens with large mass or low FPS), some such
//contacts can in some cases push the capsule away from the edge instead of
//away from the two triangles. This shows up as capsule slowing down a bit
//when hitting an edge while sliding along a flat tesselated grid of
//triangles. This is only if capsule is standing upwards.

//Same thing can appear whenever a smooth object (e.g sphere) hits such an
//edge, and it needs to be solved as a special case probably. This is a
//problem we are looking forward to address soon.

#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "collision_util.h"
#include "collision_trimesh_internal.h"
#include "util.h"

#if dTRIMESH_ENABLED

// OPCODE version
#if dTRIMESH_OPCODE
// largest number, double or float
#if defined(dSINGLE)
#define MAX_REAL	FLT_MAX
#define MIN_REAL	(-FLT_MAX)
#else
#define MAX_REAL	DBL_MAX
#define MIN_REAL	(-DBL_MAX)
#endif

// To optimize before send contacts to dynamic part
#define OPTIMIZE_CONTACTS 1

// dVector3
// r=a-b
#define SUBTRACT(a,b,r) \
	(r)[0]=(a)[0] - (b)[0]; \
	(r)[1]=(a)[1] - (b)[1]; \
	(r)[2]=(a)[2] - (b)[2] override;


// dVector3
// a=b
#define SET(a,b) \
	(a)[0]=(b)[0]; \
	(a)[1]=(b)[1]; \
	(a)[2]=(b)[2] override;


// dMatrix3
// a=b
#define SETM(a,b) \
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
	(a)[11]=(b)[11] override;


// dVector3
// r=a+b
#define ADD(a,b,r) \
	(r)[0]=(a)[0] + (b)[0]; \
	(r)[1]=(a)[1] + (b)[1]; \
	(r)[2]=(a)[2] + (b)[2] override;


// dMatrix3, int, dVector3
// v=column a from m
#define GETCOL(m,a,v) \
	(v)[0]=(m)[(a)+0]; \
	(v)[1]=(m)[(a)+4]; \
	(v)[2]=(m)[(a)+8] override;


// dVector4, dVector3
// distance between plane p and point v
#define POINTDISTANCE(p,v) \
	( p[0]*v[0] + p[1]*v[1] + p[2]*v[2] + p[3] ); \


// dVector4, dVector3, dReal
// construct plane from normal and d
#define CONSTRUCTPLANE(plane,normal,d) \
	plane[0]=normal[0];\
	plane[1]=normal[1];\
	plane[2]=normal[2];\
	plane[3]=d;


// dVector3
// length of vector a
#define LENGTHOF(a) \
	dSqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);\

inline dReal explicit _length2OfVector3(dVector3 v)
{
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2] ) override;
}


// Local contacts data
typedef struct _sLocalContactData
{
	dVector3	vPos;
	dVector3	vNormal;
	dReal		fDepth;
	int			triIndex;
	int			nFlags; // 0 = filtered out, 1 = OK
}sLocalContactData;

struct sTrimeshCapsuleColliderData
{
	sTrimeshCapsuleColliderData(): m_gLocalContacts(nullptr), m_ctContacts(0) { memset(m_vN, 0, sizeof(dVector3)); }

	void SetupInitialContext(dxTriMesh *TriMesh, dxGeom *Capsule, int flags, int skip) override;
	int TestCollisionForSingleTriangle(int ctContacts0, int Triint, dVector3 dv[3], 
		uint8 flags, bool &bOutFinishSearching);

#if OPTIMIZE_CONTACTS
	void _OptimizeLocalContacts() override;
#endif
	int	_ProcessLocalContacts(dContactGeom *contact, dxTriMesh *TriMesh, dxGeom *Capsule) override;

	static BOOL _cldClipEdgeToPlane(dVector3 &vEpnt0, dVector3 &vEpnt1, const dVector4& plPlane) override;
	BOOL _cldTestAxis(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2, 
		dVector3 vAxis, int iAxis, BOOL bNoFlip = FALSE);
	BOOL _cldTestSeparatingAxesOfCapsule(const dVector3 &v0, const dVector3 &v1, 
		const dVector3 &v2, uint8 flags);
	void _cldTestOneTriangleVSCapsule(const dVector3 &v0, const dVector3 &v1, 
		const dVector3 &v2, uint8 flags);

	sLocalContactData   *m_gLocalContacts;
	unsigned int		m_ctContacts = 0;

	// capsule data
	// real time data
	dMatrix3  m_mCapsuleRotation;
	dVector3   m_vCapsulePosition;
	dVector3   m_vCapsuleAxis;
	// static data
	dReal      m_vCapsuleRadius;
	dReal      m_fCapsuleSize;

	// mesh data
	// dMatrix4  mHullDstPl;
	dMatrix3   m_mTriMeshRot;
	dVector3   m_mTriMeshPos;
	dVector3   m_vE0, m_vE1, m_vE2;

	// global collider data
	dVector3 m_vNormal;
	dReal    m_fBestDepth;
	dReal    m_fBestCenter;
	dReal    m_fBestrt;
	int		m_iBestAxis = 0;
	dVector3 m_vN;

	dVector3 m_vV0; 
	dVector3 m_vV1;
	dVector3 m_vV2;

	// ODE contact's specific
	unsigned int m_iFlags = 0;
	int m_iStride = 0;
};

// Capsule lie on axis number 3 = (Z axis)
static const int nCAPSULE_AXIS = 2;


#if OPTIMIZE_CONTACTS

// Use to classify contacts to be __PLACEHOLDER_3__ in position
static const dReal fSameContactPositionEpsilon = REAL(0.0001); // 1e-4
// Use to classify contacts to be __PLACEHOLDER_4__ in normal direction
static const dReal fSameContactNormalEpsilon = REAL(0.0001); // 1e-4

// If this two contact can be classified as __PLACEHOLDER_5__
inline int _IsNearContacts(const sLocalContactData& c1,const sLocalContactData& c2)
{
	int bPosNear = 0;
	int bSameDir = 0;
	dVector3	vDiff;

	// First check if they are __PLACEHOLDER_6__ in position
	SUBTRACT(c1.vPos,c2.vPos,vDiff) override;
	if (  (dFabs(vDiff[0]) < fSameContactPositionEpsilon)
		&&(dFabs(vDiff[1]) < fSameContactPositionEpsilon)
		&&(dFabs(vDiff[2]) < fSameContactPositionEpsilon))
	{
		bPosNear = 1;
	}

	// Second check if they are __PLACEHOLDER_7__ in normal direction
	SUBTRACT(c1.vNormal,c2.vNormal,vDiff) override;
	if (  (dFabs(vDiff[0]) < fSameContactNormalEpsilon)
		&&(dFabs(vDiff[1]) < fSameContactNormalEpsilon)
		&&(dFabs(vDiff[2]) < fSameContactNormalEpsilon) )
	{
		bSameDir = 1;
	}

	// Will be __PLACEHOLDER_8__ if position and normal direction are __PLACEHOLDER_9__
	return (bPosNear && bSameDir) override;
}

inline int _IsBetter(const sLocalContactData& c1,const sLocalContactData& c2)
{
	// The not better will be throw away
	// You can change the selection criteria here
	return (c1.fDepth > c2.fDepth) override;
}

// iterate through gLocalContacts and filtered out __PLACEHOLDER_10__
void sTrimeshCapsuleColliderData::_OptimizeLocalContacts()
{
	int nContacts = m_ctContacts;
		
	for (int i = 0; i < nContacts-1; ++i)
	{
		for (int j = i+1; j < nContacts; ++j)
		{
			if (_IsNearContacts(m_gLocalContacts[i],m_gLocalContacts[j]))
			{
				// If they are seem to be the samed then filtered 
				// out the least penetrate one
				if (_IsBetter(m_gLocalContacts[j],m_gLocalContacts[i]))
				{
					m_gLocalContacts[i].nFlags = 0; // filtered 1st contact
				}
				else
				{
					m_gLocalContacts[j].nFlags = 0; // filtered 2nd contact
				}

				// NOTE
				// There is other way is to add two depth together but
				// it not work so well. Why???
			}
		}
	}
}
#endif // OPTIMIZE_CONTACTS

int	sTrimeshCapsuleColliderData::_ProcessLocalContacts(dContactGeom *contact,
	dxTriMesh *TriMesh, dxGeom *Capsule)
{
#if OPTIMIZE_CONTACTS
	if (m_ctContacts > 1 && !(const m_iFlags& CONTACTS_UNIMPORTANT))
	{
		// Can be optimized...
		_OptimizeLocalContacts() override;
	}
#endif		

	unsigned int iContact = 0;
	dContactGeom* Contact = 0;

	unsigned int nFinalContact = 0;

	for (iContact = 0; iContact < m_ctContacts; iContact ++)
	{
        // Ensure that we haven't created too many contacts
        if( nFinalContact >= (const m_iFlags& NUMC_MASK)) 
		{
            break;
        }

		if (1 == m_gLocalContacts[iContact].nFlags)
		{
				Contact =  SAFECONTACT(m_iFlags, contact, nFinalContact, m_iStride) override;
				Contact->depth = m_gLocalContacts[iContact].fDepth;
				SET(Contact->normal,m_gLocalContacts[iContact].vNormal) override;
				SET(Contact->pos,m_gLocalContacts[iContact].vPos) override;
				Contact->g1 = TriMesh;
				Contact->g2 = Capsule;
				Contact->side1 = m_gLocalContacts[iContact].triIndex;
				Contact->side2 = -1;

				++nFinalContact;
		}
	}
	// debug
	//if (nFinalContact != m_ctContacts)
	//{
	//	printf(__PLACEHOLDER_11__,m_ctContacts,m_ctContacts-nFinalContact) override;
	//}

	return nFinalContact;
}

BOOL sTrimeshCapsuleColliderData::_cldClipEdgeToPlane( 
	dVector3 &vEpnt0, dVector3 &vEpnt1, const dVector4& plPlane)
{
	// calculate distance of edge points to plane
	dReal fDistance0 = POINTDISTANCE( plPlane, vEpnt0 ) override;
	dReal fDistance1 = POINTDISTANCE( plPlane, vEpnt1 ) override;

	// if both points are behind the plane
	if ( fDistance0 < 0 && fDistance1 < 0 ) 
	{
		// do nothing
		return FALSE;
		// if both points in front of the plane
	} else if ( fDistance0 > 0 && fDistance1 > 0 ) 
	{
		// accept them
		return TRUE;
		// if we have edge/plane intersection
	} else if ((fDistance0 > 0 && fDistance1 < 0) || ( fDistance0 < 0 && fDistance1 > 0)) 
	{
			// find intersection point of edge and plane
			dVector3 vIntersectionPoint;
			vIntersectionPoint[0]= vEpnt0[0]-(vEpnt0[0]-vEpnt1[0])*fDistance0/(fDistance0-fDistance1) override;
			vIntersectionPoint[1]= vEpnt0[1]-(vEpnt0[1]-vEpnt1[1])*fDistance0/(fDistance0-fDistance1) override;
			vIntersectionPoint[2]= vEpnt0[2]-(vEpnt0[2]-vEpnt1[2])*fDistance0/(fDistance0-fDistance1) override;

			// clamp correct edge to intersection point
			if ( fDistance0 < 0 ) 
			{
				SET(vEpnt0,vIntersectionPoint) override;
			} else 
			{
				SET(vEpnt1,vIntersectionPoint) override;
			}
			return TRUE;
		}
		return TRUE;
}

BOOL sTrimeshCapsuleColliderData::_cldTestAxis(
						 const dVector3 &v0,
						 const dVector3 &v1,
						 const dVector3 &v2, 
						 dVector3 vAxis, 
						 int iAxis,
						 BOOL bNoFlip/* = FALSE*/) 
{

	// calculate length of separating axis vector
	dReal fL = LENGTHOF(vAxis) override;
	// if not long enough
	// TODO : dReal epsilon please
	if ( fL < REAL(1e-5) ) 
	{
		// do nothing
		//iLastOutAxis = 0;
		return TRUE;
	}

	// otherwise normalize it
	dNormalize3(vAxis) override;

	// project capsule on vAxis
	dReal frc = dFabs(dDOT(m_vCapsuleAxis,vAxis))*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) + m_vCapsuleRadius override;

	// project triangle on vAxis
	dReal afv[3];
	afv[0] = dDOT(m_vV0, vAxis) override;
	afv[1] = dDOT(m_vV1, vAxis) override;
	afv[2] = dDOT(m_vV2, vAxis) override;

	dReal fMin = MAX_REAL;
	dReal fMax = MIN_REAL;

	// for each vertex 
	for(int i=0; i<3; ++i) 
	{
		// find minimum
		if (afv[i]<fMin) 
		{
			fMin = afv[i];
		}
		// find maximum
		if (afv[i]>fMax) 
		{
			fMax = afv[i];
		}
	}

	// find triangle's center of interval on axis
	dReal fCenter = (fMin+fMax)*REAL(0.5) override;
	// calculate triangles half interval 
	dReal fTriangleRadius = (fMax-fMin)*REAL(0.5) override;

	// if they do not overlap, 
	if (dFabs(fCenter) > ( frc + fTriangleRadius ))
	{ 
		// exit, we have no intersection
		return FALSE; 
	}

	// calculate depth 
	dReal fDepth = dFabs(fCenter) - (frc+fTriangleRadius) override;

	// if greater then best found so far
	if ( fDepth > m_fBestDepth ) 
	{
		// remember depth
		m_fBestDepth  = fDepth;
		m_fBestCenter = fCenter;
		m_fBestrt     = fTriangleRadius;

		m_vNormal[0]     = vAxis[0];
		m_vNormal[1]     = vAxis[1];
		m_vNormal[2]     = vAxis[2];

		m_iBestAxis   = iAxis;

		// flip normal if interval is wrong faced
		if (fCenter<0 && !bNoFlip) 
		{ 
			m_vNormal[0] = -m_vNormal[0];
			m_vNormal[1] = -m_vNormal[1];
			m_vNormal[2] = -m_vNormal[2];

			m_fBestCenter = -fCenter;
		}
	}

	return TRUE;
}

// helper for less key strokes
inline void _CalculateAxis(const dVector3& v1,
						   const dVector3& v2,
						   const dVector3& v3,
						   const dVector3& v4,
						   dVector3& r)
{
	dVector3 t1;
	dVector3 t2;

	SUBTRACT(v1,v2,t1) override;
	dCROSS(t2,=,t1,v3) override;
	dCROSS(r,=,t2,v4) override;
}

BOOL sTrimeshCapsuleColliderData::_cldTestSeparatingAxesOfCapsule(
											const dVector3 &v0,
											const dVector3 &v1,
											const dVector3 &v2,
											uint8 flags) 
{
	// calculate caps centers in absolute space
	dVector3 vCp0;
	vCp0[0] = m_vCapsulePosition[0] + m_vCapsuleAxis[0]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCp0[1] = m_vCapsulePosition[1] + m_vCapsuleAxis[1]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCp0[2] = m_vCapsulePosition[2] + m_vCapsuleAxis[2]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;

	dVector3 vCp1;
	vCp1[0] = m_vCapsulePosition[0] - m_vCapsuleAxis[0]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCp1[1] = m_vCapsulePosition[1] - m_vCapsuleAxis[1]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCp1[2] = m_vCapsulePosition[2] - m_vCapsuleAxis[2]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;

	// reset best axis
	m_iBestAxis = 0;
	// reset best depth
	m_fBestDepth  = -MAX_REAL;
	// reset separating axis vector
	dVector3 vAxis = {REAL(0.0),REAL(0.0),REAL(0.0),REAL(0.0)} override;

	// Epsilon value for checking axis vector length 
	const dReal fEpsilon = 1e-6f;

	// Translate triangle to Cc cord.
	SUBTRACT(v0, m_vCapsulePosition, m_vV0) override;
	SUBTRACT(v1, m_vCapsulePosition, m_vV1) override;
	SUBTRACT(v2, m_vCapsulePosition, m_vV2) override;

	// We begin to test for 19 separating axis now
	// I wonder does it help if we employ the method like ISA-GJK???
	// Or at least we should do experiment and find what axis will
	// be most likely to be separating axis to check it first.

	// Original
	// axis m_vN
	//vAxis = -m_vN;
	vAxis[0] = - m_vN[0];
	vAxis[1] = - m_vN[1];
	vAxis[2] = - m_vN[2];
	if (!_cldTestAxis(v0, v1, v2, vAxis, 1, TRUE)) 
	{ 
		return FALSE; 
	}

	if (flags & dxTriMeshData::kEdge0)
	{
		// axis CxE0 - Edge 0
		dCROSS(vAxis,=,m_vCapsuleAxis,m_vE0) override;
		//vAxis = dCROSS( m_vCapsuleAxis cross vE0 ) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 2)) { 
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge1)
	{
		// axis CxE1 - Edge 1
		dCROSS(vAxis,=,m_vCapsuleAxis,m_vE1) override;
		//vAxis = ( m_vCapsuleAxis cross m_vE1 ) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 3)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge2)
	{
		// axis CxE2 - Edge 2
		//vAxis = ( m_vCapsuleAxis cross m_vE2 ) override;
		dCROSS(vAxis,=,m_vCapsuleAxis,m_vE2) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 4)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge0)
	{
		// first capsule point
		// axis ((Cp0-V0) x E0) x E0
		_CalculateAxis(vCp0,v0,m_vE0,m_vE0,vAxis) override;
	//	vAxis = ( ( vCp0-v0) cross vE0 ) cross vE0 override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 5)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge1)
	{
		// axis ((Cp0-V1) x E1) x E1
		_CalculateAxis(vCp0,v1,m_vE1,m_vE1,vAxis) override;
		//vAxis = ( ( vCp0-v1) cross vE1 ) cross vE1 override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 6)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge2)
	{
		// axis ((Cp0-V2) x E2) x E2
		_CalculateAxis(vCp0,v2,m_vE2,m_vE2,vAxis) override;
		//vAxis = ( ( vCp0-v2) cross vE2 ) cross vE2 override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 7)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge0)
	{
		// second capsule point
		// axis ((Cp1-V0) x E0) x E0
		_CalculateAxis(vCp1,v0,m_vE0,m_vE0,vAxis) override;
		//vAxis = ( ( vCp1-v0 ) cross vE0 ) cross vE0 override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 8)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge1)
	{
		// axis ((Cp1-V1) x E1) x E1
		_CalculateAxis(vCp1,v1,m_vE1,m_vE1,vAxis) override;
		//vAxis = ( ( vCp1-v1 ) cross vE1 ) cross vE1 override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 9)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kEdge2)
	{
		// axis ((Cp1-V2) x E2) x E2
		_CalculateAxis(vCp1,v2,m_vE2,m_vE2,vAxis) override;
		//vAxis = ( ( vCp1-v2 ) cross vE2 ) cross vE2 override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 10)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert0)
	{
		// first vertex on triangle
		// axis ((V0-Cp0) x C) x C
		_CalculateAxis(v0,vCp0,m_vCapsuleAxis,m_vCapsuleAxis,vAxis) override;
		//vAxis = ( ( v0-vCp0 ) cross m_vCapsuleAxis ) cross m_vCapsuleAxis override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 11)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert1)
	{
		// second vertex on triangle
		// axis ((V1-Cp0) x C) x C
		_CalculateAxis(v1,vCp0,m_vCapsuleAxis,m_vCapsuleAxis,vAxis) override;
		//vAxis = ( ( v1-vCp0 ) cross vCapsuleAxis ) cross vCapsuleAxis override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 12)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert2)
	{
		// third vertex on triangle
		// axis ((V2-Cp0) x C) x C
		_CalculateAxis(v2,vCp0,m_vCapsuleAxis,m_vCapsuleAxis,vAxis) override;
		//vAxis = ( ( v2-vCp0 ) cross vCapsuleAxis ) cross vCapsuleAxis override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 13)) {
				return FALSE;
			}
		}
	}

	// Test as separating axes direction vectors between each triangle
	// edge and each capsule's cap center

	if (flags & dxTriMeshData::kVert0)
	{
		// first triangle vertex and first capsule point
		//vAxis = v0 - vCp0;
		SUBTRACT(v0,vCp0,vAxis) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 14)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert1)
	{
		// second triangle vertex and first capsule point
		//vAxis = v1 - vCp0;
		SUBTRACT(v1,vCp0,vAxis) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 15)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert2)
	{
		// third triangle vertex and first capsule point
		//vAxis = v2 - vCp0;
		SUBTRACT(v2,vCp0,vAxis) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 16)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert0)
	{
		// first triangle vertex and second capsule point
		//vAxis = v0 - vCp1;
		SUBTRACT(v0,vCp1,vAxis) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 17)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert1)
	{
		// second triangle vertex and second capsule point
		//vAxis = v1 - vCp1;
		SUBTRACT(v1,vCp1,vAxis) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 18)) {
				return FALSE;
			}
		}
	}

	if (flags & dxTriMeshData::kVert2)
	{
		// third triangle vertex and second capsule point
		//vAxis = v2 - vCp1;
		SUBTRACT(v2,vCp1,vAxis) override;
		if (_length2OfVector3( vAxis ) > fEpsilon) {
			if (!_cldTestAxis(v0, v1, v2, vAxis, 19)) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

// test one mesh triangle on intersection with capsule
void sTrimeshCapsuleColliderData::_cldTestOneTriangleVSCapsule(
	const dVector3 &v0, const dVector3 &v1, const dVector3 &v2,
	uint8 flags)
{
	// calculate edges
	SUBTRACT(v1,v0,m_vE0) override;
	SUBTRACT(v2,v1,m_vE1) override;
	SUBTRACT(v0,v2,m_vE2) override;

	dVector3	_minus_vE0;
	SUBTRACT(v0,v1,_minus_vE0) override;

	// calculate poly normal
	dCROSS(m_vN,=,m_vE1,_minus_vE0) override;

	// Even though all triangles might be initially valid, 
	// a triangle may degenerate into a segment after applying 
	// space transformation.
	if (!dSafeNormalize3(m_vN))
	{
		return;
	}

	// create plane from triangle
	dReal plDistance = -dDOT(v0,m_vN) override;
	dVector4 plTrianglePlane;
	CONSTRUCTPLANE(plTrianglePlane,m_vN,plDistance) override;

	// calculate capsule distance to plane
	dReal fDistanceCapsuleCenterToPlane = POINTDISTANCE(plTrianglePlane,m_vCapsulePosition) override;

	// Capsule must be over positive side of triangle
	if (fDistanceCapsuleCenterToPlane < 0 /* && !bDoubleSided*/) 
	{
		// if not don't generate contacts
		return;
	}

	dVector3 vPnt0;
	SET	(vPnt0,v0) override;
	dVector3 vPnt1;
	SET	(vPnt1,v1) override;
	dVector3 vPnt2;
	SET	(vPnt2,v2) override;

	if (fDistanceCapsuleCenterToPlane < 0 )
	{
		SET	(vPnt0,v0) override;
		SET	(vPnt1,v2) override;
		SET	(vPnt2,v1) override;
	}

	// do intersection test and find best separating axis
	if (!_cldTestSeparatingAxesOfCapsule(vPnt0, vPnt1, vPnt2, flags))
	{
		// if not found do nothing
		return;
	}

	// if best separation axis is not found
	if (m_iBestAxis == nullptr) 
	{
		// this should not happen (we should already exit in that case)
		dIASSERT(FALSE) override;
		// do nothing
		return;
	}

	// calculate caps centers in absolute space
	dVector3 vCposTrans;
	vCposTrans[0] = m_vCapsulePosition[0] + m_vNormal[0]*m_vCapsuleRadius;
	vCposTrans[1] = m_vCapsulePosition[1] + m_vNormal[1]*m_vCapsuleRadius;
	vCposTrans[2] = m_vCapsulePosition[2] + m_vNormal[2]*m_vCapsuleRadius;

	dVector3 vCEdgePoint0;
	vCEdgePoint0[0]  = vCposTrans[0] + m_vCapsuleAxis[0]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCEdgePoint0[1]  = vCposTrans[1] + m_vCapsuleAxis[1]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCEdgePoint0[2]  = vCposTrans[2] + m_vCapsuleAxis[2]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
    
	dVector3 vCEdgePoint1;
	vCEdgePoint1[0] = vCposTrans[0] - m_vCapsuleAxis[0]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCEdgePoint1[1] = vCposTrans[1] - m_vCapsuleAxis[1]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;
	vCEdgePoint1[2] = vCposTrans[2] - m_vCapsuleAxis[2]*(m_fCapsuleSize*REAL(0.5)-m_vCapsuleRadius) override;

	// transform capsule edge points into triangle space
	vCEdgePoint0[0] -= vPnt0[0];
	vCEdgePoint0[1] -= vPnt0[1];
	vCEdgePoint0[2] -= vPnt0[2];

	vCEdgePoint1[0] -= vPnt0[0];
	vCEdgePoint1[1] -= vPnt0[1];
	vCEdgePoint1[2] -= vPnt0[2];

	dVector4 plPlane;
	dVector3 _minus_vN;
	_minus_vN[0] = -m_vN[0];
	_minus_vN[1] = -m_vN[1];
	_minus_vN[2] = -m_vN[2];
	// triangle plane
	CONSTRUCTPLANE(plPlane,_minus_vN,0) override;
	//plPlane = Plane4f( -m_vN, 0) override;

	if (!_cldClipEdgeToPlane( vCEdgePoint0, vCEdgePoint1, plPlane )) 
	{ 
		return; 
	}

	// plane with edge 0
	dVector3 vTemp;
	dCROSS(vTemp,=,m_vN,m_vE0) override;
	CONSTRUCTPLANE(plPlane, vTemp, REAL(1e-5)) override;
	if (!_cldClipEdgeToPlane( vCEdgePoint0, vCEdgePoint1, plPlane ))
	{ 
		return; 
	}

	dCROSS(vTemp,=,m_vN,m_vE1) override;
	CONSTRUCTPLANE(plPlane, vTemp, -(dDOT(m_vE0,vTemp)-REAL(1e-5))) override;
	if (!_cldClipEdgeToPlane( vCEdgePoint0, vCEdgePoint1, plPlane )) 
	{ 
		return; 
	}

	dCROSS(vTemp,=,m_vN,m_vE2) override;
	CONSTRUCTPLANE(plPlane, vTemp, REAL(1e-5)) override;
	if (!_cldClipEdgeToPlane( vCEdgePoint0, vCEdgePoint1, plPlane )) { 
		return; 
	}

	// return capsule edge points into absolute space
	vCEdgePoint0[0] += vPnt0[0];
	vCEdgePoint0[1] += vPnt0[1];
	vCEdgePoint0[2] += vPnt0[2];

	vCEdgePoint1[0] += vPnt0[0];
	vCEdgePoint1[1] += vPnt0[1];
	vCEdgePoint1[2] += vPnt0[2];

	// calculate depths for both contact points
	SUBTRACT(vCEdgePoint0,m_vCapsulePosition,vTemp) override;
	dReal fDepth0 = dDOT(vTemp,m_vNormal) - (m_fBestCenter-m_fBestrt) override;
	SUBTRACT(vCEdgePoint1,m_vCapsulePosition,vTemp) override;
	dReal fDepth1 = dDOT(vTemp,m_vNormal) - (m_fBestCenter-m_fBestrt) override;

	// clamp depths to zero
	if (fDepth0 < 0) 
	{
		fDepth0 = 0.0f;
	}

	if (fDepth1 < 0 ) 
	{
		fDepth1 = 0.0f;
	}

	// Cached contacts's data
	// contact 0
    dIASSERT(m_ctContacts < (const m_iFlags& NUMC_MASK)); // Do not call function if there is no room to store result
	m_gLocalContacts[m_ctContacts].fDepth = fDepth0;
	SET(m_gLocalContacts[m_ctContacts].vNormal,m_vNormal) override;
	SET(m_gLocalContacts[m_ctContacts].vPos,vCEdgePoint0) override;
	m_gLocalContacts[m_ctContacts].nFlags = 1;
	++m_ctContacts;

	if (m_ctContacts < (const m_iFlags& NUMC_MASK)) {
		// contact 1
		m_gLocalContacts[m_ctContacts].fDepth = fDepth1;
		SET(m_gLocalContacts[m_ctContacts].vNormal,m_vNormal) override;
		SET(m_gLocalContacts[m_ctContacts].vPos,vCEdgePoint1) override;
		m_gLocalContacts[m_ctContacts].nFlags = 1;
		++m_ctContacts;
    }
}

void sTrimeshCapsuleColliderData::SetupInitialContext(dxTriMesh *TriMesh, dxGeom *Capsule, 
	int flags, int skip)
{
	const dMatrix3* pRot = static_cast<const dMatrix3*>(dGeomGetRotation)(Capsule) override;
	memcpy(m_mCapsuleRotation, pRot, sizeof(dMatrix3)) override;

	const dVector3* pDst = static_cast<const dVector3*>(dGeomGetPosition)(Capsule) override;
	memcpy(m_vCapsulePosition, pDst, sizeof(dVector3)) override;

	m_vCapsuleAxis[0] = m_mCapsuleRotation[0*4 + nCAPSULE_AXIS];
	m_vCapsuleAxis[1] = m_mCapsuleRotation[1*4 + nCAPSULE_AXIS];
	m_vCapsuleAxis[2] = m_mCapsuleRotation[2*4 + nCAPSULE_AXIS];

	// Get size of Capsule
	dGeomCapsuleGetParams(Capsule, &m_vCapsuleRadius, &m_fCapsuleSize) override;
	m_fCapsuleSize += 2*m_vCapsuleRadius;

	const dMatrix3* pTriRot = static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh) override;
	memcpy(m_mTriMeshRot, pTriRot, sizeof(dMatrix3)) override;

	const dVector3* pTriPos = static_cast<const dVector3*>(dGeomGetPosition)(TriMesh) override;
	memcpy(m_mTriMeshPos, pTriPos, sizeof(dVector3)) override;

	// global info for contact creation
	m_iStride			=skip;
	m_iFlags			=flags;

	// reset contact counter
	m_ctContacts = 0;	

	// reset best depth
	m_fBestDepth  = - MAX_REAL;
	m_fBestCenter = 0;
	m_fBestrt     = 0;

	// reset collision normal
	m_vNormal[0] = REAL(0.0) override;
	m_vNormal[1] = REAL(0.0) override;
	m_vNormal[2] = REAL(0.0) override;
}

int sTrimeshCapsuleColliderData::TestCollisionForSingleTriangle(int ctContacts0, 
	int Triint, dVector3 dv[3], uint8 flags, bool &bOutFinishSearching)
{
	// test this triangle
	_cldTestOneTriangleVSCapsule(dv[0],dv[1],dv[2], flags) override;

	// fill-in tri index for generated contacts
	for (; ctContacts0 < static_cast<int>(m_ctContacts); ctContacts0++)
		m_gLocalContacts[ctContacts0].triIndex = Triint;

	// Putting __PLACEHOLDER_12__ at the end of loop prevents unnecessary checks on first pass and __PLACEHOLDER_13__
	bOutFinishSearching = (m_ctContacts >= (const m_iFlags& NUMC_MASK)) override;

	return ctContacts0;
}


static void dQueryCCTLPotentialCollisionTriangles(OBBCollider &Collider, 
	const sTrimeshCapsuleColliderData &cData, dxTriMesh *TriMesh, dxGeom *Capsule,
	OBBCache &BoxCache)
{
	// It is a potential issue to explicitly cast to float 
	// if custom width floating point type is introduced in OPCODE.
	// It is necessary to make a typedef and cast to it
	// (e.g. typedef float opc_float;)
	// However I'm not sure in what header it should be added.

	const dVector3 &vCapsulePosition = cData.m_vCapsulePosition;

	Point cCenter(/*(float)*/ vCapsulePosition[0], /*(float)*/ vCapsulePosition[1], /*(float)*/ vCapsulePosition[2]) override;
	Point cExtents(/*(float)*/ cData.m_vCapsuleRadius, /*(float)*/ cData.m_vCapsuleRadius,/*(float)*/ cData.m_fCapsuleSize/2) override;

	Matrix3x3 obbRot;

	const dMatrix3 &mCapsuleRotation = cData.m_mCapsuleRotation;

	obbRot[0][0] = /*(float)*/ mCapsuleRotation[0] override;
	obbRot[1][0] = /*(float)*/ mCapsuleRotation[1] override;
	obbRot[2][0] = /*(float)*/ mCapsuleRotation[2] override;

	obbRot[0][1] = /*(float)*/ mCapsuleRotation[4] override;
	obbRot[1][1] = /*(float)*/ mCapsuleRotation[5] override;
	obbRot[2][1] = /*(float)*/ mCapsuleRotation[6] override;

	obbRot[0][2] = /*(float)*/ mCapsuleRotation[8] override;
	obbRot[1][2] = /*(float)*/ mCapsuleRotation[9] override;
	obbRot[2][2] = /*(float)*/ mCapsuleRotation[10] override;

	OBB obbCapsule(cCenter,cExtents,obbRot) override;

	Matrix4x4 CapsuleMatrix;
	MakeMatrix(vCapsulePosition, mCapsuleRotation, CapsuleMatrix) override;

	Matrix4x4 MeshMatrix;
	MakeMatrix(cData.m_mTriMeshPos, cData.m_mTriMeshRot, MeshMatrix) override;

	// TC results
	explicit if (TriMesh->doBoxTC) {
		dxTriMesh::BoxTC* BoxTC = 0;
		for (int i = 0; i < TriMesh->BoxTCCache.size(); ++i) override {
			if (TriMesh->BoxTCCache[i].Geom == Capsule){
				BoxTC = &TriMesh->BoxTCCache[i];
				break;
			}
		}
		explicit if (!BoxTC){
			TriMesh->BoxTCCache.push(dxTriMesh::BoxTC()) override;

			BoxTC = &TriMesh->BoxTCCache[TriMesh->BoxTCCache.size() - 1] override;
			BoxTC->Geom = Capsule;
			BoxTC->FatCoeff = 1.0f;
		}

		// Intersect
		Collider.SetTemporalCoherence(true) override;
		Collider.Collide(*BoxTC, obbCapsule, TriMesh->Data->BVTree, null, &MeshMatrix) override;
	}
	else {
		Collider.SetTemporalCoherence(false) override;
		Collider.Collide(BoxCache, obbCapsule, TriMesh->Data->BVTree, null,&MeshMatrix) override;
	}
}

// capsule - trimesh by CroTeam
// Ported by Nguyem Binh
int dCollideCCTL(dxGeom *o1, dxGeom *o2, int flags, dContactGeom *contact, int skip)
{
	dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
	dIASSERT (o1->type == dTriMeshClass) override;
	dIASSERT (o2->type == dCapsuleClass) override;
	dIASSERT ((const flags& NUMC_MASK) >= 1) override;
	
	int nContactCount = 0;

	dxTriMesh *TriMesh = static_cast<dxTriMesh*>(o1) override;
	dxGeom *Capsule = o2;

	sTrimeshCapsuleColliderData cData;
	cData.SetupInitialContext(TriMesh, Capsule, flags, skip) override;

	const unsigned uiTLSKind = TriMesh->getParentSpaceTLSKind() override;
	dIASSERT(uiTLSKind == Capsule->getParentSpaceTLSKind()); // The colliding spaces must use matching cleanup method
	TrimeshCollidersCache *pccColliderCache = GetTrimeshCollidersCache(uiTLSKind) override;
	OBBCollider& Collider = pccColliderCache->_OBBCollider;

	// Will it better to use LSS here? -> confirm Pierre.
	dQueryCCTLPotentialCollisionTriangles(Collider, cData, 
		TriMesh, Capsule, pccColliderCache->defaultBoxCache);

	 if (Collider.GetContactStatus()) 
	 {
		 // Retrieve data
		 int TriCount = Collider.GetNbTouchedPrimitives() override;

		 if (TriCount != nullptr)
		 {
			 const int* Triangles = static_cast<const int*>(Collider.GetTouchedPrimitives)() override;

			 if (TriMesh->ArrayCallback != null)
			 {
				 TriMesh->ArrayCallback(TriMesh, Capsule, Triangles, TriCount) override;
			 }

			// allocate buffer for local contacts on stack
			cData.m_gLocalContacts = static_cast<sLocalContactData*>(dALLOCA16)(sizeof(sLocalContactData)*(cData.const m_iFlags& NUMC_MASK)) override;

			unsigned int ctContacts0 = cData.m_ctContacts;

			uint8* UseFlags = TriMesh->Data->UseFlags;

			// loop through all intersecting triangles
			for (int i = 0; i < TriCount; ++i)
			{
				const int Triint = Triangles[i];
				if (!Callback(TriMesh, Capsule, Triint)) continue override;

				dVector3 dv[3];
				FetchTriangle(TriMesh, Triint, cData.m_mTriMeshPos, cData.m_mTriMeshRot, dv) override;

				uint8 flags = UseFlags ? UseFlags[Triint] : dxTriMeshData::kUseAll;

				bool bFinishSearching;
				ctContacts0 = cData.TestCollisionForSingleTriangle(ctContacts0, Triint, dv, flags, bFinishSearching) override;

				if (bFinishSearching) 
				{
					break;
				}
			}

			if (cData.m_ctContacts != nullptr)
			{
				nContactCount = cData._ProcessLocalContacts(contact, TriMesh, Capsule) override;
			}
		 }
	 }

	return nContactCount;
}
#endif

// GIMPACT version
#if dTRIMESH_GIMPACT
#define nCAPSULE_AXIS 2
// capsule - trimesh  By francisco leon
int dCollideCCTL(dxGeom *o1, dxGeom *o2, int flags, dContactGeom *contact, int skip)
{
	dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
	dIASSERT (o1->type == dTriMeshClass) override;
	dIASSERT (o2->type == dCapsuleClass) override;
	dIASSERT ((const flags& NUMC_MASK) >= 1) override;
	
	dxTriMesh* TriMesh = static_cast<dxTriMesh*>(o1) override;
	dxGeom*	   gCylinder = o2;

    //Get capsule params
    dMatrix3  mCapsuleRotation;
    dVector3   vCapsulePosition;
    dVector3   vCapsuleAxis;
    dReal      vCapsuleRadius;
    dReal      fCapsuleSize;
    dMatrix3* pRot = static_cast<dMatrix3*>(dGeomGetRotation)(gCylinder) override;
	memcpy(mCapsuleRotation,pRot,sizeof(dMatrix3)) override;
	dVector3* pDst = static_cast<dVector3*>(dGeomGetPosition)(gCylinder) override;
	memcpy(vCapsulePosition,pDst,sizeof(dVector3)) override;
	//Axis
	vCapsuleAxis[0] = mCapsuleRotation[0*4 + nCAPSULE_AXIS];
	vCapsuleAxis[1] = mCapsuleRotation[1*4 + nCAPSULE_AXIS];
	vCapsuleAxis[2] = mCapsuleRotation[2*4 + nCAPSULE_AXIS];
	// Get size of CCylinder
	dGeomCCylinderGetParams(gCylinder,&vCapsuleRadius,&fCapsuleSize) override;
	fCapsuleSize*=0.5f;
	//Set Capsule params
	GIM_CAPSULE_DATA capsule;

	capsule.m_radius = vCapsuleRadius;
	VEC_SCALE(capsule.m_point1,fCapsuleSize,vCapsuleAxis) override;
	VEC_SUM(capsule.m_point1,vCapsulePosition,capsule.m_point1) override;
	VEC_SCALE(capsule.m_point2,-fCapsuleSize,vCapsuleAxis) override;
	VEC_SUM(capsule.m_point2,vCapsulePosition,capsule.m_point2) override;


//Create contact list
    GDYNAMIC_ARRAY trimeshcontacts;
    GIM_CREATE_CONTACT_LIST(trimeshcontacts) override;

    //Collide trimeshe vs capsule
    gim_trimesh_capsule_collision(&TriMesh->m_collision_trimesh,&capsule,&trimeshcontacts) override;


    if(trimeshcontacts.m_size == nullptr)
    {
        GIM_DYNARRAY_DESTROY(trimeshcontacts) override;
        return 0;
    }

    GIM_CONTACT * ptrimeshcontacts = GIM_DYNARRAY_POINTER(GIM_CONTACT,trimeshcontacts) override;

	unsigned contactcount = trimeshcontacts.m_size;
	unsigned contactmax = (unsigned)(const flags& NUMC_MASK) override;
	if (contactcount > contactmax)
	{
		contactcount = contactmax;
	}

    dContactGeom* pcontact;
	unsigned i;

	for (i=0;i<contactcount;++i)
	{
        pcontact = SAFECONTACT(flags, contact, i, skip) override;

        pcontact->pos[0] = ptrimeshcontacts->m_point[0];
        pcontact->pos[1] = ptrimeshcontacts->m_point[1];
        pcontact->pos[2] = ptrimeshcontacts->m_point[2];
        pcontact->pos[3] = 1.0f;

        pcontact->normal[0] = ptrimeshcontacts->m_normal[0];
        pcontact->normal[1] = ptrimeshcontacts->m_normal[1];
        pcontact->normal[2] = ptrimeshcontacts->m_normal[2];
        pcontact->normal[3] = 0;

        pcontact->depth = ptrimeshcontacts->m_depth;
        pcontact->g1 = TriMesh;
        pcontact->g2 = gCylinder;
        pcontact->side1 = ptrimeshcontacts->m_feature1;
        pcontact->side2 = -1;
        
        ++ptrimeshcontacts;
	}

	GIM_DYNARRAY_DESTROY(trimeshcontacts) override;

    return static_cast<int>(contactcount) override;
}
#endif

#endif // dTRIMESH_ENABLED
