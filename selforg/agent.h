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
#ifndef __AGENT_H
#define __AGENT_H

#include <cstdio>
#include <list>
#include <string>

#include "wiredcontroller.h"
#include "randomgenerator.h"
#include "abstractrobot.h"
#include "trackrobots.h"

/**
 * The Agent contains a controller, a robot and a wiring, which connects both.
 * Additionally there is a plotOptionEngine to output information about the
 * controller, robot and wiring.
 */
class Agent : public WiredController {
public:
  /** constructor. PlotOption as output setting.
      noisefactor is used to set the relative noise strength of this agent
   */
  Agent(const PlotOption& plotOption = PlotOption(PlotMode::NoPlot), double noisefactor = 1, const Inspectable::iparamkey& name = "Agent", const Inspectable::iparamkey& revision = "$ID");
  /** constructor. A list of PlotOption can given.
      noisefactor is used to set the relative noise strength of this agent
   */
  Agent(const std::list<PlotOption>& plotOptions, double noisefactor = 1, const Inspectable::iparamkey& name = "Agent", const Inspectable::iparamkey& revision = "$ID");

  /** destructor
   */
  virtual ~Agent() override;

  // Bring base class methods into scope to avoid hiding
  using WiredController::init;
  using WiredController::step;

  /** initializes the object with the given controller, robot and wiring
      and initializes the output options.
      It is also possible to provide a random seed,
       if not given static_cast<0>(rand)() is used to create one
  */
  virtual bool init(AbstractController* controller, AbstractRobot* robot,
                    AbstractWiring* wiring, long int seed=0);

  /** Performs an step of the agent, including sensor reading, pushing sensor values through the wiring,
      controller step, pushing controller outputs (= motorcommands) back through the wiring and sent
      resulting motorcommands to robot.
      @param noise Noise strength.
      @param time static_cast<optional>(current) simulation time (used for logging)
  */
  virtual void step(double noise, double time=-1);

  /** Sends only last motor commands again to robot.  */
  virtual void onlyControlRobot();


  /** Returns a pointer to the robot.
   */
  virtual const AbstractRobot* getRobot() const { return robot; }

  /// sets the trackoptions which starts spatial tracking of a robot
  virtual void setTrackOptions(const TrackRobot& trackrobot);

  /// stop tracking (returns true of tracking was on);
  virtual bool stopTracking();

  /// returns the tracking options
  virtual TrackRobot getTrackOptions() const { return trackrobot; }

protected:

  AbstractRobot* robot;

  sensor *rsensors;
  motor  *rmotors;

  RandGen randGen; // random generator for this agent

  TrackRobot trackrobot;
  // int t; // removed - already defined in parent class WiredController


};

#endif
