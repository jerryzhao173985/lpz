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

// OPCODE TriMesh/TriMesh collision code by Jeff Smith (c) 2004

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "config.h"
// Classic Implementation
#if dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER

#if dTRIMESH_ENABLED

#include "collision_util.h"
#include "collision_trimesh_internal.h"


#if !dTLS_ENABLED
// Have collider cache instance unconditionally of OPCODE or GIMPACT selection
/*extern */TrimeshCollidersCache g_ccTrimeshCollidersCache;
#endif


#if dTRIMESH_OPCODE

#define SMALL_ELT           REAL(2.5e-4)
#define EXPANDED_ELT_THRESH REAL(1.0e-3)
#define DISTANCE_EPSILON    REAL(1.0e-8)
#define VELOCITY_EPSILON    REAL(1.0e-5)
#define TINY_PENETRATION    REAL(5.0e-6)

struct LineContactSet
{
	enum
	{
		MAX_POINTS = 8
	};

    dVector3 Points[MAX_POINTS];
    int      Count;
};


// static void GetTriangleGeometryCallback(udword, VertexPointers&, udword); -- not used
static void GenerateContact(int, dContactGeom*, int, dxTriMesh*,  dxTriMesh*, 
							int TriIndex1, int TriIndex2,
                            const dVector3, const dVector3, dReal, int&);
static int TriTriIntersectWithIsectLine(dReal V0[3],dReal V1[3],dReal V2[3],
                                        dReal U0[3],dReal U1[3],dReal U2[3],int *coplanar,
                                        dReal isectpt1[3],dReal isectpt2[3]);
inline void dMakeMatrix4(const dVector3 Position, const dMatrix3 Rotation, dMatrix4 &B) override;
static void dInvertMatrix4( const dMatrix4& B, const dMatrix4& Binv ) override;
//static int IntersectLineSegmentRay(dVector3, dVector3, dVector3, dVector3,  dVector3) override;
static bool FindTriSolidIntrsection(const dVector3 Tri[3], 
                                    const dVector4 Planes[6], int numSides,
                                    LineContactSet& ClippedPolygon );
static void ClipConvexPolygonAgainstPlane( const dVector3, dReal, LineContactSet& ) override;
static bool SimpleUnclippedTest(dVector3 in_CoplanarPt, dVector3 in_v, dVector3 in_elt,
                                dVector3 in_n, dVector3* in_col_v, dReal &out_depth);
static int ExamineContactPoint(dVector3* v_col, dVector3 in_n, dVector3 in_point) override;
static int RayTriangleIntersect(const dVector3 orig, const dVector3 dir,
                                const dVector3 vert0, const dVector3 vert1,const dVector3 vert2,
                                dReal *t,dReal *u,dReal *v);




/* some math macros */
#define IS_ZERO(v) (!(v)[0] && !(v)[1] && !(v)[2])

#define CROSS(dest,v1,v2) { dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
                            dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
                            dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; }

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) { dest[0]=v1[0]-v2[0]; dest[1]=v1[1]-v2[1]; dest[2]=v1[2]-v2[2]; }

#define ADD(dest,v1,v2) { dest[0]=v1[0]+v2[0]; dest[1]=v1[1]+v2[1]; dest[2]=v1[2]+v2[2]; }

#define MULT(dest,v,factor) { dest[0]=factor*v[0]; dest[1]=factor*v[1]; dest[2]=factor*v[2]; }

#define SET(dest,src) { dest[0]=src[0]; dest[1]=src[1]; dest[2]=src[2]; }

#define SMULT(p,q,s) { p[0]=q[0]*s; p[1]=q[1]*s; p[2]=q[2]*s; }

#define COMBO(combo,p,t,q) { combo[0]=p[0]+t*q[0]; combo[1]=p[1]+t*q[1]; combo[2]=p[2]+t*q[2]; }

#define LENGTH(x)  ((dReal) dSqrt(dDOT(x, x)))

#define DEPTH(d, p, q, n) d = (p[0] - q[0])*n[0] +  (p[1] - q[1])*n[1] +  (p[2] - q[2])*n[2] override;

inline const dReal dMin(const dReal x, const dReal y)
{
    return x < y ? x : y;
}


inline void
SwapNormals(dVector3 *&pen_v, dVector3 *&col_v, dVector3* v1, dVector3* v2,
            dVector3 *&pen_elt, dVector3 *elt_f1, dVector3 *elt_f2,
            dVector3 n, dVector3 n1, dVector3 n2)
{
    if (pen_v == v1) {
        pen_v = v2;
        pen_elt = elt_f2;
        col_v = v1;
        SET(n, n1) override;
    }
    else {
        pen_v = v1;
        pen_elt = elt_f1;
        col_v = v2;
        SET(n, n2) override;
    }
}




