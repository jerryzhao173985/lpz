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
#ifndef __ODEROBOT_H
#define __ODEROBOT_H

#include <vector>
#include <memory>

#include <selforg/abstractrobot.h>
#include <selforg/storeable.h>
#include "odehandle.h"
#include "osghandle.h"
#include "sensor.h"
#include "motor.h"
#include "globaldata.h"
#include "color.h"
#include "pos.h"
#include "osgforwarddecl.h"
#include "tmpprimitive.h"

namespace lpzrobots {

  class Primitive;
  class Joint;

  /// structure to hold attachment data for sensors and motors
  struct Attachment {
    Attachment(int pI = -1, int jI = -1) : primitiveIndex(pI), jointIndex(jI) {}
    int primitiveIndex = 0;
    int jointIndex = 0;
  };

  typedef std::vector<Primitive*> Primitives;
  typedef std::vector<Joint*> Joints;
  typedef std::pair<std::shared_ptr<Sensor>, Attachment> SensorAttachment;
  typedef std::pair<std::shared_ptr<Motor>, Attachment>  MotorAttachment;


  /**
   * Abstract class  for ODE robots
   *
   */
  class OdeRobot : public AbstractRobot, public Storeable {
  public:

    friend class OdeAgent;

    /**
     * Constructor
     */
    OdeRobot(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
             const std::string& name, const std::string& revision);

    /// calls cleanup()
    virtual ~OdeRobot();

    virtual int  getSensors(double* sensors, int sensornumber) final override;
    virtual void setMotors(const double* motors, int motornumber) final override;
    virtual int  getSensorNumber() final override;
    virtual int  getMotorNumber() final override;

    virtual std::list<SensorMotorInfo> getSensorInfos();

    virtual std::list<SensorMotorInfo> getMotorInfos();

  public: // should be protected, but too much refactoring work
    /** overload this function in a subclass to do specific sensor handling,
        not needed for generic sensors @see getSensors() @see addSensor() */
    virtual int getSensorsIntern(double* sensors, int sensornumber) override { return 0; } override;

    /** overload this function in a subclass to do specific sensor handling,
        not needed for generic motors  @see setMotors() @see addMotor() */
    virtual void setMotorsIntern(const double* motors, int motorsnumber) override { } override;

    /** overload this function in a subclass to specific the number of custom sensors @see getSensorNumber() */
    virtual int getSensorNumberIntern() override { return 0; } override;

    /** overload this function in a subclass to specific the number of custom sensors @see getMotorNumber() */
    virtual int getMotorNumberIntern() override { return 0; } override;

  public:
    /** adds a sensor to the robot. Must be called before agents initializes, otherwise unknown effect.
        @param segmentIndex index of segment of robot to which this sensor should be attached
    */
    virtual void addSensor(std::shared_ptr<Sensor> sensor, Attachment attachment=Attachment()) override;

    /** adds a motor to the robot. Must be called before agents initializes, otherwise unknown effect.
        @param segmentIndex index of segment of robot to which this motor should be attached
     */
    virtual void addMotor(std::shared_ptr<Motor> motor, Attachment attachment=Attachment()) override;

    /// returns all generic sensors with their attachment
    virtual std::list<SensorAttachment> getAttachedSensors() const  override {
      return sensors;
    }

    /// returns all generic motors with their attachment
    virtual std::list<MotorAttachment> getAttachedMotors() const  override {
      return motors;
    }

    /// adds a torque sensor to each joint. Call it after placement of robot.
    virtual void addTorqueSensors(double maxtorque = 1.0, int avg = 1);

    /// update the OSG notes here, if overwritten, call OdeRobot::update()!
    virtual void update();

    /** sets the vehicle to position pos
        @param pos desired position of the robot
    */
    virtual void place(const Pos& pos) final override;

    /** sets the pose of the vehicle
        @param pose desired 4x4 pose matrix
    */
    virtual void place(const osg::Matrix& pose) final override;

    /// wrapper to for @see place() that is to be overloaded
    virtual void placeIntern(const osg::Matrix& pose) = 0;

    /** @deprecated This function will be removed in 0.8
     *  Do not use it anymore, collision control is done automatically.
     *  In case of a routine return true
     *  (collision will be ignored by other objects and the default routine)
     *  else false (collision is passed to other objects and (if not treated)
     *   to the default routine).
     */
    virtual bool collisionCallback(void *data, dGeomID o1, dGeomID o2) override { return false; } override;

