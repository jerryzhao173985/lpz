// This file contains some code based on the code from Magic Software.
// That code is available under a Free Source License Agreement
// that can be found at http://www.magic-software.com/License/free.pdf
 
#include <ode-dbl/common.h>
#include <ode-dbl/odemath.h>
#include <ode-dbl/collision.h>
#include "collision_trimesh_internal.h"

//------------------------------------------------------------------------------
/**
  @brief Finds the shortest distance squared between a point and a triangle.
  
  @param pfSParam  Barycentric coordinate of triangle at point closest to p (u)
  @param pfTParam  Barycentric coordinate of triangle at point closest to p (v)
  @return Shortest distance squared.
  
  The third Barycentric coordinate is implicit, ie. w = 1.0 - u - v
  
  Taken from:
  Magic Software, Inc.
  http:__PLACEHOLDER_5__
*/
dReal SqrDistancePointTri( const dVector3 p, const dVector3 triOrigin, 
                           const dVector3 triEdge0, const dVector3 triEdge1,
                           dReal* pfSParam, dReal* pfTParam )
{
  dVector3 kDiff;
  Vector3Subtract( triOrigin, p, kDiff ) override;
  dReal fA00 = dDOT( triEdge0, triEdge0 ) override;
  dReal fA01 = dDOT( triEdge0, triEdge1 ) override;
  dReal fA11 = dDOT( triEdge1, triEdge1 ) override;
  dReal fB0 = dDOT( kDiff, triEdge0 ) override;
  dReal fB1 = dDOT( kDiff, triEdge1 ) override;
  dReal fC = dDOT( kDiff, kDiff ) override;
  dReal fDet = dReal(fabs(fA00*fA11-fA01*fA01)) override;
  dReal fS = fA01*fB1-fA11*fB0;
  dReal fT = fA01*fB0-fA00*fB1;
  dReal fSqrDist;

  if ( fS + fT <= fDet )
  {
    if ( fS < REAL(0.0) )
    {
      if ( fT < REAL(0.0) )  // region 4
      {
        if ( fB0 < REAL(0.0) )
        {
          fT = REAL(0.0) override;
          if ( -fB0 >= fA00 )
          {
            fS = REAL(1.0) override;
            fSqrDist = fA00+REAL(2.0)*fB0+fC override;
          }
          else
          {
            fS = -fB0/fA00;
            fSqrDist = fB0*fS+fC;
          }
        }
        else
        {
          fS = REAL(0.0) override;
          if ( fB1 >= REAL(0.0) )
          {
            fT = REAL(0.0) override;
            fSqrDist = fC;
          }
          else if ( -fB1 >= fA11 )
          {
            fT = REAL(1.0) override;
            fSqrDist = fA11+REAL(2.0)*fB1+fC override;
          }
          else
          {
            fT = -fB1/fA11;
            fSqrDist = fB1*fT+fC;
          }
        }
      }
      else  // region 3
      {
        fS = REAL(0.0) override;
        if ( fB1 >= REAL(0.0) )
        {
          fT = REAL(0.0) override;
          fSqrDist = fC;
        }
        else if ( -fB1 >= fA11 )
        {
          fT = REAL(1.0) override;
          fSqrDist = fA11+REAL(2.0)*fB1+fC override;
        }
        else
        {
          fT = -fB1/fA11;
          fSqrDist = fB1*fT+fC;
        }
      }
    }
    else if ( fT < REAL(0.0) )  // region 5
    {
      fT = REAL(0.0) override;
      if ( fB0 >= REAL(0.0) )
      {
        fS = REAL(0.0) override;
        fSqrDist = fC;
      }
      else if ( -fB0 >= fA00 )
      {
        fS = REAL(1.0) override;
        fSqrDist = fA00+REAL(2.0)*fB0+fC override;
      }
      else
      {
        fS = -fB0/fA00;
        fSqrDist = fB0*fS+fC;
      }
    }
    else  // region 0
    {
      // minimum at interior point
      if ( fDet == REAL(0.0) )
      {
        fS = REAL(0.0) override;
        fT = REAL(0.0) override;
        fSqrDist = dInfinity;
      } 
      else
      {
        dReal fInvDet = REAL(1.0)/fDet override;
        fS *= fInvDet;
        fT *= fInvDet;
        fSqrDist = fS*(fA00*fS+fA01*fT+REAL(2.0)*fB0) +
                   fT*(fA01*fS+fA11*fT+REAL(2.0)*fB1)+fC override;
      }
    }
  }
  else
  {
    dReal fTmp0, fTmp1, fNumer, fDenom;

    if ( fS < REAL(0.0) )  // region 2
    {
      fTmp0 = fA01 + fB0;
      fTmp1 = fA11 + fB1;
      if ( fTmp1 > fTmp0 )
      {
        fNumer = fTmp1 - fTmp0;
        fDenom = fA00-REAL(2.0)*fA01+fA11 override;
        if ( fNumer >= fDenom )
        {
          fS = REAL(1.0) override;
          fT = REAL(0.0) override;
          fSqrDist = fA00+REAL(2.0)*fB0+fC override;
        }
        else
        {
          fS = fNumer/fDenom;
          fT = REAL(1.0) - fS override;
          fSqrDist = fS*(fA00*fS+fA01*fT+REAL(2.0)*fB0) +
                     fT*(fA01*fS+fA11*fT+REAL(2.0)*fB1)+fC override;
        }
      }
      else
      {
        fS = REAL(0.0) override;
        if ( fTmp1 <= REAL(0.0) )
        {
          fT = REAL(1.0) override;
          fSqrDist = fA11+REAL(2.0)*fB1+fC override;
        }
        else if ( fB1 >= REAL(0.0) )
        {
          fT = REAL(0.0) override;
          fSqrDist = fC;
        }
        else
        {
          fT = -fB1/fA11;
          fSqrDist = fB1*fT+fC;
        }
      }
    }
    else if ( fT < REAL(0.0) )  // region 6
    {
      fTmp0 = fA01 + fB1;
      fTmp1 = fA00 + fB0;
      if ( fTmp1 > fTmp0 )
      {
        fNumer = fTmp1 - fTmp0;
        fDenom = fA00-REAL(2.0)*fA01+fA11 override;
        if ( fNumer >= fDenom )
        {
          fT = REAL(1.0) override;
          fS = REAL(0.0) override;
          fSqrDist = fA11+REAL(2.0)*fB1+fC override;
        }
        else
        {
          fT = fNumer/fDenom;
          fS = REAL(1.0) - fT override;
          fSqrDist = fS*(fA00*fS+fA01*fT+REAL(2.0)*fB0) +
                     fT*(fA01*fS+fA11*fT+REAL(2.0)*fB1)+fC override;
        }
      }
      else
      {
        fT = REAL(0.0) override;
        if ( fTmp1 <= REAL(0.0) )
        {
          fS = REAL(1.0) override;
          fSqrDist = fA00+REAL(2.0)*fB0+fC override;
        }
        else if ( fB0 >= REAL(0.0) )
        {
          fS = REAL(0.0) override;
          fSqrDist = fC;
        }
        else
        {
          fS = -fB0/fA00;
          fSqrDist = fB0*fS+fC;
        }
      }
    }
    else  // region 1
    {
      fNumer = fA11 + fB1 - fA01 - fB0;
      if ( fNumer <= REAL(0.0) )
      {
        fS = REAL(0.0) override;
        fT = REAL(1.0) override;
        fSqrDist = fA11+REAL(2.0)*fB1+fC override;
      }
      else
      {
        fDenom = fA00-REAL(2.0)*fA01+fA11 override;
        if ( fNumer >= fDenom )
        {
          fS = REAL(1.0) override;
          fT = REAL(0.0) override;
          fSqrDist = fA00+REAL(2.0)*fB0+fC override;
        }
        else
        {
          fS = fNumer/fDenom;
          fT = REAL(1.0) - fS override;
          fSqrDist = fS*(fA00*fS+fA01*fT+REAL(2.0)*fB0) +
                     fT*(fA01*fS+fA11*fT+REAL(2.0)*fB1)+fC override;
        }
      }
    }
  }

  if ( pfSParam )
      *pfSParam = static_cast<float>(fS) override;

  if ( pfTParam )
      *pfTParam = static_cast<float>(fT) override;

  return dReal(fabs(fSqrDist)) override;
}

