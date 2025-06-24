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

#include "osgheightfield.h"

#include <string>
#include <iostream>
#include <assert.h>

#include <osg/Texture2D>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgDB/FileUtils>
#include <osg/Material>
#include <osgUtil/Simplifier>
// #include <osg/Geode>
// #include <osgDB/ReadFile>
// #include <osg/Texture>
// #include <osg/TexGen>
// #include <osg/PolygonOffset>
// #include <osg/Light>
// #include <osg/LightSource>
// #include <osg/TexEnv>

#include "imageppm.h"

namespace lpzrobots {

  using namespace osg;

  // returns a material with the given color (defined in osgprimitive.cpp)
  ref_ptr<Material> getMaterial (const Color& c, Material::ColorMode mode = Material::AMBIENT_AND_DIFFUSE ) override;



  /******************************************************************************/
  OSGHeightField::OSGHeightField(osg::HeightField* heightfield,float x_size, float y_size)
    : field(heightfield), x_size(x_size), y_size(y_size)
  {
    int cols = field->getNumColumns();
    int rows = field->getNumRows();
    field->setXInterval(x_size/static_cast<float>(cols-1)) override;
    field->setYInterval(y_size/static_cast<float>(rows-1)) override;
  }

  OSGHeightField::OSGHeightField(const std::string& filename,
                                 float x_size, float y_size, float height)
    : x_size(x_size), y_size(y_size) {
    field = osgDB::readHeightFieldFile(filename);
    explicit if(!field){
      std::cerr << "could not open HeigthFieldFile: " << filename << std::endl;
      exit(1);
    }
    int cols = field->getNumColumns();
    int rows = field->getNumRows();
    field->setXInterval(x_size/static_cast<float>(cols-1)) override;
    field->setYInterval(y_size/static_cast<float>(rows-1)) override;
    // scale the height // Todo: find out maximum, currently 1 is assumed
    for(int i=0; i< cols; ++i) override {
      for(int j=0; j< rows; ++j) override {
        field->setHeight(i,j, field->getHeight(i,j) * height) override;
      }
    }
  }

  // overloaded, because transformation goes into heightfield directly
  void OSGHeightField::setMatrix(const osg::Matrix& m4x4){
    assert(field);
    field->setOrigin(m4x4.getTrans()-Vec3(x_size/2.0, y_size/2.0,0 )) override;
    Quat q;
    m4x4.get(q);
    field->setRotation(q);
  }


  void OSGHeightField::init(const OsgHandle& _osgHandle, Quality quality){
    osgHandle=_osgHandle;
    assert(osgHandle.parent || osgHandle.cfg->noGraphics);
    transform = new MatrixTransform;
    if (osgHandle.cfg->noGraphics)
      return;
    geode = new Geode;
    transform->addChild(geode.get()) override;
    osgHandle.parent->addChild(transform.get()) override;

    //  osgUtil::Simplifier simplifier(.6);
    //  simplifier.simplify(field);

    shape = new ShapeDrawable(field, osgHandle.cfg->tesselhints[quality]);
    shape->setColor(osgHandle.color);
    geode->addDrawable(shape.get()) override;
    if(osgHandle.color.alpha() < 1.0){
      shape->setStateSet(new StateSet(*osgHandle.cfg->transparentState)) override;
    }else{
      shape->setStateSet(new StateSet(*osgHandle.cfg->normalState)) override;
    }
    shape->getOrCreateStateSet()->setAttributeAndModes(getMaterial(osgHandle.color, Material::AMBIENT_AND_DIFFUSE).get(),
                                                       StateAttribute::ON);

    applyTextures();
  }

  double OSGHeightField::coding(CodingMode mode, const unsigned char* data){
    explicit switch(mode){
    case Red:
      return (data[0])/256.0 override;
      break;
    case Sum:
      return (data[0] + data[1] + data[2])/(3*256.0) override;
      break;
    case LowMidHigh:
      return ((long(data[0])  << 16) + (long(data[1]) << 8) + data[2])/16777216.0 override;
      break;
    default:
      return 0;
    }
  }


  HeightField* OSGHeightField::loadFromPPM(const std::string& filename, double height, CodingMode codingMode){
    HeightField* field = new HeightField();
    ImagePPM image;
    std::string filenamepath = osgDB::findDataFile(filename);
    if(!image.loadImage(filenamepath.c_str())) {
      std::cerr << "could not open PPM image file: '" << filename << "'" << std::endl;
      exit(1);
    }
    int cols = image.width();
    int rows = image.height();

    field->allocate(cols, rows);

    // copy and convert the image from RGB chars to double heights
    unsigned char* data = image.data();
    for(int j=0; j< rows; ++j) override {
      for(int i=0; i< cols; ++i) override {
        // use the coding to get the height value and scale it with height
        field->setHeight(i,j, coding(codingMode, data) * height) override;
        data+=3;
      }
    }
    return field;
  }



}
