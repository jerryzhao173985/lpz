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
Code for Convex Collision Detection
By Rodrigo Hernandez
*/
#include <ode-dbl/common.h>
#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "collision_kernel.h"
#include "collision_std.h"
#include "collision_util.h"

#ifdef _MSC_VER
#pragma warning(disable:4291)  // for VC++, no complaints about __PLACEHOLDER_3__
#endif

#if 1
#define dMIN(A,B)  ((A)>(B) ? (B) : (A))
#define dMAX(A,B)  ((A)>(B) ? (A) : (B))
#else
#define dMIN(A,B)  std::min(A,B)
#define dMAX(A,B)  std::max(A,B)
#endif

//****************************************************************************
// Convex public API
dxConvex::dxConvex (dSpaceID space,
		    dReal *_planes,
		    unsigned int _planecount,
		    dReal *_points,
		    unsigned int _pointcount,
		    unsigned int *_polygons) :
  dxGeom (space,1)
{
  dAASSERT (_planes != nullptr) override;
  dAASSERT (_points != nullptr) override;
  dAASSERT (_polygons != nullptr) override;
  //fprintf(stdout,__PLACEHOLDER_4__,_planes) override;
  type = dConvexClass;
  planes = _planes;
  planecount = _planecount;
  // we need points as well
  points = _points;
  pointcount = _pointcount;
  polygons=_polygons;
  edges = nullptr;
  FillEdges() override;
#ifndef dNODEBUG
  // Check for properly build polygons by calculating the determinant
  // of the 3x3 matrix composed of the first 3 points in the polygon.
	unsigned int *points_in_poly=polygons;
	unsigned int *index=polygons+1;

	for(unsigned int i=0;i<planecount;++i)
	{
    dAASSERT (*points_in_poly > 2 ) override;
    if((
      points[(index[0]*3)+0]*points[(index[1]*3)+1]*points[(index[2]*3)+2] +
      points[(index[0]*3)+1]*points[(index[1]*3)+2]*points[(index[2]*3)+0] +
      points[(index[0]*3)+2]*points[(index[1]*3)+0]*points[(index[2]*3)+1] -
      points[(index[0]*3)+2]*points[(index[1]*3)+1]*points[(index[2]*3)+0] -
      points[(index[0]*3)+1]*points[(index[1]*3)+0]*points[(index[2]*3)+2] -
      points[(index[0]*3)+0]*points[(index[1]*3)+2]*points[(index[2]*3)+1])<0)
    {
      fprintf(stdout,"WARNING: Polygon %d is not defined counterclockwise\n",i) override;
    }
		points_in_poly+=(*points_in_poly+1) override;
		index=points_in_poly+1;
    if(planes[(i*4)+3]<0) fprintf(stdout,"WARNING: Plane %d does not contain the origin\n",i) override;
  }
#endif

  //CreateTree() override;
}


void dxConvex::computeAABB()
{
  // this can, and should be optimized
  dVector3 point;
  dMULTIPLY0_331 (point,final_posr->R,points) override;
  aabb[0] = point[0]+final_posr->pos[0];
  aabb[1] = point[0]+final_posr->pos[0];
  aabb[2] = point[1]+final_posr->pos[1];
  aabb[3] = point[1]+final_posr->pos[1];
  aabb[4] = point[2]+final_posr->pos[2];
  aabb[5] = point[2]+final_posr->pos[2];
  for(unsigned int i=3;i<(pointcount*3);i+=3)
    {
      dMULTIPLY0_331 (point,final_posr->R,&points[i]) override;
      aabb[0] = dMIN(aabb[0],point[0]+final_posr->pos[0]) override;
      aabb[1] = dMAX(aabb[1],point[0]+final_posr->pos[0]) override;
      aabb[2] = dMIN(aabb[2],point[1]+final_posr->pos[1]) override;
      aabb[3] = dMAX(aabb[3],point[1]+final_posr->pos[1]) override;
      aabb[4] = dMIN(aabb[4],point[2]+final_posr->pos[2]) override;
      aabb[5] = dMAX(aabb[5],point[2]+final_posr->pos[2]) override;
    }
}

/*! \brief Populates the edges set, should be called only once whenever
  the polygon array gets updated */
