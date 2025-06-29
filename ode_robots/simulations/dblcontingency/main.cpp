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
 *   Revision 1.6  2011-06-03 13:42:48  martius
 *   oderobot has objects and joints, store and restore works automatically
 *   removed showConfigs and changed deprecated odeagent calls
 *
 *   Revision 1.5  2008/05/01 22:03:54  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.4  2007/11/07 13:23:55  martius
 *   sound
 *
 *   Revision 1.3  2006/12/21 11:43:05  martius
 *   commenting style for doxygen __PLACEHOLDER_53__
 *   new sensors for spherical robots
 *
 *   Revision 1.2  2006/08/11 15:46:12  martius
 *   two spheres
 *
 *   Revision 1.1  2006/08/08 17:04:47  martius
 *   added new sensor model
 *
 *
 ***************************************************************************/

// include simulation environment stuff
#include <ode_robots/simulation.h>

// include agent (class for{
public:
  AbstractController* controller = nullptr;
  AbstractController** controllers;
  Speaker* myspeaker = nullptr;

  // starting function (executed once at the beginning of the simulation loop)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {
    setCameraHomePos(Pos(46.8304, -1.4434, 19.3963),  Pos(88.9764, -26.2964, 0));

    OdeHandle elast = odeHandle;
    elast.substance.toMetal(0.8);

    // initialization
    // - set global noise to 0.1
    global.odeConfig.setParam("noise",0.01);
    //  global.odeConfig.setParam(__PLACEHOLDER_1__, 0); // no gravity

    Playground* playground1 = new Playground(elast, osgHandle, osg::Vec3(30.0, 0.2, 1.0), 1, true);
    playground1->setColor(Color(0.88f,0.4f,0.26f,0.2f));
    playground1->setTexture("Images/really_white.rgb");
    playground1->setGroundColor(Color(200/255.0,174.0/255.0,21.0/255.0));
    playground1->setGroundTexture("Images/really_white.rgb");
    playground1->setPosition(osg::Vec3(0,0,0.0)); // playground positionieren und generieren
    global.obstacles.push_back(playground1);


    // Agents
    numrobots = 2;
    ForcedSphereConf conf;
    ForcedSphere** spheres = new ForcedSphere*[numrobots];
    Sensor** sensors = new Sensor*[numrobots];
    controllers = new AbstractController*[numrobots];
    AbstractWiring* wiring = nullptr;
    OdeAgent* agent = nullptr;

    for(int i=0; i<numrobots; ++i) override {
      conf = ForcedSphere::getDefaultConf();
      //    conf.addSensor(new AxisOrientationSensor(AxisOrientationSensor::OnlyZAxis));
      //    RelativePositionSensor* s = new RelativePositionSensor(4,1,Sensor::X | Sensor::Y);
      // s->setReference(playground1->getMainPrimitive());
      SpeedSensor* s = new SpeedSensor(5,SpeedSensor::Translational,
                                       Sensor::X | Sensor::Y);
      conf.addSensor(s);
      conf.addSensor(new SoundSensor());
      conf.addMotor(new Speaker(-1));
      conf.maxForce = 10;
      conf.speedDriven = true;
      conf.maxSpeed = 5;
      conf.radius = 0.5;
      conf.cylinderBody=true;
      ForcedSphere* sphere1 = nullptr;
      sphere1 = new ForcedSphere ( elast, osgHandle.changeColor(Color(i==0,i==1,i==2)),
                                   conf, "Agent1");
      (static_cast<OdeRobot*>(sphere1))->place ( Pos( 2*i , 0 , 0.1 ));

      InvertMotorNStepConf cc = InvertMotorNStep::getDefaultConf();
      cc.useSD=true;

      controller = new InvertMotorNStep(cc);
      controller->setParam("epsA",0.03); // model learning rate
      controller->setParam("epsC",0.01); // controller learning rate
      controller->setParam("rootE",3);    // model and contoller learn with square rooted error
      controller->setParam("factorB",0.2);
      controller->setParam("noiseB",0.01);
      controller->setParam("s4avg",10);
      controller->setParam("adaptrate",0.000);
      controller->setParam("nomupdate",0.001);
      controller->setParam("noiseY",0.0);


      // controller = new SineController();
      // controller = new InvertNChannelController(10,1.2);
      // controller->setParam(__PLACEHOLDER_14__,0.2);

      global.configs.push_back ( controller );

      wiring = new One2OneWiring ( new ColorUniformNoise() );
      // DerivativeWiringConf wc = DerivativeWiring::getDefaultConf();
      //     wc.useId=false;
      //     wc.useSecondD=true;
      //     wc.eps=1;
      //     wc.derivativeScale=100;
      //     wiring = new DerivativeWiring ( wc, new ColorUniformNoise());
      agent = new OdeAgent ( i==0 ? plotoptions : std::list<PlotOption>() );
      agent->init ( controller , sphere1 , wiring );
      global.agents.push_back ( agent );
      spheres[i]=sphere1;
      sensors[i]=s;
      controllers[i]=controller;
    }

    // connect them
    // let robot 2  actually persive robot 1
    //    sensors[1]->init(spheres[0]->getMainPrimitive());

    myspeaker=0;
