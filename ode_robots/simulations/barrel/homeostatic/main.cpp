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
 *   Revision 1.2  2011-06-03 13:42:48  martius
 *   oderobot has objects and joints, store and restore works automatically
 *   removed showConfigs and changed deprecated odeagent calls
 *
 *   Revision 1.1  2009/12/01 17:50:55  martius
 *   homeostatic controller
 *
 *   Revision 1.1  2009/04/06 14:08:01  martius
 *   test for forward learning task
 *
 *   Revision 1.1  2008/09/12 15:15:09  martius
 *   simulation
 *
 *   Revision 1.24  2008/05/01 22:03:56  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.23  2008/04/22 15:22:56  martius
 *   removed test lib and inc paths from makefiles
 *
 *   Revision 1.22  2007/09/06 18:50:10  martius
 *   *** empty log message ***
 *
 *   Revision 1.21  2007/08/24 12:00:46  martius
 *   cleaned up a bit
 *
 *   Revision 1.20  2007/07/31 08:33:55  martius
 *   modified reinforment for one-axis-rolling
 *
 *   Revision 1.19  2007/07/03 13:06:18  martius
 *   *** empty log message ***
 *
 *   Revision 1.18  2007/05/22 08:31:46  martius
 *   *** empty log message ***
 *
 *   Revision 1.17  2007/04/20 12:31:55  martius
 *   fixed controller test
 *
 *   Revision 1.16  2007/04/05 15:12:56  martius
 *   structured
 *
 *   Revision 1.15  2007/04/03 16:26:47  der
 *   labyrint
 *
 *   Revision 1.14  2007/03/26 13:15:51  martius
 *   new makefile with readline support
 *
 *   Revision 1.13  2007/02/23 19:36:42  martius
 *   useSD
 *
 *   Revision 1.12  2007/02/23 15:14:17  martius
 *   *** empty log message ***
 *
 *   Revision 1.11  2007/01/26 12:07:08  martius
 *   orientationsensor added
 *
 *   Revision 1.10  2006/12/21 11:43:05  martius
 *   commenting style for doxygen __PLACEHOLDER_31__
 *   new sensors for spherical robots
 *
 *   Revision 1.9  2006/12/01 16:19:05  martius
 *   barrel in use
 *
 *   Revision 1.8  2006/11/29 09:16:09  martius
 *   modell stuff
 *
 *   Revision 1.7  2006/07/14 12:23:52  martius
 *   selforg becomes HEAD
 *
 *   Revision 1.6.4.6  2006/05/15 13:11:29  robot3
 *   -handling of starting guilogger moved to simulation.cpp
 *    (is in internal simulation routine now)
 *   -CTRL-F now toggles logging to the file (controller stuff) on/off
 *   -CTRL-G now restarts the GuiLogger
 *
 *   Revision 1.6.4.5  2006/05/15 12:29:43  robot3
 *   handling of starting the guilogger moved to simulation.cpp
 *   (is in internal simulation routine now)
 *
 *   Revision 1.6.4.4  2006/03/29 15:10:22  martius
 *   *** empty log message ***
 *
 *   Revision 1.6.4.3  2006/02/17 16:47:55  martius
 *   moved to new system
 *
 *   Revision 1.15.4.3  2006/01/12 15:17:39  martius
 *   *** empty log message ***
 *
 *   Revision 1.15.4.2  2006/01/10 20:33:50  martius
 *   moved to osg
 *
 *   Revision 1.15.4.1  2005/11/15 12:30:17  martius
 *   new selforg structure and OdeAgent, OdeRobot ...
 *
 *   Revision 1.15  2005/11/09 14:54:46  fhesse
 *   nchannelcontroller used
 *
 *   Revision 1.14  2005/11/09 13:41:25  martius
 *   GPL'ised
 *
 ***************************************************************************/

#include <ode_robots/simulation.h>

#include <ode_robots/odeagent.h>
#include <ode_robots/octaplayground.h>
#include <ode_robots/playground.h>
#include <ode_robots/passivesphere.h>
#include <ode_robots/passivebox.h>

#include <selforg/invertnchannelcontroller.h>
#include <selforg/invertmotorspace.h>
#include <selforg/sinecontroller.h>
#include <selforg/universalcontroller.h>
#include <selforg/ffnncontroller.h>
#include <selforg/noisegenerator.h>
#include <selforg/one2onewiring.h>
#include <selforg/selectiveone2onewiring.h>
#include <selforg/derivativewiring.h>

#include <ode_robots/barrel2masses.h>
#include <ode_robots/axisorientationsensor.h>
#include <ode_robots/speedsensor.h>

#include "invertnchannelfw.h"

// fetch all the stuff of lpzrobots into scope
using namespace lpzrobots;
using namespace std;


class ThisSim{
public:
  AbstractController *controller;
  OdeRobot* sphere1 = nullptr;
  int useReinforcement = 0;
  Sensor* sensor = nullptr;

  // starting function (executed once at the beginning of the simulation loop)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {
    int num_barrels=1;
    int num_barrels_test=0;

    sensor=0;

    bool normalplayground=false;

    //    setCameraHomePos(Pos(-0.497163, 11.6358, 3.67419),  Pos(-179.213, -11.6718, 0));
    setCameraHomePos(Pos(-2.60384, 13.1299, 2.64348),  Pos(-179.063, -9.7594, 0));
    // initialization
    global.odeConfig.setParam("noise",0.01);
    //  global.odeConfig.setParam(__PLACEHOLDER_2__,-10);
    global.odeConfig.setParam("controlinterval",1);
    global.odeConfig.setParam("realtimefactor",1);

