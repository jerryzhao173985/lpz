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
#ifndef __SKELETON_H
#define __SKELETON_H

#include <ode_robots/oderobot.h>
#include <ode_robots/gripper.h>

namespace lpzrobots {

  class Primitive{
  public:
    double size = 0.0;       ///< scaling factor for robot static_cast<height>static_cast<double>(massfactor); ///< mass factor for all parts
    bool   useDensity = false; ///< massfactor is interpreted as a density

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

    double powerFactor = 0; ///< scale factor for maximal forces of the servos
    double relForce = 0;    ///< factor between arm force and rest
    double dampingFactor = 0; ///< scale factor for damping of the servos

    double jointLimitFactor = 0; ///< factor between servo range (XXXJointLimit, see above) and physical joint limit


    bool onlyPrimaryFunctions = false; ///< true: only leg and arm are controlable, false: all joints
    bool onlyMainParameters = false; ///< true: only a few parameters are exported for configuration

    bool handsRotating = false; ///< hands are attached with a ball joint
    bool useGripper = false;        ///< hands have a gripper: use getGrippers and add grippables
    double gripDuration = 0;    ///< time the gripper can grasp
    double releaseDuration = 0; ///< time the gripper has to release before grasping again

    bool movableHead = false;   ///< if false then no neck movement
    bool useBackJoint = false;  ///< whether to use the joint in the back
    bool backSideBend = false;  ///< whether to use a joint to bend the back sideways


    bool irSensors = false; ///< whether to use the irsensor eyes

    std::string headColor;   ///< special color of head (typically like body)
    std::string bodyColor;   ///< color of skin
    std::string trunkColor;  ///< color of upper body (pullover-color)
    std::string trouserColor;///< color of upper legs and hands (trousers-color)


    std::string headTexture; // texture of the head
    std::string bodyTexture; // texture of the body
    std::string trunkTexture; // texture of the trunk and thorax


  } SkeletonConf;


  /** should look like a humanoid
   */
  class Skeleton{
  public:

    enum SkelParts {Hip,Trunk_comp, Belly, Thorax, Neck,
                    Head_comp,
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

    virtual ~Skeleton() { destroy(); };

    static SkeletonConf getDefaultConf() const {
      SkeletonConf c;
      c.size        = 1;
      c.massfactor  = 1;
      c.relLegmass  = 1;        // unused
      c.relFeetmass = 1;        // .1; unused
      c.relArmmass  = 1;        // 0.3; unused
      c.useDensity  = false;

      c.useVelocityServos = true;
      c.powerFactor       = 1.0;
      c.relForce          = 1.0;
      c.dampingFactor     = 1.0;
      c.jointLimitFactor  = 1.1; // factor between servo range and physical limit

      c.hipPower    = 20;
      c.hipDamping  = 0.2;
      c.hipVelocity = 20;

      c.hip2Power   = 20;
      c.hip2Damping = 0.2;

      c.neckPower    = 2;
      c.neckDamping  = 0.1;
      c.neckVelocity = 20;

      c.kneePower    = 10;
      c.kneeDamping  = 0.1;
      c.kneeVelocity = 20;

      c.anklePower    = 3;
      c.ankleDamping  = 0.1;
      c.ankleVelocity = 20;

      c.armPower    = 8;
      c.armDamping  = 0.1;
      c.armVelocity = 20;

      c.elbowPower    = 4;
      c.elbowDamping  = 0.1;
      c.elbowVelocity = 20;

      c.pelvisPower    = 20;
      c.pelvisDamping  = 0.2;
      c.pelvisVelocity = 20;

      c.backPower    = 20;
      c.backDamping  = 0.1;
      c.backVelocity = 20;

      c.hipJointLimit  = 2.1;   //1.6;
      c.hip2JointLimit = .8;    //

      c.kneeJointLimit  = 2;    // 2.8; // + 300, -20 degree
      c.ankleJointLimit = M_PI/2; // - 90 + 45 degree

      c.armJointLimit   = M_PI/2; // +- 90 degree
      c.elbowJointLimit = 1.4;

      c.pelvisJointLimit = M_PI/10; // +- 18 degree

      c.neckJointLimit = M_PI/5;
      c.backJointLimit = M_PI/3;//4// // +- 60 degree (half of it to the back)

      c.onlyMainParameters   = false;
      c.onlyPrimaryFunctions = false;
      c.handsRotating        = false;
      c.movableHead          = false;
      c.useBackJoint         = true;
      c.backSideBend         = false;
      c.irSensors            = false;
      c.useGripper           = false;
      c.gripDuration         = 30;
      c.releaseDuration      = 1;

      //      c.headTexture = __PLACEHOLDER_0__;
      c.headTexture     = "Images/dusty.rgb";
      c.headColor       = "robot4";
      //  c.bodyTexture = __PLACEHOLDER_3__;
      c.bodyTexture     = "Images/dusty.rgb";
      c.bodyColor       = "robot4";
      c.trunkTexture    = "Images/dusty.rgb"; //__PLACEHOLDER_7__;
      c.trunkColor      = "robot1";
      c.trouserColor    = "robot2";
      return c;
    }

    static SkeletonConf getDefaultConfVelServos() const {
      SkeletonConf c = getDefaultConf();

      c.useVelocityServos = true;

      c.hipDamping    = 0.01;
      c.hip2Damping   = 0.01;
      c.neckDamping   = 0.01;
      c.kneeDamping   = 0.01;
      c.ankleDamping  = 0.01;
      c.armDamping    = 0.01;
      c.elbowDamping  = 0.01;
      c.pelvisDamping = 0.01;
      c.backDamping   = 0.01;


      return c;
    }


    /** sets the pose of the vehicle
        @param pose desired pose matrix
    */
    virtual void placeIntern(const osg::Matrix& pose);

    virtual int getSensorsIntern(sensor* sensors, int sensornumber);
    virtual void setMotorsIntern(const motor* motors, int motornumber);
    virtual int getSensorNumberIntern() const;
    virtual int getMotorNumberIntern() const;


    /******** CONFIGURABLE ***********/
    virtual void explicit notifyOnChange(const paramkey& key);

    /** the main object of the robot, which is used for position and speed tracking */
    virtual const Primitive* getMainPrimitive() const const { return objects[Thorax]; } // Trunk_comp

    /** returns the position of the head */
    virtual Position getHeadPosition() const;

    /** returns the position of the trunk */
    virtual Position getTrunkPosition() const;

    /// returns a the gripper list
    GripperList& getGrippers() const;


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

    OdeHandle ignoreColSpace; // odehandle with space within collisions are ignored

    std::vector<TwoAxisServo*> hipservos; // motors
    std::vector<OneAxisServo*> kneeservos; // motors
    std::vector<OneAxisServo*> ankleservos; // motors
    std::vector<TwoAxisServo*> armservos; // motors
    std::vector<OneAxisServo*> arm1servos; // motors
/*     std::vector<OneAxisServo*> headservos; // motors */
    std::vector<TwoAxisServo*> headservos; // motors

    OneAxisServo* pelvisservo; // between Hip and Trunk_comp
    std::vector<OneAxisServo*> backservos;
    std::vector<TwoAxisServo*> backservos2;

    std::vector<AngularMotor*> frictionmotors;

    GripperList grippers;
    int backbendindex = 0;
  };

}

#endif
