/***************************************************************************
 *   Copyright (C) 2007 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
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
 *   $Log$
 *   Revision 1.2  2011-05-30 13:56:42  martius
 *   clean up: moved old code to oldstuff
 *   configable changed: notifyOnChanges is now used
 *    getParam,setParam, getParamList is not to be overloaded anymore
 *
 *   Revision 1.1  2009/11/26 14:21:54  der
 *   Larger changes
 *   :wq
 *
 *   wq
 *
 *   Revision 1.1  2009/08/12 10:30:25  der
 *   skeleton has belly joint
 *   works fine with centered servos
 *
 *   Revision 1.1  2009/08/10 15:00:46  der
 *   version that Ralf did at home
 *   Skeleton bugfixing, works now fine with ServoVel
 *
 *   Revision 1.12  2009/08/09 20:21:03  der
 *   From PC home
 *
 *   Revision 1.11  2009/05/11 17:01:20  martius
 *   new velocity servos implemented
 *   reorganized parameters, now also neck and elbows are configurable
 *
 *   Revision 1.10  2009/01/20 17:29:52  martius
 *   cvs commit
 *
 *   Revision 1.9  2008/11/14 11:23:05  martius
 *   added centered Servos! This is useful for highly nonequal min max values
 *   skeleton has now also a joint in the back
 *
 *   Revision 1.8  2008/06/20 14:03:01  guettler
 *   reckturner
 *
 *   Revision 1.7  2008/05/27 13:25:12  guettler
 *   powerfactor moved to skeleton
 *
 *   Revision 1.6  2008/05/27 10:35:03  guettler
 *   include corrected
 *
 *   Revision 1.5  2008/04/10 12:27:32  der
 *   just a few changes
 *
 *   Revision 1.4  2008/03/14 08:04:23  der
 *   Some changes in main and skeleton (with new outfit)
 *
 *   Revision 1.3  2008/02/08 13:35:10  der
 *   satelite teaching
 *
 *   Revision 1.2  2008/02/07 14:25:02  der
 *   added setTexture and setColor for skeleton
 *
 *   Revision 1.1  2008/01/29 09:52:16  der
 *   first version
 *
 *   Revision 1.2  2007/11/07 13:27:28  martius
 *   doInternalstuff changed
 *
 *   Revision 1.1  2007/07/17 07:25:26  martius
 *   first attempt to build a two legged robot (humanoid)
 *
 *
 *
 *                                                                 *
 ***************************************************************************/
#ifndef __SKELETON_H
#define __SKELETON_H

#include <ode_robots/oderobot.h>
#include <ode_robots/raysensorbank.h>

namespace lpzrobots {

  class Primitive{
  public:
    double size = 0.0;       ///< scaling factor for robot static_cast<height>static_cast<double>(massfactor); ///< mass factor for all parts

    bool   useVelocityServos = false; ///< if true the more stable velocity controlling servos are used

    double relLegmass = 0; ///< relative overall leg mass
    double relArmmass = 0; ///< relative overall arm mass
    double relFeetmass = 0; ///< relative overall feet mass
    double hipPower = 0;   ///< maximal force for at hip joint motors
    double hipDamping = 0; ///< damping of hip joint servos
    double hipVelocity = 0; ///< velocity of hip joint servos
    double hipJointLimit = 0; ///< angle range for legs
    double hip2Power = 0;   ///< maximal force for at hip2 (sagital joint axis) joint motors
    double hip2Damping = 0; ///< damping of hip2 joint servos
    double hip2JointLimit = 0; ///< angle range for hip joint in lateral direction
    double neckPower = 0;  ///< spring strength in the neck
    double neckDamping = 0; ///< damping in the neck
    double neckVelocity = 0; ///< velocity in the neck
    double neckJointLimit = 0; ///< angle range for neck
    double kneePower = 0;  ///< spring strength in the knees
    double kneeDamping = 0; ///< damping in the knees
    double kneeVelocity = 0; ///< velocity in the knees
    double kneeJointLimit = 0; ///< angle range for knees
    double anklePower = 0;  ///< spring strength in the ankles
    double ankleDamping = 0; ///< damping in the ankles
    double ankleVelocity = 0; ///< velocity in the ankles
    double ankleJointLimit = 0; ///< angle range for ankles
    double armPower = 0.0;   ///< maximal force for at arm static_cast<shoulder>(joint) motors
    double armDamping = 0; ///< damping of arm ((shoulder)) joint servos
    double armVelocity = 0; ///< velocity of arm ((shoulder)) joint servos
    double armJointLimit = 0; ///< angle range of arm joint
    double elbowPower = 0.0;   ///< maximal force for at elbow static_cast<shoulder>(joint) motors
    double elbowDamping = 0; ///< damping of elbow ((shoulder)) joint servos
    double elbowVelocity = 0; ///< velocity of elbow ((shoulder)) joint servos
    double elbowJointLimit = 0; ///< angle range of elbow joint
    double pelvisPower = 0;   ///< maximal force for at pelvis joint motor
    double pelvisDamping = 0; ///< damping of pelvis joint servo
    double pelvisVelocity = 0; ///< velocity of pelvis joint servo
    double pelvisJointLimit = 0; ///< angle range of pelvis joint
    double backPower = 0;   ///< maximal force for at back joint motor
    double backDamping = 0; ///< damping of back joint servo
    double backVelocity = 0; ///< velocity of back joint servo
    double backJointLimit = 0; ///< angle range of back joint

