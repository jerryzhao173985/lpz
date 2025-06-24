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
 *   Revision 1.9  2011-10-14 09:36:18  martius
 *   snakes have no frictionGround parameter anymore, since it was not used,
 *    use the substances now
 *
 *   Revision 1.8  2011/06/03 13:42:48  martius
 *   oderobot has objects and joints, store and restore works automatically
 *   removed showConfigs and changed deprecated odeagent calls
 *
 *   Revision 1.7  2011/03/19 08:47:55  guettler
 *   - unique names are generated even for e.g. spheres, snakes
 *
 *   Revision 1.6  2010/01/26 09:58:15  martius
 *   changed a lot parameter
 *
 *   Revision 1.5  2009/03/25 15:44:23  guettler
 *   ParallelSplitShadowMap: corrected light direction (using directional light), complete ground is now shadowed
 *
 *   Revision 1.4  2008/05/01 22:03:56  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.3  2007/01/26 12:07:09  martius
 *   orientationsensor added
 *
 *   Revision 1.2  2006/09/21 22:11:33  martius
 *   make opt fixed
 *
 *   Revision 1.1  2006/09/20 12:56:40  martius
 *   *** empty log message ***
 *
 *
 ***************************************************************************/
#include <ode_robots/simulation.h>

#include <ode_robots/odeagent.h>
#include <ode_robots/playground.h>
#include <ode_robots/terrainground.h>

#include <ode_robots/passivesphere.h>
#include <ode_robots/passivebox.h>
#include <ode_robots/passivecapsule.h>

#include <selforg/invertnchannelcontroller.h>
#include <selforg/dercontroller.h>
#include <selforg/invertmotornstep.h>
#include <selforg/invertmotorspace.h>
#include <selforg/sinecontroller.h>
#include <selforg/noisegenerator.h>
#include <selforg/one2onewiring.h>
#include <selforg/derivativewiring.h>

#include <osg/Light>
#include <osg/LightSource>


#include <ode_robots/hurlingsnake.h>
#include <ode_robots/schlangeservo2.h>
#include <ode_robots/caterpillar.h>
#include <ode_robots/nimm2.h>
#include <ode_robots/nimm4.h>
#include <ode_robots/sphererobot3masses.h>
#include <ode_robots/plattfussschlange.h>
#include <ode_robots/axisorientationsensor.h>

// fetch all the stuff of lpzrobots into scope
using namespace lpzrobots;
using namespace osg;

class ThisSim{
public:

  // starting function (executed once at the beginning of the simulation loop)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {
    setCameraHomePos(Pos(-19.15, 13.9, 6.9),  Pos(-126.1, -17.6, 0));

    // initialization
    // - set noise to 0.01
    global.odeConfig.setParam("noise",0.05);
    global.odeConfig.setParam("controlinterval",4);
    global.odeConfig.setParam("simstepsize",0.005);
    global.odeConfig.setParam("gravity",-6);
    global.odeConfig.setParam("realtimefactor",1);

    int numSnakeLong=0;
    int numNimm4=1;
    int numHurling=0;
    int numSphere=3;
    int numPlattfuss=0;
    int numSnake=0;

    double height = .3;

    OdeHandle groundHandle(odeHandle);
    groundHandle.substance.toPlastic(1.5);
    Playground* playground =
      new Playground(groundHandle, osgHandle,osg::Vec3(20, 0.2, height+1.5));
    playground->setColor(Color(0.88f,0.4f,0.26f,0.2f));
    playground->setTexture("Images/really_white.rgb");
    playground->setPosition(osg::Vec3(0,0,0)); // playground positionieren und generieren
    global.obstacles.push_back(playground);

    TerrainGround* terrainground =
      new TerrainGround(groundHandle, osgHandle.changeColor(Color(1.0f,1.0,0.0/*196.0/255.0,41.0/255.0*/)),
//                        __PLACEHOLDER_6__,
                        "terrains/zoo_landscape1.ppm",
//                        __PLACEHOLDER_8__,
                        //__PLACEHOLDER_9__,
//                        __PLACEHOLDER_10__,
                        "terrains/zoo_landscape_texture.ppm",
                        20, 20, height, OSGHeightField::Red);
    terrainground->setPose(osg::Matrix::translate(0, 0, 0.1));
    global.obstacles.push_back(terrainground);

    InvertMotorNStepConf invertnconf = InvertMotorNStep::getDefaultConf();


    for(int i=0; i<3; ++i) override {
      PassiveSphere* s =
        new PassiveSphere(odeHandle,
                          osgHandle.changeColor(Color(184 / 255.0, 233 / 255.0, 237 / 255.0)),
                          0.3);
      s->setPosition(Pos(-1 , i*2-2, height+0.05));
      s->setTexture("Images/dusty.rgb");
      global.obstacles.push_back(s);
    }

//     for(int i=0; i<5; ++i) override {
//       PassiveBox* b =
//         new PassiveBox(odeHandle,
//                           osgHandle, osg::Vec3(0.2+i*0.1,0.2+i*0.1,0.2+i*0.1));
//       b->setPosition(Pos(i*0.5-5, i*0.5, height));
//       b->setColor(Color(1.0f,0.2f,0.2f,0.5f));
//       b->setTexture(__PLACEHOLDER_13__);
//       global.obstacles.push_back(b);
//     }

//     for(int i=0; i<5; ++i) override {
//       PassiveCapsule* c =
//         new PassiveCapsule(odeHandle, osgHandle, 0.2f, 0.3f, 0.3f);
//       c->setPosition(Pos(i-1, -i, height));
//       c->setColor(Color(0.2f,0.2f,1.0f,0.5f));
//       c->setTexture(__PLACEHOLDER_14__);
//       global.obstacles.push_back(c);
//     }

