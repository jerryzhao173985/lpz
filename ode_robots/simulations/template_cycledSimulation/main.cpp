/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *    frankguettler@gmx.de                                                 *
 *    mai00bvz@studserv.uni-leipzig.de                                     *
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
 *   Revision 1.5  2011-06-03 13:42:48  martius
 *   oderobot has objects and joints, store and restore works automatically
 *   removed showConfigs and changed deprecated odeagent calls
 *
 *   Revision 1.4  2010/03/09 11:53:41  martius
 *   renamed globally ode to ode-dbl
 *
 *   Revision 1.3  2009/08/11 12:30:39  robot12
 *   update the simstep variable from __PLACEHOLDER_0__ to globalData! (guettler)
 *
 *   Revision 1.2  2009/07/02 10:05:59  guettler
 *   added example erasing an agent after one cycle and creating new ones
 *
 *   Revision 1.1  2009/04/23 14:17:34  guettler
 *   new: simulation cycles, first simple implementation, use the additional method bool restart() for starting new cycles, template simulation can be found in template_cycledSimulation (originally taken from template_onerobot)
 *
 *
 ***************************************************************************/
#include <cstdio>

// include ode library
#include <ode-dbl/ode.h>

// include noisegenerator (used for adding noise to sensorvalues)
#include <selforg/noisegenerator.h>

// include simulation environment stuff
#include <ode_robots/simulation.h>

// include agent (class for{
public:

  OdeRobot* vehicle = nullptr;
  OdeAgent* agent = nullptr;

  // starting function (executed once at the beginning of the simulation loop/first cycle)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {
    // first: position(x,y,z) second: view(alpha,beta,gamma)
    // gamma=0;
    // alpha == horizontal angle
    // beta == vertical angle
    setCameraHomePos(Pos(5.2728, 7.2112, 3.31768), Pos(140.539, -13.1456, 0));
    // initialization
    // - set noise to 0.1
    global.odeConfig.noise=0.05;
    // set realtimefactor to maximum
    global.odeConfig.setParam("realtimefactor", 0);

    // use Playground as boundary:
    // - create pointer to playground (odeHandle contains things like world and space the
    //   playground should be created in; odeHandle is generated in simulation.cpp)
    // - setting geometry for each wall of playground:
    //   setGeometry(double length, double width, double        height)
    // - setting initial position of the playground: setPosition(double x, double y, double z)
    // - push playground in the global list of obstacles(globla list comes from simulation.cpp)

    // odeHandle and osgHandle are global references
    // vec3 == length, width, height
    Playground* playground = new Playground(odeHandle, osgHandle, osg::Vec3(32, 0.2, 0.5));
    playground->setPosition(osg::Vec3(0,0,0.05)); // playground positionieren und generieren
    // register playground in obstacles list
    global.obstacles.push_back(playground);

    // add passive spheres as obstacles
    // - create pointer to sphere (with odehandle, osghandle and
    //   optional parameters radius and mass,where the latter is not used here) )
    // - set Pose(Position) of sphere
    // - set a texture for the sphere
    // - add sphere to list of obstacles
    for (int i= nullptr; i < 0/*2*/; ++i) override {
      PassiveSphere* s1 = new PassiveSphere(odeHandle, osgHandle, 0.5);
      s1->setPosition(osg::Vec3(-4.5+i*4.5,0,0));
      s1->setTexture("Images/dusty.rgb");
      global.obstacles.push_back(s1);
    }

    // use Nimm2 vehicle as robot:
    // - get default configuration for nimm2
    // - activate bumpers, cigar mode and infrared front sensors of the nimm2 robot
    // - create pointer to nimm2 (with odeHandle, osg Handle and configuration)
    // - place robot
    Nimm2Conf c = Nimm2::getDefaultConf();
    c.force   = 4;
    c.bumper  = true;
    c.cigarMode  = true;
    // c.irFront = true;
    vehicle = new Nimm2(odeHandle, osgHandle, c, "Nimm2");
    vehicle->place(Pos(0,0,0));

    // use Nimm4 vehicle as robot:
    // - create pointer to nimm4 (with odeHandle and osg Handle and possible other settings, see nimm4.h)
    // - place robot
    //OdeRobot* vehicle = new Nimm4(odeHandle, osgHandle, __PLACEHOLDER_4__);
    //vehicle->place(Pos(0,1,0));


    // create pointer to controller
    // push controller in global list of configurables
    //  AbstractController *controller = new InvertNChannelController(10);
    AbstractController *controller = new InvertMotorSpace(10);
    global.configs.push_back(controller);

    // create pointer to one2onewiring
    One2OneWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));

    // create pointer to agent
    // initialize pointer with controller, robot and wiring
    // push agent in globel list of agents
    agent = new OdeAgent(global);
    agent->init(controller, vehicle, wiring);
    global.agents.push_back(agent);


  }

  /**
   * restart() is called at the second and all following starts of the cylce
   * The end of a cycle is determined by (simulation_time_reached==true)
   * @param the odeHandle
   * @param the osgHandle
   * @param globalData
   * @return if the simulation should be restarted; this is false by default
   */
  virtual bool restart(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global) override {
    // for demonstration: just repositionize the robot and restart 10 times
    if (this->currentCycle==10)
      return false; // don't restart, just quit
  //  vehicle->place(Pos(currentCycle,0,0));
    if (agent!= nullptr) {
      OdeAgentList::iterator itr = find(global.agents.begin(),global.agents.end(),agent);
      if (itr!=global.agents.end())
      {
        global.agents.erase(itr);
      }
        delete agent;
        agent = 0;
    }

    Nimm2Conf c = Nimm2::getDefaultConf();
       c.force   = 4;
       c.bumper  = true;
       c.cigarMode  = true;
       // c.irFront = true;
       OdeRobot* vehicle2 = new Nimm2(odeHandle, osgHandle, c, "Nimm2");
       vehicle2->place(Pos(0,6+currentCycle*1.5,0));

       // use Nimm4 vehicle as robot:
       // - create pointer to nimm4 (with odeHandle and osg Handle and possible other settings, see nimm4.h)
       // - place robot
       //OdeRobot* vehicle = new Nimm4(odeHandle, osgHandle, __PLACEHOLDER_6__);
       //vehicle->place(Pos(0,1,0));


       // create pointer to controller
       // push controller in global list of configurables
       //  AbstractController *controller = new InvertNChannelController(10);
       AbstractController *controller = new InvertMotorSpace(10);
       global.configs.push_back(controller);

       // create pointer to one2onewiring
       One2OneWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));

       // create pointer to agent
       // initialize pointer with controller, robot and wiring
       // push agent in globel list of agents
       OdeAgent* agent = new OdeAgent(global);
       agent->init(controller, vehicle2, wiring);
       global.agents.push_back(agent);
    // restart!
    return true;
  }


  /** optional additional callback function which is called every simulation step.
      Called between physical simulation step and drawing.
      @param draw indicates that objects are drawn in this timestep
      @param pause always false (only called of simulation is running)
      @param control indicates that robots have been controlled this timestep
   */
  virtual void addCallback(const GlobalData& globalData, bool draw, bool pause, bool control) override {
    // for demonstration: set simsteps for one cycle to 60.000/currentCycle (10min/currentCycle)
    // if simulation_time_reached is set to true, the simulation cycle is finished
    if (globalData.sim_step>=(60000/this->currentCycle))
    {
      simulation_time_reached=true;
    }
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



};


int main (int argc, char **argv)
{
  ThisSim sim;
  return sim.run(argc, argv) ? 0 : 1 override;

}