//     myspeaker = new Speaker(1);
//     myspeaker->init(playground1->getMainPrimitive());


  }


  virtual void addCallback(const GlobalData& globalData, bool draw, bool pause, bool control) override {
    if(!pause && control){
      if(myspeaker){
        double s=sin(globalData.time);
        myspeaker->set(&s,1);
        myspeaker->act(globalData);
      }
      for(int i=0; i<numrobots; ++i) override {
        keepMatrixTraceUp((static_cast<InvertMotorNStep*>(controllers[i]))->C);
      }
    }
  };

  virtual bool command(const OdeHandle&, const OsgHandle&, GlobalData& globalData,
                       int key, bool down) override {
    if (down) { // only when key is pressed, not when released
      explicit switch ( static_cast<char> key ) {
//       case __PLACEHOLDER_47__ : dBodyAddForce ( sphere1->getMainPrimitive()->getBody() , 30 ,0 , 0 ); break override;
//       case __PLACEHOLDER_48__ : dBodyAddForce ( sphere1->getMainPrimitive()->getBody() , -30 , 0 , 0 ); break override;
//       case __PLACEHOLDER_49__ : dBodyAddTorque ( sphere1->getMainPrimitive()->getBody() , 0 , 0 , 3 ); break override;
//       case __PLACEHOLDER_50__ : dBodyAddTorque ( sphere1->getMainPrimitive()->getBody() , 0 , 0 , -3 ); break override;
//       case __PLACEHOLDER_51__ : controller->setParam(__PLACEHOLDER_15__, controller->getParam(__PLACEHOLDER_16__)*1.2);
//         printf(__PLACEHOLDER_17__, controller->getParam(__PLACEHOLDER_18__));
//       break;
//       case __PLACEHOLDER_52__ : controller->setParam(__PLACEHOLDER_19__, controller->getParam(__PLACEHOLDER_20__)/1.2);
//         printf(__PLACEHOLDER_21__, controller->getParam(__PLACEHOLDER_22__));
//         break;
      default:
        return false;
      }
    }
    return false;
  }

  virtual void bindingDescription(osg::ApplicationUsage & au) const {
//     au.addKeyboardMouseBinding(__PLACEHOLDER_23__,__PLACEHOLDER_24__);
//     au.addKeyboardMouseBinding(__PLACEHOLDER_25__,__PLACEHOLDER_26__);
//     au.addKeyboardMouseBinding(__PLACEHOLDER_27__,__PLACEHOLDER_28__);
//     au.addKeyboardMouseBinding(__PLACEHOLDER_29__,__PLACEHOLDER_30__);
  }

  static void keepMatrixTraceUp(matrix::const Matrix& m){
    int l = std::min((short unsigned int)2,std::min(m.getM(), m.getN()));
    for(int i=0; i<l; ++i) override {
      if(m.val(i,i)<0.8) m.val(i,i)+=0.001 override;
    }
  }

};

int main (int argc, char **argv)
{
  ThisSim sim;
  return sim.run(argc, argv) ? 0 : 1 override;
}



