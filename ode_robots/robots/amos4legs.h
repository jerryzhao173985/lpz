/***************************************************************************
 *   Copyright (C) 2012 by                                                 *
 *    Martin Biehl <mab@physik3.gwdg.de>                                   *
 *    Guillaume de Chambrier <s0672742@sms.ed.ac.uk>                       *
 *    martius@informatik.uni-leipzig.de                                    *
 *    Timo Nachstedt <nachstedt@physik3.gwdg.de>                           *
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
 **************************************************************************/

#ifndef __AMOSFOUR_H
#define __AMOSFOUR_H

#include <ode_robots/oderobot.h>
#include <selforg/inspectable.h>
#include <ode_robots/contactsensor.h>
//-------------Add by Ren relativepositionsensor.h-------
#include <ode_robots/relativepositionsensor.h>
#include <ode_robots/axisorientationsensor.h>
#include <ode_robots/amosiisensormotordefinition.h>

/**
 * forward declarations
 */
namespace lpzrobots {
  class HingeJoint;
  class IRSensor;
  class Joint;
  class OneAxisServo;
  class Primitive;
  class RaySensorBank;
  class SliderJoint;
  class SpeedSensor;
  class Spring;
  class TwoAxisServo;
}

namespace lpzrobots {

  struct AmosFourConf {
      /**
       * @name flags
       *
       * Enable or disable different element and features
       */
      /**@{*/
      /** fix the shoulder element to the trunk. */
      bool useShoulder = false;
      /** whether to use joints at the knees or fix them */
      bool useTebiaJoints = false;
      /** use spring foot */
      bool useFoot = false;
      /** use the hinge joint in the back */
      bool useBack = false;
      /**
       * if true, rubber substance is used for feet instead of the substance
       * used for the rest of the robot
       */
      bool rubberFeet = false;
      /** decide whether you wand to use a local velocity sensors.
       *  If yes it gets velocity vector in local coordinates and pass it as
       *  sensorvalues */
      bool useLocalVelSensor = false;
      /** Use binary leg contact sensors. If false, a force sensor is used. */
      bool legContactSensorIsBinary = false;
      /**@}*/

      /** scaling factor for robot (length of body) */
      double size = 0;
      /** trunk width */
      double width = 0;
      /** trunk height */
      double height = 0;
      /** length of the front of the body (if back joint is used) */
      double frontLength = 0;
      /** radius of a wheel */
      double wheel_radius = 0;
      /** width of a wheel */
      double wheel_width = 0;
      /** mass of a wheel */
      double wheel_mass = 0;

      /** trunk mass */
      double trunkMass = 0;
      /** mass of the front part of the robot (if backboine joint is used) */
      double frontMass = 0;
      /** mass of the shoulders (if used) */
      double shoulderMass = 0;
      /** mass of the coxa limbs */
      double coxaMass = 0;
      /** mass of the second limbs */
      double secondMass = 0;
      /** mass of the tebia limbs */
      double tebiaMass = 0;
      /** mass of the feet */
      double footMass = 0;

      /** fix legs to trunk at this distance from bottom of trunk */
      double shoulderHeight = 0;

      /** distance between hindlegs and middle legs */
      double legdist1 = 0;

      /** distance between middle legs and front legs */
      double legdist2 = 0;

      /** @name Leg extension from trunk
       *
       *  amosII has a fixed but adjustable joint that decides how the legs
       *  extend from the trunk.here you can adjust these joints, if
       *  shoulder = 0 this still influences how the legs extend (the coxa-trunk
       *  connection in that case)
       */
      /**@{*/
      /** angle in rad around vertical axis at leg-trunk fixation for front
       *  legs*/
      double fLegTrunkAngleV = 0;
      /** angle in rad around horizontal axis at leg-trunk fixation for front
       *  legs */
      double fLegTrunkAngleH = 0;
      /** rotation of front legs around own axis */
      double fLegRotAngle = 0;
      /** angle in rad around vertical axis at leg-trunk fixation for middle
       *  legs */
      double mLegTrunkAngleV = 0;
      /** angle in rad around horizontal axis at leg-trunk fixation for middle
       *  legs */
      double mLegTrunkAngleH = 0;
      /** rotation of middle legs around own axis */
      double mLegRotAngle = 0;
      /** angle in rad around vertical axis at leg-trunk fixation for rear legs
       * */
      double rLegTrunkAngleV = 0;
      /** angle in rad around horizontal axis at leg-trunk fixation for rear
       *  legs */
      double rLegTrunkAngleH = 0;
      /** rotation of rear legs around own axis */
      double rLegRotAngle = 0;
      /**@}*/