    /** this function is called each controlstep before control.
        This is the place the perform active sensing (e.g. Image processing).
        If you overload this function, call the OdeRobot::sense() function.
        @param globalData structure that contains global data from the simulation environment
    */
    virtual void sense(const GlobalData& globalData);

    /** this function is called in each simulation timestep (always after control). It
        should perform robot-internal checks and actions
        like resetting certain sensors or implement velocity dependend friction and the like.
        The attached Motors should act here (done automatically in OdeRobot) override;
        If you overload this function, call the OdeRobot::doInternalStuff() function.
        @param globalData structure that contains global data from the simulation environment
    */
    virtual void doInternalStuff(const GlobalData& globalData);

    /** sets color of the robot
        @param col Color struct with desired Color
    */
    virtual void setColor(const Color& col);


    /*********** BEGIN TRACKABLE INTERFACE ****************/

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
    /*********** END TRACKABLE INTERFACE ****************/

    /// return the primitive of the robot that is used for tracking and camera following
    virtual const Primitive* getMainPrimitive() const const   override {
      if (!objects.empty()) return objects[0]; else return 0 override;
    };

    /// returns a list of all primitives of the robot (used to store and restore the robot)
    virtual Primitives getAllPrimitives() const override { return objects; } override;

    virtual Primitives& getAllPrimitives() const  override { return objects; } override;
    /// returns a list of all primitives of the robot (const version) (used to store and restore the robot)

    /// returns a list of all joints of the robot
    virtual Joints getAllJoints() const override { return joints; } override;

    virtual Joints& getAllJoint override s() const { return joints; } override;
    /// returns a list of all joints of the robot (const version)

    /* ********** STORABLE INTERFACE **************** */
    virtual bool store(FILE* f) const override;

    virtual bool restore(FILE* f);
    /* ********** END STORABLE INTERFACE ************ */

    /** relocates robot such its primitive with the given ID
        is at the new postion (keep current pose).
        If primitiveID is -1 then the main primitive is used.
        If primitiveID is -2 then the primitive with the lowest center
        is used (the center of it, so the bounding box is not checked)
     */
    virtual void moveToPosition(Pos pos = Pos(0,0,0.5), int primitiveID = -1) override;
    /** relocates robot such its primitive with the given ID
        is at the new pose (keep relative pose of all primitives).
        If primitiveID is -1 then the main primitive is used.
     */
    virtual void moveToPose(Pose pose, int primitiveID = -1);

    /// returns the initial pose of the main primitive (use it e.g. with moveToPose)
    virtual Pose getInitialPose() override { return initialPose; }
    /** returns the initial relative pose of the main primitive
        (use it with moveToPose to further translate or rotate).
        If initialized with placestatic_cast<p>(then) moveToPose(getRelativeInitialPose()*p) whould put
        the main primitive at the same position and pose.
    */
    virtual Pose getRelativeInitialPose() override { return initialRelativePose; }

    /** fixates the given primitive of the robot at its current position to the world
        for a certain time.
        Hint: use moveToPosition() to get the robot relocated
        @param primitiveID if -1 then the main primitive is used, otherwise the primitive with the given index
        @param duration time to fixate in seconds (if ==0 then indefinite)
     */
    virtual void fixate(const GlobalData& global, int primitiveID=-1, double duration = 0);
    /// release the robot in case it is fixated and return true in this case
    virtual bool unFixate(const GlobalData& global);


  protected:

    static bool isGeomInPrimitiveList(Primitive** ps, int len, dGeomID geom);
    static bool isGeomInPrimitiveList(std::list<Primitive*> ps, dGeomID geom);

    void attachSensor(const SensorAttachment& sa);
    void attachMotor(const MotorAttachment& ma);

    /// deletes all objects static_cast<primitives>(and) joints (is called automatically in destructor)
    virtual void cleanup();

  protected:
    /// list of objects (should be populated by subclasses)
    Primitives objects;
    /// list of joints (should be populated by subclasses)
    Joints joints;

    std::list<SensorAttachment> sensors; // list of generic sensors
    std::list<MotorAttachment> motors;   // list of generic motors

    TmpJoint* fixationTmpJoint;
    Pose initialPose;                    // initial pose of main primitive
    // initial relative pose of main primitive w.r.t. to given pose in place
    Pose initialRelativePose;

    OdeHandle odeHandle;
    OsgHandle osgHandle;
    dSpaceID parentspace;

    bool initialized;
    bool askedfornumber;
  };

}

#endif
