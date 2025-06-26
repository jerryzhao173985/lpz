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
#ifndef __SIMULATION_H
#define __SIMULATION_H

#include <ode-dbl/ode.h>
#include <osg/ApplicationUsage>
#include <osg/Vec3>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osgGA/KeySwitchMatrixManipulator>
#include <list>
#include <vector>
#include <string>
#include "utils/globaldata.h"
#include "osg/base.h"

// forward declarations
namespace osgViewer {
  class Viewer;
}

namespace lpzrobots {
  class OdeHandle;
  class OsgHandle;
  // GlobalData is included from utils/globaldata.h
  class LPZViewer;
  class OdeAgent;
  class PlotOption;
  typedef std::list<class Configurable*> ConfigList;
}
/*** end of forward declarations ***/


namespace lpzrobots {

  class Simulation : public Base, public osgGA::GUIEventHandler, public Callbackable {
  public:

    enum SimulationState { none, initialised, running, closed };
    enum CameraMode {Static=0, Follow, TV, Race};

    Simulation();
    virtual ~Simulation();

    /** starts the Simulation. Do not overload it.
        This function returns of the simulation is terminated.
        @return: true if closed regulary, false on error
    */
    bool run(int argc, char** argv);

    // the following function have to be overloaded.

    /// start() is called at the first start of the cycles and should create all the object (obstacles, agents...).
    virtual void start(const OdeHandle&, const OsgHandle&, GlobalData& globalData) = 0;

    // the following functions have dummy default implementations

    /**
     * restart() is called at the second and all following starts of the cylce
     * The end of a cycle is determined by (simulation_time_reached==true)
     * @param the odeHandle
     * @param the osgHandle
     * @param globalData
     * @return if the simulation should be restarted; this is false by default
     */
    virtual bool restart(const OdeHandle&, const OsgHandle&, GlobalData& globalData);

    /// end() is called at the end and should tidy up
    virtual void end(const GlobalData& globalData);
    /** config() is called when the user presses Ctrl-C
        @return false to exit program, true otherwise
    */
    virtual bool config(const GlobalData& globalData);
    /** is called if a key was pressed.
        For keycodes see: osgGA::GUIEventAdapter
        @return true if the key was handled
    */
    virtual bool command(const OdeHandle&, const OsgHandle&, GlobalData& globalData_,
                         int key, bool down) { return false; }

    /** this can be used to describe the key bindings used by command()
     */
    virtual void bindingDescription(osg::ApplicationUsage & au) const {};

    /** this can be used to print additional usage information (cmd-line options)
     */
    virtual void usage() const {};

    /** collCallback() can be used to overload the standart collision handling.
        However it is called after the robots collision handling.
        @return true if collision is treated, false otherwise
    */
    virtual bool collCallback(const OdeHandle&, void* data, dGeomID o1, dGeomID o2) { return false; }

    /** optional additional callback function which is called every simulation step.
        Called between physical simulation step and drawing.
        @param draw indicates that objects are drawn in this timestep
        @param pause always false (only called of simulation is running)
        @param control indicates that robots have been controlled this timestep
     */
    virtual void addCallback(const GlobalData& globalData_, bool draw_, bool pause_, bool ctrl) {};

    /** adds a palette file to be loaded at initialization time
        Call this before run()!
     */
    virtual void addPaletteFile(const std::string& filename, bool verbose = false);
    /** adds a color alias file to be loaded at initialization time
        Call this before run()!
     */
    virtual void addColorAliasFile(const std::string& filename, bool verbose = false);

    virtual void odeStep();

    virtual void osgStep();

    virtual void doOnCallBack(BackCaller *src, BackCaller::CallbackableType type=BackCaller::DEFAULT_CALLBACKABLE_TYPE);

  protected:
    // GUIEventHandler
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    virtual void getUsage (osg::ApplicationUsage & au) const;
    virtual bool init(int argc, char** argv);

    virtual void updateGraphics(); ///< update the graphics objects

    /** define the home position and view orientation of the camera.
        view.x is the heading angle in degree. view.y is the tilt angle in degree (nick),
        view.z is ignored
    */
    void setCameraHomePos(const osg::Vec3& eye, const osg::Vec3& view);

