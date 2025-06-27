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
#ifndef __ODEAGENT_H
#define __ODEAGENT_H

#include <selforg/agent.h>
#include <selforg/storeable.h>
#include <selforg/trackrobots.h>
#include <selforg/plotoption.h>
#include "oderobot.h"
#include "osgprimitive.h"
#include "primitive.h"
#include "operator.h"

namespace lpzrobots {
  class Joint;     // forward declaration
  
  class TraceDrawer{
  public:
    TraceDrawer() : obj(0), initialized(false) {}
    Position lastpos;
    Trackable* obj;
    TrackRobot tracker;
    Color color;
    void init();
    void close();
    /// actually write the log files and stuff
    void track(double time);
    /// draw the trace
    void drawTrace(GlobalData& global);
  protected:
    bool initialized = false;
    std::list<osg::Vec3> pnts;
  };


  // PlotOptionList is already defined in globaldata.h
  using OperatorList = std::list<Operator*>;
  using TraceDrawerList = std::list<TraceDrawer>;

  /** Specialised agent for ode robots
   */
  class OdeAgent : public Agent, public Storeable {
  public:

    /** @deprecated obsolete provide globaldata, see the other constructors
     */
    explicit OdeAgent(const ::PlotOption& plotOption = ::PlotOption(::PlotMode::NoPlot), double noisefactor = 1, const std::string& name = "OdeAgent", const std::string& revision = "$ID$") __attribute__ ((deprecated));
    /** @deprecated obsolete provide globaldata, see the other constructors
     */
    OdeAgent(const std::list<::PlotOption>& plotOptions, double noisefactor = 1, const std::string& name = "OdeAgent", const std::string& revision = "$ID$") __attribute__ ((deprecated));
    /** The plotoptions are taken from globaldata
        @param noisefactor factor for sensor noise for this agent
     */
    OdeAgent(const GlobalData& globalData, double noisefactor = 1, const std::string& name = "OdeAgent", const std::string& revision = "");
    /** Provided for convinience. A single plotoption is used as given by plotOption */
    OdeAgent(const GlobalData& globalData, const ::PlotOption& plotOption, double noisefactor = 1, const std::string& name = "OdeAgent", const std::string& revision = "");
    /** Provided for convinience. The plotoptions are taken from the given plotOptions
        (and not from globaldata, if you wish to overwrite them)
    */
    OdeAgent(const GlobalData& globalData, const PlotOptionList& plotOptions, double noisefactor = 1, const std::string& name = "OdeAgent", const std::string& revision = "");
    virtual ~OdeAgent();

    // Bring base class init methods into scope
    using Agent::init;
    
    /** initializes the object with the given controller, robot and wiring
        and initializes plotoptionengine
    */
    virtual bool init(AbstractController* controller, OdeRobot* robot, AbstractWiring* wiring,
                      long int seed = 0) {
      return Agent::init(controller, robot, wiring, seed);
    }

    virtual void step(double noise, double time) override;

    /**
     * Returns a pointer to the robot.
     */
    virtual OdeRobot* getRobot() { return static_cast<OdeRobot*>(robot);}
    
    /**
     * Returns a const pointer to the robot.
     */
    virtual const OdeRobot* getRobot() const override { return static_cast<OdeRobot*>(robot);}

    /** @deprecated use TrackRobot parameters */
    virtual int getTraceLength() {return 0;}

    /** @deprecated use TrackRobot parameters */
    virtual bool setTraceLength(int tracelength) {return true;}

    /** @deprecated use TrackRobot parameters */
    virtual void setTraceThickness(int tracethickness) { }

    /// adds tracking for individual primitives
    virtual void addTracking(unsigned int primitiveIndex,const TrackRobot& trackrobot,
                             const Color& color);
    virtual void setTrackOptions(const TrackRobot& trackrobot) override;
    virtual bool stopTracking() override;


    /****** STOREABLE **********/
    virtual bool store(FILE* f) const override;
    virtual bool restore(FILE* f) override;


    /****** OPERATORS *********/
    /// adds an operator to the agent (the operator is deleted on destruction of the agent!)
    virtual void addOperator(Operator* o, bool addToConfigurable = true );

    /** removes the given operator: it is _not_ deleted (memory wise)
        @return true on success
     */
    virtual bool removeOperator(Operator* o);
    /// removes (and deletes) all operators
    void removeOperators();

    /** fixates the given primitive of the robot at its current position to the world
        for a certain time.
        Hint: use getRobot()->moveToPosition() to get the robot relocated
        @param primitiveID if -1 then the main primitive is used, otherwise the primitive with the given index
        @param time time to fixate in seconds (if ==0 then indefinite)
     */
    virtual void fixateRobot(GlobalData& global, int primitiveID=-1, double time = 0);
    /// release the robot in case it is fixated and turns true in this case
    virtual bool unfixateRobot(GlobalData& global);

  protected:

    /**
     * continues the trace by one segment
     */
    virtual void trace(GlobalData& global);

  private:
    void constructor_helper(const GlobalData* globalData);

    TraceDrawer mainTrace;

    OperatorList operators;

    TraceDrawerList segmentTracking;
  };

}

#endif
