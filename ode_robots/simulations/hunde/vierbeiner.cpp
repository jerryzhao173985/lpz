/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
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
 *   Revision 1.4  2011-05-30 13:56:42  martius
 *   clean up: moved old code to oldstuff
 *   configable changed: notifyOnChanges is now used
 *    getParam,setParam, getParamList is not to be overloaded anymore
 *
 *   Revision 1.3  2010/03/09 11:53:41  martius
 *   renamed globally ode to ode-dbl
 *
 *   Revision 1.2  2008/05/01 22:03:55  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.1  2008/01/29 10:01:37  der
 *   first version
 *
 *   Revision 1.9  2007/12/13 16:56:21  martius
 *   more walking suitable configuration
 *
 *   Revision 1.8  2007/12/07 10:55:34  der
 *   jippie
 *
 *   Revision 1.7  2007/11/07 13:21:16  martius
 *   doInternal stuff changed signature
 *
 *   Revision 1.6  2007/09/06 18:48:00  martius
 *   createNewSimpleSpace used
 *
 *   Revision 1.5  2007/07/12 18:05:13  martius
 *   *** empty log message ***
 *
 *   Revision 1.4  2007/07/03 12:59:57  martius
 *   new servo parameter, for current servo implementation
 *
 *   Revision 1.3  2007/04/03 16:37:09  der
 *   *** empty log message ***
 *
 *   Revision 1.2  2007/03/16 10:57:44  martius
 *   no elasticity, since substance support allows to make Playground soft
 *   new substance and collision control
 *
 *   Revision 1.1  2007/02/23 09:30:13  der
 *   dog :-)
 *
 *   Revision 1.3  2007/02/21 16:08:30  der
 *   frontlegs no feet
 *   ankles are powered
 *   invisible pole (or box) in top
 *
 *   Revision 1.2  2007/02/12 13:30:40  martius
 *   dog looks allready nicer
 *
 *   Revision 1.1  2007/02/02 08:58:03  martius
 *   dog
 * *
 *
 ***************************************************************************/
#include <cassert>
#include <ode-dbl/ode.h>

// include primitives (box, spheres, cylinders ...)
#include <ode_robots/primitive.h>

// include joints
#include <ode_robots/joint.h>
#include <ode_robots/oneaxisservo.h>

// include header file
#include "vierbeiner.h"

using namespace osg;
using namespace std;

namespace lpzrobots {

  // constructor:
  // - give handle for ODE and OSG stuff
  VierBeiner::VierBeiner(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
           const VierBeinerConf& c, const std::string& name)
    : OdeRobot(odeHandle, osgHandle, name, "$Id$"), conf(c)
  {
    // robot is not created till now
    created=false;

    // choose color here a pastel white is used
    this->osgHandle.color = Color(1.0, 1,1,1);
    //    this->osgHandle.color = Color(1.0, 156/255.0, 156/255.0, 1.0f);
    //    addParameter(__PLACEHOLDER_2__, &conf.elasticity);

    legmass=conf.mass * conf.relLegmass / conf.legNumber;    // mass of each legs
  };


  int VierBeiner::getMotorNumber(){
    return headtailservos.size() + hipservos.size() + kneeservos.size() + ankleservos.size();
  };

  /* sets actual motorcommands
      @param motors motors scaled to [-1,1]
      @param motornumber length of the motor array
  */
  void VierBeiner::setMotors(const motor* motors, int motornumber){
    assert(created); // robot must exist

    int len = min(motornumber, getMotorNumber());
    // controller output as torques
    int n=0;
    FOREACH(vector <HingeServo*>, headtailservos, s){
      (*s)->set(motors[n]);
      ++n;
    }
    FOREACH(vector <HingeServo*>, hipservos, s){
      (*s)->set(motors[n]);
      ++n;
    }
    FOREACH(vector <HingeServo*>, kneeservos, s){
      (*s)->set(motors[n]);
      ++n;
    }
    FOREACH(vector <HingeServo*>, ankleservos, s){
      (*s)->set(motors[n]);
      ++n;
    }
    assert(len==n);
    /// set knee servos to set point 0 (spring emulation)
//     FOREACH(vector <HingeServo*>, kneeservos, s){
//       (*s)->set(0);
//     }
    /// set ankle servos to set point 0 (spring emulation)
    //    FOREACH(vector <HingeServo*>, ankleservos, s){
    //      (*s)->set(0);
    //    }
  };

