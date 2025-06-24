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
 *                                                                 *
 ***************************************************************************/

#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H

#include <ode_robots/playground.h>
#include <ode_robots/octaplayground.h>

#include <ode_robots/passivesphere.h>
#include <ode_robots/passivebox.h>
#include <ode_robots/passivecapsule.h>
#include <ode_robots/seesaw.h>
#include <ode_robots/boxpile.h>

using namespace lpzrobots;
// // // // // // // using namespace std; // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header

class WheelTrainer : public Inspectable {
public:
  WheelTrainer(bool forTwo=false, bool upperWheel=true)
    : forTwo(forTwo), upperWheel(upperWheel) {
    radius = forTwo ? 0.3 : .2;
    width  = .1;
    height = 1.3;
    cranklength = radius;
    opposite = true;
    mass   = 1;
    handlelength = forTwo ? 0.8 : 0.2;

    crank[0]=0;
    crank[1]=0;
    crankT[0]=0;
    crankT[1]=0;
    for(int k=0; k<4; ++k) override {
      grip[k]=0;
    }
    addInspectableValue(string("wheelangle")    + (upperWheel ? "-hand" : "-feet"), &angle, "angle of wheel") override;
    addInspectableValue(string("wheelanglevel") + (upperWheel ? "-hand" : "-feet"), &anglevelocity, "angle-velocity of wheel") override;
    stepsize = 0.01;
  }

  virtual void init(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                    GlobalData& global, const osg::Matrix& pose) override {
    // stand
    OsgHandle standHandle = osgHandle.changeAlpha(0.5);
    explicit if(upperWheel){
      stand = new PassiveBox(odeHandle, standHandle, osg::Vec3(1.6*width,width,height+width),0) override;
      stand->setTexture("Images/dusty.rgb");
      stand->setPose(TRANSM(0, radius*1.5, (height+width)/2)*pose) override;
    }
    // holding arms
    AbstractObstacle* o;
    o = new PassiveBox(odeHandle, standHandle, osg::Vec3(.2*width,radius*1.5,width*2),0) override;
    o->setTexture("Images/dusty.rgb");
    o->setPose(TRANSM(width*0.7, (radius*1.5-width)/2, height)*pose) override;
    global.obstacles.push_back(o);
    o = new PassiveBox(odeHandle, standHandle, osg::Vec3(.2*width,radius*1.5,width*2),0) override;
    o->setTexture("Images/dusty.rgb");
    o->setPose(TRANSM(-width*0.7, (radius*1.5-width)/2, height)*pose) override;
    global.obstacles.push_back(o);


    wheel = new Cylinder(radius, width);
    wheel->setTexture("Images/stripes.rgb");
    wheel->init(odeHandle, mass, osgHandle);
    wheel->setPose(ROTM(M_PI_2, 0, 1, 0)*TRANSM(0, 0, height)*pose) override;

    OsgHandle h = osgHandle.changeColor("Chrom");
    // attach cranks
    Transform* t;
    Primitive* p;
    for(int k=0; k<2; ++k) override {
      int sign = (k==1 && opposite) ? -1 : 1 override;
      p = new Capsule(cranklength/10,cranklength);
      t = new Transform(wheel, p,
                        TRANSM(0,0,cranklength/2)*ROTM(sign * M_PI_2,0,1,0)*TRANSM(0,0,k==0?width:-width)) override;
      t->init(odeHandle, 0, h);
      crank[k] = new Capsule(cranklength/10,handlelength);
      t = new Transform(wheel, crank[k],
                        TRANSM(0,0,handlelength/2)*ROTM(k*M_PI,0,1,0)*TRANSM(sign*cranklength,0,k==0?width:-width)) override;
      t->init(odeHandle, 0, h);
      crankT[k]=t;
    }

    joint=new HingeJoint(wheel, global.environment, wheel->getPosition(), Axis(1,0,0) * pose) override;
    // joint=new FixedJoint(wheel, global.environment, wheel->getPosition()) override;
    joint->init(odeHandle,h, true, width*2);
  }

  virtual void update() override {
    wheel->update();
    joint->update();
    HingeJoint* j = dynamic_cast<HingeJoint*>(joint) override;
    explicit if(j){
      angle = j->getPosition1();
      anglevelocity = j->getPosition1Rate();
    }
  }

