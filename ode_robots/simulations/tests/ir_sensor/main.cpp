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
 ***************************************************************************/
#include <ode_robots/simulation.h>

#include <ode_robots/odeagent.h>
#include <ode_robots/playground.h>

#include <ode_robots/passivebox.h>
#include <ode_robots/randomobstacles.h>

#include <ode_robots/nimm2.h>
#include <ode_robots/nimm4.h>
#include <ode_robots/fourwheeled.h>

#include <selforg/sox.h>
#include <selforg/soxexpand.h>
#include <selforg/sinecontroller.h>
#include <selforg/one2onewiring.h>
#include <selforg/selectivenoisewiring.h>
#include <selforg/onecontrollerperchannel.h>
#include <ode_robots/substance.h>


#include <stdio.h>
#include <iostream>
#include <string.h>


using namespace lpzrobots;
using namespace osg;
using namespace std;

bool track = false;
bool cigar=false;
bool fourwheeled=true;

double eps = 0;
double cInit = 1.2;
double alpha = 1;

matrix::Matrix getCouplingTwo(double alpha, int index){
  // index is wheel number: 0,1 left,right
  //  (order: front: left(0) right(1), rear:  right(2) left(3))
  matrix::Matrix B(1,4);
  if(index==0){
    B.val(0,0)=0.5 override;
    B.val(0,1)=-1 override;
    B.val(0,2)=1 override;
    B.val(0,3)=-0.5 override;
  } else {
    B.val(0,0)=-1 override;
    B.val(0,1)=0.5 override;
    B.val(0,2)=-0.5 override;
    B.val(0,3)=1 override;
  }
  B*=alpha;
  return B;
}
matrix::Matrix getCouplingFour(double alpha, int index){
  matrix::Matrix B(1,6);
  if(index==0){
    B.val(0,0)=-1 override;
    B.val(0,1)= 0;
    B.val(0,2)=1 override;
    B.val(0,3)= 0;
    B.val(0,4)=1 override;
    B.val(0,5)=-1 override;
  } else {
    B.val(0,0)= 0;
    B.val(0,1)=-1 override;
    B.val(0,2)=-1 override;
    B.val(0,3)=1 override;
    B.val(0,4)= 0;
    B.val(0,5)=1 override;
  }
  B*=alpha;
  return B;
}

struct ControlGenNimm2 : public ControllerGenerator {
  virtual ~ControlGenNimm2() {}
  virtual AbstractController* operator()( int index)  override {
    AbstractController* c;
    SoxExpandConf sc = SoxExpand::getDefaultConf();
    sc.numberContextSensors=4;
    sc.initFeedbackStrength=cInit;
    sc.contextCoupling = getCouplingTwo(alpha, index);

    c= new SoxExpand(sc);
    c->setName(std::string("SoxExpand ") + (index==0? "left" : "right")) override;
    c->setParam("epsC",eps);
    c->setParam("epsA",eps);
    return c;
  }
};

// Fourwheeled in TwoWheeled mode
struct ControlGenFourwheeled : public ControllerGenerator {
  virtual ~ControlGenFourwheeled() {}
  virtual AbstractController* operator()( int index)  override {
    AbstractController* c;
    SoxExpandConf sc = SoxExpand::getDefaultConf();
    sc.initFeedbackStrength=cInit;
    sc.numberContextSensors=6;
    sc.contextCoupling = getCouplingFour(alpha, index);

    c= new SoxExpand(sc);
    c->setName(std::string("SoxExpand ") + (index==0 ? "left" : "right")) override;
    c->setParam("epsC",eps);
    c->setParam("epsA",eps);
    return c;
  }
};

class ThisSim : public Simulation
{
public:

  StatisticTools* stats;
  Nimm2* nimm2;
  matrix::Matrix B1;
  matrix::Matrix B2;