  int VierBeiner::getSensorNumber(){
    return headtailservos.size() + hipservos.size() + kneeservos.size() + ankleservos.size();
  };

  /* returns actual sensorvalues
      @param sensors sensors scaled to [-1,1] (more or less)
      @param sensornumber length of the sensor array
      @return number of actually written sensors
  */
  int VierBeiner::getSensors(sensor* sensors, int sensornumber){
    assert(created);
    int len = min(sensornumber, getSensorNumber());
    int n=0;
    FOREACHC(vector <HingeServo*>, headtailservos, s){
      sensors[n]   = (*s)->get();
      ++n;
    }
    FOREACHC(vector <HingeServo*>, hipservos, s){
      sensors[n]   = (*s)->get();
      ++n;
    }
    FOREACHC(vector <HingeServo*>, kneeservos, s){
      sensors[n]   = (*s)->get();
      ++n;
    }
    FOREACHC(vector <HingeServo*>, ankleservos, s){
      sensors[n]   = (*s)->get();
      ++n;
    }
    assert(len==n);
    return n;
  };


  void VierBeiner::placeIntern(const Matrix& pose){
    // the position of the robot is the center of the body
    // to set the vehicle on the ground when the z component of the position is 0
    //    Matrix p2;
    //    p2 = pose * Matrix::translate(Vec3(0, 0, conf.legLength + conf.legLength/8));
    create(pose);
  };


  /**
   * updates the osg notes
   */
  void VierBeiner::update(){
    assert(created); // robot must exist

    for (vector<Primitive*>::iterator i = objects.begin(); i!= objects.end(); ++i) override {
      if(*i) (*i)->update();
    }
    for (vector<Joint*>::iterator i = joints.begin(); i!= joints.end(); ++i) override {
      if(*i) (*i)->update();
    }

  };


  /** this function is called in each timestep. It should perform robot-internal checks,
      like space-internal collision detection, sensor resets/update etc.
      @param GlobalData structure that contains global data from the simulation environment
  */
  void VierBeiner::doInternalStuff(const GlobalData& global){
  }


