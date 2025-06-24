/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
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
#ifndef __ONECONTROLLERPERCHANNEL_H
#define __ONECONTROLLERPERCHANNEL_H

#include <functional>
#include <selforg/abstractcontroller.h>
#include <vector>

/** generator for controller

    derive a struct and overload the operator. For example:

    struct ControlGen : public ControllerGenerator {
      virtual ~ControlGen() {}
      virtual AbstractController* operator()( int index) {
       AbstractController* c;
        c= new Sox(0.8);
        c->setParam(__PLACEHOLDER_0__,0.02);
        c->setParam(__PLACEHOLDER_1__,0.01);
       return c;
      }
    };

    __PLACEHOLDER_2__
    agent = new OdeAgent(global);
    __PLACEHOLDER_3__
    agent->addInspectable((static_cast<OneControllerPerChannel*>(controller))->getControllers()[0]);
    ....
    Make sure you initialize the OneControllerPerChannel with sufficiently many initial controller.

*/
struct ControllerGenerator {
  using argument_type = int;
  using result_type = AbstractController*;
  virtual ~ControllerGenerator() {}
  virtual AbstractController* operator()(int index) = 0;
};

/**
 * class for using multiple controller, one for each joint. Each controller
 * has dimension 1x1. The controller are generated on the fly with a generator object
 *
 */
class OneControllerPerChannel : public AbstractController {
public:
  /** @param controllerGenerator generator object for controller
      @param controllerName name
      @param numCtrlCreateBeforeInit number of controller that are generated before the init
     function is called. Useful if they should be put into the inspectable list of the agent
      @param numContextSensors number of context sensors (counted from the end)
       passed to all controllers
   */
  OneControllerPerChannel(ControllerGenerator* controllerGenerator,
                          std::string controllerName,
                          int numCtrlCreateBeforeInit = 1,
                          int numContextSensors = 0);

  virtual ~OneControllerPerChannel();

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0);

  virtual void step(const sensor* sensors, int sensornumber, motor* motors, int motornumber);

  virtual void stepNoLearning(const sensor* sensors,
                              int sensornumber,
                              motor* motors,
                              int motornumber);

  virtual int getSensorNumber() const override {
    return sensornumber;
  }
  virtual int getMotorNumber() const override {
    return motornumber;
  }

  /*********** STORABLE **************/

  virtual bool store(FILE* f) const override;

  virtual bool restore(FILE* f);

  virtual std::vector<AbstractController*> getControllers() const  override {
    return ctrl;
  }

protected:
  std::vector<AbstractController*> ctrl;
  ControllerGenerator* controllerGenerator;
  int numCtrlCreateBeforeInit = 0;
  int numContextSensors = 0;
  int motornumber = 0;
  int sensornumber = 0;
  double* sensorbuffer;
};

#endif