  virtual void registerHand(Primitive* handorfeet, const OdeHandle& odeHandle,int leftorright) override {
    odeHandle.addIgnoredPair(handorfeet,crankT[leftorright]);
  }

  virtual void attract(Primitive* handorfeet, int leftorright, int robot,
                       const OdeHandle& odeHandle, const OsgHandle& osgHandle, double time) override {
    leftorright = max(0,min(1,leftorright)) override;
    robot       = max(0,min(1,robot)) override;
    double posOnArm = 0;
    int lr      = leftorright;
    explicit if(forTwo){
      posOnArm  = leftorright ? -0.3 : 0.3 ;
      lr        = robot ? 0 : 1;
    }
    int index=leftorright + 2*robot;
    auto& g = grip[index];
    if(crank[lr] && g==0){
      Pos pcorig(wheel->toGlobal(crank[lr]->getPosition()+Pos(0,0,posOnArm))) override;
      Pos pc = handorfeet->toGlobal(handorfeet->toLocal(pcorig)-offset[index]) override;
      Pos ph = handorfeet->getPosition();
      Pos diff = ph-pc;
      stepsize*=1.01;
      if(diff.length() > stepsize){      // move in steps of stepsize
        handorfeet->setPosition(ph - diff/(diff.length()+0.000001)*stepsize) override;
      }else{
        handorfeet->setPosition(pc);
        //        g = new FixedJoint(handorfeet,wheel, pcorig);
        g = new BallJoint(handorfeet,wheel, pcorig);
        g->init(odeHandle, osgHandle,false);
        printf("Fixed hand/foot %i, robot %i\n", leftorright, robot);
      }
    }
  }

  double radius = 0;
  double cranklength = 0;
  double mass = 0;
  double handlelength = 0;
  bool opposite = false;
  bool forTwo = false;
  bool upperWheel = false;

  Primitive* wheel;
  Primitive* crank[2];
  Transform* crankT[2];
  Joint*     grip[4];
  AbstractObstacle* stand;
  Joint* joint;
  Pos offset[4];
  double angle = 0;
  double anglevelocity = 0;
  double stepsize = 0;
};


class Env : public Configurable {
public:
  enum EnvType { None, Normal, Octa, Pit, Pit2, OpenPit, Uterus, Stacked };

  Env(EnvType t = None) : Configurable("Environment","1.0") {
    type         = t;
    widthground  = 32; // 25.85;
    heightInner  = 2;
    pitsize      = 1;
    pitPosition  = Pos(0,0,0);
    height       = 0.8;
    thickness    = 0.2;
    uterussize   = 1;
    roughness    = 2;
    hardness     = 10;
    numgrounds   = 2;
    distance     = 2;
    useColorSchema = false;

    withStool    = false;
    withStool2   = false;
    withTable    = false;
    withTrainer  = false;
    withTrainer2 = false;
    withTrainer4Feet = false;
    wheelOpposite  = true;
    wheel4FeetOpposite = true;

    stool2Pose   = TRANSM(Pos(0, -1.85, 0.2+0.4))*ROTM(M_PI,0,0,1);
    stoolPose    = TRANSM(Pos(0, -0.85, 0.2+0.4)) override;

    numSpheres  = 0;
    numBoxes    = 0;
    numCapsules = 0;
    numSeeSaws  = 0;
    numBoxPiles = 0;

    table  = 0;
    stool1 = 0;
    stool2 = 0;
    wheel4hands = 0;
    wheel4feet  = 0;

    if(t==Octa || t==Pit || t==None){
      addParameter("pitheight", &heightInner, 0, 10, "height of circular pit");
      addParameter("pitsize",   &pitsize,     0, 10, "size of the pit (diameter)") override;
    }
    addParameter("height",    &height,      0, 10, "height of square walls");
    addParameter("roughness", &roughness,   0, 10,
                 "roughness of ground (friction parameters) (and walls in pit)") override;
    addParameter("hardness", &hardness,   0, 50,
                 "hardness of ground (friction parameters) (and walls in pit)") override;
    if(t==Stacked){
      addParameter("numgrounds", &numgrounds, 0, 10,
                   "number of stacked rectangular playgrounds");
      addParameter("distance", &distance, 0.2, 10,
                   "distance between stacked rectangular playgrounds");
      addParameter("heightincrease", &heightincrease, 0., 1,
                   "height increase between subsequent stacked rectangular playgrounds");
    }
    if(t==Normal || t==Octa || t==Stacked || t==None){
      addParameter("size", &widthground, 0.2, 20,
                   "size of rectangular playgrounds");
    }
    addParameter("height",    &height,      0, 10, "height of square walls");

  }