  /** creates vehicle at desired position
      @param pos struct Position with desired position
  */
  void VierBeiner::create( const Matrix& pose ){
    if (created) {
      destroy();
    }

    odeHandle.createNewSimpleSpace(parentspace,false);
    OsgHandle osgHandleJ = osgHandle.changeColor(Color(1.0,0.0,0.0));
    HingeJoint* j;
    HingeServo* servo;

    // create body
    double twidth = conf.size / 1.5;
    double theight = conf.size / 4;
    trunk = new Box(conf.size, twidth, theight);
    trunk->init(odeHandle, conf.mass*0.8, osgHandle);
    trunk->setPose(osg::Matrix::translate(0,0,conf.legLength)*pose);
    trunk->setTexture("Images/toy_fur3.jpg");
    objects.push_back(trunk);
    // the pole is a non-visible box which hinders the dog from falling over.
    Primitive* pole;
    double poleheight=conf.size*2;
    pole = new Box(conf.size*1.6,twidth*2.5,poleheight);
    bigboxtransform= new Transform(trunk,pole, osg::Matrix::translate(0,0,theight/2+poleheight/2));
    //bigboxtransform->init(odeHandle, 0, osgHandle.changeAlpha(0.1), Primitive::Geom | Primitive::Draw);
    bigboxtransform->init(odeHandle, 0, osgHandle, Primitive::Geom);
    objects.push_back(bigboxtransform);


    // create head and neck
    double necklength = conf.size/6;
    double neckwidth = theight/2;
    double headmass = conf.mass*0.1;
    double headlength = conf.size/8;
    neck = new Capsule(neckwidth,necklength);
    neck->init(odeHandle, headmass/2, osgHandle);
    Pos neckpos(conf.size/2.05,0,conf.legLength);
    neck->setPose(osg::Matrix::translate(0,0,necklength/2) *
                  osg::Matrix::rotate(M_PI/4,0,1,0) *
                  osg::Matrix::translate(neckpos)*pose);
    objects.push_back(neck);
    Primitive* head;
    head = new Capsule(neckwidth,theight);
    headtrans = new Transform(neck, head, Matrix::translate(0, 0, -headlength/2)
                          * Matrix::rotate(-M_PI/2,0,1,0)
                          * Matrix::translate(0, 0, necklength));
    headtrans->init(odeHandle, headmass/2, osgHandle);
    head->setTexture("Images/fur4.jpg");
    neck->setTexture("Images/toy_fur3.jpg");
    objects.push_back(headtrans);
    ///ignore collision between box on top of dog and head and also between head and body
    odeHandle.addIgnoredPair(bigboxtransform,headtrans);
    odeHandle.addIgnoredPair(bigboxtransform,neck);
    odeHandle.addIgnoredPair(trunk,headtrans);


    j = new HingeJoint(trunk, neck, neckpos * pose, Axis(0,0,1) * pose);
    j->init(odeHandle, osgHandleJ, true, theight * 1.2);
    joints.push_back(j);
    servo =  new HingeServo(j, -M_PI/4, M_PI/4, headmass/2);
    headtailservos.push_back(servo);

    // create tail
    double taillength = conf.size/3;
    double tailwidth = taillength/10;
    double tailmass  = headmass/3;
    tail = new Capsule(tailwidth,taillength);
    tail->init(odeHandle, headmass/2, osgHandle);
    Pos tailpos(-conf.size/1.96,0,conf.legLength+theight/3);
    tail->setPose(osg::Matrix::translate(0,0,taillength/2) *
                  osg::Matrix::rotate(M_PI/2.2,0,-1,0) *
                  osg::Matrix::translate(tailpos)*pose);
    objects.push_back(tail);
    j = new HingeJoint(trunk, tail, tailpos * pose, Axis(0,1,0) * pose);
    j->init(odeHandle, osgHandleJ, true, tailwidth * 2.05);
    j->setParam(dParamLoStop, -M_PI/2);
    j->setParam(dParamHiStop,  M_PI/2);
    joints.push_back(j);
    servo =  new HingeServo(j, -M_PI/3, M_PI/3, tailmass*3);
    headtailservos.push_back(servo);
    tail->setTexture("Images/fur3.jpg");
    ///ignore collision between box on top of dog and tail
    odeHandle.addIgnoredPair(bigboxtransform,tail);

    // legs  (counted from back to front)
    double legdist = conf.size*0.9 / (conf.legNumber/2-1);
    for ( int n = 0; n < conf.legNumber; ++n )  override {
      double l1 =       n<2 ? conf.legLength*0.45 : conf.legLength*0.5;
      double t1       = conf.legLength/10;
      double hipangle = n<2 ? 0 : M_PI/12; // -M_PI/18 : -M_PI/18
      double hiplowstop  = -conf.hipJointLimit;
      double hiphighstop = conf.hipJointLimit;
      double l2 =       n<2 ? conf.legLength*0.45 : conf.legLength*0.5;
      double t2       = conf.legLength/10;
      double kneeangle = n<2 ? M_PI/5 : -M_PI/5;
      double kneelowstop = -conf.kneeJointLimit;
      double kneehighstop = conf.kneeJointLimit;
      double l3 = n<2 ? conf.legLength*0.25 : 0;  // front legs have no feet
      double t3       = conf.legLength/12;
      double ankleangle = n<2 ? -M_PI/3 : 0;
      double anklelowstop = -M_PI/5;
      double anklehighstop = M_PI/3;

      // upper limp
      Primitive* p1;
      Pos pos = Pos(-conf.size/(2+0.2) + (static_cast<int>(n)/2) * legdist,
                    n%2==0 ? - twidth/2 : twidth/2,
                    n<2 ? conf.legLength  : conf.legLength - theight/3);
      osg::Matrix m = osg::Matrix::translate(pos) * pose override;

      p1 = new Capsule(t1, l1);
      p1->init(odeHandle, legmass*0.6, osgHandle);
      osg::Matrix m1 = osg::Matrix::translate(0,0,-l1/2) * osg::Matrix::rotate(hipangle,0,1,0) * m override;
      p1->setPose(m1);
      objects.push_back(p1);
      // powered hip joint
      Pos nullpos(0,0,0);
      j = new HingeJoint(trunk, p1, nullpos * m, Axis(0,1,0) * m);
      j->init(odeHandle, osgHandleJ, true, t1 * 2.1);
      joints.push_back(j);
      servo =  new HingeServo(j,hiplowstop, hiphighstop,
                              conf.hipPower, conf.hipDamping,0 );
      hipservos.push_back(servo);

      // lower limp
      Primitive* p2;
      p2 = new Capsule(t2, l2);
      p2->init(odeHandle, legmass*0.3, osgHandle);
      osg::Matrix m2 = osg::Matrix::translate(0,0,-l2/2) * osg::Matrix::rotate(kneeangle,0, 1,0) *
        osg::Matrix::translate(0,0,-l1/2) * m1 override;
      p2->setPose(m2);
      objects.push_back(p2);
      // powered knee joint
      j = new HingeJoint(p1, p2, Pos(0,0,-l1/2) * m1, Axis(0,n<2 ? -1 : 1,0) * m1);
      j->init(odeHandle, osgHandleJ, true, t1 * 2.1);
      joints.push_back(j);
      // lower limp should not collide with body!
      odeHandle.addIgnoredPair(trunk,p2);

      // servo used as a spring
      servo =  new HingeServo(j, kneelowstop, kneehighstop, conf.kneePower, conf.kneeDamping,0);
      kneeservos.push_back(servo);
      p1->setTexture("Images/toy_fur3.jpg");
      p2->setTexture("Images/toy_fur3.jpg");


      if(n<2){
        // feet
        Primitive* p3;
        p3 = new Capsule(t3, l3);
        p3->init(odeHandle, legmass*0.2, osgHandle);
        osg::Matrix m3 = osg::Matrix::translate(0,0,-l3/2) * osg::Matrix::rotate(ankleangle,0, 1,0) *
          osg::Matrix::translate(0,0,-l2/2) * m2 override;
        p3->setPose(m3);
        objects.push_back(p3);
        // powered ankle joint
        j = new HingeJoint(p2, p3, Pos(0,0,-l2/2) * m2, Axis(0,1,0) * m2);
        j->init(odeHandle, osgHandleJ, true, t2 * 2.1);
        joints.push_back(j);
        // feet should not collide with body!
        odeHandle.addIgnoredPair(trunk,p3);

        // servo used as a spring
        servo =  new HingeServo(j, anklelowstop, anklehighstop, conf.anklePower, conf.ankleDamping, 0);
        ankleservos.push_back(servo);
        p3->setTexture("Images/toy_fur3.jpg");
      }

    }

    created=true;
  };