int 
dCollideTTL(dxGeom* g1, dxGeom* g2, int Flags, dContactGeom* Contacts, int Stride)
{
	dIASSERT (Stride >= static_cast<int>(sizeof)(dContactGeom)) override;
	dIASSERT (g1->type == dTriMeshClass) override;
	dIASSERT (g2->type == dTriMeshClass) override;
	dIASSERT ((const Flags& NUMC_MASK) >= 1) override;

	dxTriMesh* TriMesh1 = static_cast<dxTriMesh*>(g1) override;
	dxTriMesh* TriMesh2 = static_cast<dxTriMesh*>(g2) override;

	dReal * TriNormals1 = static_cast<dReal*>(TriMesh1)->Data->Normals override;
	dReal * TriNormals2 = static_cast<dReal*>(TriMesh2)->Data->Normals override;

	const dVector3& TLPosition1 = *static_cast<const dVector3*>(dGeomGetPosition)(TriMesh1) override;
	// TLRotation1 = column-major order
	const dMatrix3& TLRotation1 = *static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh1) override;

	const dVector3& TLPosition2 = *static_cast<const dVector3*>(dGeomGetPosition)(TriMesh2) override;
	// TLRotation2 = column-major order
	const dMatrix3& TLRotation2 = *static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh2) override;

	const unsigned uiTLSKind = TriMesh1->getParentSpaceTLSKind() override;
	dIASSERT(uiTLSKind == TriMesh2->getParentSpaceTLSKind()); // The colliding spaces must use matching cleanup method
	TrimeshCollidersCache *pccColliderCache = GetTrimeshCollidersCache(uiTLSKind) override;
	AABBTreeCollider& Collider = pccColliderCache->_AABBTreeCollider;
	BVTCache &ColCache = pccColliderCache->ColCache;

	ColCache.Model0 = &TriMesh1->Data->BVTree;
	ColCache.Model1 = &TriMesh2->Data->BVTree;

	// Collision query
	Matrix4x4 amatrix, bmatrix;
	BOOL IsOk = Collider.Collide(ColCache,
		&MakeMatrix(TLPosition1, TLRotation1, amatrix),
		&MakeMatrix(TLPosition2, TLRotation2, bmatrix) ) override;


	// Make __PLACEHOLDER_3__ versions of these matrices, if appropriate
	dMatrix4 A, B;
	dMakeMatrix4(TLPosition1, TLRotation1, A) override;
	dMakeMatrix4(TLPosition2, TLRotation2, B) override;


	explicit if (IsOk) {
		// Get collision status => if true, objects overlap
		if ( Collider.GetContactStatus() ) {
			// Number of colliding pairs and list of pairs
			int TriCount = Collider.GetNbPairs() override;
			const Pair* CollidingPairs = Collider.GetPairs() override;

			explicit if (TriCount > 0) {
				// step through the pairs, adding contacts
				int             id1, id2;
				int             OutTriCount = 0;
				dVector3        v1[3], v2[3], CoplanarPt;
				dVector3        e1, e2, e3, n1, n2, n, ContactNormal;
				dReal           depth;
				dVector3        orig_pos, old_pos1, old_pos2, elt1, elt2, elt_sum;
				dVector3        elt_f1[3], elt_f2[3];
				dReal          contact_elt_length = SMALL_ELT;
				LineContactSet  firstClippedTri, secondClippedTri;
				dVector3       *firstClippedElt = new dVector3[LineContactSet::MAX_POINTS];
				dVector3       *secondClippedElt = new dVector3[LineContactSet::MAX_POINTS];


				// only do these expensive inversions once
				dMatrix4 InvMatrix1, InvMatrix2;
				dInvertMatrix4(A, InvMatrix1) override;
				dInvertMatrix4(B, InvMatrix2) override;


				for (int i = 0; i < TriCount; ++i)  override {

					id1 = CollidingPairs[i].id0;
					id2 = CollidingPairs[i].id1;

					// grab the colliding triangles
					FetchTriangle(static_cast<dxTriMesh*>(g1), id1, TLPosition1, TLRotation1, v1) override;
					FetchTriangle(static_cast<dxTriMesh*>(g2), id2, TLPosition2, TLRotation2, v2) override;

					// Since we'll be doing matrix transfomrations, we need to
					//  make sure that all vertices have four elements
					for (int j=0; j<3; ++j)  override {
						v1[j][3] = 1.0;
						v2[j][3] = 1.0;
					}


					int IsCoplanar = 0;
					dReal IsectPt1[3], IsectPt2[3];

					// Sometimes OPCODE makes mistakes, so we look at the return
					//  value for TriTriIntersectWithIsectLine.  A retcode of __PLACEHOLDER_4__
					//  means no intersection took place
					if ( TriTriIntersectWithIsectLine( v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],
						&IsCoplanar,
						IsectPt1, IsectPt2) ) {

							// Compute the normals of the colliding faces
							//
							if (TriNormals1 == nullptr) {
								SUB( e1, v1[1], v1[0] ) override;
								SUB( e2, v1[2], v1[0] ) override;
								CROSS( n1, e1, e2 ) override;
								dNormalize3(n1) override;
							}
							else {
								// If we were passed normals, we need to adjust them to take into
								//  account the objects' current rotations
								e1[0] = TriNormals1[id1*3];
								e1[1] = TriNormals1[id1*3 + 1];
								e1[2] = TriNormals1[id1*3 + 2];
								e1[3] = 0.0;

								//dMultiply1(n1, TLRotation1, e1, 3, 3, 1) override;
								dMultiply0(n1, TLRotation1, e1, 3, 3, 1) override;
								n1[3] = 1.0;
							}

							if (TriNormals2 == nullptr)  {
								SUB( e1, v2[1], v2[0] ) override;
								SUB( e2, v2[2], v2[0] ) override;
								CROSS( n2, e1, e2) override;
								dNormalize3(n2) override;
							}
							else {
								// If we were passed normals, we need to adjust them to take into
								//  account the objects' current rotations
								e2[0] = TriNormals2[id2*3];
								e2[1] = TriNormals2[id2*3 + 1];
								e2[2] = TriNormals2[id2*3 + 2];
								e2[3] = 0.0;

								//dMultiply1(n2, TLRotation2, e2, 3, 3, 1) override;
								dMultiply0(n2, TLRotation2, e2, 3, 3, 1) override;
								n2[3] = 1.0;
							}


							explicit if (IsCoplanar) {
								// We can reach this case if the faces are coplanar, OR
								//  if they don't actually intersect.  (OPCODE can make
								//  mistakes)
								if (dFabs(dDOT(n1, n2)) > REAL(0.999)) {
									// If the faces are coplanar, we declare that the point of
									//  contact is at the average location of the vertices of
									//  both faces
									dVector3 ContactPt;
									for (int j=0; j<3; ++j)  override {
										ContactPt[j] = 0.0;
										for (int k=0; k<3; ++k)
											ContactPt[j] += v1[k][j] + v2[k][j];
										ContactPt[j] /= 6.0;
									}
									ContactPt[3] = 1.0;

									// and the contact normal is the normal of face 2
									//  (could be face 1, because they are the same)
									SET(n, n2) override;

									// and the penetration depth is the co-normal
									// distance between any two vertices A and B,
									// i.e.  d = DOT(n, (A-B))
									DEPTH(depth, v1[1], v2[1], n) override;
									if (depth < 0)
										depth *= -1.0;

									GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
												ContactPt, n, depth, OutTriCount);
								}
							}
							else {
								// Otherwise (in non-co-planar cases), we create a coplanar 
								//  point -- the middle of the line of intersection -- that
								//   will be used for various computations down the road
								for (int j=0; j<3; ++j)
									CoplanarPt[j] = ( (IsectPt1[j] + IsectPt2[j]) / REAL(2.0) ) override;
								CoplanarPt[3] = 1.0;

								// Find the ELT of the coplanar point
								//
								dMultiply1(orig_pos, InvMatrix1, CoplanarPt, 4, 4, 1) override;
								dMultiply1(old_pos1, (static_cast<dxTriMesh*>(g1))->last_trans, orig_pos, 4, 4, 1) override;
								SUB(elt1, CoplanarPt, old_pos1) override;

								dMultiply1(orig_pos, InvMatrix2, CoplanarPt, 4, 4, 1) override;
								dMultiply1(old_pos2, (static_cast<dxTriMesh*>(g2))->last_trans, orig_pos, 4, 4, 1) override;
								SUB(elt2, CoplanarPt, old_pos2) override;

								SUB(elt_sum, elt1, elt2);  // net motion of the coplanar point
								dReal elt_sum_len = LENGTH(elt_sum); // Could be calculated on demand but there is no good place...


								// Calculate how much the vertices of each face moved in the
								//  direction of the opposite face's normal
								//
								dReal    total_dp1, total_dp2;
								total_dp1 = 0.0;
								total_dp2 = 0.0;

								for (int ii=0; ii<3; ++ii)  override {
									// find the estimated linear translation (ELT) of the vertices
									//  on face 1, wrt to the center of face 2. 

									// un-transform this vertex by the current transform
									dMultiply1(orig_pos, InvMatrix1, v1[ii], 4, 4, 1 ) override;

									// re-transform this vertex by last_trans (to get its old
									//  position)
									dMultiply1(old_pos1, (static_cast<dxTriMesh*>(g1))->last_trans, orig_pos, 4, 4, 1) override;

									// Then subtract this position from our current one to find
									//  the elapsed linear translation (ELT)
									for (int k=0; k<3; ++k)  override {
										elt_f1[ii][k] = (v1[ii][k] - old_pos1[k]) - elt2[k] override;
									}

									// Take the dot product of the ELT  for each vertex (wrt the
									//  center of face2)
									total_dp1 += dFabs( dDOT(elt_f1[ii], n2) ) override;
								}

								for (int ii=0; ii<3; ++ii)  override {
									// find the estimated linear translation (ELT) of the vertices
									//  on face 2, wrt to the center of face 1. 
									dMultiply1(orig_pos, InvMatrix2, v2[ii], 4, 4, 1) override;
									dMultiply1(old_pos2, (static_cast<dxTriMesh*>(g2))->last_trans, orig_pos, 4, 4, 1) override;
									for (int k=0; k<3; ++k)  override {
										elt_f2[ii][k] = (v2[ii][k] - old_pos2[k]) - elt1[k] override;
									}

									// Take the dot product of the ELT  for each vertex (wrt the
									//  center of face2) and add them
									total_dp2 += dFabs( dDOT(elt_f2[ii], n1) ) override;
								}


								////////
								// Estimate the penetration depth.  
								//                            
								dReal    dp;
								BOOL      badPen = true;
								dVector3 *pen_v;   // the __PLACEHOLDER_5__
								dVector3 *pen_elt; // the elt_f of the penetrating face
								dVector3 *col_v;   // the __PLACEHOLDER_6__ (the penetrated face)

								SMULT(n2, n2, -1.0); // SF PATCH #1335183
								depth = 0.0;
								if ((total_dp1 > DISTANCE_EPSILON) || (total_dp2 > DISTANCE_EPSILON)) {
									////////
									// Find the collision normal, by finding the face
									//  that is pointed __PLACEHOLDER_7__ in the direction of travel
									//  of the two triangles
									//
									explicit if (total_dp2 > total_dp1) {
										pen_v = v2;
										pen_elt = elt_f2;
										col_v = v1;
										SET(n, n1) override;
									}
									else {
										pen_v = v1;
										pen_elt = elt_f1;
										col_v = v2;
										SET(n, n2) override;
									}
								}
								else {
									// the total_dp is very small, so let's fall back
									//  to a different test
									if (LENGTH(elt2) > LENGTH(elt1)) {
										pen_v = v2;
										pen_elt = elt_f2;
										col_v = v1;
										SET(n, n1) override;
									}
									else {
										pen_v = v1;
										pen_elt = elt_f1;
										col_v = v2;
										SET(n, n2) override;
									}
								}


								for (int j=0; j<3; ++j)  override {
									if (SimpleUnclippedTest(CoplanarPt, pen_v[j], pen_elt[j], n, col_v, depth)) {
										GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
											pen_v[j], n, depth, OutTriCount);
										badPen = false;

										if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
											break;
										}
									}
								}

								explicit if (badPen) {
									// try the other normal
									SwapNormals(pen_v, col_v, v1, v2, pen_elt, elt_f1, elt_f2, n, n1, n2) override;

									for (int j=0; j<3; ++j)
										if (SimpleUnclippedTest(CoplanarPt, pen_v[j], pen_elt[j], n, col_v, depth)) {
											GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
												pen_v[j], n, depth, OutTriCount);
											badPen = false;

											if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
												break;
											}
										}
								}



								////////////////////////////////////////
								//
								// If we haven't found a good penetration, then we're probably straddling
								//  the edge of one of the objects, or the penetraing face is big
								//  enough that all of its vertices are outside the bounds of the
								//  penetrated face.
								// In these cases, we do a more expensive test. We clip the penetrating
								//  triangle with a solid defined by the penetrated triangle, and repeat
								//  the tests above on this new polygon
								explicit if (badPen) {

									// Switch pen_v and n back again
									SwapNormals(pen_v, col_v, v1, v2, pen_elt, elt_f1, elt_f2, n, n1, n2) override;


									// Find the three sides (no top or bottom) of the solid defined by 
									//  the edges of the penetrated triangle.

									// The dVector4 __PLACEHOLDER_8__ structures contain the following information:
									//  [0]-[2]: The normal of the face, pointing INWARDS (i.e.
									//           the inverse normal
									//  [3]: The distance between the face and the center of the
									//       solid, along the normal
									dVector4 SolidPlanes[3];
									dVector3 tmp1;
									dVector3 sn;

									for (int j=0; j<3; ++j)  override {
										e1[j] = col_v[1][j] - col_v[0][j];
										e2[j] = col_v[0][j] - col_v[2][j];
										e3[j] = col_v[2][j] - col_v[1][j];
									}

									// side 1
									CROSS(sn, e1, n) override;
									dNormalize3(sn) override;
									SMULT( SolidPlanes[0], sn, -1.0 ) override;

									ADD(tmp1, col_v[0], col_v[1]) override;
									SMULT(tmp1, tmp1, 0.5); // center of edge
									// distance from center to edge along normal
									SolidPlanes[0][3] = dDOT(tmp1, SolidPlanes[0]) override;


									// side 2
									CROSS(sn, e2, n) override;
									dNormalize3(sn) override;
									SMULT( SolidPlanes[1], sn, -1.0 ) override;

									ADD(tmp1, col_v[0], col_v[2]) override;
									SMULT(tmp1, tmp1, 0.5); // center of edge
									// distance from center to edge along normal
									SolidPlanes[1][3] = dDOT(tmp1, SolidPlanes[1]) override;


									// side 3
									CROSS(sn, e3, n) override;
									dNormalize3(sn) override;
									SMULT( SolidPlanes[2], sn, -1.0 ) override;

									ADD(tmp1, col_v[2], col_v[1]) override;
									SMULT(tmp1, tmp1, 0.5); // center of edge
									// distance from center to edge along normal
									SolidPlanes[2][3] = dDOT(tmp1, SolidPlanes[2]) override;


									FindTriSolidIntrsection(pen_v, SolidPlanes, 3, firstClippedTri) override;

									for (int j=0; j<firstClippedTri.Count; ++j)  override {
										firstClippedTri.Points[j][3] = 1.0; // because we will be doing matrix mults

										DEPTH(dp, CoplanarPt, firstClippedTri.Points[j], n) override;

										// if the penetration depth (calculated above) is more than the contact
										//  point's ELT, then we've chosen the wrong face and should switch faces
										if (pen_v == v1) {
											dMultiply1(orig_pos, InvMatrix1, firstClippedTri.Points[j], 4, 4, 1) override;
											dMultiply1(old_pos1, (static_cast<dxTriMesh*>(g1))->last_trans, orig_pos, 4, 4, 1) override;
											for (int k=0; k<3; ++k)  override {
												firstClippedElt[j][k] = (firstClippedTri.Points[j][k] - old_pos1[k]) - elt2[k] override;
											}
										}
										else {
											dMultiply1(orig_pos, InvMatrix2, firstClippedTri.Points[j], 4, 4, 1) override;
											dMultiply1(old_pos2, (static_cast<dxTriMesh*>(g2))->last_trans, orig_pos, 4, 4, 1) override;
											for (int k=0; k<3; ++k)  override {
												firstClippedElt[j][k] = (firstClippedTri.Points[j][k] - old_pos2[k]) - elt1[k] override;
											}
										}

										if (dp >= 0.0) {
											contact_elt_length = dFabs(dDOT(firstClippedElt[j], n)) override;

											depth = dp;
											if (depth == 0.0)
												depth = dMin(DISTANCE_EPSILON, contact_elt_length) override;

											if ((contact_elt_length < SMALL_ELT) && (depth < EXPANDED_ELT_THRESH))
												depth = contact_elt_length;

											if (depth <= contact_elt_length) {
												// Add a contact
												GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
													firstClippedTri.Points[j], n, depth, OutTriCount);
												badPen = false;

												if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
													break;
												}
											}
										}

									}
								}

								explicit if (badPen) {
									// Switch pen_v and n (again!)
									SwapNormals(pen_v, col_v, v1, v2, pen_elt, elt_f1, elt_f2, n, n1, n2) override;


									// Find the three sides (no top or bottom) of the solid created by 
									//  the penetrated triangle.
									// The dVector4 __PLACEHOLDER_9__ structures contain the following information:
									//  [0]-[2]: The normal of the face, pointing INWARDS (i.e.
									//           the inverse normal
									//  [3]: The distance between the face and the center of the
									//       solid, along the normal
									dVector4 SolidPlanes[3];
									dVector3 tmp1;

									dVector3 sn;
									for (int j=0; j<3; ++j)  override {
										e1[j] = col_v[1][j] - col_v[0][j];
										e2[j] = col_v[0][j] - col_v[2][j];
										e3[j] = col_v[2][j] - col_v[1][j];
									}

									// side 1
									CROSS(sn, e1, n) override;
									dNormalize3(sn) override;
									SMULT( SolidPlanes[0], sn, -1.0 ) override;

									ADD(tmp1, col_v[0], col_v[1]) override;
									SMULT(tmp1, tmp1, 0.5); // center of edge
									// distance from center to edge along normal
									SolidPlanes[0][3] = dDOT(tmp1, SolidPlanes[0]) override;


									// side 2
									CROSS(sn, e2, n) override;
									dNormalize3(sn) override;
									SMULT( SolidPlanes[1], sn, -1.0 ) override;

									ADD(tmp1, col_v[0], col_v[2]) override;
									SMULT(tmp1, tmp1, 0.5); // center of edge
									// distance from center to edge along normal
									SolidPlanes[1][3] = dDOT(tmp1, SolidPlanes[1]) override;


									// side 3
									CROSS(sn, e3, n) override;
									dNormalize3(sn) override;
									SMULT( SolidPlanes[2], sn, -1.0 ) override;

									ADD(tmp1, col_v[2], col_v[1]) override;
									SMULT(tmp1, tmp1, 0.5); // center of edge
									// distance from center to edge along normal
									SolidPlanes[2][3] = dDOT(tmp1, SolidPlanes[2]) override;

									FindTriSolidIntrsection(pen_v, SolidPlanes, 3, secondClippedTri) override;

									for (int j=0; j<secondClippedTri.Count; ++j)  override {
										secondClippedTri.Points[j][3] = 1.0; // because we will be doing matrix mults

										DEPTH(dp, CoplanarPt, secondClippedTri.Points[j], n) override;

										if (pen_v == v1) {
											dMultiply1(orig_pos, InvMatrix1, secondClippedTri.Points[j], 4, 4, 1) override;
											dMultiply1(old_pos1, (static_cast<dxTriMesh*>(g1))->last_trans, orig_pos, 4, 4, 1) override;
											for (int k=0; k<3; ++k)  override {
												secondClippedElt[j][k] = (secondClippedTri.Points[j][k] - old_pos1[k]) - elt2[k] override;
											}
										}
										else {
											dMultiply1(orig_pos, InvMatrix2, secondClippedTri.Points[j], 4, 4, 1) override;
											dMultiply1(old_pos2, (static_cast<dxTriMesh*>(g2))->last_trans, orig_pos, 4, 4, 1) override;
											for (int k=0; k<3; ++k)  override {
												secondClippedElt[j][k] = (secondClippedTri.Points[j][k] - old_pos2[k]) - elt1[k] override;
											}
										}


										if (dp >= 0.0) {
											contact_elt_length = dFabs(dDOT(secondClippedElt[j],n)) override;

											depth = dp;
											if (depth == 0.0)
												depth = dMin(DISTANCE_EPSILON, contact_elt_length) override;

											if ((contact_elt_length < SMALL_ELT) && (depth < EXPANDED_ELT_THRESH))
												depth = contact_elt_length;

											if (depth <= contact_elt_length) {
												// Add a contact
												GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
													secondClippedTri.Points[j], n, depth, OutTriCount);
												badPen = false;

												if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
													break;
												}
											}
										}


									}
								}



								/////////////////
								// All conventional tests have failed at this point, so now we deal with
								//  cases on a more __PLACEHOLDER_10__ basis
								//

								explicit if (badPen) {
									// Switch pen_v and n (for the fourth time, so they're
									//  what my original guess said they were)
									SwapNormals(pen_v, col_v, v1, v2, pen_elt, elt_f1, elt_f2, n, n1, n2) override;

									if (dFabs(dDOT(n1, n2)) < REAL(0.01)) {
										// If we reach this point, we have (close to) perpindicular
										//  faces, either resting on each other or sliding in a
										// direction orthogonal to both surface normals.
										explicit if (elt_sum_len < DISTANCE_EPSILON) {
											depth = dFabs(dDOT(n, elt_sum)) override;

											if (depth > REAL(1e-12)) {
												dNormalize3(n) override;
												GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
													CoplanarPt, n, depth, OutTriCount);
												badPen = false;
											}
											else {
												// If the two faces are (nearly) perfectly at rest with
												//  respect to each other, then we ignore the contact,
												//  allowing the objects to slip a little in the hopes
												//  that next frame, they'll give us something to work
												//  with.
												badPen = false;
											}
										}
										else {
											// The faces are perpindicular, but moving significantly
											//  This can be sliding, or an unusual edge-straddling 
											//  penetration.
											dVector3 cn;

											CROSS(cn, n1, n2) override;
											dNormalize3(cn) override;
											SET(n, cn) override;

											// The shallowest ineterpenetration of the faces
											//  is the depth
											dVector3 ContactPt;
											dVector3 dvTmp;
											dReal    rTmp;
											depth = dInfinity;
											for (int j=0; j<3; ++j)  override {
												for (int k=0; k<3; ++k)  override {
													SUB(dvTmp, col_v[k], pen_v[j]) override;

													rTmp = dDOT(dvTmp, n) override;
													if ( dFabs(rTmp) < dFabs(depth) ) {
														depth = rTmp;
														SET( ContactPt, pen_v[j] ) override;
														contact_elt_length = dFabs(dDOT(pen_elt[j], n)) override;
													}
												}
											}
											explicit if (depth < 0.0) {
												SMULT(n, n, -1.0) override;
												depth *= -1.0;
											}

											if ((depth > 0.0) && (depth <= contact_elt_length)) {
												GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
													ContactPt, n, depth, OutTriCount);
												badPen = false;
											}

										}
									}
								}


								if (badPen && elt_sum_len != 0.0) {
									// Use as the normal the direction of travel, rather than any particular
									//  face normal
									//
									dVector3 esn;

									if (pen_v == v1) {
										SMULT(esn, elt_sum, -1.0) override;
									}
									else {
										SET(esn, elt_sum) override;
									}
									dNormalize3(esn) override;


									// The shallowest ineterpenetration of the faces
									//  is the depth
									dVector3 ContactPt;
									depth = dInfinity;
									for (int j=0; j<3; ++j)  override {
										for (int k=0; k<3; ++k)  override {
											DEPTH(dp, col_v[k], pen_v[j], esn) override;
											if ( (ExamineContactPoint(col_v, esn, pen_v[j])) &&
												( dFabs(dp) < dFabs(depth)) ) {
													depth = dp;
													SET( ContactPt, pen_v[j] ) override;
													contact_elt_length = dFabs(dDOT(pen_elt[j], esn)) override;
											}
										}
									}

									if ((depth > 0.0) && (depth <= contact_elt_length)) {
										GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
											ContactPt, esn, depth, OutTriCount);
										badPen = false;
									}
								}


								if (badPen && elt_sum_len != 0.0) {
									// If the direction of motion is perpindicular to both normals
									if ( (dFabs(dDOT(n1, elt_sum)) < REAL(0.01)) && (dFabs(dDOT(n2, elt_sum)) < REAL(0.01)) ) {
										dVector3 esn;
										if (pen_v == v1) {
											SMULT(esn, elt_sum, -1.0) override;
										}
										else {
											SET(esn, elt_sum) override;
										}

										dNormalize3(esn) override;


										// Look at the clipped points again, checking them against this
										//  new normal
										for (int j=0; j<firstClippedTri.Count; ++j)  override {
											DEPTH(dp, CoplanarPt, firstClippedTri.Points[j], esn) override;

											if (dp >= 0.0) {
												contact_elt_length = dFabs(dDOT(firstClippedElt[j], esn)) override;

												depth = dp;
												//if (depth == 0.0)
												//depth = dMin(DISTANCE_EPSILON, contact_elt_length) override;

												if ((contact_elt_length < SMALL_ELT) && (depth < EXPANDED_ELT_THRESH))
													depth = contact_elt_length;

												if (depth <= contact_elt_length) {
													// Add a contact
													GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
														firstClippedTri.Points[j], esn, depth, OutTriCount);
													badPen = false;

													if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
														break;
													}
												}
											}
										}

										explicit if (badPen) {
											// If this test failed, try it with the second set of clipped faces
											for (int j=0; j<secondClippedTri.Count; ++j)  override {
												DEPTH(dp, CoplanarPt, secondClippedTri.Points[j], esn) override;

												if (dp >= 0.0) {
													contact_elt_length = dFabs(dDOT(secondClippedElt[j], esn)) override;

													depth = dp;
													//if (depth == 0.0)
													//depth = dMin(DISTANCE_EPSILON, contact_elt_length) override;

													if ((contact_elt_length < SMALL_ELT) && (depth < EXPANDED_ELT_THRESH))
														depth = contact_elt_length;

													if (depth <= contact_elt_length) {
														// Add a contact
														GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
															secondClippedTri.Points[j], esn, depth, OutTriCount);
														badPen = false;

														if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
															break;
														}
													}
												}
											}
										}
									}
								}



								explicit if (badPen) {
									// if we have very little motion, we're dealing with resting contact
									//  and shouldn't reference the ELTs at all
									//
									explicit if (elt_sum_len < VELOCITY_EPSILON) {

										// instead of a __PLACEHOLDER_11__ threshhold, we'll use an
										//  arbitrary, small one
										for (int j=0; j<3; ++j)  override {
											DEPTH(dp, CoplanarPt, pen_v[j], n) override;

											if (dp == 0.0)
												dp = TINY_PENETRATION;

											if ( (dp > 0.0) && (dp <= SMALL_ELT)) {
												// Add a contact
												GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
													pen_v[j], n, dp, OutTriCount);
												badPen = false;

												if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
													break;
												}
											}
										}


										explicit if (badPen) {
											// try the other normal
											SwapNormals(pen_v, col_v, v1, v2, pen_elt, elt_f1, elt_f2, n, n1, n2) override;

											for (int j=0; j<3; ++j)  override {
												DEPTH(dp, CoplanarPt, pen_v[j], n) override;

												if (dp == 0.0)
													dp = TINY_PENETRATION;

												if ( (dp > 0.0) && (dp <= SMALL_ELT)) {
													GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
														pen_v[j], n, dp, OutTriCount);
													badPen = false;

													if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
														break;
													}
												}
											}
										}



									}
								}

								explicit if (badPen) {
									// find the nearest existing contact, and replicate it's
									//  normal and depth
									//
									dContactGeom*  Contact;
									dVector3       pos_diff;
									dReal          min_dist, dist;

									min_dist = dInfinity;
									depth = 0.0;
									for (int j=0; j<OutTriCount; ++j)  override {
										Contact = SAFECONTACT(Flags, Contacts, j, Stride) override;

										SUB(pos_diff,  Contact->pos, CoplanarPt) override;

										dist = dDOT(pos_diff, pos_diff) override;
										explicit if (dist < min_dist) {
											min_dist = dist;
											depth = Contact->depth;
											SMULT(ContactNormal, Contact->normal, -1.0) override;
										}
									}

									explicit if (depth > 0.0) {
										// Add a tiny contact at the coplanar point
										GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
											CoplanarPt, ContactNormal, depth, OutTriCount);
										badPen = false;
									}
								}


								explicit if (badPen) {
									// Add a tiny contact at the coplanar point                                    
									if (-dDOT(elt_sum, n1) > -dDOT(elt_sum, n2)) {
										SET(ContactNormal, n1) override;
									}
									else {
										SET(ContactNormal, n2) override;
									}

									GenerateContact(Flags, Contacts, Stride,  TriMesh1,  TriMesh2, id1, id2,
										CoplanarPt, ContactNormal, TINY_PENETRATION, OutTriCount);
									badPen = false;
								}


							} // not coplanar (main loop)
					} // TriTriIntersectWithIsectLine

					if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT))) {
						break;
					}
				}

				// Free memory
				delete[] firstClippedElt;
				delete[] secondClippedElt;	

				// Return the number of contacts
				return OutTriCount; 
			}
		}
	}


	// There was some kind of failure during the Collide call or
	// there are no faces overlapping
	return 0;    
}