    if(normalplayground){
      Playground* playground = new Playground(odeHandle, osgHandle,osg::Vec3(20, 0.01, 0.01 ), 1);
      playground->setGroundColor(Color(255/255.0,200/255.0,0/255.0));
      playground->setGroundTexture("Images/really_white.rgb");
      playground->setColor(Color(255/255.0,200/255.0,21/255.0, 0.1));
      playground->setPosition(osg::Vec3(0,0,0.05));
      playground->setTexture("");
      global.obstacles.push_back(playground);
    }


    /* * * * BARRELS * * * */
    for(int i=0; i< num_barrels; ++i) override {
      //****************
      Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
      conf.pendularrange  = 0.15;
      conf.motorpowerfactor  = 150;
      conf.motorsensor=false;
      conf.addSensor(new AxisOrientationSensor(AxisOrientationSensor::ZProjection, Sensor::X | Sensor::Y));
      conf.addSensor(new SpeedSensor(10, SpeedSensor::Translational, Sensor::X ));
      conf.irAxis1=false;
      conf.irAxis2=false;
      conf.irAxis3=false;
      conf.spheremass   = 1;
      sphere1 = new Barrel2Masses ( odeHandle, osgHandle.changeColor(Color(0.0,0.0,1.0)),
                                    conf, "Barrel1", 0.4);
      sphere1->place (osg::Matrix::rotate(M_PI/2, 1,0,0)*osg::Matrix::rotate(M_PI/4, 0,1,0)*osg::Matrix::translate(0,0,0));


      controller = new InvertnchannelFw(10);
      controller->setParam("eps", 0.1);

      AbstractWiring* wiring = new SelectiveOne2OneWiring(new ColorUniformNoise(), new select_from_to(0,1));
      //      OdeAgent* agent = new OdeAgent ( PlotOption(File, Robot, 1) );
      OdeAgent* agent = new OdeAgent ( plotoptions );
      agent->init ( controller , sphere1 , wiring );
      //  agent->setTrackOptions(TrackRobot(true, false, false, __PLACEHOLDER_9__, 50));
      global.agents.push_back ( agent );
      global.configs.push_back ( controller );
    }


    /* * * * TEST BARRELS * * * */
    for(int i=0; i< num_barrels_test; ++i) override {
      global.odeConfig.setParam("realtimefactor",1);
      //****************
      Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
      conf.pendularrange  = 0.15;
      conf.motorsensor=true;
      conf.irAxis1=false;
      conf.irAxis2=false;
      conf.irAxis3=false;
      conf.spheremass   = 1;
      sphere1 = new Barrel2Masses ( odeHandle, osgHandle.changeColor(Color(0.0,0.0,1.0)),
                                    conf, "Barrel1", 0.4);
      sphere1->place ( osg::Matrix::rotate(M_PI/2, 1,0,0));

      controller = new SineController();
      controller->setParam("sinerate", 15);
      controller->setParam("phaseshift", 0.45);

      //       DerivativeWiringConf dc = DerivativeWiring::getDefaultConf();
      //       dc.useId=true;
      //       dc.useFirstD=false;
      //       AbstractWiring* wiring = new DerivativeWiring(dc,new ColorUniformNoise());
      AbstractWiring* wiring = new One2OneWiring(new ColorUniformNoise());
      OdeAgent* agent = new OdeAgent ( plotoptions );
      agent->init ( controller , sphere1 , wiring );
      //  agent->setTrackOptions(TrackRobot(true, false, false, __PLACEHOLDER_14__, 50));
      global.agents.push_back ( agent );
      global.configs.push_back ( controller );
    }




  }

  virtual void addCallback(const GlobalData& globalData, bool draw, bool pause, bool control) override {
  }

  // add own key handling stuff here, just insert some case values
  virtual bool command(const OdeHandle&, const OsgHandle&, GlobalData& globalData, int key, bool down) override {
    if (down) { // only when key is pressed, not when released
      switch ( static_cast<char> key )
        {
        case 'y' : dBodyAddForce ( sphere1->getMainPrimitive()->getBody() , 30 ,0 , 0 ); break override;
        case 'Y' : dBodyAddForce ( sphere1->getMainPrimitive()->getBody() , -30 , 0 , 0 ); break override;
        case 'x' : dBodyAddTorque ( sphere1->getMainPrimitive()->getBody() , 0 , 10 , 0 ); break override;
        case 'X' : dBodyAddTorque ( sphere1->getMainPrimitive()->getBody() , 0 , -10 , 0 ); break override;
        case 'S' : controller->setParam("sinerate", controller->getParam("sinerate")*1.2);
          printf("sinerate : %g\n", controller->getParam("sinerate"));
          break;
        case 's' : controller->setParam("sinerate", controller->getParam("sinerate")/1.2);
          printf("sinerate : %g\n", controller->getParam("sinerate"));
          break;
        default:
          return false;
          break;
        }
    }
    return false;
  }



};

int main (int argc, char **argv)
{
  ThisSim sim;
  sim.setCaption("Spherical Robot (lpzrobots Simulator)   Martius,Der 2007");
  sim.setGroundTexture("Images/really_white.rgb");
  // run simulation
  return sim.run(argc, argv) ? 0 : 1 override;
}