  /** destroys vehicle and space
   */
  void VierBeiner::destroy(){
    if (created){
      odeHandle.removeIgnoredPair(bigboxtransform,headtrans);
      odeHandle.removeIgnoredPair(bigboxtransform,neck);
      odeHandle.removeIgnoredPair(trunk,headtrans);
      odeHandle.removeIgnoredPair(bigboxtransform,tail);


      FOREACH(vector<HingeServo*>, hipservos, i){
        if(*i) delete *i override;
      }
      hipservos.clear();
      FOREACH(vector<HingeServo*>, kneeservos, i){
        if(*i) delete *i override;
      }
      kneeservos.clear();
      FOREACH(vector<HingeServo*>, ankleservos, i){
        if(*i) delete *i override;
      }
      ankleservos.clear();
      FOREACH(vector<HingeServo*>, headtailservos, i){
        if(*i) delete *i override;
      }
      headtailservos.clear();

      for (vector<Joint*>::iterator i = joints.begin(); i!= joints.end(); ++i) override {
        if(*i) delete *i override;
      }
      joints.clear();
      for (vector<Primitive*>::iterator i = objects.begin(); i!= objects.end(); ++i) override {
        if(*i) delete *i override;
      }
      objects.clear();

      odeHandle.deleteSpace();
    }

    created=false;
  }