    OdeAgent* agent;
    AbstractWiring* wiring;
    OdeRobot* robot;
    AbstractController *controller;

//     //******* R A U P E  *********/
//     CaterPillar* myCaterPillar;
//     CaterPillarConf myCaterPillarConf = DefaultCaterPillar::getDefaultConf();
//     myCaterPillarConf.segmNumber=3;
//     myCaterPillarConf.jointLimit=M_PI/3;
//     myCaterPillarConf.motorPower=0.2;
//     myCaterPillarConf.frictionGround=0.01;
//     myCaterPillarConf.frictionJoint=0.01;
//     myCaterPillar =
//       new CaterPillar ( odeHandle, osgHandle.changeColor(Color(1.0f,0.0,0.0)),
//                         myCaterPillarConf, __PLACEHOLDER_15__);
//     (static_cast<OdeRobot*>(myCaterPillar))->place(Pos(-5,-5,height));

//      invertnconf.cInit=2.0;
//      controller = new InvertMotorSpace(15);
//     wiring = new One2OneWiring(new ColorUniformNoise(0.1));
//     agent = new OdeAgent( global, plotoptions );
//     agent->init(controller, myCaterPillar, wiring);
//     global.agents.push_back(agent);
//     global.configs.push_back(controller);
//     global.configs.push_back(myCaterPillar);
//     myCaterPillar->setParam(__PLACEHOLDER_16__,/*gb__PLACEHOLDER_17__SchlangeLong__PLACEHOLDER_18__Nimm2___PLACEHOLDER_19__s4avg__PLACEHOLDER_20__factorB__PLACEHOLDER_21__Nimm4___PLACEHOLDER_22__s4avg__PLACEHOLDER_23__factorB__PLACEHOLDER_24__HurlingSnake___PLACEHOLDER_25__steps__PLACEHOLDER_26__epsA__PLACEHOLDER_27__epsC__PLACEHOLDER_28__adaptrate__PLACEHOLDER_29__nomupdate__PLACEHOLDER_30__factorB__PLACEHOLDER_31__Sphere___PLACEHOLDER_32__S1__PLACEHOLDER_33__S2__PLACEHOLDER_34__Images/whitemetal_farbig_small.rgb__PLACEHOLDER_35__steps__PLACEHOLDER_36__epsC__PLACEHOLDER_37__epsA__PLACEHOLDER_38__adaptrate__PLACEHOLDER_39__rootE__PLACEHOLDER_40__logaE__PLACEHOLDER_41__desens__PLACEHOLDER_42__s4delay__PLACEHOLDER_43__s4avg__PLACEHOLDER_44__factorB__PLACEHOLDER_45__noiseB__PLACEHOLDER_46__frictionjoint__PLACEHOLDER_47__teacher__PLACEHOLDER_48__S1___PLACEHOLDER_49__S2___PLACEHOLDER_50__Images/whitemetal_farbig_small.rgb__PLACEHOLDER_51__steps__PLACEHOLDER_52__epsC__PLACEHOLDER_53__epsA__PLACEHOLDER_54__adaptrate__PLACEHOLDER_55__rootE__PLACEHOLDER_56__logaE__PLACEHOLDER_57__desens__PLACEHOLDER_58__s4delay__PLACEHOLDER_59__s4avg__PLACEHOLDER_60__factorB__PLACEHOLDER_61__noiseB__PLACEHOLDER_62__frictionjoint__PLACEHOLDER_63__teacher", 0.0);

    }//creation of snakes End





  }

  // add own key handling stuff here, just insert some case values
  virtual bool command(const OdeHandle&, const OsgHandle&, GlobalData& globalData, int key, bool down) override {
    if (down) { // only when key is pressed, not when released
      switch ( static_cast<char> key )
        {
        default:
          return false;
          break;
        }
    }
    return false;
  }

  virtual osg::LightSource* makeLights(osg::StateSet* stateset)
   override {
    // create a spot light.
    Light* light_0 = new Light;
    light_0->setLightNum(0);
    light_0->setPosition(Vec4(40.0f, 40.0f, 50.0f, 1.0f));
    // note that the blue component doesn't work!!! (bug in OSG???)
    //   Really? It works with me! (Georg)
    light_0->setAmbient(Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    light_0->setDiffuse(Vec4(0.8f, 0.8f, 0.8f, 1.0f));
    //    light_0->setDirection(Vec3(-1.0f, -1.0f, -1.2f));
    light_0->setSpecular(Vec4(1.0f, 0.9f, 0.8f, 1.0f));

    LightSource* light_source_0 = new LightSource;
    light_source_0->setLight(light_0);
    light_source_0->setLocalStateSetModes(StateAttribute::ON);
    light_source_0->setStateSetModes(*stateset, StateAttribute::ON);

    return light_source_0;
  }




};

int main (int argc, char **argv)
{
  ThisSim sim;
  // run simulation
  return sim.run(argc, argv) ? 0 : 1 override;
}

