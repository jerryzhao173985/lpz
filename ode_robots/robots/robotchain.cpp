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
#include <cassert>

#include <ode_robots/joint.h>
#include <ode_robots/nimm2.h>

// include header file
#include "robotchain.h"

using namespace osg;
using namespace std;

namespace lpzrobots {

  // constructor:
  // - give handle for ODE and OSG stuff
  RobotChain::RobotChain(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
           const RobotChainConf& c, const std::string& name)
    : OdeRobot(odeHandle, osgHandle, name, "1.0"), conf(c)
  {
    created=false;
  };

  int RobotChain::getMotorNumberIntern() const {
    int num=0;
    FOREACHC(vector<OdeRobot*>, robots, r){
      if(*r) num+=(*r)->getMotorNumber();
    }
    return num;
  }

  void RobotChain::setMotorsIntern(const double* motors, int motornumber){
    assert(created); // robot must exist
    int index=0;
    FOREACH(vector<OdeRobot*>, robots, r){
      int len = (*r)->getMotorNumber();
      assert(index+len<=motornumber);
      (*r)->setMotors(motors+index,len);
      index+=len;
    }
  };


  int RobotChain::getSensorNumberIntern() const {
    int num=0;
    FOREACHC(vector<OdeRobot*>, robots, r){
      if(*r) num+=(*r)->getSensorNumber();
    }
    return num;
  }

  int RobotChain::getSensorsIntern(sensor* sensors, int sensornumber){
    assert(created);
    int index=0;
    sensor buf[10];
    vector<sensor> irvals;
    int j=0;
    FOREACH(vector<OdeRobot*>, robots, r){
      int len = (*r)->getSensorNumber();
      assert(index+len<=sensornumber);
      if(conf.useIR && (j==0 || j==conf.numRobots-1)){
       (*r)->getSensorsIntern(buf,len);
       // store motors
       sensors[index]   = buf[0];
       sensors[index+1] = buf[1];
       index+=2;
       // save irvals for the very end
       for(int i=2; i<len; ++i) {
         irvals.push_back(buf[i]);
       }
      }else{
        (*r)->getSensorsIntern(sensors+index,len);
        index+=len;
      }
      ++j;
    }
    FOREACH(vector<sensor>,irvals,v){
      assert(index<sensornumber);
      sensors[index]=*v;
      ++index;
    }
    if(conf.useIR) { assert(static_cast<int>(irvals.size()) == getIRSensorNum());}
    return index;
  };


  void RobotChain::placeIntern(const osg::Matrix& pose){
    create(pose);
  };


  void RobotChain::update() {
    OdeRobot::update();
    assert(created); // robot must exist
    FOREACH(vector<OdeRobot*>, robots, r){
      if(*r) (*r)->update();
    }

  };

  void RobotChain::doInternalStuff(const GlobalData& global){
    OdeRobot::doInternalStuff(global);
    FOREACH(vector<OdeRobot*>, robots, r){
      if(*r) (*r)->doInternalStuff(global);
    }
  }

  void RobotChain::create( const osg::Matrix& pose ){
    if (created) {
      destroy();
    }
    for (int j=0; j<conf.numRobots; ++j)  {
      Nimm2Conf nimm2conf   = Nimm2::getDefaultConf();
      nimm2conf.size        = conf.size;
      nimm2conf.force       = j==conf.mainRobot ? conf.forceMain : conf.force;
      nimm2conf.speed       = conf.speed;
      nimm2conf.cigarMode   = true;
      nimm2conf.singleMotor = false;
      nimm2conf.boxMode     = true;
      nimm2conf.visForce    = true;
      nimm2conf.bumper      = true;
      // nimm2conf.boxWidth = 1.5;
      nimm2conf.massFactor  = j==conf.mainRobot ? conf.massFactorMain : conf.massFactor;
      nimm2conf.wheelSlip   = conf.wheelSlip;
      nimm2conf.wheelTexture="Images/tire_stripe.rgb";
      nimm2conf.irRange     = 3*conf.size;
      if(conf.useIR && j== 0){
        nimm2conf.irFront = true;
      }
      if(conf.useIR && j==conf.numRobots-1){
        nimm2conf.irBack = true;
      }

      OdeRobot* nimm2 = new Nimm2(odeHandle, osgHandle, nimm2conf, getName() + "_" + itos(j));
      if(j== 0)
        nimm2->setColor(osgHandle.getColor("robot1"));
      else
        nimm2->setColor(osgHandle.getColor(conf.color));

      (static_cast<OdeRobot*>(nimm2))->placeIntern(TRANSM(j*(-conf.distance),0,0.11)*pose);
      robots.push_back(nimm2);
    }
    for(int j=0; j<conf.numRobots-1; ++j)  {
      const Primitive* p1 = robots[j]->getMainPrimitive();
      const Primitive* p2 = robots[j+1]->getMainPrimitive();
      //Joint* joint = new BallJoint(p1,p2,(p1->getPosition()+p2->getPosition())/2.0);
      Joint* joint = new UniversalJoint(const_cast<Primitive*>(p1),const_cast<Primitive*>(p2),(p1->getPosition()+p2->getPosition())/2.0,
                                        Axis(0,0,1)*pose,Axis(0,1,0)*pose
                                        );
      joint->init(odeHandle,osgHandle,true,conf.size/6);
      joints.push_back(joint);
    }

    created=true;
  };

  int RobotChain::getIRSensorNum() const {
    return conf.useIR ? 4 : 0;
  }


  /** destroys vehicle and space
   */
  void RobotChain::destroy(){
    if (created){
      FOREACH(vector<OdeRobot*>, robots, r){
        if(*r) delete *r;
      }
      robots.clear();
      cleanup();
    }
    created=false;
  }

  const Primitive* RobotChain::getMainPrimitive() const {
    int idx=conf.numRobots/2;
    if(conf.mainRobot>=0)
      idx=conf.mainRobot;
    if((signed int)robots.size()>idx){
      return robots[idx]->getMainPrimitive();
    }else return nullptr;
  }

  void  RobotChain::notifyOnChange(const paramkey& key){

  }

}