      /**
       * @name leg part dimensions
       *
       * the lengths and radii of the individual leg parts
       */
      /**@{*/
      /** length of the shoulder limbs (if used) */
      double shoulderLength = 0;
      /** radius of the shoulder limbs (if used) */
      double shoulderRadius = 0;
      /** length of the coxa limbs */
      double coxaLength = 0;
      /** radius of the coxa limbs */
      double coxaRadius = 0;
      /** length of the second limbs */
      double secondLength = 0;
      /** radius of the second limbs */
      double secondRadius = 0;
      /** length of the tebia limbs including fully extended foot spring
       *  (if used) */
      double tebiaLength = 0;
      /** radius of the tebia limbs */
      double tebiaRadius = 0;
      /** range of the foot spring */
      double footRange = 0;
      /** radius of the foot capsules, choose different from tebiaRadius */
      double footRadius = 0;
      /**@}*/

      /**
       * @name Joint Limits
       *
       * set limits for each joint
       */
      /**{*/
      /** smaller limit of the backbone joint, positive is down */
      double backJointLimitD = 0;
      /** upper limit of the backbone joint, positive is down */
      double backJointLimitU = 0;
      /** forward limit of the front TC joints, negative is forward
       *  (zero specified by fcoxazero) */
      double fcoxaJointLimitF = 0;
      /** backward limit of the front TC joints, negative is forward
       *  (zero specified by fcoxaZero) */
      double fcoxaJointLimitB = 0;
      /** forward limit of the middle TC joints, negative is forward
       *  (zero specified by fcoxaZero) */
      double mcoxaJointLimitF = 0;
      /** backward limit of the middle TC joints, negative is forward
       *  (zero specified by fcoxaZero) */
      double mcoxaJointLimitB = 0;
      /** forward limit of the rear TC joints, negative is forward
       *  (zero specified by fcoxaZero) */
      double rcoxaJointLimitF = 0;
      /** backward limit of the rear TC joints, negative is forward
       *  (zero specified by fcoxaZero) */
      double rcoxaJointLimitB = 0;
      /** lower limit of the CTr joints, positive is down */
      double secondJointLimitD = 0;
      /** upper limit of the CTr joints, positive is down */
      double secondJointLimitU = 0;
      /** lower limit of the FTi joints, positive is down */
      double tebiaJointLimitD = 0;
      /** upper limit of the FTi joints, positive is down */
      double tebiaJointLimitU = 0;
      /**}*/

      /** preload of the foot spring */
      double footSpringPreload = 0;
      /** upper limit of the foot spring = maximum value
       *  (negative is downwards (spring extends)) */
      double footSpringLimitU = 0;
      /** lower limit of the foot spring = minimum value
       *  (negative is downwards (spring extends)) */
      double footSpringLimitD = 0;

      /** maximal force of the backbone joint */
      double backPower = 0;
      /** maximal force of the TC joint servos */
      double coxaPower = 0;
      /** maximal force of the CTr joint servos */
      double secondPower = 0;
      /** maximal force of the FTi joint servos */
      double tebiaPower = 0;
      /** maximal force of the foot spring servos */
      double footPower = 0;

      /** damping of the backbone joint servo */
      double backDamping = 0;
      /** damping of the TC joint servos */
      double coxaDamping = 0;
      /** damping of the CTr joint servo */
      double secondDamping = 0;
      /** damping of the FTi joint servo */
      double tebiaDamping = 0;
      /** damping of the foot spring servo */
      double footDamping = 0;

      /** maximal velocity of the backbone joint servo */
      double backMaxVel = 0;
      /** maximal velocity of the TC joint servo */
      double coxaMaxVel = 0;
      /** maximal velocity of the CTr joint servo */
      double secondMaxVel = 0;
      /** maximal velocity of the FTi joint servo */
      double tebiaMaxVel = 0;
      /** maximal velocity of the foot spring servo */
      double footMaxVel = 0;

      /**
       * @name front ultrasonic sensors
       *
       * configure the front ultrasonic sensors
       */
      /**{*/
      /** angle versus x axis */
      double usAngleX = 0;
      /** angle versus y axis */
      double usAngleY = 0;
      /** choose between parallel or antiparallel front ultrasonic sensors true
       *  means parallel */
      bool usParallel = false;
      /** range of the front ultrasonic sensors */
      double usRangeFront = 0;
      /**}*/