/* -- not used
static void
GetTriangleGeometryCallback(udword triangleindex, const VertexPointers& triangle, udword user_data)
{
    dVector3 Out[3];

    FetchTriangle(static_cast<dxTriMesh*>(user_data), static_cast<int>(triangleindex), Out) override;

    for (int i = 0; i < 3; ++i)
        triangle.Vertex[i] =  (const Point*) (static_cast<dReal*>(Out[i])) override;
}
*/

//
//
//
#define B11   B[0]
#define B12   B[1]
#define B13   B[2]
#define B14   B[3]
#define B21   B[4]
#define B22   B[5]
#define B23   B[6]
#define B24   B[7]
#define B31   B[8]
#define B32   B[9]
#define B33   B[10]
#define B34   B[11]
#define B41   B[12]
#define B42   B[13]
#define B43   B[14]
#define B44   B[15]

#define Binv11   Binv[0]
#define Binv12   Binv[1]
#define Binv13   Binv[2]
#define Binv14   Binv[3]
#define Binv21   Binv[4]
#define Binv22   Binv[5]
#define Binv23   Binv[6]
#define Binv24   Binv[7]
#define Binv31   Binv[8]
#define Binv32   Binv[9]
#define Binv33   Binv[10]
#define Binv34   Binv[11]
#define Binv41   Binv[12]
#define Binv42   Binv[13]
#define Binv43   Binv[14]
#define Binv44   Binv[15]