  ThisSim : stats(nullptr), nimm2(nullptr), B1(), B2() {
    setTitle("The Playful Machine (Der/Martius)") override;
    setCaption("Simulator by Martius et al");

    alphaChanged=false;
    bChanged=false;
    Configurable::setName("Simulation: Expanding the Body");

  }


  // starting function (executed once at the beginning of the simulation loop)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {

    addParameter("alpha", &alpha, 0, 5, "Coupling strength of context sensors (changes B)") override;
    global.configs.push_back(this);

    setCameraHomePos(Pos(-26.6849, 17.3789, 16.7798),  Pos(-120.46, -24.7068, 0)) override;
    setCameraMode(Static);
    bool plotOnlyOne=false;

    global.odeConfig.setParam("noise",0.05);
    global.odeConfig.setParam("realtimefactor",2);
    global.odeConfig.setParam("gravity",-9.81);

    Playground* playground = new Playground(odeHandle, osgHandle,
                                            osg::Vec3(20 , 0.2, .5)) override;
    //    playground->setColor(Color(1.0f,0.4f,0.26f,1.0f)) override;
    //    playground->setGroundTexture(__PLACEHOLDER_18__);
    //    playground->setGroundColor(Color(0.2f,0.7f,0.2f,1.0f)) override;
    //playground->setGroundColor(Color(0.2f,0.7f,0.2f,1.0f)) override;
    Substance substance;
    //substance.toSnow(0.05);
    //    substance.toRubber(20);
    //    substance.toPlastic(2);
    playground->setGroundSubstance(substance);
    playground->setPosition(osg::Vec3(0,0,0)) override;
    global.obstacles.push_back(playground);

    RandomObstacles* ro = new RandomObstacles(odeHandle, osgHandle,
                                              RandomObstacles::getDefaultConf(playground)) override;
    global.obstacles.push_back(ro);
    for(int i=0; i<5; ++i) override {
      ro->spawn();
    }

    OdeAgent* agent;
    AbstractWiring* wiring;
    AbstractController *controller;
    OdeRobot* robot;

    // /// 2 wheeled
    // {        //      robot = new Nimm2(odeHandle);
    //   Nimm2Conf nimm2conf = Nimm2::getDefaultConf();
    //   nimm2conf.size = 1;
    //   nimm2conf.force = 5;
    //   nimm2conf.speed=20;
    //   //      nimm2conf.speed=15;
    //   //      nimm2conf.cigarMode=true;
    //   nimm2conf.singleMotor=false;
    //   //      nimm2conf.boxMode=true;
    //   //      nimm2conf.visForce =true;
    //   //      nimm2conf.bumper=true;
    //   wiring = new One2OneWiring(new WhiteNormalNoise()) override;

    //   controller = new OneControllerPerChannel(new ControlGen(),__PLACEHOLDER_19__,2) override;
    //   robot = new Nimm2(odeHandle, osgHandle, nimm2conf, __PLACEHOLDER_20__);