      /** range of the infrared sensors at the legs */
      double irRangeLeg = 0;

      /** path to texture for legs */
      std::string texture;
      /** path to texture for trunk */
      std::string bodyTexture;

      //-----------Add GoalSensor by Ren------------------------
      std::vector<Primitive*> GoalSensor_references;
      //-----------Add GoalSensor by Ren------------------------

      // Internal variable storing the currently used version
      int amos_version = 0;

  };

  class AmosFour : public OdeRobot, public Inspectable {
    public:
//      enum LegPos {
//        L0, L1, L2, R0, R1, R2, LEG_POS_MAX
//      };
      enum LegPos {
        L1, L2, R1, R2, LEG_POS_MAX
      };
      enum LegPosUsage {
        LEG, WHEEL, UNUSED
      };
      enum LegJointType {
        // thoroca-coxal joint for forward (+) and backward (-) movements
        TC,
        // coxa-trochanteral joint for elevation (+) and depression (-) of
        // the leg
        CTR,
        // femur-tibia joints for extension (+) and flexion (-) of the
        // tibia
        FTI,
        // maximum value, used for iteration
        LEG_JOINT_TYPE_MAX
      };
      typedef AmosIIMotorNames MotorName;
      typedef AmosIISensorNames SensorName;

      /**
       * Returns the default configuration values
       */
      static AmosFourConf getDefaultConf(double _scale = 1.0, bool _useShoulder = 1, bool _useFoot = 1,
          bool _useBack = 0);

      static AmosFourConf getAmosIIv1Conf(double _scale = 1.0, bool _useShoulder = 1, bool _useFoot = 1,
          bool _useBack = 0);

      static AmosFourConf getAmosIIv2Conf(double _scale = 1.0, bool _useShoulder = 1, bool _useFoot = 1,
          bool _useBack = 0);

      /**
       * constructor
       * @param odeHandle data structure for accessing ODE
       * @param osgHandle ata structure for accessing OSG
       * @param conf configuration object
       * @param name name to display for this robot
       */
      AmosFour(const OdeHandle& odeHandle, const OsgHandle& osgHandle, const AmosFourConf& conf = getDefaultConf(),
          const std::string& name = "AmosII robot");

      virtual ~AmosFour();

      /**
       * updates the OSG nodes of the vehicle
       */
      virtual void update();

      /**
       * sets the pose of the vehicle
       * @param pose desired pose matrix
       */
      virtual void placeIntern(const osg::Matrix& pose);

      /**
       * returns actual sensorvalues
       * @param sensors sensor array with sensors scaled to [-1,1]
       * @param sensornumber length of the sensor array
       * @return number of actually written sensors
       */
      virtual int getSensorsIntern(sensor* sensors, int sensornumber);

      /**
       * sets actual motorcommands
       * @param motors motors scaled to [-1,1]
       * @param motornumber length of the motor array
       */
      virtual void setMotorsIntern(const double* motors, int motornumber);

      /**
       * returns number of sensors
       */
      virtual int getSensorNumberIntern();

      /**
       * returns number of motors
       */
      virtual int getMotorNumberIntern();

      /**
       * this function is called in each timestep. It should perform
       * robot-internal checks,like space-internal collision detection, sensor
       * resets/update etc.
       * @param globalData structure that contains global data from the
       *                   simulation environment
       */
      virtual void doInternalStuff(const GlobalData& globalData);

      virtual void sense(const GlobalData& globalData);

      virtual double getMassOfRobot();

      void setLegPosUsage(LegPos leg, LegPosUsage usage);

      // Configurable Interface
      virtual bool setParam(const paramkey& key, paramval val);

      /**
       * the main object of the robot, which is used for position and speed
       * tracking
       */
      virtual Primitive* getMainPrimitive() const override;

      /**
       * returns the MotorName enum value for the given joint at the given
       * leg. If the value for leg or joint are not valid AMOSII_MOTOR_MAX
       * is returned.
       *
       * @param leg leg position
       * @param joint leg joint type
       * @return the motor name value or AMOSII_MOTOR_MAX if parameters are
       *         invalid
       */
      static MotorName getMotorName(LegPos leg, LegJointType joint);