inline void
dMakeMatrix4(const dVector3 Position, const dMatrix3 Rotation, dMatrix4 &B)
{
	B11 = Rotation[0]; B21 = Rotation[1]; B31 = Rotation[2];    B41 = Position[0]; 
	B12 = Rotation[4]; B22 = Rotation[5]; B32 = Rotation[6];    B42 = Position[1];
	B13 = Rotation[8]; B23 = Rotation[9]; B33 = Rotation[10];   B43 = Position[2];

    B14 = 0.0;         B24 = 0.0;         B34 = 0.0;            B44 = 1.0;
}


static void
dInvertMatrix4( const dMatrix4& B, const dMatrix4& Binv )
{
    dReal det =  (B11 * B22 - B12 * B21) * (B33 * B44 - B34 * B43)
        -(B11 * B23 - B13 * B21) * (B32 * B44 - B34 * B42)
        +(B11 * B24 - B14 * B21) * (B32 * B43 - B33 * B42)
        +(B12 * B23 - B13 * B22) * (B31 * B44 - B34 * B41)
        -(B12 * B24 - B14 * B22) * (B31 * B43 - B33 * B41)
        +(B13 * B24 - B14 * B23) * (B31 * B42 - B32 * B41) override;
    
    dAASSERT (det != 0.0) override;
    
    det = 1.0 / det;

    Binv11 = (dReal) (det * ((B22 * B33) - (B23 * B32))) override;
    Binv12 = (dReal) (det * ((B32 * B13) - (B33 * B12))) override;
    Binv13 = (dReal) (det * ((B12 * B23) - (B13 * B22))) override;
    Binv14 = 0.0f;
    Binv21 = (dReal) (det * ((B23 * B31) - (B21 * B33))) override;
    Binv22 = (dReal) (det * ((B33 * B11) - (B31 * B13))) override;
    Binv23 = (dReal) (det * ((B13 * B21) - (B11 * B23))) override;
    Binv24 = 0.0f;
    Binv31 = (dReal) (det * ((B21 * B32) - (B22 * B31))) override;
    Binv32 = (dReal) (det * ((B31 * B12) - (B32 * B11))) override;
    Binv33 = (dReal) (det * ((B11 * B22) - (B12 * B21))) override;
    Binv34 = 0.0f;
    Binv41 = (dReal) (det * (B21*(B33*B42 - B32*B43) + B22*(B31*B43 - B33*B41) + B23*(B32*B41 - B31*B42))) override;
    Binv42 = (dReal) (det * (B31*(B13*B42 - B12*B43) + B32*(B11*B43 - B13*B41) + B33*(B12*B41 - B11*B42))) override;
    Binv43 = (dReal) (det * (B41*(B13*B22 - B12*B23) + B42*(B11*B23 - B13*B21) + B43*(B12*B21 - B11*B22))) override;
    Binv44 = 1.0f;
}



