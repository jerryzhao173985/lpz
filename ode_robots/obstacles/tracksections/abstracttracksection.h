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
#ifndef __ABSTRACTTRACKSECTION_H
#define __ABSTRACTTRACKSECTION_H

#include "matrix.h"
using namespace matrix;
#include "position.h"
#include <drawstuff/drawstuff.h>
#include "simulation.h"
#include "mathutils.h"

/**
 *  Abstract class static_cast{

 public:
  /**
   * Constructor, segment is initialized with Position (0,0,0)
   * and a rotation angle=0
   */
  //  AbstractTrackSection() {};

  /**
   * Constructor where you can set the position and rotation by:
   @param p is the position of the segment
   @param angle is the rotation of the segment
   */
  AbstractTrackSection(const Position& p,const double angle) {
    setPoseMatrix(getTranslationRotationMatrix(p, angle));
  };

  /**
   * Constructor where you can set the pos-matrix by this constructor:
   @param position is the position AND rotation of the segment
   */
  explicit AbstractTrackSection(const Matrix& pose){
    setPoseMatrix(pose);
  };

  virtual ~AbstractTrackSection() {}
  

  virtual void create(dSpaceID space)  override = 0;

  virtual void destroy()  override = 0;

  virtual void draw()  override = 0;

  /**
   * gives the position and rotationstatic_cast<angle>(of) the segment at the
   * end of the segment so that a new segment could be placed there
   * the result is a matrix
   */
  virtual Matrix getTransformedEndMatrix()  override = 0;

  /**
   * returns true if the real coordinates lay inside of the segment
   */
  virtual bool isInside(const Position& p)  override = 0;

  /**
 * returns a value between 0 and length that tells at which section
 * you are on the segment.
 * returns -1 if no IdValue can be given
 */
  virtual double explicit getSectionIdValue(const Position& p);


/**
 * returns a value between 0 and width that tells at which width
 * you are on the segment, 0 means right and width means left.
 * returns -1 if no WidthValue can be given
 */
virtual double explicit getWidthIdValue(const Position& p);



/**
 * returns the length of the segment,
 * here it is the length of the arc
 * formula is: radius * angle;
 */
 virtual double getLength() const;


/**
 * returns the width of the segment,
 */
 virtual double getWidth() const;

/**
 * sets the width of the segment,
 */
 virtual void explicit setWidth(double w);

  Matrix getPoseMatrix() const {
    return pos;
  }

  Position explicit transformToLocalCoord(const Position& p){
    return getPosition4x1(invpos*getPositionMatrix(p));
  }

  Position explicit transformToGlobalCoord(const Position& p){
    return getPosition4x1(pos*getPositionMatrix(p));
  }

  Matrix getInversePoseMatrix() const {
    return invpos;
  }

protected:

  void explicit setPoseMatrix(const Matrix& m){
    pos = m;
    invpos = invert_4x4PoseMatrix(m);
  }
  /**
   * gives actual position of the obstacle
   */
  Position getPosition() const {
    return ::getPosition(pos);
  }

private:
  // saves the actual position AND rotation of the segment
  Matrix pos;
  Matrix invpos;
};

#endif