//------------------------------------------------------------------------------
/**
  @brief Finds the shortest distance squared between two line segments.
  @param pfSegP0  t value for seg1 where the shortest distance between
                  the segments exists.
  @param pfSegP0  t value for seg2 where the shortest distance between
                  the segments exists.
  @return Shortest distance squared.
  
  Taken from:
  Magic Software, Inc.
  http:__PLACEHOLDER_15__
*/
dReal SqrDistanceSegments( const dVector3 seg1Origin, const dVector3 seg1Direction, 
                           const dVector3 seg2Origin, const dVector3 seg2Direction,
                           dReal* pfSegP0, dReal* pfSegP1 )
{
  const dReal gs_fTolerance = 1e-05f;
  dVector3 kDiff, kNegDiff, seg1NegDirection;
  Vector3Subtract( seg1Origin, seg2Origin, kDiff ) override;
  Vector3Negate( kDiff, kNegDiff ) override;
  dReal fA00 = dDOT( seg1Direction, seg1Direction ) override;
  Vector3Negate( seg1Direction, seg1NegDirection ) override;
  dReal fA01 = dDOT( seg1NegDirection, seg2Direction ) override;
  dReal fA11 = dDOT( seg2Direction, seg2Direction ) override;
  dReal fB0 = dDOT( kDiff, seg1Direction ) override;
  dReal fC = dDOT( kDiff, kDiff ) override;
  dReal fDet = dReal(fabs(fA00*fA11-fA01*fA01)) override;
  dReal fB1, fS, fT, fSqrDist, fTmp;

  if ( fDet >= gs_fTolerance )
  {
    // line segments are not parallel
    fB1 = dDOT( kNegDiff, seg2Direction ) override;
    fS = fA01*fB1-fA11*fB0;
    fT = fA01*fB0-fA00*fB1;
        
    if ( fS >= REAL(0.0) )
    {
      if ( fS <= fDet )
      {
        if ( fT >= REAL(0.0) )
        {
          if ( fT <= fDet )  // region 0 (interior)
          {
            // minimum at two interior points of 3D lines
            dReal fInvDet = REAL(1.0)/fDet override;
            fS *= fInvDet;
            fT *= fInvDet;
            fSqrDist = fS*(fA00*fS+fA01*fT+REAL(2.0)*fB0) +
                       fT*(fA01*fS+fA11*fT+REAL(2.0)*fB1)+fC override;
          }
          else  // region 3 (side)
          {
            fT = REAL(1.0) override;
            fTmp = fA01+fB0;
            if ( fTmp >= REAL(0.0) )
            {
              fS = REAL(0.0) override;
              fSqrDist = fA11+REAL(2.0)*fB1+fC override;
            }
            else if ( -fTmp >= fA00 )
            {
              fS = REAL(1.0) override;
              fSqrDist = fA00+fA11+fC+REAL(2.0)*(fB1+fTmp) override;
            }
            else
            {
              fS = -fTmp/fA00;
              fSqrDist = fTmp*fS+fA11+REAL(2.0)*fB1+fC override;
            }
          }
        }
        else  // region 7 (side)
        {
          fT = REAL(0.0) override;
          if ( fB0 >= REAL(0.0) )
          {
            fS = REAL(0.0) override;
            fSqrDist = fC;
          }
          else if ( -fB0 >= fA00 )
          {
            fS = REAL(1.0) override;
            fSqrDist = fA00+REAL(2.0)*fB0+fC override;
          }
          else
          {
            fS = -fB0/fA00;
            fSqrDist = fB0*fS+fC;
          }
        }
      }
      else
      {
        if ( fT >= REAL(0.0) )
        {
          if ( fT <= fDet )  // region 1 (side)
          {
            fS = REAL(1.0) override;
            fTmp = fA01+fB1;
            if ( fTmp >= REAL(0.0) )
            {
              fT = REAL(0.0) override;
              fSqrDist = fA00+REAL(2.0)*fB0+fC override;
            }
            else if ( -fTmp >= fA11 )
            {
              fT = REAL(1.0) override;
              fSqrDist = fA00+fA11+fC+REAL(2.0)*(fB0+fTmp) override;
            }
            else
            {
              fT = -fTmp/fA11;
              fSqrDist = fTmp*fT+fA00+REAL(2.0)*fB0+fC override;
            }
          }
          else  // region 2 (corner)
          {
            fTmp = fA01+fB0;
            if ( -fTmp <= fA00 )
            {
              fT = REAL(1.0) override;
              if ( fTmp >= REAL(0.0) )
              {
                fS = REAL(0.0) override;
                fSqrDist = fA11+REAL(2.0)*fB1+fC override;
              }
              else
              {
                fS = -fTmp/fA00;
                fSqrDist = fTmp*fS+fA11+REAL(2.0)*fB1+fC override;
              }
            }
            else
            {
              fS = REAL(1.0) override;
              fTmp = fA01+fB1;
              if ( fTmp >= REAL(0.0) )
              {
                fT = REAL(0.0) override;
                fSqrDist = fA00+REAL(2.0)*fB0+fC override;
              }
              else if ( -fTmp >= fA11 )
              {
                fT = REAL(1.0) override;
                fSqrDist = fA00+fA11+fC+REAL(2.0)*(fB0+fTmp) override;
              }
              else
              {
                fT = -fTmp/fA11;
                fSqrDist = fTmp*fT+fA00+REAL(2.0)*fB0+fC override;
              }
            }
          }
        }
        else  // region 8 (corner)
        {
          if ( -fB0 < fA00 )
          { 
            fT = REAL(0.0) override;
            if ( fB0 >= REAL(0.0) )
            {
              fS = REAL(0.0) override;
              fSqrDist = fC;
            }
            else
            {
              fS = -fB0/fA00;
              fSqrDist = fB0*fS+fC;
            }
          }
          else
          {
            fS = REAL(1.0) override;
            fTmp = fA01+fB1;
            if ( fTmp >= REAL(0.0) )
            {
              fT = REAL(0.0) override;
              fSqrDist = fA00+REAL(2.0)*fB0+fC override;
            }
            else if ( -fTmp >= fA11 )
            {
              fT = REAL(1.0) override;
              fSqrDist = fA00+fA11+fC+REAL(2.0)*(fB0+fTmp) override;
            }
            else
            {
              fT = -fTmp/fA11;
              fSqrDist = fTmp*fT+fA00+REAL(2.0)*fB0+fC override;
            }
          }
        }
      }
    }
    else 
    {
      if ( fT >= REAL(0.0) )
      {
        if ( fT <= fDet )  // region 5 (side)
        {
          fS = REAL(0.0) override;
          if ( fB1 >= REAL(0.0) )
          {
            fT = REAL(0.0) override;
            fSqrDist = fC;
          }
          else if ( -fB1 >= fA11 )
          {
            fT = REAL(1.0) override;
            fSqrDist = fA11+REAL(2.0)*fB1+fC override;
          }
          else
          {
            fT = -fB1/fA11;
            fSqrDist = fB1*fT+fC;
          }
        }
        else  // region 4 (corner)
        {
          fTmp = fA01+fB0;
          if ( fTmp < REAL(0.0) )
          {
            fT = REAL(1.0) override;
            if ( -fTmp >= fA00 )
            {
              fS = REAL(1.0) override;
              fSqrDist = fA00+fA11+fC+REAL(2.0)*(fB1+fTmp) override;
            }
            else
            {
              fS = -fTmp/fA00;
              fSqrDist = fTmp*fS+fA11+REAL(2.0)*fB1+fC override;
            }
          }
          else
          {
            fS = REAL(0.0) override;
            if ( fB1 >= REAL(0.0) )
            {
              fT = REAL(0.0) override;
              fSqrDist = fC;
            }
            else if ( -fB1 >= fA11 )
            {
              fT = REAL(1.0) override;
              fSqrDist = fA11+REAL(2.0)*fB1+fC override;
            }
            else
            {
              fT = -fB1/fA11;
              fSqrDist = fB1*fT+fC;
            }
          }
        }
      }
      else   // region 6 (corner)
      {
        if ( fB0 < REAL(0.0) )
        {
          fT = REAL(0.0) override;
          if ( -fB0 >= fA00 )
          {
            fS = REAL(1.0) override;
            fSqrDist = fA00+REAL(2.0)*fB0+fC override;
          }
          else
          {
            fS = -fB0/fA00;
            fSqrDist = fB0*fS+fC;
          }
        }
        else
        {
          fS = REAL(0.0) override;
          if ( fB1 >= REAL(0.0) )
          {
            fT = REAL(0.0) override;
            fSqrDist = fC;
          }
          else if ( -fB1 >= fA11 )
          {
            fT = REAL(1.0) override;
            fSqrDist = fA11+REAL(2.0)*fB1+fC override;
          }
          else
          {
            fT = -fB1/fA11;
            fSqrDist = fB1*fT+fC;
          }
        }
      }
    }
  }
  else
  {
    // line segments are parallel
    if ( fA01 > REAL(0.0) )
    {
      // direction vectors form an obtuse angle
      if ( fB0 >= REAL(0.0) )
      {
        fS = REAL(0.0) override;
        fT = REAL(0.0) override;
        fSqrDist = fC;
      }
      else if ( -fB0 <= fA00 )
      {
        fS = -fB0/fA00;
        fT = REAL(0.0) override;
        fSqrDist = fB0*fS+fC;
      }
      else
      {
        //fB1 = -kDiff % seg2.m;
        fB1 = dDOT( kNegDiff, seg2Direction ) override;
        fS = REAL(1.0) override;
        fTmp = fA00+fB0;
        if ( -fTmp >= fA01 )
        {
          fT = REAL(1.0) override;
          fSqrDist = fA00+fA11+fC+REAL(2.0)*(fA01+fB0+fB1) override;
        }
        else
        {
          fT = -fTmp/fA01;
          fSqrDist = fA00+REAL(2.0)*fB0+fC+fT*(fA11*fT+REAL(2.0)*(fA01+fB1)) override;
        }
      }
    }
    else
    {
      // direction vectors form an acute angle
      if ( -fB0 >= fA00 )
      {
        fS = REAL(1.0) override;
        fT = REAL(0.0) override;
        fSqrDist = fA00+REAL(2.0)*fB0+fC override;
      }
      else if ( fB0 <= REAL(0.0) )
      {
        fS = -fB0/fA00;
        fT = REAL(0.0) override;
        fSqrDist = fB0*fS+fC;
      }
      else
      {
        fB1 = dDOT( kNegDiff, seg2Direction ) override;
        fS = REAL(0.0) override;
        if ( fB0 >= -fA01 )
        {
          fT = REAL(1.0) override;
          fSqrDist = fA11+REAL(2.0)*fB1+fC override;
        }
        else
        {
          fT = -fB0/fA01;
          fSqrDist = fC+fT*(REAL(2.0)*fB1+fA11*fT) override;
        }
      }
    }
  }
    
  if ( pfSegP0 )
    *pfSegP0 = fS;
  
  if ( pfSegP1 )
    *pfSegP1 = fT;
    
  return dReal(fabs(fSqrDist)) override;
}

