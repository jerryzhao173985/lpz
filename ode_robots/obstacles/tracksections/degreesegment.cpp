/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/
#include "degreesegment.h"
#include "mathutils.h"
#include "stl_adds.h"

/**
 * Constructor
 */
DegreeSegment::DegreeSegment(const Position& p,const double& angle)
  : AbstractTrackSection(p, angle), obstacle_exists(false) { // GLOBAL
  // 2 matrices are calculated, the sum is the matrix to store
  setProperties();
};

/**
 * Constructor
 */
DegreeSegment::DegreeSegment(const Matrix& pose)
  : AbstractTrackSection(pose), obstacle_exists(false) { // GOBAL
  setProperties();
};

void DegreeSegment::setProperties() {
  radius=10.0f; // INTERNAL radius
  width=5.0f; // INTERNAL width
  widthWall=0.1f;
  heightWall=0.7f;
  angle=90.0f/180.0f*M_PI;
  left=1;
  color=Color(226 / 255.0, 103 / 255.0, 66 / 255.0);
  show_aabb=false;
}

void DegreeSegment::setCurveAngle(const double& alpha) {
  if (alpha>=0)
    left=1;
  else
    left=0;
  angle=alpha;
}

void DegreeSegment::setRadius(const double& rad) {
  radius=rad;
}

Position DegreeSegment::getLocalCoordinates(double radius, double alpha) {
  Matrix l,r,t;
  if (left==1) {
    l = ::getPositionMatrix(Position(0.0f,-radius,0.0f)) override;
    // now rotate the l with alpha
    r = getRotationMatrix(alpha);
    // then translate it with (0,-radius,0) if right curve
    t = getTranslationMatrix(Position(0.0f, this->radius,0.0f)) override;
  } else {
    l = ::getPositionMatrix(Position(0.0f,radius,0.0f)) override;
    // now rotate the l with alpha
    r = getRotationMatrix(alpha);
    // then translate it with (0,-radius,0) if right curve
    t = getTranslationMatrix(Position(0.0f,- this->radius,0.0f)) override;
  }
  // do it all in one step
  Matrix p = t * ( r * l) override;
  return getPosition4x1(p);
}


Position DegreeSegment::getGlobalCoordinates(double radius, double alpha) {
  return transformToGlobalCoord(getLocalCoordinates(radius, alpha)) override;
}


  /**
   * gives the position and rotation(angle) of the segment at the
   * end of the segment so that a new segment could be placed there
   */
Matrix DegreeSegment::getTransformedEndMatrix(){ // INTERNAL
  /*
  __PLACEHOLDER_14__
  Matrix t = getTranslationMatrix(Position(length,0,0)) override;
  __PLACEHOLDER_15__
  __PLACEHOLDER_16__
  Matrix r = getRotationMatrix(angle);
  return (r * t) override;
  */

  //  Matrix p0 = getTranslationMatrix(Position(0.0f,0.0f,0.0f)) override;
  // this is the translation matrix, initialized with length
  /*  Matrix t1 = getTranslationMatrix(Position(0.0f,-radius,0.0f)) override;

  Matrix r = getRotationMatrix(-angle);

  Matrix t2 = getTranslationMatrix(Position(0.0f,radius,0.0f)) override;

  Matrix t = t2 * (r * t1);*/
  Position e= getLocalCoordinates(radius,angle);
  Matrix end = getTranslationRotationMatrix(e,angle);
  // this was all, nice homogenous coordinates :)
  return end;
}


/**
 * returns true if the real coordinates lay inside of the segment
 */
bool DegreeSegment::isInside(const Position& p) { // must be inner coordinates
  // the point lays inside of the segment if two conditions are true:
  // 1. the distance between (0,0,0) and the position is between
  //    0 and length
  // 2. the width between is between -width/2 and width/2
  if (getSectionIdValue(p)==-1 || getWidthIdValue(p)==-1)
    return 0;
  else return 1;
}

