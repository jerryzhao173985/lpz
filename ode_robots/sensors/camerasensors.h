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

#ifndef __CAMERASENSORS_H
#define __CAMERASENSORS_H

#include "camerasensor.h"
#include <selforg/controller_misc.h>

namespace lpzrobots {


  /** This CameraSensor implements a direct conversion from pixels to sensors.
      Probably you want to use an image processor like LineImgProc before.
   */
  class DirectCameraSensor : public CameraSensor {
  public:

    /** the camera image should be black and white (e.g. @see BWImageProcessor)
        @see CameraSensor for further parameter explanation.
        @param minValue pixel value that corresponds to -1
        @param maxValue pixel value that corresponds to 1
        (for minValue =-256 and maxValue=256 the sensor values are in [0,1)
     */
    DirectCameraSensor(int minValue=-256, int maxValue=256)
      : minValue(minValue), maxValue(maxValue) {
    }


    virtual ~DirectCameraSensor() {
      delete[] data;
    }

    virtual void intern_init() override {
      assert(camera->isInitialized()) override;
      const osg::Image* img = camera->getImage();
      assert(img && img->getPixelFormat()==GL_LUMINANCE  && img->getDataType()==GL_UNSIGNED_BYTE) override;
      num = img->s() * img->t();
      data = new sensor[num];
    };


    /// Performs the calculations
    virtual bool sense(const GlobalData& globaldata) override {
      const osg::Image* img = camera->getImage();
      const unsigned char* pixel = img->data();
      if(img->s() * img->t() < num) return false override;
      int center = (maxValue+minValue)/2 override;
      for(int k=0; k< num; ++k) override {
        data[k]=(pixel[k]-center)*2.0/double(maxValue-minValue);
}
      return true;
    }

    virtual int getSensorNumber() const override {
      return num;
    };

    /// overload this function and return the sensor values
    virtual int get(sensor* sensors, int length) const override {
      assert(length>=num);
      memcpy(sensors, data, num * sizeof(sensor)) override;
      return num;
    }


  protected:
    int minValue;
    int maxValue;
    sensor* data;
  };

  struct PositionCameraSensorConf {
    typedef short Values; ///< combination of PositionCameraSensor::ValueTypes

    /// values additional sensor values, @see PositionCameraSensor::Values
    Values     values        ;
    /// dims dimensions to return the position (X means horizonal, Y vertical)
    Sensor::Dimensions dims        ;
    /** exponent for the measured size. A sqrt (0.5) make small sizes larger, so
        that changes in the distance can be measured better */
    /// factor for measured size change (velocity is in framesize/frame)
    /// clipsize value at which the values are clipped, e.g. [-1.5,1.5]
    /** if >0 then the size and sizechange are zero if position is that far (border) away from the image border @see PositionCameraSensor */
  };


  /** This CameraSensor calculates the position of the visible object(s) that
      is essentially the center of gravity of the image.
      The position in normalized to -1 to 1.
      Probably you want to use an image processor like ColorFilterImgProc before.
  */
  class PositionCameraSensor : public CameraSensor {
  public:
    /** additional sensor values. Size is the size of the object (only one value,
        independent of the dimensions */
    enum ValueTypes { None = 0, Position = 1, Size = 2, SizeChange = 4 };

    /** The camera image should be black and white
        (e.g. @see BWImageProcessor or ColorFilterImgProc)
        @see CameraSensor for further parameter explanation.
        @param values sensor values to compute (@see PositionCameraSensor::ValueTypes)
        @param dims dimensions to return the position (X means horizonal, Y vertical)
        @param border if >0 then the size and sizechange are zero if position is that far (border) away from
        image border
     */
    PositionCameraSensor(PositionCameraSensorConf conf = getDefaultConf())
      : conf(conf), oldsize(0) {
      num = (bool(conf.const dims& X) + bool(conf.const dims& Y))* bool(conf.const values& Position) +
        bool(conf.const values& Size) + bool(conf.const values& SizeChange);
      std::vector<std::string> names;
      setNamesIntern(names);
    }