    /**
     * Sets the mode of the camera, the numbers are the same like the keys
     * @param mode see CameraMode
     */
    void setCameraMode(const CameraMode& mode);

    /// start video recording (write frames to namestatic_cast<XXX>(folder))
    bool startVideoRecording(const char* name);
    /// stop video recording
    bool stopVideoRecording();

    /**
     * Sets the agent to be watched with the camera.
     * @param agent to set
     */
    void setWatchedAgent(OdeAgent* agent);

    /// returns the watched agent (or 0)
    const OdeAgent* getWatchedAgent() const;

    static void nearCallback_TopLevel(void *data, dGeomID o1, dGeomID o2);
    static void nearCallback(void *data, dGeomID o1, dGeomID o2);
    bool control_c_pressed();

    // plotoptions is a list of possible online output,
    // if the list is empty no online gnuplot windows and no logging to file occurs.
    // The list is modified with commandline options, see run() in simulation.cpp
    std::list<::PlotOption>& plotoptions; // points now to globaldata.plotoptions


    /**
     *  shows all parameters of all given configurable objects
     *  @deprecated this is handled by simulation itself, do not call this function anymore
     */
    __attribute__ ((deprecated)) void showParams(const ConfigList& configs) {}

  private:
    void insertCmdLineOption(int& argc,char**& argv);
    bool loop();
    /// clears obstacle and agents lists and delete entries
    void tidyUp(GlobalData& globalData);

  protected:
    /// returns false if the program is to exit
    virtual bool processCmdLine(int argc, char** argv);
    void resetSyncTimer();
    long timeOfDayinMS();

    std::list<std::pair<std::string, double> > parseKeyValuePairs(const std::string& kv);
  private:
    static void control_c(int i);
    static void cmd_handler_exit();
    static void cmd_handler_init();
    static void cmd_begin_input();
    static void cmd_end_input();

    // Commandline interface stuff
    static void main_usage(const char* progname);

    bool storeOdeRobotsCFG();


  protected:
    GlobalData globalData;
    class VideoStream* videostream;

    long realtimeoffset = 0;
    long simtimeoffset = 0;
    double truerealtimefactor = 0; // calculated true speed
    bool justresettimes = false;      // true if we just reset sync times
    bool drawContacts = false;

    int windowWidth;
    int windowHeight;

    int defaultFPS;      // default framerate

    bool pause = false;
    bool simulation_time_reached = false;
    long int simulation_time;
    bool noGraphics = false;
    bool useKeyHandler = false;

    // use globalData.sim_step instead
     //long sim_step;

    int guiloggerinterval = 0;
    int filelogginginterval = 0;
    int matrixvizinterval = 0;

    /// parameters for configurables set on commandline
    std::string initConfParams;

    char odeRobotsCfg[256]; /// < filename of config file

    //  CameraType camType; // default is a non-moving and non-rotating camera
    //  OdeRobot* viewedRobot; // the robot who is viewed from the camera

    /// the current cycle; the simulation restarts if restart() returns true
    int currentCycle = 0;

    class CameraHandle* cameraHandle;

    bool useOdeThread;
    bool useOsgThread;
    bool useQMPThreads; // decides if quick mp is used in this simulation
    bool inTaskedMode;

    std::string windowName;

  protected:
    SimulationState state;
    osg::ArgumentParser* arguments;
    LPZViewer* viewer; // inherited from osgViewer::Viewer
    osgGA::KeySwitchMatrixManipulator* keyswitchManipulator;

    static int ctrl_C;
    char** orig_argv;

    std::list<std::string> paletteFiles;
    std::list<std::string> colorAliasFiles;
    bool verboseColorLoading = false;


    // -conf detected in console arguments?
    bool startConfigurator = false;

    // multiprocessoring stuff
    pthread_t odeThread;
    pthread_t osgThread;
    bool odeThreadCreated = false;
    bool osgThreadCreated = false;

  private:
    bool commandline_param_dummy = false;

  };

  /** creates a new directory with the stem base, which is not yet there
      (using subsequent numbers)  and returns its name in newdir */
  void createNewDir(const char* base, char *newdir);
}

#endif