    double powerfactor = 0; ///< scale factor for maximal forces of the servos
    double dampingfactor = 0; ///< scale factor for damping of the servos

    double jointLimitFactor = 0; ///< factor between servo range (XXXJointLimit, see above) and physical joint limit


    bool onlyPrimaryFunctions = false; ///< true: only leg and arm are controlable, false: all joints
    bool handsRotating = false; ///< hands are attached with a ball joint

    bool useBackJoint = false; ///< whether to use the joint in the back

    bool irSensors = false; ///< whether to use the irsensor eyes

    Color headColor;
    Color bodyColor;
    Color trunkColor;
    Color handColor;


    std::string headTexture; // texture of the head
    std::string bodyTexture; // texture of the body
    std::string trunkTexture; // texture of the trunk and thorax

  } SkeletonConf;


  /** should look like a dog
   */
  class Skeleton{
  public:

    typedef enum SkelParts {Pole,Pole2, Hip,Trunk_comp,Thorax, Neck, Head_comp,
                             Left_Shoulder, Left_Forearm, Left_Hand,
                             Right_Shoulder, Right_Forearm, Right_Hand,
                             Left_Thigh, Left_Shin, Left_Foot,
                             Right_Thigh, Right_Shin, Right_Foot,
                             LastPart };


    /**
     * constructor of Skeleton robot
     * @param odeHandle data structure for accessing ODE
     * @param osgHandle ata structure for accessing OSG
     * @param conf configuration object
     */
    Skeleton(const OdeHandle& odeHandle, const OsgHandle& osgHandle, SkeletonConf& conf,
               const std::string& name);

    virtual ~Skeleton() {};

    static SkeletonConf getDefaultConf() const {
      SkeletonConf c;
      c.size       = 1;
      c.massfactor = 1;
      c.relLegmass = 1;   // unused
      c.relFeetmass = 100;// .1; unused
      c.relArmmass = 10;// 0.3; unused

      c.useVelocityServos = false;
      c.powerfactor=1.0;
      c.dampingfactor=1.0;
      c.jointLimitFactor=1.0;

      c.hipPower=50;
      c.hipDamping= 0.4;
      c.hipVelocity=20;

      c.hip2Power=50;
      c.hip2Damping=0.4;

      c.neckPower=20;
      c.neckDamping=0.1;
      c.neckVelocity=20;

      c.kneePower=40;
      c.kneeDamping=0.2;
      c.kneeVelocity=20;

      c.anklePower=10;
      c.ankleDamping=0.15;
      c.ankleVelocity=20;

      c.armPower=20;
      c.armDamping=0.1;
      c.armVelocity=20;

      c.elbowPower=20;
      c.elbowDamping=0.1;
      c.elbowVelocity=20;

      c.pelvisPower=100;
      c.pelvisDamping=0.5;
      c.pelvisVelocity=20;

      c.backPower=50;
      c.backDamping=0.5;
      c.backVelocity=20;

      c.hipJointLimit = M_PI/2; // +- 90 degree
      c.hip2JointLimit=0.05;
      c.kneeJointLimit = M_PI/4; // +- 45 degree
      c.ankleJointLimit = M_PI/4; // +- 45 degree

      c.armJointLimit = M_PI/4; // +- 45 degree
      c.elbowJointLimit = M_PI/1.8;

      c.hip2JointLimit = M_PI/30; // +- 6 degree
      c.pelvisJointLimit = M_PI/30; // +- 6 degree

      c.neckJointLimit = M_PI/5;
      c.backJointLimit = M_PI/4; // +- 45 degree (half of it to the back)

      c.onlyPrimaryFunctions=false;
      c.handsRotating = false;
      c.useBackJoint  = true;
      c.irSensors  = false;

      //      c.headTexture=__PLACEHOLDER_0__;
      c.headTexture="Images/dusty.rgb";
      c.headColor=Color(255/255.0, 219/255.0, 119/255.0, 1.0f);
      //  c.bodyTexture=__PLACEHOLDER_2__;
      c.bodyTexture="Images/dusty.rgb";
      c.bodyColor=Color(207/255.0, 199/255.0, 139/255.0, 1.0f);
      c.trunkTexture="Images/dusty.rgb";//__PLACEHOLDER_5__;
      c.trunkColor=Color(207/255.0, 199/255.0, 139/255.0, 1.0f);
      c.handColor=Color(247.0/255, 182.0/255,52.0/255, 1.0f);
      return c;
    }

