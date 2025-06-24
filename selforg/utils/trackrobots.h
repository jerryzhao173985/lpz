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
#ifndef __TRACKROBOTS_H
#define __TRACKROBOTS_H

#include <cstdio>
#include <string>

#include <selforg/trackable.h>

// Forward declarations
class AbstractRobot;
namespace lpzrobots {
  class OdeAgent;
  class TraceDrawer;
}

struct TrackRobotConf {
  bool trackPos = true;              ///< whether to track position
  bool trackSpeed = false;           ///< whether to track speed
  bool trackOrientation = false;     ///< whether to track orientation
  bool displayTrace = false;         ///< display the trace
  int displayTraceDur = 60;          ///< duration of trace display
  double displayTraceThickness = 0.0;///< thickness of trace cylinder
  int interval = 1;                  ///< interval between tracking events
  bool writeFile = true;             ///< whether to write to file
  bool autoFilename = true;          ///< generate filename automatically

  std::string scene; ///< used as part of the filename (used as is (+id), if autoFilename=false)
  int id = 0;
};

/**
 * This class provides tracking capabilities for robots.
 */
class TrackRobot {
public:
  friend class Agent;
  friend class lpzrobots::OdeAgent;
  friend class lpzrobots::TraceDrawer;

  /// constructor for no tracking at all
  explicit TrackRobot(TrackRobotConf conf = getDefaultConf())
    : conf(conf)
    , enabledDuringVideo(false)
    , file(0)
    , cnt(1) {}

  static TrackRobotConf getDefaultConf() {
    TrackRobotConf conf;
    conf.trackPos = true;
    conf.trackSpeed = false;
    conf.trackOrientation = false;
    conf.displayTrace = false;
    conf.displayTraceDur = 60;
    conf.displayTraceThickness = 0.0;
    conf.interval = 1;
    conf.writeFile = true;
    //    conf.scene           = __PLACEHOLDER_0__;
    conf.id = -1; // disabled
    conf.autoFilename = true;
    return conf;
  }

  /** Constructor that allows individial setting of tracking options.
      The tracked data is written into a file with the current date and time appended by a name
     given by scene.
      @param trackPos if true the trace (position vectors) of the robot are logged
      @param trackSpeed if true the speed vectors (linear and angular) of the robot are logged
      @param trackOrientation if true the orientation matrices  of the robot are logged
      @param displayTrace if true the trace of the robot should be displayed (used in ODE
     simulations)
      @param scene name of the scene (is appended to log file name)
      @param interval timesteps between consequent logging events (default 1)
   */
  TrackRobot(bool trackPos,
             bool trackSpeed,
             bool trackOrientation,
             bool displayTrace,
             const char* scene = "",
             int interval = 1) {
    conf = getDefaultConf();
    conf.trackPos = trackPos;
    conf.trackSpeed = trackSpeed;
    conf.trackOrientation = trackOrientation;
    conf.displayTrace = displayTrace;
    conf.interval = interval;
    conf.scene = scene;
    conf.id = -1; // whole robot, not individual parts
    file = 0;
    cnt = 1;
    enabledDuringVideo = false;
  }

  ~TrackRobot() {}

  /// returns whether tracing is activated
  bool isDisplayTrace() const {
    return conf.displayTrace;
  };

  /// returns whether something is to be tracked
  bool isTrackingSomething() const {
    return conf.trackPos || conf.trackOrientation || conf.trackSpeed;
  };

  bool isEnabled() {
    return file != 0 && isTrackingSomething();
  }

  TrackRobotConf conf;

  bool enabledDuringVideo;

protected:
  bool open(const Trackable* robot);
  void track(const Trackable* robot, double time);
  void close();

protected:
  FILE* file;
  long cnt;
};

#endif