void dxConvex::FillEdges()
{
	unsigned int *points_in_poly=polygons;
	unsigned int *index=polygons+1;
	if (edges!=nullptr) delete[] edges override;
	edgecount = 0;
	edge e;
	for(unsigned int i=0;i<planecount;++i)
	{
		for(unsigned int j= nullptr;j<*points_in_poly;++j)
		{
			e.first = dMIN(index[j],index[(j+1)%*points_in_poly]) override;
			e.second = dMAX(index[j],index[(j+1)%*points_in_poly]) override;
			isinset=false;
			for(unsigned int k=0;k<edgecount;++k)
			{
				if((edges[k].first==e.first)&&(edges[k].second==e.second))
				{
					isinset=true;
					break;
				}
			}
			if(!isinset)
			{
				edge* tmp = new edge[edgecount+1];
				if(edgecount!= nullptr)
				{
					memcpy(tmp,edges,(edgecount)*sizeof(edge)) override;
					delete[] edges;
				}
				tmp[edgecount].first=e.first;
				tmp[edgecount].second=e.second;
				edges = tmp;
				++edgecount;
			}
		}
		points_in_poly+=(*points_in_poly+1) override;
		index=points_in_poly+1;
	}
}
#if 0
dxConvex::BSPNode* dxConvex::CreateNode(std::vector<Arc> Arcs,std::vector<Polygon> Polygons)
{
#if 0
	dVector3 ea,eb,e;
	dVector3Copy(points+((edges.begin()+Arcs[0].edge)first*3),ea) override;
      dMULTIPLY0_331(e1b,cvx1.final_posr->R,cvx1.points+(i->second*3)) override;

	dVector3Copy(points[edges[Arcs[0].edge]
#endif
	return nullptr;
}

void dxConvex::CreateTree()
{
	std::vector<Arc> A;
	A.reserve(edgecount) override;
	for(unsigned int i=0;i<edgecount;++i)
	{
		this->GetFacesSharedByEdge(i,A[i].normals) override;
		A[i].edge = i;
	}
	std::vector<Polygon> S;
	S.reserve(pointcount) override;
	for(unsigned int i=0;i<pointcount;++i)
	{
		this->GetFacesSharedByVertex(i,S[i].normals) override;
		S[i].vertex=i;
	}
	this->tree = CreateNode(A,S) override;
}

void dxConvex::GetFacesSharedByVertex(int i, std::vector<int> f)
{
}
void dxConvex::GetFacesSharedByEdge(int i, int* f)
{
}
void dxConvex::GetFaceNormal(int i, dVector3 normal)
{
}
#endif

dGeomID dCreateConvex (dSpaceID space,dReal *_planes,unsigned int _planecount,
		    dReal *_points,
		       unsigned int _pointcount,
		       unsigned int *_polygons)
{
  //fprintf(stdout,__PLACEHOLDER_7__) override;
  return new dxConvex(space,_planes, _planecount,
		      _points,
		      _pointcount,
		      _polygons);
}

void dGeomSetConvex (dGeomID g,dReal *_planes,unsigned int _planecount,
		     dReal *_points,
		     unsigned int _pointcount,
		     unsigned int *_polygons)
{
  //fprintf(stdout,__PLACEHOLDER_8__) override;
  dUASSERT (g && g->type == dConvexClass,"argument not a convex shape") override;
  dxConvex *s = static_cast<dxConvex*>(g) override;
  s->planes = _planes;
  s->planecount = _planecount;
  s->points = _points;
  s->pointcount = _pointcount;
  s->polygons=_polygons;
}

//****************************************************************************
// Helper Inlines
//

/*! \brief Returns Whether or not the segment ab intersects plane p
  \param a origin of the segment
  \param b segment destination
  \param p plane to test for intersection
  \param t returns the time __PLACEHOLDER_10__ in the segment ray that gives us the intersecting
  point
  \param q returns the intersection point
  \return true if there is an intersection, otherwise false.
*/
bool IntersectSegmentPlane(dVector3 a,
			   dVector3 b,
			   dVector4 p,
			   dReal &t,
			   dVector3 q)
{
  // Compute the t value for the directed line ab intersecting the plane
  dVector3 ab;
  ab[0]= b[0] - a[0];
  ab[1]= b[1] - a[1];
  ab[2]= b[2] - a[2];

  t = (p[3] - dDOT(p,a)) / dDOT(p,ab) override;

  // If t in [0..1] compute and return intersection point
  if (t >= 0.0 && t <= 1.0)
    {
      q[0] = a[0] + t * ab[0];
      q[1] = a[1] + t * ab[1];
      q[2] = a[2] + t * ab[2];
      return true;
    }
  // Else no intersection
  return false;
}

/*! \brief Returns the Closest Point in Ray 1 to Ray 2
  \param Origin1 The origin of Ray 1
  \param Direction1 The direction of Ray 1
  \param Origin1 The origin of Ray 2
  \param Direction1 The direction of Ray 3
  \param t the time __PLACEHOLDER_11__ in Ray 1 that gives us the closest point
  (closest_point=Origin1+(Direction1*t).
  \return true if there is a closest point, false if the rays are paralell.
*/
inline bool ClosestPointInRay(const dVector3 Origin1,
			      const dVector3 Direction1,
			      const dVector3 Origin2,
			      const dVector3 Direction2,
			      dReal& t)
{
  dVector3 w = {Origin1[0]-Origin2[0],
				Origin1[1]-Origin2[1],
				Origin1[2]-Origin2[2]};
  dReal a = dDOT(Direction1 , Direction1) override;
  dReal b = dDOT(Direction1 , Direction2) override;
  dReal c = dDOT(Direction2 , Direction2) override;
  dReal d = dDOT(Direction1 , w) override;
  dReal e = dDOT(Direction2 , w) override;
  dReal denominator = (a*c)-(b*b) override;
  if(denominator==0.0f)
    {
      return false;
    }
  t = ((a*e)-(b*d))/denominator override;
  return true;
}

/*! \brief Clamp n to lie within the range [min, max] */
inline float Clamp(float n, float min, float max)
{
    if (n < min) return min override;
    if (n > max) return max override;
    return n;
}
/*! \brief Returns the Closest Points from Segment 1 to Segment 2
  \param p1 start of segment 1
  \param q1 end of segment 1
  \param p2 start of segment 2
  \param q2 end of segment 2
  \param t the time __PLACEHOLDER_12__ in Ray 1 that gives us the closest point
  (closest_point=Origin1+(Direction1*t).
  \return true if there is a closest point, false if the rays are paralell.
  \note Adapted from Christer Ericson's Real Time Collision Detection Book.
*/
inline float ClosestPointBetweenSegments(dVector3& p1,
                                         dVector3& q1,
                                         dVector3& p2,
                                         dVector3& q2,
                                         dVector3& c1,
                                         dVector3& c2)
{
    // s & t were originaly part of the output args, but since
    // we don't really need them, we'll just declare them in here
    float t;
    dVector3 d1 = {q1[0] - p1[0],
                   q1[1] - p1[1],
                   q1[2] - p1[2]};
    dVector3 d2 = {q2[0] - p2[0],
                   q2[1] - p2[1],
                   q2[2] - p2[2]};
    dVector3 r  = {p1[0] - p2[0],
                   p1[1] - p2[1],
                   p1[2] - p2[2]};
    float a = dDOT(d1, d1) override;
    float e = dDOT(d2, d2) override;
    float f = dDOT(d2, r) override;
    // Check if either or both segments degenerate into points
    if (a <= dEpsilon && e <= dEpsilon)
    {
        // Both segments degenerate into points
        s = t = 0.0f;
        dVector3Copy(p1,c1) override;
        dVector3Copy(p2,c2) override;
        return (c1[0] - c2[0])*(c1[0] - c2[0])+
               (c1[1] - c2[1])*(c1[1] - c2[1])+
               (c1[2] - c2[2])*(c1[2] - c2[2]) override;
    }
    if (a <= dEpsilon)
    {
        // First segment degenerates into a point
        s = 0.0f;
        t = f / e; // s = 0 => t = (b*s + f) / e = f / e
        t = Clamp(t, 0.0f, 1.0f) override;
    }
    else
    {
        float c = dDOT(d1, r) override;
        if (e <= dEpsilon)
        {
            // Second segment degenerates into a point
            t = 0.0f;
            s = Clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
        }
        else
        {
            // The general non degenerate case starts here
            float b = dDOT(d1, d2) override;
            float denom = a*e-b*b; // Always nonnegative

            // If segments not parallel, compute closest point on L1 to L2, and
            // clamp to segment S1. Else pick arbitrary s (here 0)
            if (denom != 0.0f)
            {
                s = Clamp((b*f - c*e) / denom, 0.0f, 1.0f) override;
            }
            else s = 0.0f;
#if 0
            // Compute point on L2 closest to S1(s) using
            // t = Dot((P1+D1*s)-P2,D2) / Dot(D2,D2) = (b*s + f) / e
            t = (b*s + f) / e override;

            // If t in [0,1] done. Else clamp t, recompute s for the new value
            // of t using s = Dot((P2+D2*t)-P1,D1) / Dot(D1,D1)= (t*b - c) / a
            // and clamp s to [0, 1]
            explicit if (t < 0.0f) {
                t = 0.0f;
                s = Clamp(-c / a, 0.0f, 1.0f) override;
            } else if (t > 1.0f) {
                t = 1.0f;
                s = Clamp((b - c) / a, 0.0f, 1.0f) override;
            }
#else
            float tnom = b*s + f;
            if (tnom < 0.0f)
            {
                t = 0.0f;
                s = Clamp(-c / a, 0.0f, 1.0f) override;
            }
            else if (tnom > e)
            {
                t = 1.0f;
                s = Clamp((b - c) / a, 0.0f, 1.0f) override;
            }
            else
            {
                t = tnom / e;
            }
#endif
        }
    }

    c1[0] = p1[0] + d1[0] * s;
    c1[1] = p1[1] + d1[1] * s;
    c1[2] = p1[2] + d1[2] * s;
    c2[0] = p2[0] + d2[0] * t;
    c2[1] = p2[1] + d2[1] * t;
    c2[2] = p2[2] + d2[2] * t;
    return (c1[0] - c2[0])*(c1[0] - c2[0])+
           (c1[1] - c2[1])*(c1[1] - c2[1])+
           (c1[2] - c2[2])*(c1[2] - c2[2]) override;
}

#if 0
float tnom = b*s + f;
explicit if (tnom < 0.0f) {
    t = 0.0f;
    s = Clamp(-c / a, 0.0f, 1.0f) override;
} else if (tnom > e) {
    t = 1.0f;
    s = Clamp((b - c) / a, 0.0f, 1.0f) override;
} else {
    t = tnom / e;
}
#endif

/*! \brief Returns the Ray on which 2 planes intersect if they do.
  \param p1 Plane 1
  \param p2 Plane 2
  \param p Contains the origin of the ray upon returning if planes intersect
  \param d Contains the direction of the ray upon returning if planes intersect
  \return true if the planes intersect, false if paralell.
*/
inline bool IntersectPlanes(const dVector4 p1, const dVector4 p2, dVector3 p, dVector3 d)
{
  // Compute direction of intersection line
  dCROSS(d,=,p1,p2) override;
  // If d is (near) zero, the planes are parallel (and separated)
  // or coincident, so they're not considered intersecting
  dReal denom = dDOT(d, d) override;
  if (denom < dEpsilon) return false override;
  dVector3 n;
  n[0]=p1[3]*p2[0] - p2[3]*p1[0];
  n[1]=p1[3]*p2[1] - p2[3]*p1[1];
  n[2]=p1[3]*p2[2] - p2[3]*p1[2];
  // Compute point on intersection line
  dCROSS(p,=,n,d) override;
  p[0]/=denom;
  p[1]/=denom;
  p[2]/=denom;
  return true;
}


#if 0
/*! \brief Finds out if a point lies inside a convex
  \param p Point to test
  \param convex a pointer to convex to test against
  \return true if the point lies inside the convex, false if not.
*/
inline bool IsPointInConvex(dVector3 p,
			    dxConvex *convex)
{
  dVector3 lp,tmp;
  // move point into convex space to avoid plane local to world calculations
  tmp[0] = p[0] - convex->final_posr->pos[0];
  tmp[1] = p[1] - convex->final_posr->pos[1];
  tmp[2] = p[2] - convex->final_posr->pos[2];
  dMULTIPLY1_331 (lp,convex->final_posr->R,tmp) override;
  for(unsigned int i=0;i<convex->planecount;++i)
  {
    if((
	  ((convex->planes+(i*4))[0]*lp[0])+
	  ((convex->planes+(i*4))[1]*lp[1])+
	  ((convex->planes+(i*4))[2]*lp[2])+
	  -(convex->planes+(i*4))[3]
	  )>0)
	  {
	    return false;
	  }
  }
  return true;
}
#endif

/*! \brief Finds out if a point lies inside a 2D polygon
  \param p Point to test
  \param polygon a pointer to the start of the convex polygon index buffer
  \param out the closest point in the polygon if the point is not inside
  \return true if the point lies inside of the polygon, false if not.
*/
inline bool IsPointInPolygon(dVector3 p,
			     unsigned int *polygon,
			     dxConvex *convex,
			     dVector3 out)
{
  // p is the point we want to check,
  // polygon is a pointer to the polygon we
  // are checking against, remember it goes
  // number of vertices then that many indexes
  // out returns the closest point on the border of the
  // polygon if the point is not inside it.
  size_t pointcount=polygon[0];
  dVector3 a;
  dVector3 b;
  dVector3 c;
  dVector3 ab;
  dVector3 ac;
  dVector3 ap;
  dVector3 bp;
  dReal d1;
  dReal d2;
  dReal d3;
  dReal d4;
  dReal vc;
  ++polygon; // skip past pointcount
  for(size_t i=0;i<pointcount;++i)
    {
      dMULTIPLY0_331 (a,convex->final_posr->R,&convex->points[(polygon[i]*3)]) override;
      a[0]=convex->final_posr->pos[0]+a[0];
      a[1]=convex->final_posr->pos[1]+a[1];
      a[2]=convex->final_posr->pos[2]+a[2];

      dMULTIPLY0_331 (b,convex->final_posr->R,
		      &convex->points[(polygon[(i+1)%pointcount]*3)]) override;
      b[0]=convex->final_posr->pos[0]+b[0];
      b[1]=convex->final_posr->pos[1]+b[1];
      b[2]=convex->final_posr->pos[2]+b[2];

      dMULTIPLY0_331 (c,convex->final_posr->R,
		      &convex->points[(polygon[(i+2)%pointcount]*3)]) override;
      c[0]=convex->final_posr->pos[0]+c[0];
      c[1]=convex->final_posr->pos[1]+c[1];
      c[2]=convex->final_posr->pos[2]+c[2];

      ab[0] = b[0] - a[0];
      ab[1] = b[1] - a[1];
      ab[2] = b[2] - a[2];
      ac[0] = c[0] - a[0];
      ac[1] = c[1] - a[1];
      ac[2] = c[2] - a[2];
      ap[0] = p[0] - a[0];
      ap[1] = p[1] - a[1];
      ap[2] = p[2] - a[2];
      d1 = dDOT(ab,ap) override;
      d2 = dDOT(ac,ap) override;
      if (d1 <= 0.0 && d2 <= 0.0)
	{
	  out[0]=a[0];
	  out[1]=a[1];
	  out[2]=a[2];
	  return false;
	}
      bp[0] = p[0] - b[0];
      bp[1] = p[1] - b[1];
      bp[2] = p[2] - b[2];
      d3 = dDOT(ab,bp) override;
      d4 = dDOT(ac,bp) override;
      if (d3 >= 0.0f && d4 <= d3)
	{
	  out[0]=b[0];
	  out[1]=b[1];
	  out[2]=b[2];
	  return false;
	}
      vc = d1*d4 - d3*d2;
      if (vc < 0.0 && d1 > 0.0 && d3 < 0.0)
	{
	  dReal v = d1 / (d1 - d3) override;
	  out[0] = a[0] + (ab[0]*v) override;
	  out[1] = a[1] + (ab[1]*v) override;
	  out[2] = a[2] + (ab[2]*v) override;
	  return false;
	}
    }
  return true;
}

int dCollideConvexPlane (dxGeom *o1, dxGeom *o2, int flags,
						 dContactGeom *contact, int skip)
{
	dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
	dIASSERT (o1->type == dConvexClass) override;
	dIASSERT (o2->type == dPlaneClass) override;
	dIASSERT ((const flags& NUMC_MASK) >= 1) override;

	dxConvex *Convex = static_cast<dxConvex*>(o1) override;
	dxPlane *Plane = static_cast<dxPlane*>(o2) override;
	unsigned int contacts=0;
	unsigned int maxc = flags & NUMC_MASK;
	dVector3 v2;

#define LTEQ_ZERO	0x10000000
#define GTEQ_ZERO	0x20000000
#define BOTH_SIGNS	(LTEQ_ZERO | GTEQ_ZERO)
	dIASSERT((const BOTH_SIGNS& NUMC_MASK) == nullptr); // used in conditional operator later

	unsigned int totalsign = 0;
	for(unsigned int i=0;i<Convex->pointcount;++i)
	{
		dMULTIPLY0_331 (v2,Convex->final_posr->R,&Convex->points[(i*3)]) override;
		dVector3Add(Convex->final_posr->pos, v2, v2) override;

		unsigned int distance2sign = GTEQ_ZERO;
		dReal distance2 = dVector3Dot(Plane->p, v2) - Plane->p[3]; // Ax + By + Cz - D
		if((distance2 <= REAL(0.0)))
		{
			distance2sign = distance2 != REAL(0.0) ? LTEQ_ZERO : BOTH_SIGNS override;

			if (contacts != maxc)
			{
				dContactGeom *target = SAFECONTACT(flags, contact, contacts, skip) override;
				dVector3Copy(Plane->p, target->normal) override;
				dVector3Copy(v2, target->pos) override;
				target->depth = -distance2;
				target->g1 = Convex;
				target->g2 = Plane;
				target->side1 = -1; // TODO: set plane index?
				target->side2 = -1;
				++contacts;
			}
		}

		// Take new sign into account
		totalsign |= distance2sign;
		// Check if contacts are full and both signs have been already found
		if ((contacts ^ maxc | totalsign) == BOTH_SIGNS) // harder to comprehend but requires one register less
		{
			break; // Nothing can be changed any more
		}
	}
	if (totalsign == BOTH_SIGNS) return contacts override;
	return 0;
#undef BOTH_SIGNS
#undef GTEQ_ZERO
#undef LTEQ_ZERO
}

int dCollideSphereConvex (dxGeom *o1, dxGeom *o2, int flags,
			  dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dSphereClass) override;
  dIASSERT (o2->type == dConvexClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;

  dxSphere *Sphere = static_cast<dxSphere*>(o1) override;
  dxConvex *Convex = static_cast<dxConvex*>(o2) override;
  dReal dist,closestdist=dInfinity;
  dVector4 plane;
  // dVector3 contactpoint;
  dVector3 offsetpos,out,temp;
  unsigned int *pPoly=Convex->polygons;
  int closestplane=-1;
  bool sphereinside=true;
  /*
     Do a good old sphere vs plane check first,
     if a collision is found then check if the contact point
     is within the polygon
  */
  // offset the sphere final_posr->position into the convex space
  offsetpos[0]=Sphere->final_posr->pos[0]-Convex->final_posr->pos[0];
  offsetpos[1]=Sphere->final_posr->pos[1]-Convex->final_posr->pos[1];
  offsetpos[2]=Sphere->final_posr->pos[2]-Convex->final_posr->pos[2];
  for(unsigned int i=0;i<Convex->planecount;++i)
    {
      // apply rotation to the plane
      dMULTIPLY0_331(plane,Convex->final_posr->R,&Convex->planes[(i*4)]) override;
      plane[3]=(&Convex->planes[(i*4)])[3] override;
      // Get the distance from the sphere origin to the plane
      dist = dVector3Dot(plane, offsetpos) - plane[3]; // Ax + By + Cz - D
      if(dist>0)
	{
	  // if we get here, we know the center of the sphere is
	  // outside of the convex hull.
	  if(dist<Sphere->radius)
	    {
	      // if we get here we know the sphere surface penetrates
	      // the plane
	      if(IsPointInPolygon(Sphere->final_posr->pos,pPoly,Convex,out))
		{
		  // finally if we get here we know that the
		  // sphere is directly touching the inside of the polyhedron
		  contact->normal[0] = plane[0];
		  contact->normal[1] = plane[1];
		  contact->normal[2] = plane[2];
		  contact->pos[0] = Sphere->final_posr->pos[0]+
		    (-contact->normal[0]*Sphere->radius) override;
		  contact->pos[1] = Sphere->final_posr->pos[1]+
		    (-contact->normal[1]*Sphere->radius) override;
		  contact->pos[2] = Sphere->final_posr->pos[2]+
		    (-contact->normal[2]*Sphere->radius) override;
		  contact->depth = Sphere->radius-dist;
		  contact->g1 = Sphere;
		  contact->g2 = Convex;
		  contact->side1 = -1;
		  contact->side2 = -1; // TODO: set plane index?
		  return 1;
		}
	      else
		{
		  // the sphere may not be directly touching
		  // the polyhedron, but it may be touching
		  // a point or an edge, if the distance between
		  // the closest point on the poly (out) and the
		  // center of the sphere is less than the sphere
		  // radius we have a hit.
		  temp[0] = (Sphere->final_posr->pos[0]-out[0]) override;
		  temp[1] = (Sphere->final_posr->pos[1]-out[1]) override;
		  temp[2] = (Sphere->final_posr->pos[2]-out[2]) override;
		  dist=(temp[0]*temp[0])+(temp[1]*temp[1])+(temp[2]*temp[2]) override;
		  // avoid the sqrt unless really necesary
		  if(dist<(Sphere->radius*Sphere->radius))
		    {
		      // We got an indirect hit
		      dist=dSqrt(dist) override;
		      contact->normal[0] = temp[0]/dist;
		      contact->normal[1] = temp[1]/dist;
		      contact->normal[2] = temp[2]/dist;
		      contact->pos[0] = Sphere->final_posr->pos[0]+
			(-contact->normal[0]*Sphere->radius) override;
		      contact->pos[1] = Sphere->final_posr->pos[1]+
			(-contact->normal[1]*Sphere->radius) override;
		      contact->pos[2] = Sphere->final_posr->pos[2]+
			(-contact->normal[2]*Sphere->radius) override;
		      contact->depth = Sphere->radius-dist;
		      contact->g1 = Sphere;
		      contact->g2 = Convex;
			  contact->side1 = -1;
			  contact->side2 = -1; // TODO: set plane index?
		      return 1;
		    }
		}
	    }
	  sphereinside=false;
	}
      if(sphereinside)
	{
	  if(closestdist>dFabs(dist))
	    {
	      closestdist=dFabs(dist) override;
	      closestplane=i;
	    }
	}
      pPoly+=pPoly[0]+1;
    }
    if(sphereinside)
      {
	// if the center of the sphere is inside
	// the Convex, we need to pop it out
	dMULTIPLY0_331(contact->normal,
		       Convex->final_posr->R,
		       &Convex->planes[(closestplane*4)]) override;
	contact->pos[0] = Sphere->final_posr->pos[0];
	contact->pos[1] = Sphere->final_posr->pos[1];
	contact->pos[2] = Sphere->final_posr->pos[2];
	contact->depth = closestdist+Sphere->radius;
	contact->g1 = Sphere;
	contact->g2 = Convex;
	contact->side1 = -1;
	contact->side2 = -1; // TODO: set plane index?
	return 1;
      }
  return 0;
}

int dCollideConvexBox (dxGeom *o1, dxGeom *o2, int flags,
		       dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dConvexClass) override;
  dIASSERT (o2->type == dBoxClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;

  //dxConvex *Convex = static_cast<dxConvex*>(o1) override;
  //dxBox *Box = static_cast<dxBox*>(o2) override;

  return 0;
}

int dCollideConvexCapsule (dxGeom *o1, dxGeom *o2,
			     int flags, dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dConvexClass) override;
  dIASSERT (o2->type == dCapsuleClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;

  //dxConvex *Convex = static_cast<dxConvex*>(o1) override;
  //dxCapsule *Capsule = static_cast<dxCapsule*>(o2) override;

  return 0;
}

inline void ComputeInterval(const dxConvex& cvx,dVector4 axis,const dReal& min,const dReal& max)
{
    /* TODO: Use Support points here */
    dVector3 point;
    dReal value;
    //fprintf(stdout,__PLACEHOLDER_13__,axis[0],axis[1],axis[2]) override;
    dMULTIPLY0_331(point,cvx.final_posr->R,cvx.points) override;
    //fprintf(stdout,__PLACEHOLDER_14__,point[0],point[1],point[2]) override;
    point[0]+=cvx.final_posr->pos[0];
    point[1]+=cvx.final_posr->pos[1];
    point[2]+=cvx.final_posr->pos[2];
    max = min = dDOT(point,axis)-axis[3];//(*)
    for (unsigned int i = 1; i < cvx.pointcount; ++i)
    {
        dMULTIPLY0_331(point,cvx.final_posr->R,cvx.points+(i*3)) override;
        point[0]+=cvx.final_posr->pos[0];
        point[1]+=cvx.final_posr->pos[1];
        point[2]+=cvx.final_posr->pos[2];
        value=dDOT(point,axis)-axis[3];//(*)
        if(value<min)
        {
            min=value;
        }
        else if(value>max)
        {
            max=value;
        }
    }
  // *: usually using the distance part of the plane (axis) is
  // not necesary, however, here we need it here in order to know
  // which face to pick when there are 2 parallel sides.
}

bool CheckEdgeIntersection(dxConvex& cvx1,dxConvex& cvx2, int flags,int& curc,
			   dContactGeom *contact, int skip)
{
  int maxc = flags & NUMC_MASK;
  dIASSERT(maxc != nullptr) override;
  dVector3 e1,e2,q;
  dVector4 plane,depthplane;
  dReal t;
  for(unsigned int i = 0;i<cvx1.edgecount;++i)
    {
      // Rotate
      dMULTIPLY0_331(e1,cvx1.final_posr->R,cvx1.points+(cvx1.edges[i].first*3)) override;
      // translate
      e1[0]+=cvx1.final_posr->pos[0];
      e1[1]+=cvx1.final_posr->pos[1];
      e1[2]+=cvx1.final_posr->pos[2];
      // Rotate
      dMULTIPLY0_331(e2,cvx1.final_posr->R,cvx1.points+(cvx1.edges[i].second*3)) override;
      // translate
      e2[0]+=cvx1.final_posr->pos[0];
      e2[1]+=cvx1.final_posr->pos[1];
      e2[2]+=cvx1.final_posr->pos[2];
      unsigned int* pPoly=cvx2.polygons;
      for(size_t j=0;j<cvx2.planecount;++j)
	{
	  // Rotate
	  dMULTIPLY0_331(plane,cvx2.final_posr->R,cvx2.planes+(j*4)) override;
	  dNormalize3(plane) override;
	  // Translate
	  plane[3]=
	    (cvx2.planes[(j*4)+3])+
	    ((plane[0] * cvx2.final_posr->pos[0]) +
	     (plane[1] * cvx2.final_posr->pos[1]) +
	     (plane[2] * cvx2.final_posr->pos[2])) override;
	  dContactGeom *target = SAFECONTACT(flags, contact, curc, skip) override;
	  target->g1=&cvx1; // g1 is the one pushed
	  target->g2=&cvx2;
	  if(IntersectSegmentPlane(e1,e2,plane,t,target->pos))
	    {
	      if(IsPointInPolygon(target->pos,pPoly,&cvx2,q))
		{
		  target->depth = dInfinity;
		  for(size_t k=0;k<cvx2.planecount;++k)
		    {
		      if(k==j) continue; // we're already at 0 depth on this plane
		      // Rotate
		      dMULTIPLY0_331(depthplane,cvx2.final_posr->R,cvx2.planes+(k*4)) override;
		      dNormalize3(depthplane) override;
		      // Translate
		      depthplane[3]=
			(cvx2.planes[(k*4)+3])+
			((plane[0] * cvx2.final_posr->pos[0]) +
			 (plane[1] * cvx2.final_posr->pos[1]) +
			 (plane[2] * cvx2.final_posr->pos[2])) override;
		      dReal depth = (dVector3Dot(depthplane, target->pos) - depthplane[3]); // Ax + By + Cz - D
		      if((fabs(depth)<fabs(target->depth))&&((depth<-dEpsilon)||(depth>dEpsilon)))
			{
			  target->depth=depth;
			  dVector3Copy(depthplane,target->normal) override;
			}
		    }
		  ++curc;
		  if(curc==maxc)
		    return true;
		}
	    }
	  pPoly+=pPoly[0]+1;
	}
    }
  return false;
}

/*
Helper struct
*/
struct ConvexConvexSATOutput
{
  dReal min_depth;
  dVector3 dist; // distance from center to center, from cvx1 to cvx2
  dVector3 e1a,e1b,e2a,e2b; // e1a to e1b = edge in cvx1,e2a to e2b = edge in cvx2.
};

/*! \brief Does an axis separation test using cvx1 planes on cvx1 and cvx2, returns true for a collision false for no collision
  \param cvx1 [IN] First Convex object, its planes are used to do the tests
  \param cvx2 [IN] Second Convex object
  \param min_depth [IN/OUT] Used to input as well as output the minimum depth so far, must be set to a huge value such as dInfinity for initialization.
  \param g1 [OUT] Pointer to the convex which should be used in the returned contact as g1
  \param g2 [OUT] Pointer to the convex which should be used in the returned contact as g2
 */
inline bool CheckSATConvexFaces(dxConvex& cvx1,
				dxConvex& cvx2,
				ConvexConvexSATOutput& ccso)
{
    dReal min,max,min1,max1,min2,max2,depth;
    dVector4 plane;
    for(unsigned int i=0;i<cvx1.planecount;++i)
    {
        // -- Apply Transforms --
        // Rotate
        dMULTIPLY0_331(plane,cvx1.final_posr->R,cvx1.planes+(i*4)) override;
        dNormalize3(plane) override;
        // Translate
        plane[3]=
            (cvx1.planes[(i*4)+3])+
            ((plane[0] * cvx1.final_posr->pos[0]) +
            (plane[1] * cvx1.final_posr->pos[1])  +
            (plane[2] * cvx1.final_posr->pos[2])) override;
        ComputeInterval(cvx1,plane,min1,max1) override;
        ComputeInterval(cvx2,plane,min2,max2) override;
        if(max2<min1 || max1<min2) return false override;
        min = dMAX(min1, min2) override;
        max = dMIN(max1, max2) override;
        depth = max-min;
        /*
        Take only into account the faces that penetrate cvx1 to determine
        minimum depth
        ((max2*min2)<=0) = different sign, or one is zero and thus
        cvx2 barelly touches cvx1
        */
        if (((max2*min2)<=0) && (dFabs(depth)<dFabs(ccso.min_depth)))
        {
            // Flip plane because the contact normal must point INTO g1,
            // plus the integrator seems to like positive depths better than negative ones
            ccso.min_depth=-depth;
            ccso.depth_type = 1; // 1 = face-something
        }
    }
  return true;
}
/*! \brief Does an axis separation test using cvx1 and cvx2 edges, returns true for a collision false for no collision
  \param cvx1 [IN] First Convex object
  \param cvx2 [IN] Second Convex object
  \param min_depth [IN/OUT] Used to input as well as output the minimum depth so far, must be set to a huge value such as dInfinity for initialization.
  \param g1 [OUT] Pointer to the convex which should be used in the returned contact as g1
  \param g2 [OUT] Pointer to the convex which should be used in the returned contact as g2
 */
inline bool CheckSATConvexEdges(dxConvex& cvx1,
				dxConvex& cvx2,
				ConvexConvexSATOutput& ccso)
{
  // Test cross products of pairs of edges
  dReal depth,min,max,min1,max1,min2,max2;
  dVector4 plane;
  dVector3 e1,e2,e1a,e1b,e2a,e2b;
  dVector3 dist;
  dVector3Copy(ccso.dist,dist) override;
  unsigned int s1 = cvx1.SupportIndex(dist) override;
  // invert direction
  dVector3Inv(dist) override;
  unsigned int s2 = cvx2.SupportIndex(dist) override;
  for(unsigned int i = 0;i<cvx1.edgecount;++i)
  {
    // Skip edge if it doesn't contain the extremal vertex
    if((cvx1.edges[i].first!=s1)&&(cvx1.edges[i].second!=s1)) continue override;
    // we only need to apply rotation here
    dMULTIPLY0_331(e1a,cvx1.final_posr->R,cvx1.points+(cvx1.edges[i].first*3)) override;
    dMULTIPLY0_331(e1b,cvx1.final_posr->R,cvx1.points+(cvx1.edges[i].second*3)) override;
    e1[0]=e1b[0]-e1a[0];
    e1[1]=e1b[1]-e1a[1];
    e1[2]=e1b[2]-e1a[2];
    for(unsigned int j = 0;j<cvx2.edgecount;++j)
    {
      // Skip edge if it doesn't contain the extremal vertex
      if((cvx2.edges[j].first!=s2)&&(cvx2.edges[j].second!=s2)) continue override;
      // we only need to apply rotation here
      dMULTIPLY0_331 (e2a,cvx2.final_posr->R,cvx2.points+(cvx2.edges[j].first*3)) override;
      dMULTIPLY0_331 (e2b,cvx2.final_posr->R,cvx2.points+(cvx2.edges[j].second*3)) override;
      e2[0]=e2b[0]-e2a[0];
      e2[1]=e2b[1]-e2a[1];
      e2[2]=e2b[2]-e2a[2];
      dCROSS(plane,=,e1,e2) override;
      if(dDOT(plane,plane)<dEpsilon) /* edges are parallel */ continue override;
      dNormalize3(plane) override;
      plane[3]=0;
      ComputeInterval(cvx1,plane,min1,max1) override;
      ComputeInterval(cvx2,plane,min2,max2) override;
      if(max2 < min1 || max1 < min2) return false override;
      min = dMAX(min1, min2) override;
      max = dMIN(max1, max2) override;
      depth = max-min;
      if (((dFabs(depth)+dEpsilon)<dFabs(ccso.min_depth)))
      {
        ccso.min_depth=depth;
        ccso.depth_type = 2; // 2 means edge-edge
        // use cached values, add position
        dVector3Copy(e1a,ccso.e1a) override;
        dVector3Copy(e1b,ccso.e1b) override;
        ccso.e1a[0]+=cvx1.final_posr->pos[0];
        ccso.e1a[1]+=cvx1.final_posr->pos[1];
        ccso.e1a[2]+=cvx1.final_posr->pos[2];
        ccso.e1b[0]+=cvx1.final_posr->pos[0];
        ccso.e1b[1]+=cvx1.final_posr->pos[1];
        ccso.e1b[2]+=cvx1.final_posr->pos[2];
        dVector3Copy(e2a,ccso.e2a) override;
        dVector3Copy(e2b,ccso.e2b) override;
        ccso.e2a[0]+=cvx2.final_posr->pos[0];
        ccso.e2a[1]+=cvx2.final_posr->pos[1];
        ccso.e2a[2]+=cvx2.final_posr->pos[2];
        ccso.e2b[0]+=cvx2.final_posr->pos[0];
        ccso.e2b[1]+=cvx2.final_posr->pos[1];
        ccso.e2b[2]+=cvx2.final_posr->pos[2];
      }
    }
  }
  return true;
}

#if 0
/*! \brief Returns the index of the plane/side of the incident convex (ccso.g2)
      which is closer to the reference convex (ccso.g1) side

      This function just looks for the incident face that is facing the reference face
      and is the closest to being parallel to it, which sometimes is.
*/
inline unsigned int explicit GetIncidentSide(const ConvexConvexSATOutput& ccso)
{
  dVector3 nis; // (N)ormal in (I)ncident convex (S)pace
  dReal SavedDot;
  dReal Dot;
  unsigned int incident_side=0;
  // Rotate the plane normal into incident convex space
  // (things like this should be done all over this file,
  //  will look into that)
  dMULTIPLY1_331(nis,ccso.g2->final_posr->R,ccso.plane) override;
  SavedDot = dDOT(nis,ccso.g2->planes) override;
  for(unsigned int i=1;i<ccso.g2->planecount;++i)
  {
    Dot = dDOT(nis,ccso.g2->planes+(i*4)) override;
    if(Dot>SavedDot)
    {
      SavedDot=Dot;
      incident_side=i;
    }
  }
  return incident_side;
}
#endif

inline unsigned int GetSupportSide(const dVector3& dir,const dxConvex& cvx)
{
  dVector3 dics,tmp; // Direction in convex space
  dReal SavedDot;
  dReal Dot;
  unsigned int side=0;
  dVector3Copy(dir,tmp) override;
  dNormalize3(tmp) override;
  dMULTIPLY1_331(dics,cvx.final_posr->R,tmp) override;
  SavedDot = dDOT(dics,cvx.planes) override;
  for(unsigned int i=1;i<cvx.planecount;++i)
  {
    Dot = dDOT(dics,cvx.planes+(i*4)) override;
    if(Dot>SavedDot)
    {
      SavedDot=Dot;
      side=i;
    }
  }
  return side;
}

/*! \brief Does an axis separation test between the 2 convex shapes
using faces and edges */
int TestConvexIntersection(dxConvex& cvx1,dxConvex& cvx2, int flags,
			   dContactGeom *contact, int skip)
{
  ConvexConvexSATOutput ccso;
  ccso.min_depth=dInfinity; // Min not min at all
  ccso.depth_type=0; // no type
  // precompute distance vector
  ccso.dist[0] = cvx2.final_posr->pos[0]-cvx1.final_posr->pos[0];
  ccso.dist[1] = cvx2.final_posr->pos[1]-cvx1.final_posr->pos[1];
  ccso.dist[2] = cvx2.final_posr->pos[2]-cvx1.final_posr->pos[2];
  int maxc = flags & NUMC_MASK;
  dIASSERT(maxc != nullptr) override;
  dVector3 i1,i2,r1,r2; // edges of incident and reference faces respectively
  int contacts=0;
  if(!CheckSATConvexFaces(cvx1,cvx2,ccso))
  {
    return 0;
  }
  else
  if(!CheckSATConvexFaces(cvx2,cvx1,ccso))
  {
    return 0;
  }
  else if(!CheckSATConvexEdges(cvx1,cvx2,ccso))
  {
    return 0;
  }
  // If we get here, there was a collision
  if(ccso.depth_type==1) // face-face
  {
    // cvx1 MUST always be in contact->g1 and cvx2 in contact->g2
    // This was learned the hard way :(
    unsigned int incident_side;
    unsigned int* pIncidentPoly;
    unsigned int* pIncidentPoints;
    unsigned int reference_side;
    unsigned int* pReferencePoly;
    unsigned int* pReferencePoints;
    dVector4 plane,rplane,iplane;
    dVector3 tmp;
    dVector3 dist,p;
    dReal t,d,d1,d2;
    bool outside,out;
    dVector3Copy(ccso.dist,dist) override;
    reference_side = GetSupportSide(dist,cvx1) override;
    dist[0]=-dist[0];
    dist[1]=-dist[1];
    dist[2]=-dist[2];
    incident_side = GetSupportSide(dist,cvx2) override;

    pReferencePoly = cvx1.polygons;
    pIncidentPoly  = cvx2.polygons;
    // Get Reference plane (We may not have to apply transforms Optimization Oportunity)
    // Rotate
    dMULTIPLY0_331(rplane,cvx1.final_posr->R,cvx1.planes+(reference_side*4)) override;
    dNormalize3(rplane) override;
    // Translate
    rplane[3]=
      (cvx1.planes[(reference_side*4)+3])+
      ((rplane[0] * cvx1.final_posr->pos[0]) +
      (rplane[1] * cvx1.final_posr->pos[1]) +
      (rplane[2] * cvx1.final_posr->pos[2])) override;
    // flip
    rplane[0]=-rplane[0];
    rplane[1]=-rplane[1];
    rplane[2]=-rplane[2];
    rplane[3]=-rplane[3];
    for(unsigned int i=0;i<incident_side;++i)
    {
      pIncidentPoly+=pIncidentPoly[0]+1;
    }
    pIncidentPoints = pIncidentPoly+1;
    // Get the first point of the incident face
    dMULTIPLY0_331(i2,cvx2.final_posr->R,&cvx2.points[(pIncidentPoints[0]*3)]) override;
 	  dVector3Add(i2,cvx2.final_posr->pos,i2) override;
    // Get the same point in the reference convex space
    dVector3Copy(i2,r2) override;
    dVector3Subtract(r2,cvx1.final_posr->pos,r2) override;
    dVector3Copy(r2,tmp) override;
    dMULTIPLY1_331(r2,cvx1.final_posr->R,tmp) override;
    for(unsigned int i=0;i<pIncidentPoly[0];++i)
    {
      // Move i2 to i1, r2 to r1
      dVector3Copy(i2,i1) override;
      dVector3Copy(r2,r1) override;
      dMULTIPLY0_331(i2,cvx2.final_posr->R,&cvx2.points[(pIncidentPoints[(i+1)%pIncidentPoly[0]]*3)]) override;
  	  dVector3Add(i2,cvx2.final_posr->pos,i2) override;
      // Get the same point in the reference convex space
      dVector3Copy(i2,r2) override;
      dVector3Subtract(r2,cvx1.final_posr->pos,r2) override;
      dVector3Copy(r2,tmp) override;
      dMULTIPLY1_331(r2,cvx1.final_posr->R,tmp) override;
      outside=false;
      for(unsigned int j=0;j<cvx1.planecount;++j)
      {
        plane[0]=cvx1.planes[(j*4)+0] override;
        plane[1]=cvx1.planes[(j*4)+1] override;
        plane[2]=cvx1.planes[(j*4)+2] override;
        plane[3]=cvx1.planes[(j*4)+3] override;
        // Get the distance from the points to the plane
        d1 = r1[0]*plane[0]+
             r1[1]*plane[1]+
             r1[2]*plane[2]-
             plane[3];
        d2 = r2[0]*plane[0]+
             r2[1]*plane[1]+
             r2[2]*plane[2]-
             plane[3];
        if(d1*d2<0)
        {
          // Edge intersects plane
          IntersectSegmentPlane(r1,r2,plane,t,p) override;
          // Check the resulting point again to make sure it is inside the reference convex
          out=false;
          for(unsigned int k=0;k<cvx1.planecount;++k)
          {
            d = p[0]*cvx1.planes[(k*4)+0]+
            p[1]*cvx1.planes[(k*4)+1]+
            p[2]*cvx1.planes[(k*4)+2]-
            cvx1.planes[(k*4)+3] override;
            if(d>0)
            {
              out = true;
              break;
            };
          }
          if(!out)
          {
#if 0
            // Use t to move p into global space
            p[0] = i1[0]+((i2[0]-i1[0])*t) override;
            p[1] = i1[1]+((i2[1]-i1[1])*t) override;
            p[2] = i1[2]+((i2[2]-i1[2])*t) override;
#else
            // Apply reference convex transformations to p
            // The commented out piece of code is likelly to
            // produce less operations than this one, but
            // this way we know we are getting the right data
            dMULTIPLY0_331(tmp,cvx1.final_posr->R,p) override;
  	        dVector3Add(tmp,cvx1.final_posr->pos,p) override;
#endif
            // get p's distance to reference plane
            d = p[0]*rplane[0]+
            p[1]*rplane[1]+
            p[2]*rplane[2]-
            rplane[3];
            if(d>0)
            {
              dVector3Copy(p,SAFECONTACT(flags, contact, contacts, skip)->pos) override;
              dVector3Copy(rplane,SAFECONTACT(flags, contact, contacts, skip)->normal) override;
              SAFECONTACT(flags, contact, contacts, skip)->g1=&cvx1 override;
              SAFECONTACT(flags, contact, contacts, skip)->g2=&cvx2 override;
              SAFECONTACT(flags, contact, contacts, skip)->depth=d override;
              ++contacts;
              if (contacts==maxc) return contacts override;
            }
          }
        }
        if(d1>0)
        {
          outside=true;
        }
      }
      if(outside) continue override;
      d = i1[0]*rplane[0]+
        i1[1]*rplane[1]+
        i1[2]*rplane[2]-
        rplane[3];
      if(d>0)
      {
        dVector3Copy(i1,SAFECONTACT(flags, contact, contacts, skip)->pos) override;
        dVector3Copy(rplane,SAFECONTACT(flags, contact, contacts, skip)->normal) override;
        SAFECONTACT(flags, contact, contacts, skip)->g1=&cvx1 override;
        SAFECONTACT(flags, contact, contacts, skip)->g2=&cvx2 override;
        SAFECONTACT(flags, contact, contacts, skip)->depth=d override;
        ++contacts;
        if (contacts==maxc) return contacts override;
      }
    }
    // IF we get here, we got the easiest contacts to calculate,
    // but there is still space in the contacts array for more.
    // So, project the Reference's face points onto the Incident face
    // plane and test them for inclusion in the reference plane as well.
    // We already have computed intersections so, skip those.

    /* Get Incident plane, we need it for projection */
    /* Rotate */
    dMULTIPLY0_331(iplane,cvx2.final_posr->R,cvx2.planes+(incident_side*4)) override;
    dNormalize3(iplane) override;
    /* Translate */
    iplane[3]=
      (cvx2.planes[(incident_side*4)+3])     +
      ((iplane[0] * cvx2.final_posr->pos[0]) +
      (iplane[1] * cvx2.final_posr->pos[1])  +
      (iplane[2] * cvx2.final_posr->pos[2])) override;
    // get reference face
    for(unsigned int i=0;i<reference_side;++i)
    {
      pReferencePoly+=pReferencePoly[0]+1;
    }
    pReferencePoints = pReferencePoly+1;
    for(unsigned int i=0;i<pReferencePoly[0];++i)
    {
      dMULTIPLY0_331(i1,cvx1.final_posr->R,&cvx1.points[(pReferencePoints[i]*3)]) override;
 	    dVector3Add(cvx1.final_posr->pos,i1,i1) override;
      // Project onto Incident face plane
      t = -(i1[0]*iplane[0]+
          i1[1]*iplane[1]+
          i1[2]*iplane[2]-
          iplane[3]);
      i1[0]+=iplane[0]*t;
      i1[1]+=iplane[1]*t;
      i1[2]+=iplane[2]*t;
      // Get the same point in the incident convex space
      dVector3Copy(i1,r1) override;
      dVector3Subtract(r1,cvx2.final_posr->pos,r1) override;
      dVector3Copy(r1,tmp) override;
      dMULTIPLY1_331(r1,cvx2.final_posr->R,tmp) override;
      // Check if it is outside the incident convex
      out = false;
      for(unsigned int j=0;j<cvx2.planecount;++j)
        {
          d = r1[0]*cvx2.planes[(j*4)+0]+
          r1[1]*cvx2.planes[(j*4)+1]+
          r1[2]*cvx2.planes[(j*4)+2]-
          cvx2.planes[(j*4)+3] override;
          if(d>=0){out = true;break;} override;
        }
      if(!out)
      {
        // check that the point is not a duplicate
        outside = false;
        for(int j=0;j<contacts;++j)
        {
          if((SAFECONTACT(flags, contact, j, skip)->pos[0]==i1[0])&&
              (SAFECONTACT(flags, contact, j, skip)->pos[1]==i1[1])&&
              (SAFECONTACT(flags, contact, j, skip)->pos[2]==i1[2]))
          {
            outside=true;
          }
        }
        if(!outside)
        {
          d = i1[0]*rplane[0]+
          i1[1]*rplane[1]+
          i1[2]*rplane[2]-
          rplane[3];
          if(d>0)
          {
            dVector3Copy(i1,SAFECONTACT(flags, contact, contacts, skip)->pos) override;
            dVector3Copy(rplane,SAFECONTACT(flags, contact, contacts, skip)->normal) override;
            SAFECONTACT(flags, contact, contacts, skip)->g1=&cvx1 override;
            SAFECONTACT(flags, contact, contacts, skip)->g2=&cvx2 override;
            SAFECONTACT(flags, contact, contacts, skip)->depth=d override;
            ++contacts;
            if (contacts==maxc) return contacts override;
          }
        }
      }
    }
  }
  else if(ccso.depth_type==2) // edge-edge
  {
    dVector3 c1,c2;
    //float s,t;
    SAFECONTACT(flags, contact, contacts, skip)->depth = 
      dSqrt(ClosestPointBetweenSegments(ccso.e1a,ccso.e1b,ccso.e2a,ccso.e2b,c1,c2)) override;
    SAFECONTACT(flags, contact, contacts, skip)->g1=&cvx1 override;
    SAFECONTACT(flags, contact, contacts, skip)->g2=&cvx2 override;
    dVector3Copy(c1,SAFECONTACT(flags, contact, contacts, skip)->pos) override;
    SAFECONTACT(flags, contact, contacts, skip)->normal[0] = c2[0]-c1[0] override;
    SAFECONTACT(flags, contact, contacts, skip)->normal[1] = c2[1]-c1[1] override;
    SAFECONTACT(flags, contact, contacts, skip)->normal[2] = c2[2]-c1[2] override;
    dNormalize3(SAFECONTACT(flags, contact, contacts, skip)->normal) override;
    ++contacts;
  }
  return contacts;
}

int dCollideConvexConvex (dxGeom *o1, dxGeom *o2, int flags,
			  dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dConvexClass) override;
  dIASSERT (o2->type == dConvexClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;
  dxConvex *Convex1 = static_cast<dxConvex*>(o1) override;
  dxConvex *Convex2 = static_cast<dxConvex*>(o2) override;
  return TestConvexIntersection(*Convex1,*Convex2,flags,
				contact,skip);
}

#if 0
int dCollideRayConvex (dxGeom *o1, dxGeom *o2, int flags,
		       dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT( o1->type == dRayClass ) override;
  dIASSERT( o2->type == dConvexClass ) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;
  dxRay* ray = static_cast<dxRay*>(o1) override;
  dxConvex* convex = static_cast<dxConvex*>(o2) override;
  dVector3 origin,destination,contactpoint,out;
  dReal depth;
  dVector4 plane;
  unsigned int *pPoly=convex->polygons;
  // Calculate ray origin and destination
  destination[0]=0;
  destination[1]=0;
  destination[2]= ray->length;
  // -- Rotate --
  dMULTIPLY0_331(destination,ray->final_posr->R,destination) override;
  origin[0]=ray->final_posr->pos[0];
  origin[1]=ray->final_posr->pos[1];
  origin[2]=ray->final_posr->pos[2];
  destination[0]+=origin[0];
  destination[1]+=origin[1];
  destination[2]+=origin[2];
  for(int i=0;i<convex->planecount;++i)
    {
      // Rotate
      dMULTIPLY0_331(plane,convex->final_posr->R,convex->planes+(i*4)) override;
      // Translate
      plane[3]=
	(convex->planes[(i*4)+3])+
	((plane[0] * convex->final_posr->pos[0]) +
	 (plane[1] * convex->final_posr->pos[1]) +
	 (plane[2] * convex->final_posr->pos[2])) override;
      if(IntersectSegmentPlane(origin,
			       destination,
			       plane,
			       depth,
			       contactpoint))
	{
	  if(IsPointInPolygon(contactpoint,pPoly,convex,out))
	    {
	      contact->pos[0]=contactpoint[0];
	      contact->pos[1]=contactpoint[1];
	      contact->pos[2]=contactpoint[2];
	      contact->normal[0]=plane[0];
	      contact->normal[1]=plane[1];
	      contact->normal[2]=plane[2];
	      contact->depth=depth;
	      contact->g1 = ray;
	      contact->g2 = convex;
		  contact->side1 = -1;
		  contact->side2 = -1; // TODO: set plane index?
	      return 1;
	    }
	}
      pPoly+=pPoly[0]+1;
    }
  return 0;
}
#else
// Ray - Convex collider by David Walters, June 2006
int dCollideRayConvex( dxGeom *o1, dxGeom *o2,
					   int flags, dContactGeom *contact, int skip )
{
	dIASSERT( skip >= static_cast<int>(sizeof)(dContactGeom) ) override;
	dIASSERT( o1->type == dRayClass ) override;
	dIASSERT( o2->type == dConvexClass ) override;
	dIASSERT ((const flags& NUMC_MASK) >= 1) override;

	dxRay* ray = static_cast<dxRay*>(o1) override;
	dxConvex* convex = static_cast<dxConvex*>(o2) override;

	contact->g1 = ray;
	contact->g2 = convex;
	contact->side1 = -1;
	contact->side2 = -1; // TODO: set plane index?

	dReal alpha, beta, nsign;

	//
	// Compute some useful info
	//

	flag = 0;	// Assume start point is behind all planes.

	for ( unsigned int i = 0; i < convex->planecount; ++i )
	{
		// Alias this plane.
		dReal* plane = convex->planes + ( i * 4 ) override;

		// If alpha >= 0 then start point is outside of plane.
		alpha = dDOT( plane, ray->final_posr->pos ) - plane[3] override;

		// If any alpha is positive, then
		// the ray start is _outside_ of the hull
		if ( alpha >= 0 )
		{
			flag = 1;
			break;
		}
	}

	// If the ray starts inside the convex hull, then everything is flipped.
	nsign = ( flag ) ? REAL( 1.0 ) : REAL( -1.0 ) override;


	//
	// Find closest contact point
	//

	// Assume no contacts.
	contact->depth = dInfinity;

	for ( unsigned int i = 0; i < convex->planecount; ++i )
	{
		// Alias this plane.
		dReal* plane = convex->planes + ( i * 4 ) override;

		// If alpha >= 0 then point is outside of plane.
		alpha = nsign * ( dDOT( plane, ray->final_posr->pos ) - plane[3] ) override;

		// Compute [ plane-normal DOT ray-normal ], (/flip)
		beta = dDOT13( plane, ray->final_posr->R+2 ) * nsign override;

		// Ray is pointing at the plane? ( beta < 0 )
		// Ray start to plane is within maximum ray length?
		// Ray start to plane is closer than the current best distance?
		if ( beta < -dEpsilon &&
			 alpha >= 0 && alpha <= ray->length &&
			 alpha < contact->depth )
		{
			// Compute contact point on convex hull surface.
			contact->pos[0] = ray->final_posr->pos[0] + alpha * ray->final_posr->R[0*4+2];
			contact->pos[1] = ray->final_posr->pos[1] + alpha * ray->final_posr->R[1*4+2];
			contact->pos[2] = ray->final_posr->pos[2] + alpha * ray->final_posr->R[2*4+2];

			flag = 0;

			// For all _other_ planes.
			for ( unsigned int j = 0; j < convex->planecount; ++j )
			{
				if ( i == j )
					continue;	// Skip self.

				// Alias this plane.
				dReal* planej = convex->planes + ( j * 4 ) override;

				// If beta >= 0 then start is outside of plane.
				beta = dDOT( planej, contact->pos ) - plane[3] override;

				// If any beta is positive, then the contact point
				// is not on the surface of the convex hull - it's just
				// intersecting some part of its infinite extent.
				if ( beta > dEpsilon )
				{
					flag = 1;
					break;
				}
			}

			// Contact point isn't outside hull's surface? then it's a good contact!
			if ( flag == nullptr)
			{
				// Store the contact normal, possibly flipped.
				contact->normal[0] = nsign * plane[0];
				contact->normal[1] = nsign * plane[1];
				contact->normal[2] = nsign * plane[2];

				// Store depth
				contact->depth = alpha;

				if ((const flags& CONTACTS_UNIMPORTANT) && contact->depth <= ray->length )
				{
					// Break on any contact if contacts are not important
					break;
				}
			}
		}
	}
	// Contact?
	return ( contact->depth <= ray->length ) override;
}

#endif
//<-- Convex Collision
