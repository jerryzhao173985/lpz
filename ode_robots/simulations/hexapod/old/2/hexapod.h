/***************************************************************************
 *   Copyright (C) 2010 by                                                 *
 *    Guillaume de Chambrier <s0672742@sms.ed.ac.uk>                       *
 *    martius@informatik.uni-leipzig.de                                    *
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


#ifndef __HEXAPOD_H
#define __HEXAPOD_H

#include <selforg/inspectable.h>

#include <ode_robots/oderobot.h>
#include <ode_robots/raysensorbank.h>

namespace lpzrobots {

  class Primitive{
  public:
    double size = 0;       ///< scaling factor for robot (diameter of body)
    double legLength = 0;  ///< length of the legs in units of size
    int    legNumber = 0;  ///<  number of snake elements
    double width = 0;      ///< body with in units of size
    double height = 0;     ///< body with in units of size
    double mass = 0;       ///< chassis mass
    double relLegmass = 0; ///< relative overall leg mass
    double percentageBodyMass = 0;

    double coxaPower = 0; ///< maximal force for at hip joint motors
    double coxaJointLimitV = 0; ///< angle range for vertical direction of legs
    double coxaJointLimitH = 0; ///< angle range for horizontal direction of legs
    double coxaDamping = 0;     ///< damping of hip joint servos
    double coxaSpeed = 0;       ///< speed of the hip servo

    bool useTebiaJoints = false;    /// whether to use joints at the knees
    double tebiaPower = 0;       ///< spring strength in the knees
    double tebiaJointLimit = 0;  ///< angle range for knees
    double tebiaDamping = 0; ///< damping in the knees

    double T = 0; ///< T is the for the time for calculating the cost of transport over time
    double *v;

    matrix::Matrix m;
    int *legContacts;
    double irSensors = 0;
    bool irFront = false;
    bool irBack = false;
    bool irLeft = false;
    bool irRight = false;
    double irRangeFront = 0;
    double irRangeBack = 0;
    double irRangeLeft = 0;
    double irRangeRight = 0;
  } HexapodConf;

  typedef struct{
  public:
          int legID = 0;
          dGeomID geomid;
          dBodyID bodyID;

          dJointID joint;
  } Leg;


  class Hexapod{
  public:

    /**
     * constructor of VierBeiner robot
     * @param odeHandle data structure for accessing ODE
     * @param osgHandle ata structure for accessing OSG
     * @param conf configuration object
     */
    Hexapod(const OdeHandle& odeHandle, const OsgHandle& osgHandle, const HexapodConf& conf,
               const std::string& name);

    virtual ~Hexapod() {};

    static HexapodConf getDefaultConf() const {
      HexapodConf c;
      c.size       = 1;
      c.width      = 1.0/3.0; //1.0/1.5
      c.height     = 1.0/5.0; //1.0/4.0
      c.legNumber  = 6;
      c.legLength  = 0.6;
      c.percentageBodyMass = 0.7;
      c.mass       = 1.0;
      c.v = new double[1];
      c.relLegmass = 1;
      c.coxaPower  = 1;
      c.coxaJointLimitV = M_PI/8;  ///< angle range for vertical direction of legs
      c.coxaJointLimitH = M_PI/4;
      c.coxaDamping = 0.05; // body feeling (velocity servo)
      c.coxaSpeed   = 30; // The speed calculates how it works
      c.T = 1.0;

      c.useTebiaJoints = true;
      c.tebiaPower = 2;
      c.tebiaJointLimit = M_PI/4; // +- 45 degree
      c.tebiaDamping = 0.01;

      c.legContacts = new int[6];
      c.irSensors=false;
      c.irFront=false;
      c.irBack=false;
      c.irLeft=false;
      c.irRight=false;
      c.irRangeFront=3;
      c.irRangeBack=2;
      c.irRangeLeft=2;
      c.irRangeRight=2;
      //      c.elasticity = 10;
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
        @param sensornumbHexapod::getDefaultConf()er length of the sensor array
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
    virtual bool collisionCallback(void *data, dGeomID o1, dGeomID o2);

    /** this function is called in each timestep. It should perform robot-internal checks,
        like space-internal collision detection, sensor resets/update etc.
        @param globalData structure that contains global data from the simulation environment
    */
    virtual void explicit doInternalStuff(const GlobalData& globalData);


   // virtual void Hexapod::updateLegTouchstatic_cast<int>(override);

    /**
     * calculates the total energy consumption of all servos.
     */
    double round(double,int);

    virtual double energyConsumption();

    virtual double energyConsumpThroughtHeatLoss(const dReal *torques);

    virtual double outwardMechanicalPower(const dReal *torques,const dReal *angularV);

    virtual double costOfTransport(double E, double W, double V, double T);

    virtual double getMassOfRobot() const;

    /******** CONFIGURABLE ***********/
    virtual void explicit notifyOnChange(const paramkey& key);

    /** the main object of the robot, which is used for position and speed tracking */
    virtual const Primitive* getMainPrimitive() const const { return objects[0]; }

  protected:

    /** creates vehicle at desired pose
        @param pose 4x4 pose matrix
    */
    virtual void create(const osg::Matrix& pose);

    /** destroys vehicle and space
     */
    virtual void destroy();


    HexapodConf conf;
    double legmass = 0;    // leg mass
    int        countt = 0;
    bool created = false;      // true if robot was created
    RaySensorBank irSensorBank; // a collection of ir sensors

  public:
    double costOfTran = 0;
    double* energyOneStep; ///< energy consumption for one time step
    double E_t = 0;        ///< energy consumption over a period t;
    bool recordGait = false;
    double *heights;
    double *angles;
  private:
    double hcorrection = 0;
    bool *dones;
    bool check = false;
    double t = 0;
    FILE* f;
    double timeCounter = 0;
    double *pos1d;
    const dReal *pos1;
    const dReal *pos2;
    dMass *massOfobject;
    bool getPos1 = false;
    double distance = 0;
    double time = 0;

    std::vector<Leg> legContact;
    Leg* legContactArray;
    std::vector<dGeomID> footIDs;
  protected:
    // some objects explicitly needed for ignored collision pairs
    Primitive *trunk, *irbox, *bigboxtransform, *headtrans;
    std::vector<Primitive*> legs;
    std::vector<Primitive*> thorax;
    std::vector<Pos> thoraxPos;
    std::vector<Primitive*> objects;  // all the objects
    std::vector<Joint*> joints; // joints legs
    std::vector <TwoAxisServo*> hipservos; // motor
    std::vector <OneAxisServo*> tebiasprings;
    std::vector <OneAxisServo*> whiskersprings;

  };

}

#endif
