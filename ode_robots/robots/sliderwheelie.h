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
#ifndef __SLIDERWHEELIE_H
#define __SLIDERWHEELIE_H

#include<vector>
#include <cassert>

#include "oderobot.h"
#include"primitive.h"
#include "joint.h"
#include "angularmotor.h"

#include "oneaxisservo.h"


namespace lpzrobots {

  typedef struct {
  public:
    /* typedef */ enum MotorType {Servo, CenteredServo, AngularMotor };

    MotorType motorType;  ///< whether to use servos or angular motors
    int segmNumber = 0;     //  number of snake elements
    double segmLength = 0.0; // length of one snake element
    double segmDia = 0.0;    //  diameter of a snake element
    double segmMass = 0.0;   //  mass of one snake element
    double motorPower = 0.0; //  power of the servos
    double motorDamp = 0.0;  //  damping of servos
    double powerRatio = 0.0; //  power of the servos
    double sensorFactor = 0.0;  //  scale for sensors
    double frictionGround = 0.0; // friction with ground
    double frictionJoint = 0.0;  // friction within joint
    double jointLimitIn = 0.0;   
    double jointLimitOut = 0.0;
    double sliderLength = 0.0;
    bool showCenter = false;
    std::string texture;  ///< texture for segments
  } SliderWheelieConf;


  /**
   * This is a class, which models an annular robot.
   * It consists of a number of equal elements, each linked
   * by a joint powered by 1 servo
   **/
  class SliderWheelie : public OdeRobot {
  private:

 std::vector <AngularMotor*> angularMotors;
    SliderWheelieConf conf;

    std::vector <HingeServo*> hingeServos;
    std::vector <SliderServo*> sliderServos;

    Primitive* center = nullptr; // virtual center object (position updated on setMotors)
    DummyPrimitive* dummycenter = nullptr; // virtual center object (here we can also update velocity)
  public:
    SliderWheelie(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                  const SliderWheelieConf& conf, const std::string& name,
                  const std::string& revision = "");

    virtual ~SliderWheelie();

    static SliderWheelieConf getDefaultConf() {
      SliderWheelieConf conf;
      conf.segmNumber = 8;       //  number of snake elements
      conf.segmLength = 0.4;     // length of one snake element
      conf.segmDia    = 0.2;     //  diameter of a snake element
      conf.segmMass   = 0.4;     //  mass of one snake element
      conf.motorPower = 5;       //  power of the servos
      conf.motorDamp  = 0.01;    //  damping of servos
      conf.powerRatio = 2;       //  power of the servos
      conf.sensorFactor    = 1;   //  scale for sensors
      conf.frictionGround  = 0.8; // friction with ground
      conf.frictionJoint   = 0.0; // friction within joint
      conf.jointLimitIn    =  M_PI/3;
      conf.jointLimitOut   =  -1; // automatically set to 2*M_PI/segm_num
      conf.sliderLength    =  1;
      conf.motorType       = SliderWheelieConf::CenteredServo; // use centered servos
      conf.showCenter      = false;
      conf.texture         = "";
      return conf;
    }

    virtual void placeIntern(const osg::Matrix& pose) override;

    virtual void update() override;

    virtual void doInternalStuff(const GlobalData& global) override;

    virtual void setMotorsIntern( const double* motors, int motornumber ) override;

    virtual int getSensorsIntern( sensor* sensors, int sensornumber ) override;

    virtual int getSensorNumberIntern() const override { assert(created);
      return hingeServos.size()+angularMotors.size()+sliderServos.size(); }

    virtual int getMotorNumberIntern() const override { assert(created);
      return hingeServos.size()+angularMotors.size()+sliderServos.size(); }

    virtual Primitive* getMainPrimitive() const override {
      if(dummycenter) return dummycenter;
      else if(!objects.empty()){
        return (objects[0]);
      }else return 0;
    }

    virtual std::vector<Primitive*> getAllPrimitives() const override { return objects;}

    /******** CONFIGURABLE ***********/
    virtual void notifyOnChange(const paramkey& key);

  private:
    bool created = false;
    std::vector<Primitive*> objects;  // for compatibility with getAllPrimitives
    
    static void mycallback(void *data, dGeomID o1, dGeomID o2);

    virtual void create(const osg::Matrix& pose);
    virtual void destroy();
  };

}

#endif