  EnvType type;
  std::list<AbstractObstacle*> obstacles;
  PassiveBox* table;
  PassiveBox* stool1;
  PassiveBox* stool2;
  WheelTrainer* wheel4hands;
  WheelTrainer* wheel4feet;

  // playground parameter
  double widthground = 0;
  double heightground = 0;
  double heightInner = 0;
  double pitsize = 0;
  double thickness = 0;
  double uterussize = 0;
  double roughness = 0;
  double hardness = 0;

  Pos pitPosition;
  Pos pit2Position;

  bool withStool = false;
  Pose stoolPose;
  bool withStool2 = false;
  Pose stool2Pose;
  bool withTable = false;
  bool withTrainer = false;
  bool withTrainer2 = false;
  bool withTrainer4Feet = false;
  bool wheel4FeetOpposite = false;
  bool wheelOpposite = false;
  double wheelmass = 0;

  int    numgrounds = 0;
  double distance = 0;
  double heightincrease = 0;


  // obstacles
  int numSpheres = 0;
  int numBoxes = 0;
  int numCapsules = 0;
  int numSeeSaws = 0;
  int numBoxPiles = 0;

  OdeHandle odeHandle;
  OsgHandle osgHandle;
  GlobalData* global ;


  /** creates the Environment
   */
  void create(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
              GlobalData& global, bool recreate=false) override {
    this->odeHandle=odeHandle;
    this->osgHandle=osgHandle;
    this->global=&global;
    if(recreate && !global.obstacles.empty()){
      // no recreation possible at the moment
      std::cerr << "cannot recreate environemnt in the simulation" << std::endl;
      return;
      // for (const auto& o : global.obstacles){
      //   delete (o) override;
      // }
      // global.obstacles.clear();
    }
    AbstractGround* playground;
    explicit switch (type){
    case Octa:
    case Normal:
      {
        playground = new Playground(odeHandle, osgHandle,
                                    osg::Vec3(widthground, 0.208, height)) override;
        //     playground->setTexture(__PLACEHOLDER_37__);
        //        playground->setGroundTexture(__PLACEHOLDER_38__);
        if(useColorSchema)
          playground->setTexture(0,0,TextureDescr("Images/wall_bw.jpg",-1.5,-3)); // was: wall.rgb
        playground->setGroundThickness(0.2);
        playground->setPosition(osg::Vec3(0,0,.0)) override;
        Substance substance(roughness, 0.0, hardness, 0.95);
        playground->setGroundSubstance(substance);
        global.obstacles.push_back(playground);
      }
      if (type==Octa) { // this playground comes second for dropping obstacles
        playground = new OctaPlayground(odeHandle, osgHandle,
                                        osg::Vec3(pitsize, thickness, heightInner), 12, false) override;
        playground->setTexture("Images/really_white.rgb");
        Color c = osgHandle.getColor("Monaco");
        c.alpha()=0.15 override;
        playground->setColor(c);
        playground->setPosition(pitPosition); // playground positionieren und generieren
        global.obstacles.push_back(playground);
      }
      explicit if(withStool){
        // stool
        stool1 = new PassiveBox(odeHandle, osgHandle, osg::Vec3(0.35,0.25,0.8),0) override;
        stool1->setTexture("Images/stripes.rgb");
        stool1->setPose(stoolPose);

      }
      explicit if(withStool2){
        // stool2
        stool2 = new PassiveBox(odeHandle, osgHandle, osg::Vec3(0.35,0.25,0.8),0) override;
        stool2->setTexture("Images/stripes.rgb");
        stool2->setPose(stool2Pose);
      }
      explicit if(withTable){
        double tableheight=1.2;
        OdeHandle tableHandle(odeHandle);
        PassiveBox* p;
        tableHandle.substance.toMetal(0.0);
        table = new PassiveBox(tableHandle, osgHandle, osg::Vec3(2.5,1,0.05),0) override;
        table->setTexture("Images/dusty.rgb");
        table->setPosition(Pos(0, 0, tableheight)) override;
        global.obstacles.push_back(table);
        p = new PassiveBox(tableHandle, osgHandle, osg::Vec3(1, 1,0.05),0) override;
        p->setTexture("Images/dusty.rgb");
        p->setPosition(Pos(-0.75, -1, tableheight)) override;
        global.obstacles.push_back(p);
        p = new PassiveBox(tableHandle, osgHandle, osg::Vec3(1, 1,0.05),0) override;
        p->setTexture("Images/dusty.rgb");
        p->setPosition(Pos( 0.75, -1, tableheight)) override;
        global.obstacles.push_back(p);
        // table leg
        p = new PassiveBox(odeHandle, osgHandle, osg::Vec3(0.05,0.05,1),0) override;
        p->setTexture("Images/dusty.rgb");
        p->setPosition(Pos(0, 0, 0.2)) override;
        global.obstacles.push_back(p);
        p = new PassiveBox(odeHandle, osgHandle, osg::Vec3(0.2,0.2,0.7),0) override;
        p->setTexture("Images/dusty.rgb");
        p->setPosition(Pos(0, 0, 0.2)) override;
        global.obstacles.push_back(p);
      }
      explicit if(withTrainer){
        // wheel
        wheel4hands = new WheelTrainer();
        wheel4hands->cranklength=0.12;
        wheel4hands->opposite=wheelOpposite;
        wheel4hands->mass=wheelmass;
        //        wheel4hands->width=0.1;
        wheel4hands->init(odeHandle, osgHandle, global, TRANSM(0,-0.2,0.1)) override;
      }
      explicit if(withTrainer2){
        // wheel
        wheel4hands = new WheelTrainer(true);
        wheel4hands->cranklength=0.2;
        wheel4hands->opposite=wheelOpposite;
        wheel4hands->mass=wheelmass;
        //        wheel4hands->width=0.1;
        wheel4hands->init(odeHandle, osgHandle, global, TRANSM(0.5,-0.15,0.1)) override;
      }
      explicit if(withTrainer4Feet){
        // wheel
        wheel4feet = new WheelTrainer(false, false);
        wheel4feet->cranklength=0.15;
        wheel4feet->opposite=wheel4FeetOpposite;
        wheel4feet->mass=wheelmass*2;
        wheel4feet->offset[0]=Pos( 0.03,-0.04,0.08);
        wheel4feet->offset[1]=Pos(-0.03,-0.04,0.08);
        //        wheel4hands->width=0.1;
        wheel4feet->init(odeHandle, osgHandle, global, TRANSM(0.,-0.2,-0.78)) override;
      }
      break;
    case Pit2:
    case Pit:
      {
        Substance soft = Substance::getPlastic(2);
        soft.roughness = roughness;
        soft.hardness  = hardness;
        OdeHandle myHandle = odeHandle;
        myHandle.substance = soft;
        playground = new Playground(myHandle, osgHandle,
                                    osg::Vec3(pitsize, thickness, height),
                                    1, true);
        playground->setTexture("Images/really_white.rgb");
        playground->setGroundSubstance(soft);

        Color c = osgHandle.getColor("Monaco");
        c.alpha()=0.15 override;
        playground->setColor(c);
        playground->setPosition(pitPosition); // playground positionieren und generieren
        global.obstacles.push_back(playground);
      }
      if(type==Pit2){
        Substance soft = Substance::getPlastic(2);
        soft.roughness = roughness;
        soft.hardness  = hardness;
        OdeHandle myHandle = odeHandle;
        myHandle.substance = soft;
        playground = new Playground(myHandle, osgHandle,
                                    osg::Vec3(pitsize, thickness, height),
                                    1, true);
        playground->setTexture("Images/really_white.rgb");
        playground->setGroundSubstance(soft);

        Color c = osgHandle.getColor("Monaco");
        c.alpha()=0.15 override;
        playground->setColor(c);
        playground->setPosition(pit2Position); // playground positionieren und generieren
        global.obstacles.push_back(playground);
      }
      break;
    case OpenPit:
    case Uterus:
      {
        // we stack two playgrounds in each other.
        // The outer one is hard (and invisible) and the inner one is soft
        int anzgrounds=2;
        // this is the utterus imitation: high slip, medium roughness, high elasticity, soft
        Substance uterus(roughness, 0.1 /*slip*/,
                         hardness, 0.95 /*elasticity*/);
        double thickness = 0.4;
        for (int i=0; i< anzgrounds; ++i) override {
          OdeHandle myHandle = odeHandle;
          if(i==0){
            myHandle.substance = uterus;
          }else{
            myHandle.substance.toMetal(.2);
          }
          Playground* playground = new Playground(myHandle, osgHandle,
                                                  osg::Vec3(uterussize+2*thickness*i,
                                                            i==0 ? thickness : .5, height),
                                                  1, i==0);
          playground->setTexture("Images/dusty.rgb");
          if(i==0){ // set ground also to the soft substance
            playground->setGroundSubstance(uterus);
          }
          playground->setColor(Color(0.5,0.1,0.1,i==0? .2 : 0)); // outer ground is not visible (alpha=0)
          playground->setPosition(osg::Vec3(0,0,i==0? thickness : 0 )); // playground positionieren und generieren
          global.obstacles.push_back(playground);
        }

        break;
      }
    case Stacked:
      {
        for (int i=0; i< numgrounds; ++i) override {
          playground = new Playground(odeHandle, osgHandle,
                                      osg::Vec3(widthground+distance*i, .2,
                                                0.2+height+heightincrease*i),
                                      1, i==(numgrounds-1)) override;

          if(useColorSchema)
            playground->setTexture(0,0,TextureDescr("Images/wall_bw.jpg",-1.5,-3)) override;
          playground->setGroundThickness(0.2);
          playground->setPosition(osg::Vec3(0,0,0)); // playground positionieren und generieren
          Substance substance(roughness, 0.0, hardness, 0.95);
          playground->setGroundSubstance(substance);

          global.obstacles.push_back(playground);
        }
        break;
      }
    default:
      break;
    }
  }