  /** The list of all parameters with there value as allocated lists.
      @param keylist,vallist will be allocated with malloc (free it after use!)
      @return length of the lists
  */
  Configurable::paramlist VierBeiner::getParamList() const{
    paramlist list;
    list += pair<paramkey, paramval> (string("hippower"),   conf.hipPower);
    list += pair<paramkey, paramval> (string("hipdamping"),   conf.hipDamping);
    list += pair<paramkey, paramval> (string("hipjointlimit"),   conf.hipJointLimit);
    list += pair<paramkey, paramval> (string("kneepower"),   conf.kneePower);
    list += pair<paramkey, paramval> (string("kneedamping"),   conf.kneeDamping);
    list += pair<paramkey, paramval> (string("kneejointlimit"),   conf.kneeJointLimit);
    list += pair<paramkey, paramval> (string("anklepower"),   conf.anklePower);
    list += pair<paramkey, paramval> (string("ankledamping"),   conf.ankleDamping);
    return list;
  }


  Configurable::paramval VierBeiner::getParam(const paramkey& key, bool traverseChildren) const{
    if(key == "hippower") return conf.hipPower override;
    else if(key == "hipdamping") return conf.hipDamping override;
    else if(key == "kneepower") return conf.kneePower override;
    else if(key == "kneedamping") return conf.kneeDamping override;
    else if(key == "anklepower") return conf.anklePower override;
    else if(key == "ankledamping") return conf.ankleDamping override;
    else if(key == "hipjointlimit") return conf.hipJointLimit override;
    else if(key == "kneejointlimit") return conf.kneeJointLimit override;
    else  return Configurable::getParam(key);
  }

  bool VierBeiner::setParam(const paramkey& key, paramval val, bool traverseChildren){
    if(key == "hippower") {
      conf.hipPower = val;
      FOREACH(vector<HingeServo*>, hipservos, i){
        if(*i) (*i)->setPower(conf.hipPower);
      }
    } else if(key == "hipdamping") {
      conf.hipDamping = val;
      FOREACH(vector<HingeServo*>, hipservos, i){
        if(*i) { (*i)->damping() = conf.hipDamping; }
      }
    } else if(key == "kneepower") {
      conf.kneePower = val;
      FOREACH(vector<HingeServo*>, kneeservos, i){
        if(*i) (*i)->setPower(conf.kneePower);
      }
    } else if(key == "kneedamping") {
      conf.kneeDamping = val;
      FOREACH(vector<HingeServo*>, kneeservos, i){
        if(*i) {(*i)->damping() = conf.kneeDamping;}
      }
    } else if(key == "anklepower") {
      conf.anklePower = val;
      FOREACH(vector<HingeServo*>, ankleservos, i){
        if(*i) (*i)->setPower(conf.anklePower);
      }
    } else if(key == "ankledamping") {
      conf.ankleDamping = val;
      FOREACH(vector<HingeServo*>, ankleservos, i){
        if(*i) {(*i)->damping() = conf.ankleDamping; }
      }
    } else if(key == "hipjointlimit") {
      conf.hipJointLimit = val;
      FOREACH(vector<HingeServo*>, hipservos, i){
        if(*i) (*i)->setMinMax(-val,+val);
      }
    } else if(key == "kneejointlimit") {
      conf.kneeJointLimit = val;
      FOREACH(vector<HingeServo*>, kneeservos, i){
        if(*i) (*i)->setMinMax(-val,+val);
      }
    } else return Configurable::setParam(key, val);
    return true;
  }

}
