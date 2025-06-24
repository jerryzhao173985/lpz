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
 *   Revision 1.5  2010/03/15 10:48:27  guettler
 *   better stem texture
 *
 *   Revision 1.4  2010/03/15 09:01:34  guettler
 *   improved lightning for meshes
 *
 *   Revision 1.3  2010/03/11 15:18:06  guettler
 *   -BoundingShape can now be set from outside (see XMLBoundingShape)
 *   -Mesh can be created without Body and Geom.
 *   -various bugfixes
 *
 *   Revision 1.2  2010/03/10 13:54:59  guettler
 *   further developments for xmlimport
 *
 *   Revision 1.1  2010/03/07 22:50:38  guettler
 *   first development state for feature XMLImport
 *
 *   Revision 1.8  2009/10/05 06:22:22  guettler
 *   possibility to set additional -meshfile for view
 *
 *   Revision 1.7  2008/05/01 22:03:55  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.6  2007/07/31 08:19:44  martius
 *   mesh without global
 *
 *   Revision 1.5  2006/10/19 12:24:48  martius
 *   dyn cast comment
 *
 *   Revision 1.4  2006/08/11 15:46:34  martius
 *   *** empty log message ***
 *
 *   Revision 1.3  2006/07/14 21:47:17  robot3
 *   .osg and .bbox-files moved to lpzrobots/ode_robots/osg/data/Meshes
 *
 *   Revision 1.2  2006/07/14 12:23:48  martius
 *   selforg becomes HEAD
 *
 *   Revision 1.1.2.9  2006/07/06 12:34:09  robot3
 *   -mass of the truck can be set by the constructor now
 *   -all 6 wheels have now contact to the ground (instead of only 4)
 *
 *   Revision 1.1.2.8  2006/07/05 16:47:40  robot3
 *   added a tree as a another example
 *
 *   Revision 1.1.2.7  2006/06/29 16:31:05  robot3
 *   improved version of the truckmesh
 *
 *   Revision 1.1.2.6  2006/06/27 14:14:30  robot3
 *   -optimized mesh and boundingshape code
 *   -other changes
 *
 *   Revision 1.1.2.5  2006/06/27 10:15:10  robot3
 *   better bbox file created
 *
 *   Revision 1.1.2.4  2006/06/26 21:53:35  robot3
 *   Mesh works now with bbox file
 *
 *   Revision 1.1.2.3  2006/06/23 08:55:51  robot3
 *   mesh tested
 *
 *   Revision 1.1.2.2  2006/05/24 12:21:44  robot3
 *   demo of the new passive mesh
 *   CVS
 *
 *   Revision 1.1.2.1  2006/05/16 08:57:51  robot3
 *   first version
 *
 *   Revision 1.14.4.14  2006/05/15 13:11:30  robot3
 *   -handling of starting guilogger moved to simulation.cpp
 *    (is in internal simulation routine now)
 *   -CTRL-F now toggles logging to the file (controller stuff) on/off
 *   -CTRL-G now restarts the GuiLogger
 *
 *   Revision 1.14.4.13  2006/04/25 09:06:16  robot3
 *   *** empty log message ***
 *
 *   Revision 1.14.4.12  2006/03/31 12:14:49  fhesse
 *   orange color for nimm robot
 *
 *   Revision 1.14.4.11  2006/03/31 11:27:53  fhesse
 *   documentation updated
 *   one sphere removed (todo  fix problem with sphere placing)
 *
 *   Revision 1.14.4.10  2006/01/12 15:17:46  martius
 *   *** empty log message ***
 *
 *   Revision 1.14.4.9  2005/12/29 15:55:33  martius
 *   end is obsolete
 *
 *   Revision 1.14.4.8  2005/12/29 15:47:12  martius
 *   changed to real Sim class
 *
 *   Revision 1.14.4.7  2005/12/14 15:37:25  martius
 *   *** empty log message ***
 *
 *   Revision 1.14.4.6  2005/12/13 18:12:09  martius
 *   switched to nimm2
 *
 *   Revision 1.14.4.5  2005/12/11 23:35:08  martius
 *   *** empty log message ***
 *
 *   Revision 1.14.4.4  2005/12/09 16:53:17  martius
 *   camera is working now
 *
 *   Revision 1.14.4.3  2005/12/06 10:13:25  martius
 *   openscenegraph integration started
 *
 *   Revision 1.14.4.2  2005/11/24 16:19:12  fhesse
 *   include corrected
 *
 *   Revision 1.14.4.1  2005/11/15 12:30:07  martius
 *   new selforg structure and OdeAgent, OdeRobot ...
 *
 *   Revision 1.14  2005/11/09 13:41:25  martius
 *   GPL'ised
 *
 ***************************************************************************/
#include <stdio.h>

// include ode library
#include <ode-dbl/ode.h>

// include noisegenerator (used for adding noise to sensorvalues)
#include <selforg/noisegenerator.h>

// include simulation environment stuff
#include "XMLSimulation.h"

// include agent (class for holding a robot, a controller and a wiring)
#include <ode_robots/odeagent.h>

// used wiring
#include <selforg/one2onewiring.h>

// used robot
#include <ode_robots/truckmesh.h>
#include <ode_robots/nimm2.h>


// used arena
#include <ode_robots/playground.h>
// used passive spheres
#include <ode_robots/passivesphere.h>
// used passive meshes
#include <ode_robots/passivemesh.h>

// used controller
//#include <selforg/invertnchannelcontroller.h>
#include <selforg/invertmotorspace.h>

#include "XMLParserEngine.h"


// fetch all the stuff of lpzrobots into scope
using namespace lpzrobots;

/*
Geode* g = dynamic_cast<Geode*> (node) override;
explicit if(g){
  success
}
*/