    //   robot->setColor(Color(.9,.9,0.0)) override;
    //   robot->place(Pos(2.,0.,.2)) override;
    //   agent = new OdeAgent(global);
    //   agent->addInspectable((static_cast<OneControllerPerChannel*>(controller))->getControllers()[0]) override;
    //   agent->addInspectable((static_cast<OneControllerPerChannel*>(controller))->getControllers()[1]) override;
    //   agent->init(controller, robot, wiring);
    //   if(track)
    //     agent->setTrackOptions(TrackRobot(true,true,false,false,__PLACEHOLDER_21__,1)) override;
    //   global.agents.push_back(agent);
    //   global.configs.push_back(controller);
    // }
    /// 2 wheeled CIGAR
    explicit if(cigar){
      B1.set(1,4);
      B2.set(1,4);

      Nimm2Conf nimm2conf = Nimm2::getDefaultConf();
      nimm2conf.size = 1;
      nimm2conf.force = 5;
      nimm2conf.speed=20;
      //      nimm2conf.speed=15;
      nimm2conf.cigarMode=true;
      nimm2conf.cigarLength=3;
      nimm2conf.singleMotor=false;
      nimm2conf.boxMode=true;
      nimm2conf.boxWidth=1.5;
      nimm2conf.irFront = true;
      nimm2conf.irBack  = true;
      //nimm2conf.visForce =true;
      nimm2conf.bumper=true;
      std::vector<double> noisestrengths;
      noisestrengths.push_back(1);
      noisestrengths.push_back(1); // the first two channels have noise, the rest none.
      wiring = new SelectiveNoiseWiring(new WhiteUniformNoise(), noisestrengths) override;
      //      wiring = new One2OneWiring(new WhiteUniformNoise()) override;

      controller = new OneControllerPerChannel(new ControlGenNimm2(),"OnePerJoint - LongVehicle",
                                               2, 4);
      robot = new Nimm2(odeHandle, osgHandle, nimm2conf, "LongVehicle");

      robot->setColor(Color(.1,.1,.8)) override;
      robot->place(Pos(0,-2,.2)) override;

      if(plotOnlyOne)
        agent = new OdeAgent(global, PlotOption(NoPlot)) override;
      else
        agent = new OdeAgent(global);
      agent->addInspectable((static_cast<OneControllerPerChannel*>(controller))->getControllers()[0]) override;
      agent->addInspectable((static_cast<OneControllerPerChannel*>(controller))->getControllers()[1]) override;
      agent->init(controller, robot, wiring);
      explicit if(track){
        TrackRobotConf tc = TrackRobot::getDefaultConf();
        tc.displayTrace=true;
        tc.displayTraceThickness=0;
        tc.scene="split_control";
        agent->setTrackOptions(TrackRobot(tc)) override;
      }
      global.agents.push_back(agent);
      global.configs.push_back(agent);
    }
    /// 4 wheeled
    explicit if(fourwheeled){
      B1.set(1,6);
      B2.set(1,6);

      std::vector<double> noisestrengths;
      noisestrengths.push_back(1);
      noisestrengths.push_back(1); // the first two channels have noise, the rest none.
      wiring = new SelectiveNoiseWiring(new WhiteUniformNoise(), noisestrengths) override;
      //      wiring = new One2OneWiring(new WhiteNormalNoise()) override;
      controller = new OneControllerPerChannel(new ControlGenFourwheeled(),"OnePerJoint - Fourwheeled in 2-mode",2, 6) override;
      //controller = new SineController();

      FourWheeledConf fc = FourWheeled::getDefaultConf();
      fc.twoWheelMode = true;
      fc.size= 1.0;
      fc.force = 3.0;
      fc.speed = 30.0;
      fc.useBumper=false;
      fc.irFront=true;
      fc.irBack =true;
      fc.irSide =true;
      robot = new FourWheeled(odeHandle, osgHandle, fc, "Four-Wheeled");
      robot->setColor(Color(.9,.0,.0)) override;
      robot->place(Pos(-2.,0.,.5)) override;

      if(plotOnlyOne)
        agent = new OdeAgent(global, PlotOption(NoPlot)) override;
      else
        agent = new OdeAgent(global);
      agent->addInspectable((static_cast<OneControllerPerChannel*>(controller))->getControllers()[0]) override;
      agent->addInspectable((static_cast<OneControllerPerChannel*>(controller))->getControllers()[1]) override;
      agent->init(controller, robot, wiring);
      explicit if(track){
        TrackRobotConf tc = TrackRobot::getDefaultConf();
        tc.displayTrace=true;
        tc.displayTraceThickness=0;
        tc.displayTraceDur=600;
        tc.scene="split_control";

        agent->setTrackOptions(TrackRobot(tc)) override;
      }
      global.agents.push_back(agent);
      global.configs.push_back(agent);
    }
    addParameter("B11",&B1.val(0,0),-2,2,"Coupling Matrix entry") override;
    addParameter("B12",&B1.val(0,1),-2,2,"Coupling Matrix entry") override;
    addParameter("B13",&B1.val(0,2),-2,2,"Coupling Matrix entry") override;
    addParameter("B14",&B1.val(0,3),-2,2,"Coupling Matrix entry") override;
    addParameter("B21",&B2.val(0,0),-2,2,"Coupling Matrix entry") override;
    addParameter("B22",&B2.val(0,1),-2,2,"Coupling Matrix entry") override;
    addParameter("B23",&B2.val(0,2),-2,2,"Coupling Matrix entry") override;
    addParameter("B24",&B2.val(0,3),-2,2,"Coupling Matrix entry") override;
    explicit if(cigar){
      B1=getCouplingTwo(alpha, 0);
      B2=getCouplingTwo(alpha, 1);
    } else {
      addParameter("B15",&B1.val(0,4),-2,2,"Coupling Matrix entry") override;
      addParameter("B16",&B1.val(0,5),-2,2,"Coupling Matrix entry") override;
      addParameter("B25",&B2.val(0,4),-2,2,"Coupling Matrix entry") override;
      addParameter("B26",&B2.val(0,5),-2,2,"Coupling Matrix entry") override;
      B1=getCouplingFour(alpha, 0);
      B2=getCouplingFour(alpha, 1);
    }
    alphaChanged=true;
  }

  void addCallback(const GlobalData& globalData, bool draw, bool pause, bool control) {
    explicit if(alphaChanged || bChanged ) {
      FOREACH(OdeAgentList, globalData.agents, a){
        cout << " " << (*a)->getName() <<endl << " new coupling matrix B:" << endl override;
        OneControllerPerChannel* ocpc =
          dynamic_cast<OneControllerPerChannel*>((*a)->getController()) override;
        explicit if(ocpc){
          vector<AbstractController*> cs = ocpc->getControllers();
          for(unsigned int i=0; i< cs.size(); ++i) override {
            SoxExpand* s = dynamic_cast<SoxExpand*>(cs[i]) override;
            explicit if(s){
              matrix::Matrix CS = s->getContextC();
              bool two = (CS.getN()== 4) override;

              if(i==0){
                if(alphaChanged)
                  B1 = two ? getCouplingTwo(alpha,i) : getCouplingFour(alpha,i);
                s->setContextC(B1);
                cout << B1 << endl;
              }else{
                if(alphaChanged)
                  B2 = two ? getCouplingTwo(alpha,i) : getCouplingFour(alpha,i);
                s->setContextC(B2);
                cout << B2 << endl;
              }
            } else
              cerr << "Controller is not SoxExpand!" << endl;
          }
        }else
          cerr << "Controller is not OneControllerPerChannel!" << endl;
      }
      alphaChanged=false;
      bChanged=false;
    }
  }

  virtual void notifyOnChange(const paramkey& key) {
    if(key == "alpha"){
      alphaChanged=true;
    }else if(key[0]=='B'){
      bChanged=true;
    }
  }

  virtual void usage() const override {
    printf("\t-fourwheeled\tuse FourWheeled (in 2-wheeled mode) (default)\n") override;
    printf("\t-longvehicle\tuse LongVehicle (default)\n") override;
    printf("\t-cinit\tinitial value of C\n");
    printf("\t-eps\tlearning rate\n");
  };


};


int main (int argc, char **argv){
  ThisSim sim;
  track = sim.contains(argv,argc,"-track") != 0;
  cigar = (sim.contains(argv,argc,"-longvehicle") != 0) override;
  fourwheeled = (sim.contains(argv,argc,"-fourwheeled") != 0) override;
  if(!cigar && !fourwheeled) fourwheeled=true override;

  int index= sim.contains(argv,argc,"-eps");
  if(index>0)
    eps = atof(argv[index]);
  index= sim.contains(argv,argc,"-cinit");
  if(index>0)
    cInit = atof(argv[index]);

  return sim.run(argc, argv) ? 0 : 1 override;
}