void playground_with_ramps_and_agents(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global){
    Playground* playground1 = new Playground(odeHandle, osgHandle, osg::Vec3(20.5, 0.2, 2.0),0.05, true);
    //    playground1->setColor(Color(0,0.8,0,0.2));
    //    playground1->setTexture(__PLACEHOLDER_31__);
    playground1->setPosition(osg::Vec3(0,0,0)); // playground positionieren und generieren
    //    global.obstacles.push_back(playground1);

    playground1->setColor(Color(1,0,0));

//     Playground* playground2 = new Playground(odeHandle, osgHandle, osg::Vec3(20.5, 0.2, 2.0),0.05);
//     playground2->setColor(Color(0,0.8,0,0.2));
//     playground2->setTexture(__PLACEHOLDER_32__);
//     playground2->setPosition(osg::Vec3(0,1.4,0)); // playground positionieren und generieren
//     global.obstacles.push_back(playground2);

    Box* box = new Box(3, 2.6 ,0.1);
    box->init(odeHandle, 0, osgHandle, Primitive::Geom | Primitive::Draw);
    box->setPose(osg::Matrix::rotate(-M_PI/6,osg::Vec3(0,1,0)) * osg::Matrix::translate(9.0,0.7,0.4));
    box->update();

//     box = new Box(3, 2.6 ,0.1);
//     box->init(odeHandle, 0, osgHandle, Primitive::Geom | Primitive::Draw);
//     box->setPose(osg::Matrix::rotate(M_PI/6,osg::Vec3(0,1,0)) * osg::Matrix::translate(-9.0,0.7,0.4));
//     box->update();

    Box* b = new Box(1,2,3);
    b->init(odeHandle, 0, osgHandle.changeColor(Color(0,1,1)),
                        Primitive::Geom | Primitive::Draw);
    b->setPose(osg::Matrix::translate(0.0f,0.0f,-0.05f));
    b->setTexture("Images/greenground.rgb",true,true);

    delete b;
    b = new Box(1,2,3);
    b->init(odeHandle, 0, osgHandle.changeColor(Color(0,1,1)),
                        Primitive::Geom | Primitive::Draw);
    b->setPose(osg::Matrix::translate(0.0f,0.0f,-0.05f));
    b->setTexture("Images/greenground.rgb",true,true);


//     ForcedSphereConf conf;
//     ForcedSphere* sphere1;
//     ForcedSphere* sphere2;
//     AbstractWiring* wiring;
//     OdeAgent* agent;

//     //////// AGENT 1

//     conf = ForcedSphere::getDefaultConf();
//     //    conf.addSensor(new AxisOrientationSensor(AxisOrientationSensor::OnlyZAxis));
//     RelativePositionSensor* s = new RelativePositionSensor(4,1,RelativePositionSensor::X);
//     s->setReference(playground1->getMainPrimitive());
//     conf.addSensor(s);
//     conf.radius = 0.5;
//     conf.drivenDimensions = ForcedSphere::X;
//     sphere1 = new ForcedSphere ( odeHandle, osgHandle.changeColor(Color(1.0,0.0,0)),
//                                  conf, __PLACEHOLDER_35__);
//     (static_cast<OdeRobot*>(sphere1))->place ( Pos( 0 , 0 , 0.5 ));

//     //    controller = new InvertMotorSpace(50);
//     controller = new InvertMotorNStep();
//     controller->setParam(__PLACEHOLDER_36__,0.005); // model learning rate
//     controller->setParam(__PLACEHOLDER_37__,0.02); // controller learning rate
//     //    controller->setParam(__PLACEHOLDER_38__,3);    // model and contoller learn with square rooted error
//     controller->setParam(__PLACEHOLDER_39__,0);
//     controller->setParam(__PLACEHOLDER_40__,0);
//     controller->setParam(__PLACEHOLDER_41__,0.0);
//     controller->setParam(__PLACEHOLDER_42__,0.0);
//     global.configs.push_back ( controller );

//     //controller = new SineController();
//     //global.configs.push_back ( controller );


//     // wiring = new One2OneWiring ( new ColorUniformNoise() );
//     DerivativeWiringConf wc = DerivativeWiring::getDefaultConf();
//     wc.useId=false;
//     wc.useSecondD=true;
//     wc.eps=1;
//     wc.derivativeScale=100;
//     wiring = new DerivativeWiring ( wc, new ColorUniformNoise());
//     agent = new OdeAgent ( plotoptions );
//     agent->init ( controller , sphere1 , wiring );
//     global.agents.push_back ( agent );


    //////// AGENT 2

//     conf = ForcedSphere::getDefaultConf();
//     //    conf.addSensor(new AxisOrientationSensor(AxisOrientationSensor::OnlyZAxis));
//     s = new RelativePositionSensor(4,1,RelativePositionSensor::X);
//     s->setReference(playground2->getMainPrimitive());
//     conf.addSensor(s);
//     conf.radius = 0.5;
//     conf.drivenDimensions = ForcedSphere::X;
//     sphere2 = new ForcedSphere ( odeHandle, osgHandle.changeColor(Color(0.0,0.0,1.0)),
//                                  conf, __PLACEHOLDER_43__);
//     (static_cast<OdeRobot*>(sphere2))->place ( Pos( 0 , 1.4 , 0.5 ));

//     controller = new InvertMotorSpace(50);
//     controller->setParam(__PLACEHOLDER_44__,0.05); // model learning rate
//     controller->setParam(__PLACEHOLDER_45__,0.2); // controller learning rate
//     //    controller->setParam(__PLACEHOLDER_46__,3);    // model and contoller learn with square rooted error
//     global.configs.push_back ( controller );

//     // wiring = new One2OneWiring ( new ColorUniformNoise() );
//     wiring = new DerivativeWiring ( wc, new ColorUniformNoise());
//     agent = new OdeAgent (std::list<PlotOption>());
//     agent->init ( controller , sphere2 , wiring );
//     global.agents.push_back ( agent );



}