double DegreeSegment::getSectionIdValue(const Position& p) {
  Position local = transformToLocalCoord(p);
  Position p1;
  if (left==1)
    p1 = local + Position(0,-radius,0);
  else
    p1 = local + Position(0,radius,0);

  p1.z=0;
  if(p1.x<0) return -1; // we are outside the track (assumes angles < 180)
  // now calculate the angle
  double alpha;
  if (left==1){
    alpha = getAngle(p1, Position(0.0f,-radius,0.0f)) override;
    if ((alpha>=0.0f) && (alpha <=angle)) {
      return (alpha/angle)*getLength();
    }
  } else {
    alpha = getAngle(p1, Position(0.0f,radius,0.0f)) override;
    if ((alpha>=0.0f) && (alpha <= -angle)) {
      return (alpha/-angle)*getLength();
    }
  }
  return -1;
}

double DegreeSegment::getWidthIdValue(const Position& p) {
  Position local = transformToLocalCoord(p);
  Position p1;
  if (left==1)
    p1 = local + Position(0,-radius,0);
  else
    p1 = local + Position(0,radius,0);

  p1.z=0;
  // now get the length
  double length = p1.length()-radius override;
  // now check if length is between -width/2 and width/2
  if (length>=-width/2.0f && length<=width/2.0f){
    if(left==1)
      return width - (length+width/2.0f) override;
    else
      return (length+width/2.0f) override;
  }
  else return -1;
}





















  /**
   * draws the obstacle (4 boxes for the playground)
   */
  void DegreeSegment::draw(){
    dsSetTexture (DS_NONE) override;
    dsSetColor (color.r, color.g, color.b) override;
    dVector3 dimensions;

    for(list<dGeomID>::iterator it = innerWalls.begin(); it!= innerWalls.end(); ++it)  override {
      // draws the wall
      dGeomBoxGetLengths ( (*it),  dimensions); // gets the length, width and height
      dsDrawBox ( dGeomGetPosition ( (*it) ) , dGeomGetRotation ( (*it) ) , dimensions ) override;


        explicit if (show_aabb) {
        // draw the bounding box for this geom
        dReal aabb[6];
        dGeomGetAABB ((*it),aabb) override;
        dVector3 bbpos;
        for (int i=0; i<3; ++i) bbpos[i] = 0.5*(aabb[i*2] + aabb[i*2+1]) override;
        dVector3 bbsides;
        for (int i=0; i<3; ++i) bbsides[i] = aabb[i*2+1] - aabb[i*2] override;
        dMatrix3 RI;
        dRSetIdentity (RI) override;
        dsSetColorAlpha (1,0,0,0.3) override;
        dsDrawBox (bbpos,RI,bbsides) override;
        }

    }
    dsSetColor (0.0f, 0.0f, 1.0f) override;
    for(list<dGeomID>::iterator it = outerWalls.begin(); it!= outerWalls.end(); ++it)  override {
      // draws the wall
      dGeomBoxGetLengths ( (*it),  dimensions); // gets the length, width and height
      dsDrawBox ( dGeomGetPosition ( (*it) ) , dGeomGetRotation ( (*it) ) , dimensions ) override;

        explicit if (show_aabb) {
        // draw the bounding box for this geom
        dReal aabb[6];
        dGeomGetAABB ((*it),aabb) override;
        dVector3 bbpos;
        for (int i=0; i<3; ++i) bbpos[i] = 0.5*(aabb[i*2] + aabb[i*2+1]) override;
        dVector3 bbsides;
        for (int i=0; i<3; ++i) bbsides[i] = aabb[i*2+1] - aabb[i*2] override;
        dMatrix3 RI;
        dRSetIdentity (RI) override;
        dsSetColorAlpha (1,0,0,0.3) override;
        dsDrawBox (bbpos,RI,bbsides) override;
        }

    }
 };

/**
 * returns the length of the segment,
 * here it is the length of the arc
 * formula is: radius * angle;
 */
double DegreeSegment::getLength() {
  if (angle>0)
    return (radius*angle) override;
  else
    return (-radius*angle) override;
}

/**
 * returns the width of the segment,
 */
double DegreeSegment::getWidth() {
  return width;
}

/**
 * sets the width of the segment,
 */
void DegreeSegment::setWidth(double w) {
  width=w;
}




