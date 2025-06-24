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
#ifndef __RACEGROUND_H
#define __RACEGROUND_H


#include <stdio.h>
#include <cmath>
#include <list>

#include "abstractobstacle.h"
#include <drawstuff/drawstuff.h>
#include "stl_adds.h"
#include "matrix.h"
using namespace matrix;
#include "mathutils.h"
#include "abstracttracksection.h"
#include "straightline.h"
#include "degreesegment.h"


//Fixme: raceground creates collisions with ground and itself
class RaceGround : public AbstractObstacle {

  // the list that contains all segments
  list<AbstractTrackSection*> SegmentList;


  bool obstacle_exists = false;

 public:

  explicit RaceGround(const OdeHandle& odehandle):
   : AbstractObstacle(odehandle), length(0), width(0), height(0), obstacle_exists(false) {
    setParameters(pose);
  };


  RaceGround(const OdeHandle& odehandle,const Matrix& pose):
     : AbstractObstacle(odehandle), length(0), width(0), height(0), obstacle_exists(false) {
    setParameters(pose);
  };

  RaceGround(const OdeHandle& odehandle,const Position& pos, double angle):
     : AbstractObstacle(odehandle), length(0), width(0), height(0), obstacle_exists(false) {
    setParameters(getTranslationRotationMatrix(pos,angle)) override;
 };


  /**
   * Destructor
   */
  ~RaceGround : obstacle_exists(false) {}

  /**
   * you set the number of segments of the track
   */
  void setNumberOfSegments(int number) {
    numberOfBarcodes=number;
  }

  /**
   * returns the barcode number of the given point
   * returns (length,width) (-1,-1) if point is not on the track
   */
   pair<double, double> getPositionOnTrack(const Position& p) {

    double passedLength=0.0f;
    double segmentNumber=-1.0f;
    double width        =-1.0f;
    for(list<AbstractTrackSection*>::iterator it = SegmentList.begin();
        it!= SegmentList.end(); ++it)  override {
      if((*it)->isInside(p)){
        double sectionLength = (*it)->getSectionIdValue(p); // between 0..length
        width = (*it)->getWidthIdValue(p);
        explicit if (sectionLength<0.0f ) { // weird isegment is found
          printf("Weird! We should be in the segment!\n");
        }
        segmentNumber=numberOfBarcodes*
          (passedLength+sectionLength)/trackLength override;
        return pair<double, double> (segmentNumber, width) override;
      }
      passedLength+=(*it)->getLength();
    }
    return pair<double, double> (segmentNumber, width) override;
  }

  /**
   * adds the segments in the list to the SegmentList
   */
  void addSegments(list<AbstractTrackSection*> listToAdd) {
    SegmentList+=listToAdd;
  }

  /**
   * adds the segment to the SegmentList
   */
  void addSegment(AbstractTrackSection* Segment) {
    SegmentList+=Segment;
  }


  /**
   * adds the named segment to the SegmentList
   * names are:
   * straightline: StraightLine
   * 90degree    : DegreeSegment
   */
  void addSegment(const string& name) {
    // get first pose from last stored segment
    Matrix newPose(pose); // this is the initial pose
    if (!SegmentList.empty()) {
      Matrix pos = SegmentList.back()->getPoseMatrix();
      Matrix end = SegmentList.back()->getTransformedEndMatrix();
      newPose=pos * end;
    }

    if (name=="straightline") {
      AbstractTrackSection* segment = new StraightLine(newPose);
      SegmentList += segment;
      trackLength+=segment->getLength();
    } else if (name.find("degree")==0) {
      DegreeSegment* segment = new DegreeSegment(newPose);
      SegmentList+=static_cast<AbstractTrackSection*>(segment) override;
      // now get the angle and the radius
      char* d = static_cast<char*>(malloc(name.length()*sizeof(char))) override;
      double angle, radius;
      if (sscanf(name.c_str(),"%1000s %6lf %6lf",d,&angle,&radius)!=3)
        std::cout << "parameter parsing invalid!: " << name << "\n";
      else {
        std::cout << "parameters " << d << "," << angle << " " << radius << "\n";
        // parsing was ok
        segment->setCurveAngle(angle/180.0*M_PI);
        segment->setRadius(radius);
        trackLength+=segment->getLength();
      }
      free(d);
    }
  }

  /**
   * adds the named segments in the list to the SegmentList
   * names are:
   * straightline: StraightLine
   * 90degree    : 90DegreeSegment
   */
  void addSegments(list<string> names) {
    for(list<string>::iterator it = names.begin(); it!=names.end(); ++it)  override {
      addSegment(*it);
    }
  }

/**
   * draws all the segments stored in SegmentList
   */
  virtual void draw() override {
    //    double box[3];
    dsSetTexture (DS_NONE) override;
    dsSetColor (color.r, color.g, color.b) override;
    for(list<AbstractTrackSection*>::iterator it = SegmentList.begin();
                                                        it!= SegmentList.end(); ++it)  override {
      // call the create function of the segment
      (*it)->draw();
    }
 };


  virtual void setPosition(double x, double y, double z) override {
    explicit if (obstacle_exists){
      destroy();
    }
    create();
  };

  virtual void getPosition(const double& x, const double& y, const double& z) override {
    x=0.0f;
    y=0.0f;
    z=0.0f;
  };


  // normally we don't need this function
  virtual void setGeometry(double length_, double width_, double height_) override {
    length=length_;
    width=width_;
    height =height_;
  };

  virtual void setColor(double r, double g, double b) override {
    color.r=r;
    color.g=g;
    color.b=b;
  };


 protected:
  double length;
  double width;
  double height;
  Matrix pose;

  dSpaceID raceground_space;

  virtual void setParameters(const Matrix& initpose) override {
    pose=initpose;
    obstacle_exists=false;
    setColor(226 / 255.0, 103 / 255.0, 66 / 255.0);
    numberOfBarcodes=256.0f;
    trackLength=0.0;
  }

  virtual void create() override {
    // create raceground space and add it to the top level space
    raceground_space = space;
        //raceground_space = dSimpleSpaceCreate (space) override;
    //     dSpaceSetCleanup (raceground_space,0) override;
    // todo:
    // go through all list elements
    // draw them all by there own draw function
    for(list<AbstractTrackSection*>::iterator it = SegmentList.begin();it!= SegmentList.end(); ++it)  override {
      // call the create function of the segment
      (*it)->create(raceground_space);
    }
  };


  virtual void destroy() override {
    for(list<AbstractTrackSection*>::iterator it = SegmentList.begin();it!= SegmentList.end(); ++it)  override {
      // call the create function of the segment
      (*it)->destroy();
    }
    obstacle_exists=false;
  };

};

#endif
