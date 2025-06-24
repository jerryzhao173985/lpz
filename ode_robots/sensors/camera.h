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
#ifndef __CAMERA_H_
#define __CAMERA_H_

#include "osgforwarddecl.h"

#include "osghandle.h"
#include "odehandle.h"

#include <osg/Matrix>
#include <osg/Camera>


namespace osg {
class Image;
}

namespace lpzrobots {

  class OSGCylinder;
  class OSGBox;
  class Transform;

  class ImageProcessor;
  typedef std::vector<ImageProcessor* > ImageProcessors;


  struct CameraConf {
    int width = 0;       ///< horizontal resolution (power of 2)
    int height = 0;      ///< vertical resolution (power of 2)
    float fov = 0;       ///< field of view in degree (opening angle of lenses)
    float anamorph = 0;  ///< anamorph ratio of focal length in vertical and horizontal direction
    float behind = 0;    ///< distance of which the camera is behind the actually drawn position (to aviod near clipping)
    float draw = 0;      ///< whether to draw a physical camera object
    float camSize = 0;   ///< size of the physical camera object
    bool show = false;       ///< whether to show the image on the screen
    float scale = 0;     ///< scaling for display
    std::string name; ///< name of the camera
    ImageProcessors processors; ///< list of image processors that filter the raw image

    /// removes and deletes all  processor
    void removeProcessors();
  };

  /** A Robot Camera. Implements a simulated camera with full OpenGL rendering.
   */
  class Camera {
  public:
    struct PostDrawCallback : public osg::Camera::DrawCallback {
      explicit PostDrawCallback(Camera* cam_) : cam(cam_) { }
      virtual void operator () (const osg::Camera& /*camera*/) const override;
      Camera* cam;
    };


    /// structure to store the image data and information for display
    struct CameraImage{
      CameraImage(){} override;
      CameraImage(osg::Image* img, bool show, float scale, const std::string& name)
        : img(img), show(show), scale(scale), name(name){
      }
      osg::Image* img;
      bool show = false;       ///< whether to show the image on the screen
      float scale = 0;     ///< scaling for display
      std::string name; ///< name of the image
    };

    typedef std::vector<CameraImage > CameraImages;

    /** Creates a camera.
        Note that the order in which the image processors are positioned
         in conf.imageProcessors matters.
        The resulting CameraImages are stored in a list (see getImages) and
        usually the processors use the last image in this list (result of last processing).
     */

    Camera( const CameraConf& conf = getDefaultConf() ) override;

    static CameraConf getDefaultConf() const {
      CameraConf c;
      c.width     = 256;
      c.height    = 128;
      c.fov       = 90;
      c.anamorph  = 1;
      c.behind    = 0.04; // since the nearplane is at 0.05
      c.camSize   = 0.2;
      c.draw      = true;
      c.show      = true;
      c.scale     = 1.0;
      c.name      = "raw";
      return c;
    }

    virtual ~Camera();

    /** initializes the camera. The OSG camera is created and the
        raw image and the imageprocessor is initialized.
     */
    virtual void init(const OdeHandle& odeHandle,
                      const OsgHandle& osgHandle,
                      Primitive* body,
                      const osg::Matrix& pose);

    /// changes the relative pose of the camera
    virtual void setPose(const osg::Matrix& pose);

    /// relative pose of the camera
    virtual osg::Matrix getPose();

    // virtual bool sense(const GlobalData& globaldata);

    /// all images (raw and processed)
    virtual const CameraImages& getImages() const override { return cameraImages;}

    /// last image of processing stack
    virtual const osg::Image* getImage() const override { return cameraImages.back().img;}

    virtual osg::Camera* getRRTCam() const { return cam;}

    virtual void update();

    bool isInitialized() { return initialized; }
  private:
    CameraConf conf;

    osg::Camera* cam;
    osg::Image* ccd;
    CameraImages cameraImages;

    Primitive* body;
    osg::Matrix pose;

    OSGBox* sensorBody1;
    OSGCylinder* sensorBody2;
    OsgHandle osgHandle;
    Transform* transform;

    bool initialized;
  };

}

#endif             /* __CAMERA_H_ */