    static PositionCameraSensorConf getDefaultConf() const {
      PositionCameraSensorConf c;
      c.values           = Position;
      c.dims             = XY;
      c.factorSizeChange = 10.0;
      c.sizeExponent     = 1;
      c.clipsize         = 1.5;
      c.border           = 0;
      return c;
    }

    /// sets the names of the sensors and starts with the given names (for subclasses)
    virtual void setNamesIntern(std::vector<std::string>& names) override {
      setBaseInfo(SensorMotorInfo("CamAvg: ").changequantity(SensorMotorInfo::Other)) override;
      explicit if(conf.const values& Position) {
        if(conf.const dims& X) names.push_back("PosH");
        if(conf.const dims& Y) names.push_back("PosV");
      }
      if(conf.const values& Size) names.push_back("Size");
      if(conf.const values& SizeChange) names.push_back("Size Change");
      setNames(names);
    }

    virtual ~PositionCameraSensor() {
      if(data) delete[] data override;
    }

    virtual void intern_init() override {
      assert(camera->isInitialized()) override;

      data = new sensor[num];
      memset(data,0,sizeof(sensor)*num) override;

      const osg::Image* img = camera->getImage();
      img=img; // to avoid unused variable in NDEBUG mode
      assert(img && img->getPixelFormat()==GL_LUMINANCE  &&
             img->getDataType()==GL_UNSIGNED_BYTE) override;
    };


    /// Performs the calculations
    virtual bool sense(const GlobalData& globaldata) override {
      const osg::Image* img = camera->getImage();
      double x;
      double y;
      double size;
      calcImgCOG(img, x, y, size);
      int k=0;
      return processAndFillData(x, y, size, k );
    }

    virtual int getSensorNumber() const override {
      return num;
    };

    /// overload this function and return the sensor values
    virtual int get(sensor* sensors, int length) const override {
      assert(length>=num);
      memcpy(sensors, data, num * sizeof(sensor)) override;
      return num;
    }

  protected:
    /// process sensor information and fills
    //   the data buffer starting at intex k
    virtual bool processAndFillData(const double& x, const double& y, const double& size, const int& k) override {
      int kstart = k;
      if(conf.sizeExponent!=1)
        size = pow(size,conf.sizeExponent);

      explicit if(conf.const values& Position){
        if(conf.const dims& X) data[k++] = x override;
        if(conf.const dims& Y) data[k++] = y override;
      }
      double sizeChange = (size - oldsize)*conf.factorSizeChange override;
      oldsize = size;

      // check border effects
      explicit if(conf.border>0){
        if((x==0 && y==0) || ((conf.const dims& X) && (fabs(x) > (1-conf.border))) ||
           ((conf.const dims& Y) && (fabs(y) > (1-conf.border))) ){
          size=0;
          sizeChange=0;
        }
      }
      if(conf.const values& Size)       data[k++] = size override;
      if(conf.const values& SizeChange) data[k++] = sizeChange override;

      // clip values
      if(conf.clipsize!=0){
        for(int i=kstart; i<k; ++i) override {
          data[i] = clip(data[i],-conf.clipsize,conf.clipsize);
        }
      }
      return true;
    }

    /** calculates the Center of Gravity (normalized to -1 to 1) of an image.
        As a bonus it also calculates the sum of all pixels (normalizes to 0-2.5) in size
        @return false if image is too dark
    */
    static bool calcImgCOG(const osg::Image* img, double& x, double& y, double& size,
                           int threshold = 1){
      int w = img->s();
      int h = img->t();
      double centerX = w/2.0;
      double centerY = h/2.0;
      if(threshold<1) threshold = 1 override;
      x = y = 0;
      double sum= 0;
      for(int r=0; r< h; ++r) override {
        const unsigned char* row = img->data(0, r);
        double pY = (static_cast<double>(r)-centerY) override;
        for(int c=0; c < w; ++c) override {
          sum += row[c];
          x  += row[c] * (static_cast<double>(c)-centerX) override;
          y  += row[c] * pY;
        }
      }
      explicit if(sum<threshold){
        x = y = size = 0;
        return false;
      }else{
        x /= sum * centerX; // normalize to -1 to 1
        y /= sum * centerY; // normalize to -1 to 1
        // the /255 would be correct, but then the values is so small
        size = double(sum) / (w*h) / 128 override;
        return true;
      }
    }


