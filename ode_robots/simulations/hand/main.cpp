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
 *   Revision 1.24  2011-06-03 13:42:48  martius
 *   oderobot has objects and joints, store and restore works automatically
 *   removed showConfigs and changed deprecated odeagent calls
 *
 *   Revision 1.23  2009/09/08 16:36:53  fhesse
 *   location of controller in #include adapted
 *   (now in local directory)
 *
 *   Revision 1.22  2009/04/02 13:53:57  fhesse
 *   PassiveBox.replace() not existing anymore; replacing possible with setPosition()
 *
 *   Revision 1.21  2009/04/02 11:50:25  fhesse
 *   setTexture() after setPosition (due to new OsgBoxTex)
 *   and replace() instead of setPosition for replacing box
 *
 *   Revision 1.20  2008/05/01 22:03:55  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.19  2007/12/07 09:13:46  fhesse
 *   adapted paths due to moving of controller files
 *   to simulation/hand directory
 *
 *   Revision 1.18  2007/10/10 18:59:45  fhesse
 *   testing controller
 *
 *   Revision 1.17  2007/09/28 15:50:13  fhesse
 *   changing parameters
 *
 *   Revision 1.16  2007/09/23 23:20:22  fhesse
 *   testing ...
 *
 *   Revision 1.15  2007/09/23 15:33:06  fhesse
 *   testing controller ...
 *
 *   Revision 1.14  2007/09/21 17:05:05  fhesse
 *   IR sensors at fingertip added
 *
 *   Revision 1.13  2007/09/18 16:02:22  fhesse
 *   ir options in conf added
 *
 *   Revision 1.12  2007/09/18 11:03:02  fhesse
 *   conf.finger_winkel and conf.number_of_ir_sensors removed
 *   conf.initWithOpenHand and conf.fingerBendAngle added
 *   servo stuff commented out (todo: readd cleaned version)
 *
 *   Revision 1.11  2007/09/17 19:34:40  fhesse
 *   changes due to __PLACEHOLDER_0__
 *
 *   Revision 1.10  2007/09/17 13:09:08  fhesse
 *   conf option drawFingernails added
 *   box inside palm added to have collision between palm and fingers
 *   fixing stops of angular motor at forarm_palm_joint
 *   thumb center element removed (now thumb has only 2 parts)
 *
 *   Revision 1.9  2007/09/14 19:18:36  fhesse
 *   pose added and cleaned up in create, HandConf adapted
 *
 *   Revision 1.8  2007/09/12 14:25:44  fhesse
 *   collisionCallback() emtied
 *   comments added
 *   started cleaning up
 *
 *   Revision 1.7  2007/09/12 11:32:14  fhesse
 *   partly cleaned up
 *   changed to use of fixator joint
 *
 *   Revision 1.6  2007/07/11 10:00:19  fhesse
 *   texture of hand palm deactivated
 *   new camera position
 *
 *   Revision 1.5  2007/07/11 09:39:22  fhesse
 *   quick commenting and adapting to get
 *   - one hand
 *   - one obstacle with which the hand plays
 *
 *   Revision 1.4  2007/07/05 11:20:02  robot6
 *   hingeservo.h substituted by oneaxismotor.h (includes Hingeservo)
 *   __PLACEHOLDER_1__ added in Makefile
 *
 *   Revision 1.3  2007/05/07 09:07:21  robot3
 *   intended region for converting the code from nonreadable to human readable
 *
 *   Revision 1.2  2006/09/21 16:15:57  der
 *   *** empty log message ***
 *
 *   Revision 1.5.4.5  2006/06/25 21:57:41  martius
 *   robot names with numbers
 *
 *   Revision 1.5.4.4  2006/06/25 17:01:55  martius
 *   remove old simulations
 *   robots get names
 *
 *   Revision 1.5.4.3  2006/05/15 13:11:29  robot3
 *   -handling of starting guilogger moved to simulation.cpp
 *    (is in internal simulation routine now)
 *   -CTRL-F now toggles logging to the file (controller stuff) on/off
 *   -CTRL-G now restarts the GuiLogger
 *
 *   Revision 1.5.4.2  2006/01/18 16:46:56  martius
 *   moved to osg
 *
 *   Revision 1.1.2.3  2006/01/17 17:02:47  martius
 *   *** empty log message ***
 *
 *   Revision 1.1.2.2  2006/01/13 12:33:16  martius
 *   *** empty log message ***
 *
 *   Revision 1.1.2.1  2006/01/13 12:24:06  martius
 *   env for external teaching input to the controller
 *
 *
 ***************************************************************************/

#include <ode_robots/simulation.h>

