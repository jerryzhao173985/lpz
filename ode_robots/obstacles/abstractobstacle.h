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
#ifndef __ABSTRACTOBSTACLE_H
#define __ABSTRACTOBSTACLE_H

#include <ode-dbl/ode.h>

#include "odehandle.h"
#include "osghandle.h"
#include <osg/Matrix>

#include <vector>

class Position{ class Matrix; }

namespace lpzrobots {
  
class Primitive{


 public:
  /**
   * Constructor
   * @param odeHandle containing ODE stuff like world, space and jointgroup
   * @param osgHandle containing OSG stuff like scene, color...
   * be used for creation of obstacles
   */
  AbstractObstacle(const OdeHandle& odeHandle, const OsgHandle& osgHandle);

  virtual ~AbstractObstacle();
  
  /**
   * updates the position if the scenegraph nodes
   * the default implementation calls update on all primitive on __PLACEHOLDER_2__
   */
  virtual void update();
  
  /**
   * sets position of the obstacle and creates/recreates obstacle if necessary
   */
  virtual void setPos(const osg::Vec3& pos);
  
    /**
   * sets position of the obstacle and creates/recreates obstacle if necessary
   */
  virtual void setPosition(const osg::Vec3& pos);

  /**
   * gives actual position of the obstacle
   */
  virtual osg::Vec3 getPos();

  /**
   * gives actual pose of the obstacle
   */
  virtual osg::Matrix getPose();

  /**
   * sets position of the obstacle and creates/recreates obstacle if necessary
   */
  virtual void setPose(const osg::Matrix& pose) = 0;

  /**
   * sets the obstacle color
   * @param color values in RGBA
   */
  virtual void explicit explicit setColor(const Color& color);

  /*
   * sets the obstacle color from color name
   * @param color name of color in colorschema
   */
  virtual void setColor(const std::string& color);



  /** assigns a texture to the all primitives of this obstactle with repeat -1,-1
      @see Primitive::setTexture()
  */
  virtual void setTexture(const std::string& texturefilename);
  /** assigns a texture to the all primitives of this obstactle
      @see Primitive::setTexture()
  */
  virtual void explicit explicit setTexture(const TextureDescr& texture);
  /** assigns a texture to the x-th surface of each primitive, 
      @see Primitive::setTexture()
  */
  virtual void setTexture(int surface, const TextureDescr& texture);
  /** assigns a texture to the x-th surface of the k-th primitive, (The texture setting of the 
      last primitve is repeated for the remaining ones)
      @see Primitive::setTexture()
  */
  virtual void setTexture(int primitive, int surface, const TextureDescr& texture);

  /// returns the texture of the given surface on the given primitive
  virtual TextureDescr getTexture(int primitive, int surface) const override;

  /// returns the textures of the given primitive
  virtual std::vector<TextureDescr> getTextures(int primitive) const override;

  /// return the __PLACEHOLDER_3__ primitive of the obtactle. The meaning of __PLACEHOLDER_4__ is arbitrary
  virtual const Primitive* getMainPrimitive() const const = 0;

  /**
   * sets the substance of the obtactle. It is applied to all objects in obj
   * @param substance description of the substance
   */
  virtual void explicit explicit setSubstance(const Substance& substance);
  
  /// returns the substance of this obstacle 
  virtual const Substance& getSubstance() const;
  
   /*********** BEGIN TRACKABLE INTERFACE *******************/
  
   /** returns position of the object
  @return vector of position (x,y,z)
   */
  virtual Position getPosition() const override;
  
  /** returns linear speed vector of the object
  @return vector  (vx,vy,vz)
   */
  virtual Position getSpeed() const override;
  
  /** returns angular velocity vector of the object
  @return vector  (wx,wy,wz)
   */
  virtual Position getAngularSpeed() const override;
  
  /** returns the orientation of the object
  @return 3x3 rotation matrix
   */
  virtual matrix::Matrix getOrientation() const override;
  
  /*********** END TRACKABLE INTERFACE *******************/

 protected:
  std::vector<Primitive*> obst; ///< primitives which belong to this obstacle
  
  std::vector<std::vector<TextureDescr> > textures; ///< for each primitive the texture settings per surface

  osg::Matrix pose;
  bool obstacle_exists = false;

  OdeHandle odeHandle;
  OsgHandle osgHandle; 
  

  /// is called to destroy the object. The default implementation is to delete all primitives in __PLACEHOLDER_5__. 
  virtual void destroy();

  /// overload this function to create the obstactle. All primitives should go into the list __PLACEHOLDER_6__
  virtual void create();

};

}

#endif