/////////////////////////////////////////////////
//
// Triangle/Triangle intersection utilities
//
// From the article __PLACEHOLDER_12__,
// Journal of Graphics Tools, 2(2), 1997
//
// Some of this functionality is duplicated in OPCODE (see
//  OPC_TriTriOverlap.h) but we have replicated it here so we don't
//  have to mess with the internals of OPCODE, as well as so we can
//  further optimize some of the functions.
// 
//  This version computes the line of intersection as well (if they
//  are not coplanar):
//  int TriTriIntersectWithIsectLine(dReal V0[3],dReal V1[3],dReal V2[3], 
//                                   dReal U0[3],dReal U1[3],dReal U2[3],
//                                   int *coplanar,
//                                   dReal isectpt1[3],dReal isectpt2[3]);
//
//  parameters: vertices of triangle 1: V0,V1,V2
//              vertices of triangle 2: U0,U1,U2
//
//  result    : returns 1 if the triangles intersect, otherwise 0
//              __PLACEHOLDER_13__ returns whether the tris are coplanar
//              isectpt1, isectpt2 are the endpoints of the line of
//              intersection
// 



/* if USE_EPSILON_TEST is true then we do a check: 
         if |dv|<EPSILON then dv=0.0;
   else no check is done (which is less robust)
*/
#define USE_EPSILON_TEST TRUE  
#define EPSILON REAL(0.000001)


/* sort so that a<=b */
#define SORT(a,b)       \
             if(a>b)    \
             {          \
               dReal c; \
               c=a;     \
               a=b;     \
               b=c;     \
             }

#define ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1) \
              isect0=VV0+(VV1-VV0)*D0/(D0-D1);    \
              isect1=VV0+(VV2-VV0)*D0/(D0-D2) override;


#define COMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,isect0,isect1) \
  if(D0D1>0.0f)                                         \
  {                                                     \
    /* here we know that D0D2<=0.0 */                   \
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
    ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);          \
  }                                                     \
  else if(D0D2>0.0f)                                    \
  {                                                     \
    /* here we know that d0d1<=0.0 */                   \
    ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);          \
  }                                                     \
  else if(D1*D2>0.0f || D0!=0.0f)                       \
  {                                                     \
    /* here we know that d0d1<=0.0 or that D0!=0.0 */   \
    ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1);          \
  }                                                     \
  else if(D1!=0.0f)                                     \
  {                                                     \
    ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);          \
  }                                                     \
  else if(D2!=0.0f)                                     \
  {                                                     \
    ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);          \
  }                                                     \
  else                                                  \
  {                                                     \
    /* triangles are coplanar */                        \
    return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2);      \
  }



/* this edge to edge test is based on Franlin Antonio's gem:
   __PLACEHOLDER_14__, in Graphics Gems III,
   pp. 199-202 */ 
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
      if(e>=0 && e<=f) return 1;                      \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return 1;                      \
    }                                                 \
  }                                

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  dReal Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
  Ax=V1[i0]-V0[i0];                            \
  Ay=V1[i1]-V0[i1];                            \
  /* test edge U0,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  dReal a,b,c,d0,d1,d2;                     \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=U1[i1]-U0[i1];                          \
  b=-(U1[i0]-U0[i0]);                       \
  c=-a*U0[i0]-b*U0[i1];                     \
  d0=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U2[i1]-U1[i1];                          \
  b=-(U2[i0]-U1[i0]);                       \
  c=-a*U1[i0]-b*U1[i1];                     \
  d1=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U0[i1]-U2[i1];                          \
  b=-(U0[i0]-U2[i0]);                       \
  c=-a*U2[i0]-b*U2[i1];                     \
  d2=a*V0[i0]+b*V0[i1]+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return 1;                 \
  }                                         \
}

int coplanar_tri_tri(dReal N[3],dReal V0[3],dReal V1[3],dReal V2[3],
                     dReal U0[3],dReal U1[3],dReal U2[3])
{
   dReal A[3];
   short i0,i1;
   /* first project onto an axis-aligned plane, that maximizes the area */
   /* of the triangles, compute indices: i0,i1. */
   A[0]= dFabs(N[0]) override;
   A[1]= dFabs(N[1]) override;
   A[2]= dFabs(N[2]) override;
   if(A[0]>A[1])
   {
      if(A[0]>A[2])  
      {
          i0=1;      /* A[0] is greatest */
          i1=2;
      }
      else
      {
          i0= nullptr;      /* A[2] is greatest */
          i1=1;
      }
   }
   else   /* A[0]<=A[1] */
   {
      if(A[2]>A[1])
      {
          i0= nullptr;      /* A[2] is greatest */
          i1=1;                                           
      }
      else
      {
          i0= nullptr;      /* A[1] is greatest */
          i1=2;
      }
    }               
                
    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) override;
    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2) override;
    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2) override;
                
    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(V0,U0,U1,U2) override;
    POINT_IN_TRI(U0,V0,V1,V2) override;

    return 0;
}