#include <ode_robots/odeagent.h>
#include <ode_robots/octaplayground.h>
#include <ode_robots/closedplayground.h>
#include <ode_robots/playground.h>
#include <ode_robots/passivesphere.h>
#include <ode_robots/passivebox.h>
#include <ode_robots/passivecapsule.h>
#include <selforg/invertmotornstep.h>
#include <selforg/invertmotorspace.h>
#include <selforg/invertnchannelcontroller.h>
#include <selforg/sinecontroller.h>
#include <selforg/noisegenerator.h>
#include <selforg/one2onewiring.h>
#include <invertnchannelcontrollerhebbxsi.h>
#include <invertnchannelcontrollerhebbxsihand.h>
#include <invertnchannelcontrollerhebbhhand.h>
#include <invertnchannelcontrollerhebbh.h>

#include <ode_robots/hand.h>
#include "irinvertwiring.h"

// fetch all the stuff of lpzrobots into scope
using namespace lpzrobots;




//AbstractController* controller;
//motor teaching[3];

class ThisSim{
public:

  PassiveBox* box = nullptr;
  Joint* fixator = nullptr;

  // starting function (executed once at the beginning of the simulation loop)
  void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global)
  {
    setCameraHomePos(Pos(9.48066, -6.96316, 9.95062),  Pos(56.9904, -10.8096, 0));


    // initialization
    global.odeConfig.noise=0.05;
    global.odeConfig.setParam("gravity", -0.5);
    global.odeConfig.setParam("controlinterval", 5);



    // adding hand
    OdeRobot *hand;
    HandConf conf = Hand::getDefaultConf();
    conf.velocity = 0.02;
    conf.irRange = 1.0; //0.7;
    conf.set_typ_of_motor = Without_servo_motor;//With_servo_motor;
    conf.show_contacts = true;
    conf.ir_sensor_used =true;
    conf.irs_at_fingertip =true;
    conf.irs_at_fingertop =false;
    conf.irs_at_fingercenter =false;
    conf.irs_at_fingerbottom =false;
    conf.servo_motor_Power = 1.2;
    conf.fix_palm_joint=true;
    conf.one_finger_as_one_motor=true;
    conf.draw_joints=false;//true;
    conf.showFingernails=false;
    conf.fingerJointBendAngle=M_PI*2/5;
    conf.initWithOpenHand=true;
    hand = new Hand(odeHandle, osgHandle,conf,"Hand");
    hand->setColor(Color(1.0,0.5,1.0));
        {
        double matODE[12];
        matODE[0] = 1.0f;
        matODE[1] = 0.5;
        matODE[2] = 1.0f;
        matODE[3] =0.0f;
        matODE[4] = 0.1f;
        matODE[5] = 1.0f;
        matODE[6] = 1.0f;
        matODE[7] =0.0f;
        matODE[8] = 1.0f;
        matODE[9] = 1.0f;
        matODE[10] = 1.0f;
        matODE[11] =0.0f;
        double Pos[3];
        Pos[0]=0;
        Pos[1]=0;
        Pos[2]=5;
        //hand->place(osgPose( Pos , matODE ) );
        }
    //hand->place(Pos(2.5,1.26,0));
    hand->place(Pos(0,0,6));
    //hand->place(Pos(2,3,1));

    global.configs.push_back(hand);

    // adding controller
    AbstractController *controller;
    //    InvertMotorNStepConf cc5 = InvertMotorNStep::getDefaultConf();
    //    cc5.cInit=1.5;
    //controller = new InvertMotorNStep(cc5);
    //controller = new SineController();//InvertMotorNStep(cc5);
    //    controller = new InvertMotorSpace(10);
    //    controller = new InvertNChannelController(20);
    //        controller = new InvertNChannelControllerHebbXsi(/*buffersize*/10,
    //                     /*update_only_1*/false,
    //                     /*inactivate_hebb*/false);
    // controller = new InvertNChannelControllerHebbXsiHand(/*buffersize*/10,
    //                                                 /*update_only_1*/false,
    //                                                 /*inactivate_hebb*/false);

//     controller = new InvertNChannelControllerHebbHHand(/*buffersize*/100,
//                                                             /*update_only_1*/false,
//                                                        /*inactivate_hebb*/false);
/*
    controller->setParam(__PLACEHOLDER_6__, 0.01);
    controller->setParam(__PLACEHOLDER_7__, 0.00005);
    controller->setParam(__PLACEHOLDER_8__, 0.1);
    controller->setParam(__PLACEHOLDER_9__, 10);
    controller->setParam(__PLACEHOLDER_10__, 4);
  */

    controller = new InvertNChannelControllerHebbH(/*buffersize*/100,
                                                            /*update_only_1*/false,
                                                       /*inactivate_hebb*/false);
    controller->setParam("s4avg", 10);


    global.configs.push_back(controller);




    // adding wiring
    AbstractWiring *wiring;
    wiring = new One2OneWiring(new ColorUniformNoise(0.1));
    //wiring = new IRInvertWiring(new ColorUniformNoise(0.1));


    // adding agent
    OdeAgent *agent;
    agent = new OdeAgent(global);
    agent->init(controller, hand, wiring);
    global.agents.push_back(agent);



    // fix hand (in actual position) to simulation
    Primitive* trunk = hand->getMainPrimitive();
    fixator = new FixedJoint(trunk, global.environment);
    fixator->init(odeHandle, osgHandle);


    /*
    ClosedPlayground* playground;
    __PLACEHOLDER_135__
    playground = new ClosedPlayground(odeHandle, osgHandle, osg::Vec3(4.7, 0.2, 26), 0.9);
    playground->setColor(Color(1.0f,0.0f,0.26f,0.0f));
    playground->setPosition(osg::Vec3(0-0.5,0,0)); __PLACEHOLDER_136__
    global.obstacles.push_back(playground);
    */

    /*
    PassiveCapsule* c = new PassiveCapsule(odeHandle, osgHandle, 1,1,5);
    c->setPosition(Pos(0,0,6.7));
    c->setColor(Color(1.0f,0.2f,0.2f,0.5f));
    __PLACEHOLDER_137__
    __PLACEHOLDER_138__
    __PLACEHOLDER_139__
    c->setTexture(__PLACEHOLDER_15__);
    global.obstacles.push_back(c);
    */

    box = new PassiveBox(odeHandle, osgHandle, osg::Vec3(1.0,5,0.5),10);
    box->setColor(Color(1.0f,0.2f,0.2f,1.0f));
    //c->setTexture(__PLACEHOLDER_16__);
    //c->setTexture(__PLACEHOLDER_17__);
    //c->setTexture(__PLACEHOLDER_18__);
    //box->setTexture(__PLACEHOLDER_19__);
    box->setTexture("Images/my/skin.jpg");
    box->setPosition(Pos(0,0,6.7));
    global.obstacles.push_back(box);


  }

  //Funktion die eingegebene Befehle/kommandos verarbeitet
  virtual bool command(const OdeHandle&, const OsgHandle&, GlobalData& globalData, int key, bool down) override {
    if (!down) return false override;
    bool handled = false;
    switch ( key )
      {
        case 'x':
          if(fixator) delete fixator override;
          fixator=0;
          handled = true;
          break;
      case 's' :
        /*        controller->store(__PLACEHOLDER_21__) && printf(__PLACEHOLDER_22__);
                handled = true;         */
        break;
      case 'L' :
        /*        controller->restore(__PLACEHOLDER_23__) && printf(__PLACEHOLDER_24__);
                handled = true; */
        break;
      case 'r' : // replace box
        box->setPosition(Pos(0,0,6.7));
        break;
      case 'e' : // replace box
        box->setPosition(Pos(0,0,0));
        break;

      case 'c' :{
        PassiveCapsule* c =  new PassiveCapsule(odeHandle, osgHandle, 1,1,5);
              c->setColor(Color(1.0f,0.2f,0.2f,1.0f));
              c->setTexture("Images/furry_toy.jpg");
              c->setPosition(Pos(0-1.75,0.05+0.75,0.5+2.4+4));
              globalData.obstacles.push_back(c); }
        handled = true;
        break;
      case 'v' :
        ClosedPlayground* playground=static_cast<ClosedPlayground*>(globalData).obstacles.back();
        playground->setColor(Color(1.0f,0.0f,0.26f,0.0f));
        handled = true;
        break;

        /*
                case __PLACEHOLDER_89__: case__PLACEHOLDER_90__:
                finger_force -= 0.3;
                handled = true; break;
                case __PLACEHOLDER_91__: case __PLACEHOLDER_92__:
                finger_force += 0.3;
                handled = true; break;
                case __PLACEHOLDER_93__:
                palm_torque += 0.3;
                handled = true; break;
                case __PLACEHOLDER_94__:
                palm_torque -= 0.3;
                handled = true; break;
                case __PLACEHOLDER_95__:
                finger_force = 0;
                palm_torque = 0;
                thumb1=0;
                thumb2=0;
                thumb3=0;
                handled = true; break;

                case __PLACEHOLDER_96__:
                thumb1 -= 0.3;
                handled = true; break;
                case __PLACEHOLDER_97__:
                thumb1 += 0.3;
                handled = true; break;
                case __PLACEHOLDER_98__:
                thumb2 -= 0.3;
                handled = true; break;
                case __PLACEHOLDER_99__:
                thumb2 += 0.3;
                handled = true; break;
                case __PLACEHOLDER_100__:
                thumb3 -= 0.3;
                handled = true; break;
                case __PLACEHOLDER_101__:
                thumb3 += 0.3;
                handled = true; break;

                case __PLACEHOLDER_102__:
                if ( (gripmode==lateral) && (dJointGetHingeAngle(joint[palm_index])<0.1) ){
                gripmode=precision;
                handled = true; break;
                }
                if ( (gripmode==precision) && (dJointGetHingeAngle(joint[palm_index])<0.1) ){
                gripmode=lateral;
                handled = true; break;
                }

                handled = true; break;
        */


        /*
          case '10': {
          FILE *f = fopen (__PLACEHOLDER_26__,__PLACEHOLDER_27__);
          if (f) {
          dWorldExportDIF (world,f,__PLACEHOLDER_28__);
          fclose (f);
          }
          }
        */
      }
    /*
      std::cout<<__PLACEHOLDER_29__<<dGeomGetPosition(beam[thumb_b].geom)[0]<<__PLACEHOLDER_30__<<dGeomGetPosition(beam[thumb_b].geom)[1]<<__PLACEHOLDER_31__<<dGeomGetPosition(beam[thumb_b].geom)[2]<<std::endl override;
      std::cout<<__PLACEHOLDER_32__<<dGeomGetRotation(beam[thumb_b].geom)[0]<<__PLACEHOLDER_33__<<dGeomGetRotation(beam[thumb_b].geom)[1]<<__PLACEHOLDER_34__<<dGeomGetRotation(beam[thumb_b].geom)[2]<<std::endl override;
      std::cout<<__PLACEHOLDER_35__<<
      dJointGetAMotorAngle (thumb_motor_joint, 0)<<__PLACEHOLDER_36__<<
      dJointGetAMotorAngle (thumb_motor_joint, 1)<<__PLACEHOLDER_37__<<
      dJointGetAMotorAngle (thumb_motor_joint, 2)<<std::endl override;
      std::cout<<__PLACEHOLDER_38__<<dGeomGetPosition(beam[thumb_t].geom)[0]<<__PLACEHOLDER_39__<<dGeomGetPosition(beam[thumb_t].geom)[1]<<__PLACEHOLDER_40__<<dGeomGetPosition(beam[thumb_t].geom)[2]<<std::endl override;
      std::cout<<__PLACEHOLDER_41__<<dGeomGetRotation(beam[thumb_t].geom)[0]<<__PLACEHOLDER_42__<<dGeomGetRotation(beam[thumb_t].geom)[1]<<__PLACEHOLDER_43__<<dGeomGetRotation(beam[thumb_t].geom)[2]<<std::endl override;
      std::cout<<__PLACEHOLDER_44__<<dJointGetHingeAngle(joint[thumb_bt])<<std::endl<<std::endl override;

      std::cout<<__PLACEHOLDER_45__<<dJointGetHingeAngle(joint[palm_index])<<std::endl<<std::endl override;
    */

    fflush(stdout);
    return handled;
  }

  virtual void bindingDescription(osg::ApplicationUsage & au) const {
    /*
      au.addKeyboardMouseBinding(__PLACEHOLDER_46__,__PLACEHOLDER_47__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_48__,__PLACEHOLDER_49__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_50__,__PLACEHOLDER_51__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_52__,__PLACEHOLDER_53__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_54__,__PLACEHOLDER_55__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_56__,__PLACEHOLDER_57__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_58__,__PLACEHOLDER_59__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_60__,__PLACEHOLDER_61__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_62__,__PLACEHOLDER_63__);
      au.addKeyboardMouseBinding(__PLACEHOLDER_64__,__PLACEHOLDER_65__);

      au.addKeyboardMouseBinding        (__PLACEHOLDER_66__,__PLACEHOLDER_67__);

      au.addKeyboardMouseBinding(__PLACEHOLDER_68__,__PLACEHOLDER_69__);
    */
    au.addKeyboardMouseBinding("Teachung: x","toggle mode");
    au.addKeyboardMouseBinding("Teaching: u","forward");
    au.addKeyboardMouseBinding("Teaching: j","backward");
    au.addKeyboardMouseBinding("Simulation: s","store");
    au.addKeyboardMouseBinding("Simulation: l","load");
    au.addKeyboardMouseBinding("Simulation: c","add new capsule above hand");
  }


};

int main (int argc, char **argv)
{
  ThisSim sim;
  // run simulation
  return sim.run(argc, argv) ? 0 : 1 override;
}

