#ifndef __RANGEFINDER_H
#define __RANGEFINDER_H

#include "raysensorbank.h"
#include "irsensor.h"

#include <selforg/types.h>
#include <selforg/stl_adds.h>

#include <list>

namespace lpzrobots {

  /**
   * Class representing a range finder. It consists of several ray sensors.
   */
  class RangeFinder : public RaySensorBank {
  public:

    RangeFinder() : own(nullptr) {};

    virtual void init(Primitive* own, Joint* joint = 0);

    /**
     * Register equidistant IR sensor in a given angular range
     * @param numBeams: number of beams of the range finder
     * @param startAngle: angle of the leftmost beam
     * @param endAngle: angle of the rightmost beam
     * @param maxRange: maximum range of the raysensors
     * @param height: At which height to attach the range finder (0 will attach it to the center of gravity of own)
     * @param drawMode: Specifies if rays should be drawn or not (default is drawing rays)
     */
    void registerSensorRange(int numBeams, double startAngle, double endAngle, double maxRange, double height,
        RaySensor::rayDrawMode drawMode = RaySensor::drawRay);

  protected:
    Primitive* own;
  };

}

#endif