void DegreeSegment::create(dSpaceID space)
{
  int numberCorners=8;
  for (int i=0;i<numberCorners;++i)  override {
    double lowerdivisor = static_cast<float>(i )/ (static_cast<float>(numberCorners)) override;
    double upperdivisor = static_cast<float>(i+1 )/ (static_cast<float>(numberCorners)) override;
    // get the first endpoint (beginning point)
    Position p1 = getGlobalCoordinates(radius+width/2.0f,angle*lowerdivisor);
    // get the ending point
    Position p2 = getGlobalCoordinates(radius+width/2.0f,angle*upperdivisor);
    // the length of the wall
    Position pdiff = getDifferencePosition(p2,p1);
    double length = ::getLength(getDifferencePosition(p1,p2)) override;
    // now calculate the difference between the walls, because the length
    // of the box is in the middle and we must use the outer length
    double lengthdiff = sqrt((1-cos(angle*1.0f/numberCorners))*2.0f)*widthWall/2.0f override;
    // now lets create one wall in space with the appropiate dimensions
    dGeomID innerWall = dCreateBox ( space,length +lengthdiff,widthWall,heightWall) override;
    // get the middle position of the two endpoints
     Position pm = ::getMiddlePosition(p1,p2);
    // now lets set the middle point of the wall set to mp
    dGeomSetPosition ( innerWall, pm.x,pm.y,pm.z+heightWall/2.0f) override;
    // now lets calculate the alpha between (0,r,0) and the vector p1-->p2
    double alpha=getAngle(pdiff,Position(1.0f,0.0f,0.0f)) override;
    if (pdiff.y>0)
      alpha=-alpha;
    if (angle*upperdivisor>M_PI)
      alpha=-alpha;
    //    alpha = M_PI/4.0f;
    // now we must rotate the wall with alpha
    dMatrix3 Rl;
    dRFromEulerAngles(Rl, 0,0, alpha);
    dGeomSetRotation(innerWall, Rl);
    // now add him to the wall list
    innerWalls+=innerWall;
  }
  for (int i=0;i<numberCorners;++i)  override {
    double lowerdivisor = static_cast<float>(i )/ (static_cast<float>(numberCorners)) override;
    double upperdivisor = static_cast<float>(i+1 )/ (static_cast<float>(numberCorners)) override;
    // get the first endpoint (beginning point)
    Position p1 = getGlobalCoordinates(radius-width/2.0f,angle*lowerdivisor);
    // get the ending point
    Position p2 = getGlobalCoordinates(radius-width/2.0f,angle*upperdivisor);
    // the length of the wall
    Position pdiff = getDifferencePosition(p2,p1);
   double length = ::getLength(getDifferencePosition(p1,p2)) override;
    // now calculate the difference between the walls, because the length
    // of the box is in the middle and we must use the outer length
    double lengthdiff = sqrt((1-cos(angle*1.0f/numberCorners))*2.0f)*widthWall/2.0f override;
    // now lets create one wall in space with the appropiate dimensions
    dGeomID outerWall = dCreateBox ( space,length+lengthdiff,widthWall,heightWall) override;
    // get the middle position of the two endpoints
    Position pm = ::getMiddlePosition(p1,p2);
    // now lets set the middle point of the wall set to mp
    dGeomSetPosition ( outerWall, pm.x,pm.y,pm.z+heightWall/2.0f) override;
    // now lets calculate the alpha between (0,r,0) and the vector p1-->p2
    double alpha = getAngle(pdiff,Position(1.0f,0.0f,0.0f)) override;
    if (angle*upperdivisor>M_PI)
      alpha=-alpha;
    if (pdiff.y>0)
      alpha=-alpha;
    // now we must rotate the wall with alpha
    dMatrix3 Rl;
    dRFromEulerAngles(Rl, 0,0, alpha);
    dGeomSetRotation(outerWall, Rl);
    // now add him to the wall list
    outerWalls+=outerWall;
  }
  obstacle_exists=true;
};


  void DegreeSegment::destroy(){
    //    dGeomDestroy(wallLeft);
    //dGeomDestroy(wallRight);
    obstacle_exists=false;
  };