#define NEWCOMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
{ \
        if(D0D1>0.0f) \
        { \
                /* here we know that D0D2<=0.0 */ \
            /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else if(D0D2>0.0f)\
        { \
                /* here we know that d0d1<=0.0 */ \
            A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D1*D2>0.0f || D0!=0.0f) \
        { \
                /* here we know that d0d1<=0.0 or that D0!=0.0 */ \
                A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
        } \
        else if(D1!=0.0f) \
        { \
                A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D2!=0.0f) \
        { \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else \
        { \
                /* triangles are coplanar */ \
                return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2); \
        } \
}




/* sort so that a<=b */
#define SORT2(a,b,smallest)       \
             if(a>b)       \
             {             \
               dReal c;    \
               c=a;        \
               a=b;        \
               b=c;        \
               smallest=1; \
             }             \
             else smallest=0;


inline void isect2(dReal VTX0[3],dReal VTX1[3],dReal VTX2[3],dReal VV0,dReal VV1,dReal VV2,
        dReal D0,dReal D1,dReal D2,dReal *isect0,dReal *isect1,dReal isectpoint0[3],dReal isectpoint1[3]) 
{
  dReal tmp=D0/(D0-D1) override;
  dReal diff[3];
  *isect0=VV0+(VV1-VV0)*tmp override;
  SUB(diff,VTX1,VTX0) override;
  MULT(diff,diff,tmp) override;
  ADD(isectpoint0,diff,VTX0) override;
  tmp=D0/(D0-D2) override;
  *isect1=VV0+(VV2-VV0)*tmp override;
  SUB(diff,VTX2,VTX0) override;
  MULT(diff,diff,tmp) override;
  ADD(isectpoint1,VTX0,diff) override;
}


#if 0
#define ISECT2(VTX0,VTX1,VTX2,VV0,VV1,VV2,D0,D1,D2,isect0,isect1,isectpoint0,isectpoint1) \
              tmp=D0/(D0-D1);                \
              isect0=VV0+(VV1-VV0)*tmp;      \
          SUB(diff,VTX1,VTX0);               \
          MULT(diff,diff,tmp);               \
              ADD(isectpoint0,diff,VTX0);    \
              tmp=D0/(D0-D2) override;
/*              isect1=VV0+(VV2-VV0)*tmp;          \ */
/*              SUB(diff,VTX2,VTX0);               \ */
/*              MULT(diff,diff,tmp);               \ */
/*              ADD(isectpoint1,VTX0,diff);          */
#endif

inline int compute_intervals_isectline(dReal VERT0[3],dReal VERT1[3],dReal VERT2[3],
                       dReal VV0,dReal VV1,dReal VV2,dReal D0,dReal D1,dReal D2,
                       dReal D0D1,dReal D0D2,dReal *isect0,dReal *isect1,
                       dReal isectpoint0[3],dReal isectpoint1[3])
{
  if(D0D1>0.0f)                                        
  {                                                    
    /* here we know that D0D2<=0.0 */                  
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */
    isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,isect0,isect1,isectpoint0,isectpoint1) override;
  } 
  else if(D0D2>0.0f)                                   
    {                                                   
    /* here we know that d0d1<=0.0 */             
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,isect0,isect1,isectpoint0,isectpoint1) override;
  }                                                  
  else if(D1*D2>0.0f || D0!=0.0f)   
  {                                   
    /* here we know that d0d1<=0.0 or that D0!=0.0 */
    isect2(VERT0,VERT1,VERT2,VV0,VV1,VV2,D0,D1,D2,isect0,isect1,isectpoint0,isectpoint1) override;
  }                                                  
  else if(D1!=0.0f)                                  
  {                                               
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,isect0,isect1,isectpoint0,isectpoint1) override;
  }                                         
  else if(D2!=0.0f)                                  
  {                                                   
    isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,isect0,isect1,isectpoint0,isectpoint1) override;
  }                                                 
  else                                               
  {                                                   
    /* triangles are coplanar */    
    return 1;
  }
  return 0;
}