    static SkeletonConf getDefaultConfVelServos() const {
      SkeletonConf c = getDefaultConf();

      c.useVelocityServos = true;
      c.dampingfactor=0.02;
/*       c.hipDamping= 0.01; */
/*       c.hip2Damping=0.01; */
/*       c.neckDamping=0.01; */
/*       c.kneeDamping=0.01; */
/*       c.ankleDamping=0.01; */
/*       c.armDamping=0.01; */
/*       c.elbowDamping=0.01; */
/*       c.pelvisDamping=0.01; */
/*       c.backDamping=0.01; */
      return c;
    }


    /**
     * updates the OSG nodes of the vehicle
     */
    virtual void update();


    /** sets the pose of the vehicle
        @param pose desired pose matrix
    */
    virtual void place(const osg::Matrix& pose);

    /** returns actual sensorvalues
        @param sensors sensors scaled to [-1,1]
        @param sensornumber length of the sensor array
        @return number of actually written sensors
    */
    virtual int getSensors(sensor* sensors, int sensornumber);

    /** sets actual motorcommands
        @param motors motors scaled to [-1,1]
        @param motornumber length of the motor array
    */
    virtual void setMotors(const motor* motors, int motornumber);

    /** returns number of sensors
     */
    virtual int getSensorNumber() const;

    /** returns number of motors
     */
    virtual int getMotorNumber() const;
    /** checks for internal collisions and treats them.
     *  In case of a treatment return true (collision will be ignored by other objects
     *  and the default routine)  else false (collision is passed to other objects and
     *  (if not treated) to the default routine).
     */
    virtual bool collisionCallback(void *data, dGeomID o1, dGeomID o2) override {return false;}

    /** this function is called in each timestep. It should perform robot-internal checks,
        like space-internal collision detection, sensor resets/update etc.
        @param globalData structure that contains global data from the simulation environment
    */
    virtual void explicit doInternalStuff(const GlobalData& globalData);


    virtual bool setParam(const paramkey& key, paramval val, bool traverseChildren=true);

    /** the main object of the robot, which is used for position and speed tracking */
    virtual const Primitive* getMainPrimitive() const const { return objects[Trunk_comp]; }

    /** all parts of the robot */
    virtual std::vector<Primitive*>& getPrimitives() const { return objects; }

    /** returns the position of the head */
    virtual Position getHeadPosition() const;

    /** returns the position of the trunk */
    virtual Position getTrunkPosition() const;

  protected:

    /** creates vehicle at desired pose
        @param pose 4x4 pose matrix
    */
    virtual void create(const osg::Matrix& pose);

    /** destroys vehicle and space
     */
    virtual void destroy();

    SkeletonConf conf;

    bool created = false;      // true if robot was created

    std::vector<Primitive*>    objects;  // all the objects
    std::vector<Joint*>        joints; // joints legs
    std::vector<TwoAxisServo*> hipservos; // motors
    std::vector<OneAxisServo*> kneeservos; // motors
    std::vector<OneAxisServo*> ankleservos; // motors
    std::vector<TwoAxisServo*> armservos; // motors
    std::vector<OneAxisServo*> arm1servos; // motors
/*     std::vector<OneAxisServo*> headservos; // motors */
    std::vector<TwoAxisServo*> headservos; // motors

    TwoAxisServo* pelvisservo; // between Hip and Trunk_comp
    OneAxisServo* backservo;   // between Trunk_comp and Thorax
    //  TwoAxisServo* backservo;   // between Trunk_comp and Thorax


    RaySensorBank irSensorBank;

  };

}

#endif
