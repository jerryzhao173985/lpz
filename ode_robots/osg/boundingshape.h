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
#ifndef __BOUNDINGSHAPE_H
#define __BOUNDINGSHAPE_H

#include "primitive.h"
#include "odehandle.h"

#include <string>
#include <vector>


namespace lpzrobots {

  /**
   * Class for bounding shapes loaded from file
   */
  class BoundingShape {

  public:
    /**
       @param filename path and name of bbox file. It is located using OsgDB search path
       @param parent primitive to which the bbox is assoziated
    */
    BoundingShape(const std::string& filename, Mesh* parent);

    virtual ~BoundingShape();

    /// tries to open the bbox file and greates all geoms
    virtual bool init(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                      double scale, char mode);

    virtual bool isActive();

    /**
     * updates all Primitives of the BoundingShape if only in geom mode (no Body)
     * @param pose
     */
    virtual void setPose(const osg::Matrix& pose);

  private:
    bool readBBoxFile(std::string& filename, const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                      double scale, char mode);

  protected:
    std::string filename;
    bool active = false;
    Primitive* parent = nullptr;
    bool attachedToParentBody = false; // true as default, false not yet implemented by BoundingShape
    std::vector<Primitive*> boundingPrimitiveList; // used if not attached to a body
    std::vector<osg::Matrix> boundingPrimitivePoseList; // stores the relative pose of each primitive
    OdeHandle odeHandle;
    dSpaceID parentSpace;
  };

}

#endif