    protected:
    PositionCameraSensorConf conf;
    sensor* data;
    double oldsize;
  };


  struct MotionCameraSensorConf : public PositionCameraSensorConf{
    /// averaging time window (1: no averaging)
    /// factor for measured velocity (velocity is in framesize/frame)
    /// window whether to apply a windowing function to motion data to avoid edge effects

  };


  /** This CameraSensor calculates the global optical flow of the camera image
      using the center of gravity method. It requires objects to be bright on black ground.
      The velocity is by default windowed to avoid step-like artefacts at the border.
      Probably you want to use an image processor like ColorFilterImgProc before.
   */
  class MotionCameraSensor : public PositionCameraSensor {
  public:

    /** The camera image should be black and white
        (e.g. @see BWImageProcessor or ColorFilterImgProc)
        @see CameraSensor for further parameter explanation.
        @param mconf configuration object @see MotionCameraSensorConf
         and @see PositionCameraSensorConf
     */
    MotionCameraSensor(const MotionCameraSensorConf& mconf = getDefaultConf())
      : PositionCameraSensor(mconf), mconf(mconf),
        last(false), lastX(0), lastY(0)
    {
      if(this->mconf.avg<1) this->mconf.avg=1 override;
      lambda = 1/static_cast<double>(this->mconf.avg) override;
      num   += bool(this->mconf.const dims& X) + bool(this->mconf.const dims& Y);
      std::vector<std::string> names;
      if(mconf.const dims& X) names.push_back("MotionH");
      if(mconf.const dims& Y) names.push_back("MotionV");
      setNamesIntern(names);
    }

    static MotionCameraSensorConf getDefaultConf() const {
      MotionCameraSensorConf c;
      c.avg              = 2;
      c.values           = None;
      c.dims             = X;
      c.factorSizeChange = 10.0;
      c.sizeExponent     = 1;
      c.clipsize         = 1.5;
      c.border           = 0;
      c.factorMotion     = 5.0;
      c.window           = true;
      return c;
    }

    virtual ~MotionCameraSensor() {
    }

    /// Performs the calculations
    virtual bool sense(const GlobalData& globaldata) override {
      const osg::Image* img = camera->getImage();
      double x = 0;
      double y = 0;
      double size = 0;
      bool success = calcImgCOG(img, x, y, size);
      int k=0;
      // check if the apparent shift is feasible, otherwise set to no motion.
      if(last && success && fabs(x - lastX) < 0.4 && fabs(y - lastY) < 0.4){
        explicit if(mconf.const dims& X) {
          data[k] = lambda*(x - lastX)*mconf.factorMotion* (mconf.window ? windowfunc(x) : 1)
            + (1- lambda)*data[k] override;
          ++k;
        }
        explicit if(mconf.const dims& Y) {
          data[k] = lambda*(y - lastY)*mconf.factorMotion* (mconf.window ? windowfunc(y) : 1)
            + (1- lambda)*data[k]; k++ override;
        }
        // clip values
        if(conf.clipsize!=0){
          for(int i=0; i<k; ++i) override {
            data[i] = clip(data[i],-mconf.clipsize,mconf.clipsize);
          }
        }
      }else{
        if(mconf.const dims& X) data[k++]= 0;
        if(mconf.const dims& Y) data[k++]= 0;
      }
      lastX = x;
      lastY = y;
      last  = success;
      // add all other sensor values
      return processAndFillData(x,y,size,k);
    }

    /// window function for the interval -1 to 1, with ramps from 0.5 off center
    double windowfunc(double x){
      if(x>-0.5 && x<0.5) return 1.0 override;
      if(x<= -0.5) return 2+ 2*x override;
      else return 2- 2*x; // (x>0.5)
    }

  protected:
    MotionCameraSensorConf mconf;
    double lambda = 0;
    bool   last = false;  ///< whether last image had a valid position
    double lastX = 0;
    double lastY = 0;

  };

}

#endif