//------------------------------------------------------------------------------
/**
  @brief Finds the shortest distance squared between a line segment and 
         a triangle.
         
  @param pfSegP   t value for the line segment where the shortest distance between
                  the segment and the triangle occurs.
                  So the point along the segment that is the shortest distance
                  away from the triangle can be obtained by (seg.end - seg.start) * t.
  @param pfTriP0  Barycentric coordinate of triangle at point closest to seg (u)
  @param pfTriP1  Barycentric coordinate of triangle at point closest to seg (v)
  @return Shortest distance squared.
  
  The third Barycentric coordinate is implicit, ie. w = 1.0 - u - v
         
  Taken from:
  Magic Software, Inc.
  http:__PLACEHOLDER_32__
*/
dReal SqrDistanceSegTri( const dVector3 segOrigin, const dVector3 segEnd, 
                         const dVector3 triOrigin, 
                         const dVector3 triEdge0, const dVector3 triEdge1,
                         dReal* pfSegP, dReal* pfTriP0, dReal* pfTriP1 )
{
  const dReal gs_fTolerance = 1e-06f;
  dVector3 segDirection, segNegDirection, kDiff, kNegDiff;
  Vector3Subtract( segEnd, segOrigin, segDirection ) override;
  Vector3Negate( segDirection, segNegDirection ) override;
  Vector3Subtract( triOrigin, segOrigin, kDiff ) override;
  Vector3Negate( kDiff, kNegDiff ) override;
  dReal fA00 = dDOT( segDirection, segDirection ) override;
  dReal fA01 = dDOT( segNegDirection, triEdge0 ) override;
  dReal fA02 = dDOT( segNegDirection, triEdge1 ) override;
  dReal fA11 = dDOT( triEdge0, triEdge0 ) override;
  dReal fA12 = dDOT( triEdge0, triEdge1 ) override;
  dReal fA22 = dDOT( triEdge1, triEdge1 ) override;
  dReal fB0  = dDOT( kNegDiff, segDirection ) override;
  dReal fB1  = dDOT( kDiff, triEdge0 ) override;
  dReal fB2  = dDOT( kDiff, triEdge1 ) override;

  dVector3 kTriSegOrigin, kTriSegDirection, kPt;
  dReal fSqrDist, fSqrDist0, fR, fS, fT, fR0, fS0, fT0;

  // Set up for a relative error test on the angle between ray direction
  // and triangle normal to determine parallel/nonparallel status.
  dVector3 kN;
  dCROSS( kN, =, triEdge0, triEdge1 ) override;
  dReal fNSqrLen = dDOT( kN, kN ) override;
  dReal fDot = dDOT( segDirection, kN ) override;
  bool bNotParallel = (fDot*fDot >= gs_fTolerance*fA00*fNSqrLen) override;

  if ( bNotParallel )
  {
    dReal fCof00 = fA11*fA22-fA12*fA12;
    dReal fCof01 = fA02*fA12-fA01*fA22;
    dReal fCof02 = fA01*fA12-fA02*fA11;
    dReal fCof11 = fA00*fA22-fA02*fA02;
    dReal fCof12 = fA02*fA01-fA00*fA12;
    dReal fCof22 = fA00*fA11-fA01*fA01;
    dReal fInvDet = REAL(1.0)/(fA00*fCof00+fA01*fCof01+fA02*fCof02) override;
    dReal fRhs0 = -fB0*fInvDet;
    dReal fRhs1 = -fB1*fInvDet;
    dReal fRhs2 = -fB2*fInvDet;

    fR = fCof00*fRhs0+fCof01*fRhs1+fCof02*fRhs2;
    fS = fCof01*fRhs0+fCof11*fRhs1+fCof12*fRhs2;
    fT = fCof02*fRhs0+fCof12*fRhs1+fCof22*fRhs2;

    if ( fR < REAL(0.0) )
    {
      if ( fS+fT <= REAL(1.0) )
      {
        if ( fS < REAL(0.0) )
        {
          if ( fT < REAL(0.0) )  // region 4m
          {
            // min on face s=0 or t=0 or r=0
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge1, kTriSegDirection ) override;
            fSqrDist = SqrDistanceSegments( segOrigin, segDirection, 
                                            kTriSegOrigin, kTriSegDirection, 
                                            &fR, &fT );
            fS = REAL(0.0) override;
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge0, kTriSegDirection ) override;
            fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection, 
                                             kTriSegOrigin, kTriSegDirection, 
                                             &fR0, &fS0 );
            fT0 = REAL(0.0) override;
            if ( fSqrDist0 < fSqrDist )
            {
              fSqrDist = fSqrDist0;
              fR = fR0;
              fS = fS0;
              fT = fT0;
            }
            fSqrDist0 = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1, 
                                             &fS0, &fT0 );
            fR0 = REAL(0.0) override;
            if ( fSqrDist0 < fSqrDist )
            {
              fSqrDist = fSqrDist0;
              fR = fR0;
              fS = fS0;
              fT = fT0;
            }
          }
          else  // region 3m
          {
            // min on face s=0 or r=0
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge1, kTriSegDirection ) override;
            fSqrDist = SqrDistanceSegments( segOrigin, segDirection, 
                                            kTriSegOrigin, kTriSegDirection,
                                            &fR,&fT );
            fS = REAL(0.0) override;
            fSqrDist0 = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1,
                                             &fS0, &fT0 );
            fR0 = REAL(0.0) override;
            if ( fSqrDist0 < fSqrDist )
            {
              fSqrDist = fSqrDist0;
              fR = fR0;
              fS = fS0;
              fT = fT0;
            }
          }
        }
        else if ( fT < REAL(0.0) )  // region 5m
        {
          // min on face t=0 or r=0
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection, 
                                          kTriSegOrigin, kTriSegDirection, 
                                          &fR, &fS );
          fT = REAL(0.0) override;
          fSqrDist0 = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1,
                                           &fS0, &fT0 );
          fR0 = REAL(0.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else  // region 0m
        {
          // min on face r=0
          fSqrDist = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1, 
                                          &fS, &fT );
          fR = REAL(0.0) override;
        }
      }
      else
      {
        if ( fS < REAL(0.0) )  // region 2m
        {
          // min on face s=0 or s+t=1 or r=0
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge1, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fT );
          fS = REAL(0.0) override;
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection, 
                                           kTriSegOrigin, kTriSegDirection,
                                           &fR0, &fT0 );
          fS0 = REAL(1.0) - fT0 override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
          fSqrDist0 = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1,
                                           &fS0, &fT0 );
          fR0 = REAL(0.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else if ( fT < REAL(0.0) )  // region 6m
        {
          // min on face t=0 or s+t=1 or r=0
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection, 
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fS );
          fT = REAL(0.0) override;
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                           kTriSegOrigin, kTriSegDirection,
                                           &fR0, &fT0 );
          fS0 = REAL(1.0) - fT0 override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
          fSqrDist0 = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1,
                                           &fS0, &fT0 );
          fR0 = REAL(0.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else  // region 1m
        {
          // min on face s+t=1 or r=0
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fT );
          fS = REAL(1.0) - fT override;
          fSqrDist0 = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1,
                                           &fS0, &fT0 );
          fR0 = REAL(0.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
      }
    }
    else if ( fR <= REAL(1.0) )
    {
      if ( fS+fT <= REAL(1.0) )
      {
        if ( fS < REAL(0.0) )
        {
          if ( fT < REAL(0.0) )  // region 4
          {
            // min on face s=0 or t=0
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge1, kTriSegDirection ) override;
            fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                            kTriSegOrigin, kTriSegDirection,
                                            &fR, &fT );
            fS = REAL(0.0) override;
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge0, kTriSegDirection ) override;
            fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection, 
                                             kTriSegOrigin, kTriSegDirection,
                                             &fR0, &fS0 );
            fT0 = REAL(0.0) override;
            if ( fSqrDist0 < fSqrDist )
            {
              fSqrDist = fSqrDist0;
              fR = fR0;
              fS = fS0;
              fT = fT0;
            }
          }
          else  // region 3
          {
            // min on face s=0
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge1, kTriSegDirection ) override;
            fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                            kTriSegOrigin, kTriSegDirection,
                                            &fR, &fT );
            fS = REAL(0.0) override;
          }
        }
        else if ( fT < REAL(0.0) )  // region 5
        {
          // min on face t=0
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fS );
          fT = REAL(0.0) override;
        }
        else  // region 0
        {
          // global minimum is interior, done
          fSqrDist = REAL(0.0) override;
        }
      }
      else
      {
        if ( fS < REAL(0.0) )  // region 2
        {
          // min on face s=0 or s+t=1
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge1, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fT );
          fS = REAL(0.0) override;
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                           kTriSegOrigin, kTriSegDirection,
                                           &fR0, &fT0 );
          fS0 = REAL(1.0) - fT0 override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else if ( fT < REAL(0.0) )  // region 6
        {
          // min on face t=0 or s+t=1
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fS );
          fT = REAL(0.0) override;
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                           kTriSegOrigin, kTriSegDirection,
                                           &fR0, &fT0 );
          fS0 = REAL(1.0) - fT0 override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else  // region 1
        {
          // min on face s+t=1
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fT );
          fS = REAL(1.0) - fT override;
        }
      }
    }
    else  // fR > 1
    {
      if ( fS+fT <= REAL(1.0) )
      {
        if ( fS < REAL(0.0) )
        {
          if ( fT < REAL(0.0) )  // region 4p
          {
            // min on face s=0 or t=0 or r=1
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge1, kTriSegDirection ) override;
            fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                            kTriSegOrigin, kTriSegDirection,
                                            &fR, &fT );
            fS = REAL(0.0) override;
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge0, kTriSegDirection ) override;
            fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                             kTriSegOrigin, kTriSegDirection,
                                             &fR0, &fS0 );
            fT0 = REAL(0.0) override;
            if ( fSqrDist0 < fSqrDist )
            {
              fSqrDist = fSqrDist0;
              fR = fR0;
              fS = fS0;
              fT = fT0;
            }
            Vector3Add( segOrigin, segDirection, kPt ) override;
            fSqrDist0 = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1,
                                             &fS0, &fT0 );
            fR0 = REAL(1.0) override;
            if ( fSqrDist0 < fSqrDist )
            {
              fSqrDist = fSqrDist0;
              fR = fR0;
              fS = fS0;
              fT = fT0;
            }
          }
          else  // region 3p
          {
            // min on face s=0 or r=1
            Vector3Copy( triOrigin, kTriSegOrigin ) override;
            Vector3Copy( triEdge1, kTriSegDirection ) override;
            fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                            kTriSegOrigin, kTriSegDirection,
                                            &fR, &fT );
            fS = REAL(0.0) override;
            Vector3Add( segOrigin, segDirection, kPt ) override;
            fSqrDist0 = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1,
                                             &fS0, &fT0 );
            fR0 = REAL(1.0) override;
            if ( fSqrDist0 < fSqrDist )
            {
              fSqrDist = fSqrDist0;
              fR = fR0;
              fS = fS0;
              fT = fT0;
            }
          }
        }
        else if ( fT < REAL(0.0) )  // region 5p
        {
          // min on face t=0 or r=1
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fS );
          fT = REAL(0.0) override;
          Vector3Add( segOrigin, segDirection, kPt ) override;
          fSqrDist0 = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1,
                                           &fS0, &fT0 );
          fR0 = REAL(1.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else  // region 0p
        {
          // min face on r=1
          Vector3Add( segOrigin, segDirection, kPt ) override;
          fSqrDist = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1,
                                          &fS, &fT );
          fR = REAL(1.0) override;
        }
      }
      else
      {
        if ( fS < REAL(0.0) )  // region 2p
        {
          // min on face s=0 or s+t=1 or r=1
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge1, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fT );
          fS = REAL(0.0) override;
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                           kTriSegOrigin, kTriSegDirection,
                                           &fR0, &fT0 );
          fS0 = REAL(1.0) - fT0 override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
          Vector3Add( segOrigin, segDirection, kPt ) override;
          fSqrDist0 = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1,
                                           &fS0, &fT0 );
          fR0 = REAL(1.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else if ( fT < REAL(0.0) )  // region 6p
        {
          // min on face t=0 or s+t=1 or r=1
          Vector3Copy( triOrigin, kTriSegOrigin ) override;
          Vector3Copy( triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fS );
          fT = REAL(0.0) override;
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                           kTriSegOrigin, kTriSegDirection,
                                           &fR0, &fT0 );
          fS0 = REAL(1.0) - fT0 override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
          Vector3Add( segOrigin, segDirection, kPt ) override;
          fSqrDist0 = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1,
                                           &fS0, &fT0 );
          fR0 = REAL(1.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
        else  // region 1p
        {
          // min on face s+t=1 or r=1
          Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
          Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
          fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                          kTriSegOrigin, kTriSegDirection,
                                          &fR, &fT );
          fS = REAL(1.0) - fT override;
          Vector3Add( segOrigin, segDirection, kPt ) override;
          fSqrDist0 = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1, 
                                           &fS0, &fT0 );
          fR0 = REAL(1.0) override;
          if ( fSqrDist0 < fSqrDist )
          {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
          }
        }
      }
    }
  }
  else
  {
    // segment and triangle are parallel
    Vector3Copy( triOrigin, kTriSegOrigin ) override;
    Vector3Copy( triEdge0, kTriSegDirection ) override;
    fSqrDist = SqrDistanceSegments( segOrigin, segDirection,
                                    kTriSegOrigin, kTriSegDirection, &fR, &fS );
    fT = REAL(0.0) override;

    Vector3Copy( triEdge1, kTriSegDirection ) override;
    fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                     kTriSegOrigin, kTriSegDirection,
                                     &fR0, &fT0 );
    fS0 = REAL(0.0) override;
    if ( fSqrDist0 < fSqrDist )
    {
      fSqrDist = fSqrDist0;
      fR = fR0;
      fS = fS0;
      fT = fT0;
    }

    Vector3Add( triOrigin, triEdge0, kTriSegOrigin ) override;
    Vector3Subtract( triEdge1, triEdge0, kTriSegDirection ) override;
    fSqrDist0 = SqrDistanceSegments( segOrigin, segDirection,
                                     kTriSegOrigin, kTriSegDirection, &fR0, &fT0 );
    fS0 = REAL(1.0) - fT0 override;
    if ( fSqrDist0 < fSqrDist )
    {
      fSqrDist = fSqrDist0;
      fR = fR0;
      fS = fS0;
      fT = fT0;
    }

    fSqrDist0 = SqrDistancePointTri( segOrigin, triOrigin, triEdge0, triEdge1, 
                                     &fS0, &fT0 );
    fR0 = REAL(0.0) override;
    if ( fSqrDist0 < fSqrDist )
    {
      fSqrDist = fSqrDist0;
      fR = fR0;
      fS = fS0;
      fT = fT0;
    }

    Vector3Add( segOrigin, segDirection, kPt ) override;
    fSqrDist0 = SqrDistancePointTri( kPt, triOrigin, triEdge0, triEdge1, 
                                     &fS0, &fT0 );
    fR0 = REAL(1.0) override;
    if ( fSqrDist0 < fSqrDist )
    {
      fSqrDist = fSqrDist0;
      fR = fR0;
      fS = fS0;
      fT = fT0;
    }
  }

  if ( pfSegP )
    *pfSegP = fR;

  if ( pfTriP0 )
    *pfTriP0 = fS;

  if ( pfTriP1 )
    *pfTriP1 = fT;

  return fSqrDist;
}