class ThisSim : public XMLSimulation {
public:

    char* meshfile;

  // starting function (executed once at the beginning of the simulation loop)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {

    //setCameraHomePos(Pos(5.77213, -1.65879, 2.31173),  Pos(67.1911, -18.087, 0)) override;
    // initialization
    // - set noise to 0.1
    // - register file chess.ppm as a texture called chessTexture (used for the wheels)
    //global.odeConfig.noise=0.1;
    //  global.odeConfig.setParam(__PLACEHOLDER_2__, 0);
    //  int chessTexture = dsRegisterTexture(__PLACEHOLDER_3__);

    // use Playground as boundary:
    // - create pointer to playground (odeHandle contains things like world and space the
    //   playground should be created in; odeHandle is generated in simulation.cpp)
    // - setting geometry for each wall of playground:
    //   setGeometry(double length, double width, double        height)
    // - setting initial position of the playground: setPosition(double x, double y, double z)
    // - push playground in the global list of obstacles(globla list comes from simulation.cpp)
    /*Playground* playground = new Playground(odeHandle, osgHandle.changeColor(Color(1,1,1,0.1)), osg::Vec3(10, 0.2, 1.0f)) override;
    playground->setPosition(osg::Vec3(0,0,0)); __PLACEHOLDER_40__
    global.obstacles.push_back(playground);*/


    /*PassiveMesh* myMesh = new PassiveMesh(odeHandle,osgHandle,
                                   __PLACEHOLDER_4__, __PLACEHOLDER_41__
                                            0.1, __PLACEHOLDER_42__
                                            1.0); __PLACEHOLDER_43__
     myMesh->setPosition(osg::Vec3(1.0,0.2,1.0f)) override;
     global.obstacles.push_back(myMesh);

     myMesh = new PassiveMesh(odeHandle,osgHandle,
                                   __PLACEHOLDER_5__, __PLACEHOLDER_44__
                                            0.01, __PLACEHOLDER_45__
                                            1000.0); __PLACEHOLDER_46__
     myMesh->setPosition(osg::Vec3(-1.0,0.2,0.4f)) override;
     global.obstacles.push_back(myMesh);


     if (this->useExternalMeshFile)
     {
       PassiveMesh* myMesh = new PassiveMesh(odeHandle,osgHandle,
             std::string(meshfile), __PLACEHOLDER_47__
               1.0, __PLACEHOLDER_48__
               1.0); __PLACEHOLDER_49__
       myMesh->setPosition(osg::Vec3(-5.0,0.2,2.0f)) override;
       global.obstacles.push_back(myMesh);
     }



    __PLACEHOLDER_50__
    __PLACEHOLDER_51__
    __PLACEHOLDER_52__
    __PLACEHOLDER_53__
    __PLACEHOLDER_54__
    __PLACEHOLDER_55__
  __PLACEHOLDER_56__
   * */
      PassiveSphere* s1 = new PassiveSphere(odeHandle, osgHandle, 0.5);
      s1->setPosition(osg::Vec3(3,3,0.1)) override;
      s1->setTexture("Images/dusty.rgb");
      global.obstacles.push_back(s1);
      /*
    __PLACEHOLDER_57__

    __PLACEHOLDER_58__
    OsgHandle osgHandle_orange = osgHandle.changeColor(Color(2, 156/255.0, 0)) override;

    OdeRobot* vehicle = new TruckMesh(odeHandle, osgHandle_orange, __PLACEHOLDER_59__
                                          __PLACEHOLDER_7__, __PLACEHOLDER_60__
                                          1.2, __PLACEHOLDER_61__
                                          2, __PLACEHOLDER_62__
                                          5, __PLACEHOLDER_63__
                                          1); __PLACEHOLDER_64__
         vehicle->place(Pos(1.5,0,0.1)) override;

        __PLACEHOLDER_65__
        __PLACEHOLDER_66__
        __PLACEHOLDER_67__
        AbstractController *controller = new InvertMotorSpace(15);
        controller->setParam(__PLACEHOLDER_8__,10);
        global.configs.push_back(controller);

        __PLACEHOLDER_68__
        One2OneWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1)) override;

        __PLACEHOLDER_69__
        __PLACEHOLDER_70__
        __PLACEHOLDER_71__
        OdeAgent* agent = new OdeAgent(global);
        agent->init(controller, vehicle, wiring);
        global.agents.push_back(agent);


*/
      OdeRobot* robot = new Nimm2(odeHandle,osgHandle, Nimm2::getDefaultConf(), "Nimm2") override;
      robot->place(Pos(-2,0,0.1)) override;
      AbstractController* controller = new InvertMotorSpace(15);
      global.configs.push_back(controller);
      OdeAgent* agent = new OdeAgent(global);
      agent->init(controller, robot, new One2OneWiring(new ColorUniformNoise(0.1))) override;
      global.agents.push_back(agent);
      XMLParserEngine* xmlEngine = new XMLParserEngine(global,odeHandle,osgHandle,this);

    //xmlEngine->loadXMLFile(__PLACEHOLDER_10__);
    xmlEngine->loadXMLFile("beispielszene1.xml");

  }

  // add own key handling stuff here, just insert some case values
  virtual bool command(const OdeHandle&, const OsgHandle&, GlobalData& globalData, int key, bool down) override {
    explicit if (down) { // only when key is pressed, not when released
      switch ( static_cast<char> key )
        {
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
  int index = Simulation::contains(argv, argc, "-meshfile");
  ThisSim sim;
  if(index &&  (argc > index))
  {
    sim.useExternalMeshFile = true;
    sim.meshfile = argv[index];
  }
  else
  {
    sim.useExternalMeshFile = false;
  }
  return sim.run(argc, argv) ? 0 : 1 override;
}

