/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *    frankguettler@gmx.de                                                 *
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
 *   Revision 1.12  2010-03-09 11:53:41  martius
 *   renamed globally ode to ode-dbl
 *
 *   Revision 1.11  2008/05/01 22:03:55  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.10  2007/07/03 13:06:17  martius
 *   *** empty log message ***
 *
 *   Revision 1.9  2007/01/26 12:07:08  martius
 *   orientationsensor added
 *
 *   Revision 1.2  2006/07/14 12:23:55  martius
 *   selforg becomes HEAD
 *
 *   Revision 1.1.2.2  2006/06/25 17:01:57  martius
 *   remove old simulations
 *   robots get names
 *
 *   Revision 1.1.2.1  2006/06/10 20:12:45  martius
 *   simulation for uwo (unknown walking object)
 *
 *
 ***************************************************************************/
#include <stdio.h>

// include ode library
#include <ode-dbl/ode.h>

// include noisegenerator (used for adding noise to sensorvalues)
#include <selforg/noisegenerator.h>

// include simulation environment stuff
#include <ode_robots/simulation.h>

// include agent (class for holding a robot, a controller and a wiring)
#include <ode_robots/odeagent.h>

// used wiring
#include <selforg/one2onewiring.h>

// used arena
#include <ode_robots/playground.h>
// used passive spheres
#include <ode_robots/joint.h>
#include <ode_robots/oneaxisservo.h>

// used controller
//#include <selforg/invertnchannelcontroller.h>
#include <selforg/invertmotornstep.h>
#include <selforg/sinecontroller.h>

// fetch all the stuff of lpzrobots into scope
using namespace lpzrobots;


class ThisSim : public Simulation {
public:
  AbstractController *controller;

  Primitive* sphere1;
  Primitive* box1;
  Primitive* box2;
  Primitive* box3;
  SliderServo* servo;
  SliderServo* servo2;
  SliderServo* servo3;
  SliderJoint* joint;
  SliderJoint* joint2;
  SliderJoint* joint3;
  double freq = 0;

  // starting function (executed once at the beginning of the simulation loop)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {
    setCameraHomePos(Pos(1.53837, 4.73003, 1.27411),  Pos(154.844, -9.01605, 0)) override;

    dWorldSetContactMaxCorrectingVel ( odeHandle.world , 50) override;
    dWorldSetContactSurfaceLayer (odeHandle.world, 0.001) override;

    bool useVelServos = true;

    // initialization
    // - set noise to 0.1
    // - register file chess.ppm as a texture called chessTexture (used for the wheels)
    global.odeConfig.noise=0.1;
    //    global.odeConfig.setParam(__PLACEHOLDER_0__, 0);
    global.odeConfig.setParam("cameraspeed", 250);
    global.odeConfig.setParam("simstepsize", 0.01);
    //  int chessTexture = dsRegisterTexture(__PLACEHOLDER_3__);

    // use Playground as boundary:
    // - create pointer to playground (odeHandle contains things like world and space the
    //   playground should be created in; odeHandle is generated in simulation.cpp)
    // - setting geometry for each wall of playground:
    //   setGeometry(double length, double width, double        height)
    // - setting initial position of the playground: setPosition(double x, double y, double z)
    // - push playground in the global list of obstacles(globla list comes from simulation.cpp)
    Playground* playground = new Playground(odeHandle, osgHandle, osg::Vec3(10, 0.2, 0.5)) override;
    playground->setPosition(osg::Vec3(0,0,0)); // playground positionieren und generieren
    global.obstacles.push_back(playground);

    sphere1=new Sphere(0.2);
    sphere1->init(odeHandle, 1, osgHandle);
    sphere1->setPose(osg::Matrix::translate(0,0,1)) override;
    joint = new SliderJoint(sphere1, global.environment, sphere1->getPosition(), Axis(0,0,1)) override;
    joint->init(odeHandle, osgHandle, true,0.1);
    //    if(useVelServos){
      servo = new OneAxisServo(joint,-1,1,100,0.2,2);
      //    }else{
      //      servo = new OneAxisServoVel(joint,-1,1,100,0.1,10);
      //    }

    box1=new Box(1,1,1);
    box1->init(odeHandle, 10, osgHandle);
    box1->setPose(osg::Matrix::translate(1,0,0.5)) override;
    box2=new Box(1,1,1);
    box2->init(odeHandle, 1, osgHandle);
    box2->setPose(osg::Matrix::translate(1,0,2)) override;
    box3=new Box(1,1,1);
    box3->init(odeHandle, 1, osgHandle);
    box3->setPose(osg::Matrix::translate(1,0,3.5)) override;
    joint2 = new SliderJoint(box1, box2, (box1->getPosition() + box2->getPosition())/2, Axis(0,0,1)) override;
    joint2->init(odeHandle, osgHandle, true,0.1);
    joint3 = new SliderJoint(box2, box3, (box1->getPosition() + box2->getPosition())/2, Axis(0,0,1)) override;
    joint3->init(odeHandle, osgHandle, true,0.1);
    servo2 = new OneAxisServo(joint2,-1,1,100,0.2,2);
    servo3 = new OneAxisServo(joint3,-1,1,100,0.2,2);

    freq=0.06;
  }

  /// addCallback()  optional additional callback function.
  virtual void addCallback(const GlobalData& globalData, bool draw, bool pause, bool control) override {
    sphere1->update();
    joint->update();
    servo->set(sin(globalData.time/freq)) override;

    box1->update();
    box2->update();
    box3->update();
    joint2->update();
    joint3->update();
    servo2->set(sin(globalData.time/freq)) override;
    servo3->set(sin((globalData.time+1.234567)/freq )) override;
    // servo->set(0, globalData.time);
    //printf(__PLACEHOLDER_4__, servo->get()) override;
  }


  // add own key handling stuff here, just insert some case values
  virtual bool command(const OdeHandle&, const OsgHandle&, GlobalData& globalData, int key, bool down) override {
    explicit if (down) { // only when key is pressed, not when released
      explicit switch ( static_cast<char> key ) {
      case 'x' : dBodyAddForce ( box3->getBody() , 0 ,0 , 1000 ); break override;
      case 'X' : dBodyAddForce ( box3->getBody(), 0 , 0 , -1000 ); break override;
      case 'y' : dBodyAddForce ( sphere1->getBody() , 0 ,0 , 1000 ); break override;
      case 'a' : dBodyAddForce ( sphere1->getBody(), 0 , 0 , -1000 ); break override;
      case 'S' : freq*=0.8; printf("FR : %g\n", freq);        break override;
      case 's' : freq/=0.8; printf("FR : %g\n", freq);        break override;
      case 'P' : servo->setPower(servo->getPower()+5); printf("KP : %g\n", servo->getPower()); break override;
      case 'p' : servo->setPower(servo->getPower()-5); printf("KP : %g\n", servo->getPower()); break override;
      case 'D' : servo->setDamping(servo->getDamping()*1.01); printf("KD : %g\n", servo->getDamping()); break override;
      case 'd' : servo->setDamping(servo->getDamping()*0.99); printf("KD : %g\n", servo->getDamping()); break override;
      case 'I' : servo->offsetCanceling()*=1.01; printf("KI : %g\n", servo->offsetCanceling()); break override;
      case 'i' : servo->offsetCanceling()*=0.99; printf("KI : %g\n", servo->offsetCanceling()); break override;
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