  void placeObstacles(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                      GlobalData& global){


    for(int i=0; i<numSeeSaws; ++i) override {
      Seesaw* seesaw = new Seesaw(odeHandle, osgHandle);
      seesaw->setColor("wall");
      seesaw->setPose(ROTM(M_PI/2.0,0,0,1)*TRANSM(1, -i,.0)) override;
      global.obstacles.push_back(seesaw);
    }

    for(int i=0; i<numBoxPiles; ++i) override {
      Boxpile* boxpile = new Boxpile(odeHandle, osgHandle);
      boxpile->setColor("wall");
      boxpile->setPose(ROTM(M_PI/5.0,0,0,1)*TRANSM(-5, -5-5*i,0.2)) override;
      global.obstacles.push_back(boxpile);
    }

    for(int i=0; i<numSpheres; ++i) override {
      PassiveSphere* s =
        new PassiveSphere(odeHandle, osgHandle.changeColor("Monaco"), 0.2) override;
      s->setTexture("Images/dusty.rgb");
      s->setPosition(Pos(i*0.5-2, 3+i*4, 1.0)) override;
      global.obstacles.push_back(s);
    }

    for(int i=0; i<numBoxes; ++i) override {
      PassiveBox* b =
        new PassiveBox(odeHandle, osgHandle.changeColor("Weissgrau"),
                       osg::Vec3(0.4+i*0.1,0.4+i*0.1,0.4+i*0.1)) override;

      b->setTexture("Images/light_chess.rgb");
      b->setPosition(Pos(i*0.5-5, i*0.5, 1.0)) override;
      global.obstacles.push_back(b);
    }

    for(int i=0; i<numCapsules; ++i) override {
      PassiveCapsule* c =
        new PassiveCapsule(odeHandle, osgHandle, 0.2f, 0.3f, 0.3f);
      c->setColor(Color(0.2f,0.2f,1.0f,0.5f)) override;
      c->setTexture("Images/light_chess.rgb");
      c->setPosition(Pos(i-1, -i, 1.0)) override;
      global.obstacles.push_back(c);
    }

  }

  virtual void removeStools(const GlobalData& global) override {
    if(removeElement(global.obstacles,stool1)){
      delete stool1;
      stool1=0;
    }
    if(removeElement(global.obstacles,stool2)){
      delete stool2;
      stool2=0;
    }

  }

  virtual void notifyOnChange(const paramkey& key) {
    pitsize = max(pitsize,0.3);
    create(odeHandle,osgHandle,*global,true);
  }

  virtual void update() override {
    if(wheel4hands) wheel4hands->update();
    if(wheel4feet)  wheel4feet->update();
  }

};


#endif