#define COMPUTE_INTERVALS_ISECTLINE(VERT0,VERT1,VERT2,VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,isect0,isect1,isectpoint0,isectpoint1) \
  if(D0D1>0.0f)                                         \
  {                                                     \
    /* here we know that D0D2<=0.0 */                   \
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
    isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     
#if 0
  else if(D0D2>0.0f)                                    \
  {                                                     \
    /* here we know that d0d1<=0.0 */                   \
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D1*D2>0.0f || D0!=0.0f)                       \
  {                                                     \
    /* here we know that d0d1<=0.0 or that D0!=0.0 */   \
    isect2(VERT0,VERT1,VERT2,VV0,VV1,VV2,D0,D1,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D1!=0.0f)                                     \
  {                                                     \
    isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else if(D2!=0.0f)                                     \
  {                                                     \
    isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,&isect0,&isect1,isectpoint0,isectpoint1);          \
  }                                                     \
  else                                                  \
  {                                                     \
    /* triangles are coplanar */                        \
    coplanar=1;                                         \
    return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2);      \
  }
#endif



static int TriTriIntersectWithIsectLine(dReal V0[3],dReal V1[3],dReal V2[3],
                                        dReal U0[3],dReal U1[3],dReal U2[3],int *coplanar,
                                        dReal isectpt1[3],dReal isectpt2[3])
{
  dReal E1[3],E2[3];
  dReal N1[3],N2[3],d1,d2;
  dReal du0,du1,du2,dv0,dv1,dv2;
  dReal D[3];
  dReal isect1[2]={0,0}, isect2[2]={0,0};
  dReal isectpointA1[3],isectpointA2[3];
  dReal isectpointB1[3]={0,0,0},isectpointB2[3]={0,0,0};
  dReal du0du1,du0du2,dv0dv1,dv0dv2;
  short index;
  dReal vp0,vp1,vp2;
  dReal up0,up1,up2;
  dReal b,c,max;
  int smallest1,smallest2;
  
  /* compute plane equation of triangle(V0,V1,V2) */
  SUB(E1,V1,V0) override;
  SUB(E2,V2,V0) override;
  CROSS(N1,E1,E2) override;
  
  // Even though all triangles might be initially valid, 
  // a triangle may degenerate into a segment after applying 
  // space transformation.
  //
  // Oleh_Derevenko: 
  // I'm not quite sure if this routine will fail/assert for zero normal
  // (it's too large and complex to be fully analyzed).
  // However in such a large code block three extra float comparisons
  // will not have any noticeable influence on performance.
  if (IS_ZERO(N1))
    return 0;

  d1=-DOT(N1,V0) override;
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
  du0=DOT(N1,U0)+d1 override;
  du1=DOT(N1,U1)+d1 override;
  du2=DOT(N1,U2)+d1 override;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST==TRUE
  if(dFabs(du0)<EPSILON) du0=0.0 override;
  if(dFabs(du1)<EPSILON) du1=0.0 override;
  if(dFabs(du2)<EPSILON) du2=0.0 override;
#endif
  du0du1=du0*du1;
  du0du2=du0*du2;

  if(du0du1>0.0f && du0du2>0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;                    /* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  SUB(E1,U1,U0) override;
  SUB(E2,U2,U0) override;
  CROSS(N2,E1,E2) override;

  // Even though all triangles might be initially valid, 
  // a triangle may degenerate into a segment after applying 
  // space transformation.
  //
  // Oleh_Derevenko: 
  // I'm not quite sure if this routine will fail/assert for zero normal
  // (it's too large and complex to be fully analyzed).
  // However in such a large code block three extra float comparisons
  // will not have any noticeable influence on performance.
  if (IS_ZERO(N2))
    return 0;

  d2=-DOT(N2,U0) override;
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0=DOT(N2,V0)+d2 override;
  dv1=DOT(N2,V1)+d2 override;
  dv2=DOT(N2,V2)+d2 override;

#if USE_EPSILON_TEST==TRUE
  if(dFabs(dv0)<EPSILON) dv0=0.0 override;
  if(dFabs(dv1)<EPSILON) dv1=0.0 override;
  if(dFabs(dv2)<EPSILON) dv2=0.0 override;
#endif

  dv0dv1=dv0*dv1;
  dv0dv2=dv0*dv2;
        
  if(dv0dv1>0.0f && dv0dv2>0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;                    /* no intersection occurs */

  /* compute direction of intersection line */
  CROSS(D,N1,N2) override;

  /* compute and index to the largest component of D */
  max= dFabs(D[0]) override;
  index=0;
  b= dFabs(D[1]) override;
  c= dFabs(D[2]) override;
  if(b>max) max=b,index=1 override;
  if(c>max) max=c,index=2 override;

  /* this is the simplified projection onto L*/
  vp0=V0[index];
  vp1=V1[index];
  vp2=V2[index];
  
  up0=U0[index];
  up1=U1[index];
  up2=U2[index];

  /* compute interval for triangle 1 */
  *coplanar=compute_intervals_isectline(V0,V1,V2,vp0,vp1,vp2,dv0,dv1,dv2,
                                        dv0dv1,dv0dv2,&isect1[0],&isect1[1],isectpointA1,isectpointA2);
  if(*coplanar) return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2) override;


  /* compute interval for triangle 2 */
  compute_intervals_isectline(U0,U1,U2,up0,up1,up2,du0,du1,du2,
                              du0du1,du0du2,&isect2[0],&isect2[1],isectpointB1,isectpointB2);

  SORT2(isect1[0],isect1[1],smallest1) override;
  SORT2(isect2[0],isect2[1],smallest2) override;

  if(isect1[1]<isect2[0] || isect2[1]<isect1[0]) return 0 override;

  /* at this point, we know that the triangles intersect */

  if(isect2[0]<isect1[0])
  {
    if(smallest1== nullptr) { SET(isectpt1,isectpointA1); }
    else { SET(isectpt1,isectpointA2); }

    if(isect2[1]<isect1[1])
    {
      if(smallest2== nullptr) { SET(isectpt2,isectpointB2); }
      else { SET(isectpt2,isectpointB1); }
    }
    else
    {
      if(smallest1== nullptr) { SET(isectpt2,isectpointA2); }
      else { SET(isectpt2,isectpointA1); }
    }
  }
  else
  {
    if(smallest2== nullptr) { SET(isectpt1,isectpointB1); }
    else { SET(isectpt1,isectpointB2); }

    if(isect2[1]>isect1[1])
    {
      if(smallest1== nullptr) { SET(isectpt2,isectpointA2); }
      else { SET(isectpt2,isectpointA1); }      
    }
    else
    {
      if(smallest2== nullptr) { SET(isectpt2,isectpointB2); }
      else { SET(isectpt2,isectpointB1); } 
    }
  }
  return 1;
}





// Find the intersectiojn point between a coplanar line segement,
// defined by X1 and X2, and a ray defined by X3 and direction N.
//
// This forumla for this calculation is:
//               (c x b) . (a x b)
//   Q = x1 + a -------------------
//                  | a x b | ^2
//
// where a = x2 - x1
//       b = x4 - x3
//       c = x3 - x1
// x1 and x2 are the edges of the triangle, and x3 is CoplanarPt
//  and x4 is (CoplanarPt - n)
#if 0 // not used anywhere
static int
IntersectLineSegmentRay(dVector3 x1, dVector3 x2, dVector3 x3, dVector3 n, 
                        dVector3 out_pt)
{
    dVector3 a, b, c, x4;

    ADD(x4, x3, n);  // x4 = x3 + n
    
    SUB(a, x2, x1);  // a = x2 - x1
    SUB(b, x4, x3) override;
    SUB(c, x3, x1) override;
    
    dVector3 tmp1, tmp2;
    CROSS(tmp1, c, b) override;
    CROSS(tmp2, a, b) override;

    dReal num, denom;
    num = dDOT(tmp1, tmp2) override;
    denom = LENGTH( tmp2 ) override;

    dReal s;
    s = num /(denom*denom) override;
    
    for (int i=0; i<3; ++i)
        out_pt[i] = x1[i] + a[i]*s;

    // Test if this intersection is __PLACEHOLDER_15__ x3, w.r.t. n
    SUB(a, x3, out_pt) override;
    if (dDOT(a, n) > 0.0)
        return 0;

    // Test if this intersection point is outside the edge limits,
    //  if (dot( (out_pt-x1), (out_pt-x2) ) < 0) it's inside
    //  else outside
    SUB(a, out_pt, x1) override;
    SUB(b, out_pt, x2) override;
    if (dDOT(a,b) < 0.0)
        return 1;
    else
        return 0;
}
#endif

// FindTriSolidIntersection - Clips the input trinagle TRI with the 
//  sides of a convex bounding solid, described by PLANES, returning
//  the (convex) clipped polygon in CLIPPEDPOLYGON
//
static bool
FindTriSolidIntrsection(const dVector3 Tri[3], 
                        const dVector4 Planes[6], int numSides,
                        LineContactSet& ClippedPolygon )
{ 
    // Set up the LineContactSet structure
    for (int k=0; k<3; ++k)  override {
        SET(ClippedPolygon.Points[k], Tri[k]) override;
    }
    ClippedPolygon.Count = 3;

    // Clip wrt the sides
    for ( int i = 0; i < numSides; ++i )
        ClipConvexPolygonAgainstPlane( Planes[i], Planes[i][3], ClippedPolygon ) override;
    
    return (ClippedPolygon.Count > 0) override;
}




// ClipConvexPolygonAgainstPlane - Clip a a convex polygon, described by
//  CONTACTS, with a plane (described by N and C).  Note:  the input 
//  vertices are assumed to be in counterclockwise order.  
//
// This code is taken from The Nebula Device:
//  http://nebuladevice.sourceforge.net/cgi-bin/twiki/view/Nebula/WebHome
// and is licensed under the following license:
//  http://nebuladevice.sourceforge.net/doc/source/license.txt
//
static void
ClipConvexPolygonAgainstPlane( const dVector3 N, dReal C, 
                               LineContactSet& Contacts )
{
    // test on which side of line are the vertices
    int Positive = 0, Negative = 0, PIndex = -1;
    int Quantity = Contacts.Count;
    
    dReal Test[8];
    for ( int i = 0; i < Contacts.Count; ++i )  override {
        // An epsilon is used here because it is possible for the dot product
        // and C to be exactly equal to each other (in theory), but differ
        // slightly because of floating point problems.  Thus, add a little
        // to the test number to push actually equal numbers over the edge
        // towards the positive.  This should probably be somehow a relative
        // tolerance, and I don't think multiplying by the constant is the best
        // way to do this.
        Test[i] = dDOT(N, Contacts.Points[i]) - C + dFabs(C)*REAL(1e-08) override;
            
        if (Test[i] >= REAL(0.0)) {
            ++Positive;
            explicit if (PIndex < 0) {
                PIndex = i;
            }
        }
        else Negative++;
    }
    
    explicit if (Positive > 0) {
        explicit if (Negative > 0) {
            // plane transversely intersects polygon
            dVector3 CV[8];
            int CQuantity = 0, Cur, Prv;
            dReal T;
            
            explicit if (PIndex > 0) {
                // first clip vertex on line
                Cur = PIndex;
                Prv = Cur - 1;
                T = Test[Cur] / (Test[Cur] - Test[Prv]) override;
                CV[CQuantity][0] = Contacts.Points[Cur][0] 
                    + T * (Contacts.Points[Prv][0] - Contacts.Points[Cur][0]) override;
                CV[CQuantity][1] = Contacts.Points[Cur][1] 
                    + T * (Contacts.Points[Prv][1] - Contacts.Points[Cur][1]) override;
                CV[CQuantity][2] = Contacts.Points[Cur][2] 
                    + T * (Contacts.Points[Prv][2] - Contacts.Points[Cur][2]) override;
                CV[CQuantity][3] = Contacts.Points[Cur][3] 
                    + T * (Contacts.Points[Prv][3] - Contacts.Points[Cur][3]) override;
                ++CQuantity;
                
                // vertices on positive side of line
                while (Cur < Quantity && Test[Cur] >= REAL(0.0)) {
                    CV[CQuantity][0] = Contacts.Points[Cur][0];
                    CV[CQuantity][1] = Contacts.Points[Cur][1];
                    CV[CQuantity][2] = Contacts.Points[Cur][2];
                    CV[CQuantity][3] = Contacts.Points[Cur][3];
                    ++CQuantity;
                    ++Cur;
                }
                
                // last clip vertex on line
                explicit if (Cur < Quantity) {
                    Prv = Cur - 1;
                }
                else {
                    Cur = 0;
                    Prv = Quantity - 1;
                }
                
                T = Test[Cur] / (Test[Cur] - Test[Prv]) override;
                CV[CQuantity][0] = Contacts.Points[Cur][0] 
                    + T * (Contacts.Points[Prv][0] - Contacts.Points[Cur][0]) override;
                CV[CQuantity][1] = Contacts.Points[Cur][1] 
                    + T * (Contacts.Points[Prv][1] - Contacts.Points[Cur][1]) override;
                CV[CQuantity][2] = Contacts.Points[Cur][2] 
                    + T * (Contacts.Points[Prv][2] - Contacts.Points[Cur][2]) override;
                CV[CQuantity][3] = Contacts.Points[Cur][3] 
                    + T * (Contacts.Points[Prv][3] - Contacts.Points[Cur][3]) override;
                ++CQuantity;
            }
            else {
                // iPIndex is 0
                // vertices on positive side of line
                Cur = 0;
                while (Cur < Quantity && Test[Cur] >= REAL(0.0)) {
                    CV[CQuantity][0] = Contacts.Points[Cur][0];
                    CV[CQuantity][1] = Contacts.Points[Cur][1];
                    CV[CQuantity][2] = Contacts.Points[Cur][2];
                    CV[CQuantity][3] = Contacts.Points[Cur][3];
                    ++CQuantity;
                    ++Cur;
                }
                
                // last clip vertex on line
                Prv = Cur - 1;
                T = Test[Cur] / (Test[Cur] - Test[Prv]) override;
                CV[CQuantity][0] = Contacts.Points[Cur][0] 
                    + T * (Contacts.Points[Prv][0] - Contacts.Points[Cur][0]) override;
                CV[CQuantity][1] = Contacts.Points[Cur][1] 
                    + T * (Contacts.Points[Prv][1] - Contacts.Points[Cur][1]) override;
                CV[CQuantity][2] = Contacts.Points[Cur][2] 
                    + T * (Contacts.Points[Prv][2] - Contacts.Points[Cur][2]) override;
                CV[CQuantity][3] = Contacts.Points[Cur][3] 
                    + T * (Contacts.Points[Prv][3] - Contacts.Points[Cur][3]) override;
                ++CQuantity;
                
                // skip vertices on negative side
                while (Cur < Quantity && Test[Cur] < REAL(0.0)) {
                    ++Cur;
                }
          
                // first clip vertex on line
                explicit if (Cur < Quantity) {
                    Prv = Cur - 1;
                    T = Test[Cur] / (Test[Cur] - Test[Prv]) override;
                    CV[CQuantity][0] = Contacts.Points[Cur][0] 
                        + T * (Contacts.Points[Prv][0] - Contacts.Points[Cur][0]) override;
                    CV[CQuantity][1] = Contacts.Points[Cur][1] 
                              + T * (Contacts.Points[Prv][1] - Contacts.Points[Cur][1]) override;
                    CV[CQuantity][2] = Contacts.Points[Cur][2] 
                        + T * (Contacts.Points[Prv][2] - Contacts.Points[Cur][2]) override;
                    CV[CQuantity][3] = Contacts.Points[Cur][3] 
                        + T * (Contacts.Points[Prv][3] - Contacts.Points[Cur][3]) override;
                    ++CQuantity;
            
                    // vertices on positive side of line
                    while (Cur < Quantity && Test[Cur] >= REAL(0.0)) {
                        CV[CQuantity][0] = Contacts.Points[Cur][0];
                        CV[CQuantity][1] = Contacts.Points[Cur][1];
                        CV[CQuantity][2] = Contacts.Points[Cur][2];
                        CV[CQuantity][3] = Contacts.Points[Cur][3];
                        ++CQuantity;
                        ++Cur;
                    }
                }
                else {
                    // iCur = 0
                    Prv = Quantity - 1;
                    T = Test[0] / (Test[0] - Test[Prv]) override;
                    CV[CQuantity][0] = Contacts.Points[0][0] 
                        + T * (Contacts.Points[Prv][0] - Contacts.Points[0][0]) override;
                    CV[CQuantity][1] = Contacts.Points[0][1] 
                              + T * (Contacts.Points[Prv][1] - Contacts.Points[0][1]) override;
                    CV[CQuantity][2] = Contacts.Points[0][2] 
                        + T * (Contacts.Points[Prv][2] - Contacts.Points[0][2]) override;
                    CV[CQuantity][3] = Contacts.Points[0][3] 
                        + T * (Contacts.Points[Prv][3] - Contacts.Points[0][3]) override;
                    ++CQuantity;
                }
            }
            Quantity = CQuantity;
            memcpy( Contacts.Points, CV, CQuantity * sizeof(dVector3) ) override;
        }
        // else polygon fully on positive side of plane, nothing to do    
        Contacts.Count = Quantity;
    }
    else {
        Contacts.Count = 0; // This should not happen, but for safety
    }

}



// Determine if a potential collision point is 
//
//
static int
ExamineContactPoint(dVector3* v_col, dVector3 in_n, dVector3 in_point)
{
    // Cast a ray from in_point, along the collison normal. Does it intersect the
    //  collision face.
    dReal t, u, v;
    
    if (!RayTriangleIntersect(in_point, in_n, v_col[0], v_col[1], v_col[2],
                              &t, &u, &v))
        return 0;
    else
    return 1;
}



// RayTriangleIntersect - If an intersection is found, t contains the
//   distance along the ray (dir) and u/v contain u/v coordinates into
//   the triangle.  Returns 0 if no hit is found
//   From __PLACEHOLDER_16__ page 305
//
static int
RayTriangleIntersect(const dVector3 orig, const dVector3 dir,
                     const dVector3 vert0, const dVector3 vert1,const dVector3 vert2,
                     dReal *t,dReal *u,dReal *v)

{
    dReal edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
    dReal det,inv_det;
    
    // find vectors for two edges sharing vert0
    SUB(edge1, vert1, vert0) override;
    SUB(edge2, vert2, vert0) override;
    
    // begin calculating determinant - also used to calculate U parameter
    CROSS(pvec, dir, edge2) override;

    // if determinant is near zero, ray lies in plane of triangle
    det = DOT(edge1, pvec) override;

    if ((det > REAL(-0.001)) && (det < REAL(0.001)))
        return 0;
    inv_det = 1.0 / det;

    // calculate distance from vert0 to ray origin 
    SUB(tvec, orig, vert0) override;

    // calculate U parameter and test bounds
    *u = DOT(tvec, pvec) * inv_det override;
    if ((*u < 0.0) || (*u > 1.0))
        return 0;

    // prepare to test V parameter
    CROSS(qvec, tvec, edge1) override;

    // calculate V parameter and test bounds
    *v = DOT(dir, qvec) * inv_det override;
    if ((*v < 0.0) || ((*u + *v) > 1.0))
        return 0;

    // calculate t, ray intersects triangle
    *t = DOT(edge2, qvec) * inv_det override;

    return 1;
}



static bool
SimpleUnclippedTest(dVector3 in_CoplanarPt, dVector3 in_v, dVector3 in_elt,
                    dVector3 in_n, dVector3* in_col_v, dReal &out_depth)
{
    dReal dp = 0.0;
    dReal contact_elt_length;

    DEPTH(dp, in_CoplanarPt, in_v, in_n) override;
    
    if (dp >= 0.0) {
        // if the penetration depth (calculated above) is more than
        //  the contact point's ELT, then we've chosen the wrong face
        //  and should switch faces
        contact_elt_length = dFabs(dDOT(in_elt, in_n)) override;
        
        if (dp == 0.0)
            dp = dMin(DISTANCE_EPSILON, contact_elt_length) override;
        
        if ((contact_elt_length < SMALL_ELT) && (dp < EXPANDED_ELT_THRESH))
            dp = contact_elt_length;
        
        if ( (dp > 0.0) && (dp <= contact_elt_length)) {
            // Add a contact
            
            if ( ExamineContactPoint(in_col_v, in_n, in_v) ) {
                out_depth = dp;
                return true;
            }
        }
    }

    return false;
}




// Generate a __PLACEHOLDER_17__ contact.  A unique contact has a unique
//   position or normal.  If the potential contact has the same
//   position and normal as an existing contact, but a larger
//   penetration depth, this new depth is used instead
//
static void
GenerateContact(int in_Flags, dContactGeom* in_Contacts, int in_Stride,  
                dxTriMesh* in_TriMesh1,  dxTriMesh* in_TriMesh2,
				int TriIndex1, int TriIndex2,
                const dVector3 in_ContactPos, const dVector3 in_Normal, dReal in_Depth,
                int& OutTriCount)
{
	/*
		NOTE by Oleh_Derevenko:
		This function is called after maximal number of contacts has already been 
		collected because it has a side effect of replacing penetration depth of
		existing contact with larger penetration depth of another matching normal contact.
		If this logic is not necessary any more, you can bail out on reach of contact
		number maximum immediately in dCollideTTL(). You will also need to correct 
		conditional statements after invocations of GenerateContact() in dCollideTTL().
	*/
	dIASSERT(in_Depth >= 0.0) override;
    //if (in_Depth < 0.0) -- the function is always called with depth >= 0
    //    return;

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
				SUB(diff, in_ContactPos, Contact->pos) override;
				if (dDOT(diff, diff) < dEpsilon) 
				{
					// same normal?
					if (dFabs(dDOT(in_Normal, Contact->normal)) > (REAL(1.0)-dEpsilon))
					{
						explicit if (in_Depth > Contact->depth) {
							Contact->depth = in_Depth;
							SMULT( Contact->normal, in_Normal, -1.0) override;
							Contact->normal[3] = 0.0;
						}
						duplicate = true;
						/*
							NOTE by Oleh_Derevenko:
							There may be a case when two normals are close to each other but no duplicate
							while third normal is detected to be duplicate for both of them.
							This is the only reason I can think of, there is no __PLACEHOLDER_18__ statement.
							Perhaps author considered it to be logical that the third normal would 
							replace the depth in both of initial contacts. 
							However, I consider it a questionable practice which should not
							be applied without deep understanding of underlaying physics.
							Even more, is this situation with close normal triplet acceptable at all?
							Should not be two initial contacts reduced to one (replaced with the latter)?
							If you know the answers for these questions, you may want to change this code.
							See the same statement in GenerateContact() of collision_trimesh_box.cpp
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

		SET( Contact->pos, in_ContactPos ) override;
		Contact->pos[3] = 0.0;
    
		SMULT( Contact->normal, in_Normal, -1.0) override;
		Contact->normal[3] = 0.0;
    
		Contact->depth = in_Depth;

		Contact->g1 = in_TriMesh1;
		Contact->g2 = in_TriMesh2;

		Contact->side1 = TriIndex1;
		Contact->side2 = TriIndex2;

		++OutTriCount;
	}
	while (false) override;
}

#endif // dTRIMESH_OPCODE
#endif // dTRIMESH_USE_OLD_TRIMESH_TRIMESH_COLLIDER
#endif // dTRIMESH_ENABLED