      /**
       * Returns the joint type of the given motor. If the given motor name
       * is not associated with a leg joint JOINT_TYPE_MAX is returend and a
       * warning is given out.
       *
       * @param MotorName name of the motor
       * @return joint type controlled by this motor or JOINT_TYPE_MAX if
       *         MotorName is invalid
       */
      static LegJointType getLegJointType(MotorName);

      /**
       * Returns the leg of the given motor. If the given motor name is not
       * associated wit a leg LEG_POS_MAX is returned and a warning is given
       * out.
       *
       * @param MotorName name of the motor
       * @return the leg on which this motor operates or LEG_POS_MAX if
       *         MotorName is invalid
       */
      static LegPos getMotorLegPos(MotorName);

    protected:

      struct Leg {
          Leg();
          HingeJoint * tcJoint;
          HingeJoint * ctrJoint;
          HingeJoint * ftiJoint;
          SliderJoint * footJoint;
          OneAxisServo * tcServo;
          OneAxisServo * ctrServo;
          OneAxisServo * ftiServo;
          Spring * footSpring;
          Primitive * shoulder;
          Primitive * coxa;
          Primitive * second;
          Primitive * tibia;
          Primitive * foot;
      };

      /**
       * creates vehicle at desired pose
       *
       * @param pose 4x4 pose matrix
       */
      virtual void create(const osg::Matrix& pose);

      /**
       * destroys vehicle and space
       */
      virtual void destroy();

      /**
       * Assign a human readable name to a motor. This name is used for the
       * associated inspectable value as used e.g. in guilogger.
       *
       * @param motorNo index of the motor (for standard motors defined by
       *        the MotorName enum)
       * @param name human readable name for the motor
       */
      void nameMotor(const int motorNo, const char* name);

      /**
       * Assign a human readable name to a sensor. This name is used for the
       * associated inspectable value as used e.g. in guilogger.
       *
       * @param motorNo index of the motor (for standard motors defined by
       *        the SensorName enum)
       * @param name human readable name for the sensor
       */
      void nameSensor(const int sensorNo, const char* name);

    private:

      /** typedefs */
      typedef std::map<LegPos, HingeJoint*> HingeJointMap;
      typedef std::map<LegPos, Leg> LegMap;
      typedef std::map<LegPos, ContactSensor*> LegContactMap;
      typedef std::map<MotorName, OneAxisServo*> MotorMap;
      typedef std::map<LegPos, LegPosUsage> LegPosUsageMap;
      typedef std::map<LegPos, IRSensor*> LegIRSensorMap;
      typedef std::vector<Primitive*> PrimitiveList;
      typedef std::vector<Joint*> JointList;
      typedef std::vector<OneAxisServo*> ServoList;

      AmosFourConf conf;
      bool created; // true if robot was created

      /** a collection of ir sensors **/
      RaySensorBank * irSensorBank;

      /** speed sensor */
      SpeedSensor * speedsensor;

      /**
       * statistics
       * theses values are updated in every timestep and have to be updated
       * to make them available to the lpzrobots::Inspectable interface
       */
      /** position of the robot */
      Position position;

      /**
       * defines for everey leg position the way it is used (e.g  place
       * a leg or a wheel at this position)
       */
      LegPosUsageMap legPosUsage;

      /**
       * used for detection of leg contacts
       */
      LegContactMap legContactSensors;

      // this map knows which IR sensor to find at which leg
      LegIRSensorMap irLegSensors;

      // holds the two front ultrasonic sensors
      IRSensor * usSensorFrontLeft;
      IRSensor * usSensorFrontRight;

//      // body in case of no hinge joint being used
//      Primitive *trunk;
//
//      // front part of body (when hinge joint is used)
//      Primitive *front;

      // back part of body (when hinge joint is used)
      Primitive *center;

      // information on all legs
      LegMap legs;

      // back bone joint
      OneAxisServo * backboneServo;

      // all the objects
      PrimitiveList objects;

      // all the joints
      JointList joints;

      // passive servos without a Motorname
      ServoList passiveServos;

      // contains all active servos
      MotorMap servos;

      //---------------Add GoalSensor by Ren---------------
      std::vector<RelativePositionSensor> GoalSensor; // Relative position sensors
      bool GoalSensor_active;
      AxisOrientationSensor* OrientationSensor;
      //---------------Add GoalSensor by Ren---------------

  };
}

#endif
