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
/*
** Started on  Mon Oct 22 10:50:47 2007 Georg Martius
** Last update Mon Oct 22 10:50:47 2007 Georg Martius
*/
#ifndef     WIREDCONTROLLER_H_
#define     WIREDCONTROLLER_H_

#include "plotoptionengine.h"
#include "backcaller.h"
#include "types.h"
#include "inspectable.h"
#include "configurable.h"
#include "randomgenerator.h"
#include "abstractcontroller.h"
#include "abstractwiring.h"

#include <cstdio>
#include <list>
#include <utility>
#include <string>


/**
 * The WiredController contains functionality to wrap 
 * a robot controller with wiring and noise generator.
 */
class WiredController : public Inspectable, public Configurable {
public:
  /** constructor. PlotOption as output setting.
      noisefactor is used to set the relative noise strength of this agent
   */
  WiredController(const PlotOption& plotOption = PlotOption(PlotMode::NoPlot), double noisefactor = 1, const iparamkey& name = "WiredController", const iparamkey& revision = "$ID");
  /** constructor. A list of PlotOption can given.
      noisefactor is used to set the relative noise strength of this agent
   */
  WiredController(const std::list<PlotOption>& plotOptions, double noisefactor = 1, const iparamkey& name = "WiredController", const iparamkey& revision = "$ID");

  /** destructor
   */
  virtual ~WiredController() override;

  /** initializes the object with the given controller and wiring
      and initializes the output options
      It is also possible to provide a random seed,
       if not given static_cast<0>(rand)() is used to create one
  */
  virtual bool init(AbstractController* controller, AbstractWiring* wiring,
                    int robotsensornumber, int robotmotornumber,
                    const std::list<SensorMotorInfo>& robotSensorInfos,
                    const std::list<SensorMotorInfo>& robotMotorInfos,
                    RandGen* randGen=nullptr);

  /** Performs an step of the controller, which includes
      pushing sensor values through the wiring,
      controller step,
      pushing controller outputs (= motorcommands) back through the wiring
      @param sensors sensors inputs scaled to [-1,1]
      @param sensornumber length of the sensor array
      @param motors motors outputs. MUST have enough space for motor values!
      @param motornumber length of the provided motor array

      @param noise Noise strength.
      @param time static_cast<optional>(current) simulation time (used for logging)
  */
  virtual void step(const sensor* sensors, int sensornumber,
                    motor* motors, int motornumber,
                    double noise, double time=-1);

  /** Enables the motor babbling mode for given number of steps (typically 1000).
      Optionally a controller can be
      given that is used for the babbling (default is MotorBabbler) (deleted automatically).
      During motor babbling the function motorbabbling of the normal controller is called instead of step.
   */
  virtual void startMotorBabblingMode(int steps, AbstractController* babblecontroller = 0);

  virtual const AbstractController* getMotorBabbler() const { return motorBabbler; }

  /** stops the motor babbling mode. */
  virtual void stopMotorBabblingMode() { motorBabblingSteps = 0; }
  /// returns true if in motorbabbling mode
  virtual bool getMotorBabblingMode() { return motorBabblingSteps > 0; }


  /** adds the PlotOptions to the list of plotoptions
      If a plotoption with the same Mode exists, then the old one is deleted first
   */
  virtual PlotOption addPlotOption(const PlotOption& plotoption);

  /** adds a new PlotOption and initializes it
      @see addPlotOption
  */
  virtual bool addAndInitPlotOption(const PlotOption& plotOption);

  /** removes the PlotOptions with the given type
      @return true if sucessful, false otherwise
   */
  virtual bool removePlotOption(const PlotMode& mode);

  /**
     write comment to output streams (PlotOptions). For instance changes in parameters.
     see PlotOptionEngine
  */
  virtual void writePlotComment(const char* cmt, bool addSpace=true);

  /** Returns a pointer to the controller.
   */
  virtual const AbstractController* getController() const { return controller;}

  /** Returns a pointer to the wiring.
   */
  virtual const AbstractWiring* getWiring() const { return wiring;}

protected:
  /**
   * Plots controller sensor- and motorvalues and internal controller parameters.
   * @param time simulation time
   */
  virtual void plot(double time);


  AbstractController* controller;
  AbstractWiring* wiring;

  /// number of sensors of robot
  int rsensornumber = 0;
  /// number of motors of robot
  int rmotornumber = 0;
  /// number of sensors of comntroller
  int csensornumber = 0;
  /// number of motors of comntroller
  int cmotornumber = 0;

  /// factor that is  muliplied with noise stength
  double noisefactor = 0;

  motor  *cmotors;
  sensor *csensors;

  void internInit();

 protected:
  AbstractController* motorBabbler;
  int motorBabblingSteps = 0;

  PlotOptionEngine plotEngine;

  bool initialised = false;

  std::list<Callbackable* > callbackables;

  long int t;
};

#endif       /* !WIREDCONTROLLER_H_ */
