# LPZRobots -- a simulator for robotic experiments for Self-Organization of Control

This is a 3D physics simulator that comes with a collection of algorithms, simulations, and tools
developed by the Robotics Group for Self-Organization of Control.

Now this project is translated and migrated from QT4 with QT3 support to QT5 with cmake 4 and make it compatable and compile on Mac ARM. 

## Documentation ##
see the project page: <http://robot.informatik.uni-leipzig.de/software/?lang=en>
and the online help: <http://robot.informatik.uni-leipzig.de/software/doc/html/index.html>

## Overview ##
It consists of the following directories:

  - selforg : controllers together with a small framework for using them,
      developed in the robotic group yielding at self-organized behavior for various kinds of machines.
  - ode_robots : physics simulator based on ODE
      (Open Dynamics Engine, see <http://www.ode.org>).
      This includes robots, obstacles, utilities, stuff for visualization with OSG
        (OpenSceneGraph, see <http://www.openscenegraph.org>) and so on.
  - guilogger : application that coordinates multiple gnuplot
      windows and allows for an interactive display of data that is sent per pipe from another processes. (will be started from ode_robots)
  - matrixvix : application for interactive display of changing matrix and vector data
  - configurator : a library implementing a GUI to change the parameters interactively
        which is otherwise done on the console
  - ga\_tools : genetic algorithms framework that can be used to together with
        ode_robots or for independent simulations (not well maintained) program
  - opende : directory with a snapshot of the open dynamics engine (release 0.11.1)
                  renamed to ode-dbl in order to avoid conflicts with packaged single
                  precision versions. It contains the capsule-box collision bugfix
                  which is upstream (in svn)


**Navigation:** [Main Page](https://www.google.com/search?q=%23robot-simulator-of-the-robotics-group-for-self-organization-of-control) | [Related Pages](https://www.google.com/search?q=%23related-pages) | [Namespaces](https://www.google.com/search?q=%23namespace-list) | [Classes](https://www.google.com/search?q=%23) | [Files](https://www.google.com/search?q=%23) | [Examples](https://www.google.com/search?q=%23examples)

-----

## Documentation of lpzrobots

  - **ODE-Robots Simulations**
  - **ODE-Robots Collision handling**
  - **Colors and Color Alias Sets**
  - **Robot Operators/Manipulators**
  - **Deprecated List**

### Organization:

**Max-Planck Institute for Mathematics in the Sciences** Inselstr. 22, 04103 Leipzig, Germany  
Dr. Georg Martius & Prof. Dr. Ralf Der

**University Leipzig** Institute for Computer Science  
Dept. Intelligent Systems  
Frank Güttler  
Research Group: Technische Informatik

### People:

Georg Martius, Ralf Der, Frank Hesse, Frank Güttler, Jörn Hoffmann

**Former Contributors:** Antonia Siegert, Marcel Kretschmann, Dominic Schneider, Claus Stadler

-----

## General

This is a collection of algorithms, simulations, and tools developed by the Robotics Group for Self-Organization of Control ([http://robot.informatik.uni-leipzig.de](http://robot.informatik.uni-leipzig.de)).

It consists of the following directories (click for details):

  - `selforg`: controllers together with a small framework for using them, developed in the robotic group of Leipzig university yielding at self-organized behavior for various kinds of machines.
  - `ode_robots`: physics simulator based on ODE (Open Dynamics Engine, see [http://www.ode.org](http://www.ode.org)). This includes such as robots, obstacles, utilities, stuff for visualization with OSG (OpenSceneGraph, see [http://www.openscenegraph.org](http://www.openscenegraph.org)) and so on.
  - `guilogger`: application that coordinates multiple gnuplot windows and allows for an interactive display of data that is sent per pipe from another program.
  - `matrixvix`: application for interactive display of changing matrix and vector data.
  - `configurator`: a library implementing a GUI to change the parameters interactively which is otherwise done on the console.
  - `ga_tools`: genetic algorithms framework that can be used to together with ode\_robots or for independent simulations (not well maintained).
  - `opende`: directory with a snapshot of the open dynamics engine (release 0.11.1) renamed to `ode-dbl` in order to avoid conflicts with packaged single precision versions. It contains the capsule-box collision bugfix which is upstream (in svn) (please follow the link for installation hints).

-----

## Installation & Startup

You have two different ways to get the simulator to work.

**A) Install a package for your distribution.** This is the quick 'n' easy way if you have root permissions, see **Way A: Package installation**.

**B) Download the source tar ball and compile the simulator yourself**, see **Way B: Installation from source**.

### Way A: Package installation

#### Ubuntu

A package repository is hosted at [https://launchpad.net/\~georg-martius/+archive/lpzrobots](https://launchpad.net/~georg-martius/+archive/lpzrobots).

Do on a terminal (or read the link: "read about installing" on that page):

```bash
sudo add-apt-repository ppa:georg-martius/lpzrobots
sudo apt-get update
```

Then you can install `lpzrobots` as any other package do:

```bash
sudo apt-get install guilogger lpzrobots-oderobots
```

This will automatically install all other dependencies. That's it\!

Now you can copy the sample simulations from `/usr/share/lpzrobots/` to your home directory and continue reading in section **Run example Simulations**.

#### Debian based systems (deb)

There is an install makefile located here: [http://robot.informatik.uni-leipzig.de/software/packages/deb/install\_deb\_source.makefile](http://robot.informatik.uni-leipzig.de/software/packages/deb/install_deb_source.makefile). Save it on your disk in an empty directory and run on the console:

```bash
su -c 'apt-get install make'
make -f install_deb_source.makefile
```

That will download the newest packages, compile them and install it. If something fails you can also redo parts of the process, see `make -f install_deb_source.makefile help`.

The packages are: `guilogger`, `matrixviz`, `ode-dbl`, `lpzrobots-selforg`, and `lpzrobots-oderobots`. You can uninstall them later using the package manager (apt-get or synaptics).

Alternatively, you can do it step by step as follows: Download all files from [http://robot.informatik.uni-leipzig.de/software/packages/deb/current/](http://robot.informatik.uni-leipzig.de/software/packages/deb/current/).

Then do:

```bash
# here you should not be root
dpkg-source -x guilogger-0.*.dsc
cd guilogger-0.*
dpkg-buildpackage -rfakeroot -b -uc
cd ../

# become root (e.g. with su or sudo -s)
dpkg -i guilogger-0.*.deb
```

This you have to repeat for each package (e.g. replace `guilogger` by `ode-dbl`, `lpzrobots-selforg`, and `lpzrobots-oderobots`).

### Way B: Installation from source

**Linux**
Check the `Dependencies` file for required packages.

**MAC**
We recommend to install the ODE and OSG via macports ([http://www.macports.org](http://www.macports.org)). After installation of macports type on a console `sudo port install osg OpenSceneGraph` and see the `Dependencies` file. We will update the mac-related install procedures soon.

**Instructions**

1.  Download the source tar ball from [http://robot.informatik.uni-leipzig.de/software/current](http://robot.informatik.uni-leipzig.de/software/current).
2.  Unpack file (`tar -xvzf lpzrobots*.tar`).
3.  Change into `lpzrobots` directory.
4.  Call `make help` to get a help display and continue with `make all`. This will do everything. The first time you call it it will configure your build (you can reconfigure later with `make conf`).

The following modules are compiled:

  - `matrixviz` (not strictly necessary, only for displaying neuronal network parameters online)
  - `guilogger` (not strictly necessary, only for displaying parameters online (recommended))
  - `configurator` library (not strictly necessary, only for changing parameters conveniently)
  - `selforg`
  - `opende` (our ode version with double precision) (required)
  - `ode_robots`
  - `ga_tools`

Please note, that the `make` call will not fail if either of them failed to compile, because they are optional. You can type `make guilogger` and `make matrixviz` to compile them separately.

If you do it step by step as displayed with `make help` you need to use `sudo make xxxx` if the installation is into a system directory (`make all` will do use it automatically if required). Note that the `PATH` variable needs to contain the `PREFIX/bin` (as checked by the configuration process). If you have multiple `lpzrobots` installations make sure the prefix for the current `lpzrobots` comes first in the `PATH` variable.

Now you are done with the installation and you can try a sample simulation see next section.

-----

## Run example Simulations

Simulations are located in `ode_robots/simulations/`, `ode_robots/examples/`, and `selforg/simulations/`. These folders you can find in the lpzrobots tar files or if installed on your system under `/usr/share/lpzrobots` or `/usr/local/share/lpzrobots`. In the latter case copy the simulations to your home directory first.

To start a simulation go into a simulation directory.

1.  Call `make` to compile it.
2.  You can start the simulation by `./start`.

For example when you want to start the `template_sphererobot` simulation type:

```bash
cd ode_robots/simulations/template_sphererobot
./start
```

For optimization you can also use `make opt` which produces `start_opt`. This is recommended to use after testing the code.

### Command line options

The following options are available (type `./start -h` for a full list):

`Usage: ./start [-g [interval]] [-f [interval]] [-r seed] [-x WxH] [-fs] [-pause] [-shadow N] [-noshadow] [-drawboundings] [-simtime [min]] [-threads N] [-odethread] [-osgthread] [-savecfg]`

  - `-g interval`: use guilogger (default interval 1)
  - `-f interval`: write logging file (default interval 5)
  - `-m interval`: use matrixviz (default interval 10)
  - `-s "-disc|ampl|freq val"`: use soundMan
  - `-r seed`: random number seed
  - `-x WxH`\*: window size of width(W) x height(H) is used (default 640x480)
  - `-fs`: fullscreen mode
  - `-pause`: start in pause mode
  - `-nographics`: start without any graphics
  - `-noshadow`: disables shadows and shaders (same as -shadow 0)
  - `-shadow [0..5]]`\*: sets the type of the shadow to be used
      - `0`: no shadow, `1`: ShadowVolume, `2`: ShadowTexture, `3`: ParallelSplitShadowMap
      - `4`: SoftShadowMap, `5`: ShadowMap (default)
  - `-shadowsize size`\*: sets the size of the shadow texture (default 2048)
  - `-drawboundings`: enables the drawing of the bounding shapes of the meshes
  - `-simtime min`: limited simulation time in minutes
  - `-savecfg`: save the configuration file with the values given by the cmd line
  - `-threads N`: number of threads to use (default is the number of processors)
  - `-odethread`\*: if given the ODE runs in its own thread. -\> Sensors are delayed by 1
  - `-osgthread`\*: if given the OSG runs in its own thread (recommended)

`*` *this parameter can be set in the configuration file `~/.lpzrobots/ode_robots.cfg`*

On some machines the program might crash right away because of graphic card incompatibilities, try: `./start -noshadow`

Have a look at the console after starting the program, there you will find some further information for the usage of the program. E.g. with `Ctrl+C` (on the terminal) you get an interactive console interface which can be used to modify parameters on the fly.

**Note on MatrixViz keyboard shortcut:** The MatrixViz visualization tool is launched with `Ctrl+V` (not Ctrl+M) when the simulation window is focused. This is because Ctrl+M is intercepted by the terminal as a carriage return. You can also start MatrixViz directly using the `-m` command line option.

For starting your own simulation see paragraph "How to Start Your Own Simulation" in `ode_robots`. For well-documented examples of a `main.cpp` of a simulation and a robot `.cpp` file click the tab "Examples" at the top of this page.

-----

## Documentation

  - This manual can be found at [http://robot.informatik.uni-leipzig.de/software](http://robot.informatik.uni-leipzig.de/software)
  - More information on the used self-organization algorithm is available at [http://robot.informatik.uni-leipzig.de/research](http://robot.informatik.uni-leipzig.de/research)
  - The original ODE documentation can be found at [http://opende.sourceforge.net/wiki/index.php/Main\_Page](http://opende.sourceforge.net/wiki/index.php/Main_Page)
  - The OSG documentation can be found at [http://www.openscenegraph.org/projects/osg/wiki/Support](http://www.openscenegraph.org/projects/osg/wiki/Support)

-----

## Related Pages

### ODE-Robots Simulations

The simulations directory (`ode_robots/simulations`) contains different simulations mostly originated from our work on the self-organization of behavior.

You can just go into one directory, type `make` and after successful compilation `start` to start a simulation.

For convenience you should start with one of the template directories like `template_sphererobot`. Both the simulation file `main.cpp` of the `template_sphererobot` simulation and robot definitions in `sphererobot3masses.cpp` are really well commented and therefore recommended as a first example. You can easily find them by clicking on the tab "Examples" at the top of the page.

For creating your own simulation just type:

```bash
$> ./createNewSimulation.sh template_sphererobot my_sim
```

This creates the directory `my_sim`. Adapt it to your needs, run `make` and `start` and it should work.

### ODE-Robots Collision handling

Since version 0.4 the collision handling was redesigned significantly. It has the following features:

  - Material properties for each object (Substance)
  - No user specific collision code necessary
  - Automatic ignorance of collisions between connected bodies
  - User defined ignorance pairs
  - Fine-grain callback functions on collisions possible (Substance)

**How to set material properties**
Modify `Substance` in your local `OdeHandle`, e.g. `odeHandle.substance.toRubber(20);`

```cpp
// create Primitive with odeHandle
Primitive* box = new Box(1,1,1);
box->init(odeHandle, ...);
```

This would change the substance to a soft rubber. Once you have created a Primitive you can change its properties in the same way: `box.substance.toMetal(0.6);`

**How to disable collisions between two primitives**

```cpp
odeHandle.addIgnoredPair(p1,p2);
// and to remove
odeHandle.removeIgnoredPair(p1,p2);
```

This mechanism is used internally for primitives that are connected by joints. So in most cases you don't have to worry about that.

**How to disable collisions within an entire space**
When you create the space with `odeHandle.createNewSimpleSpace(parentspace,true/false);` you can set the flag to `true` to ignore all internal collisions. Alternatively you can change this for an existing space with: `odeHandle.addIgnoredSpace(spaceID);`

```cpp
// and to remove
odeHandle.removeIgnoredSpace(spaceID);
```

### Colors and Color Alias Sets

Since 0.7 we support color palettes and schemas in `ode_robots`. The basic idea is simple: We have a list of colors (loaded from gimp palette files) that carry a name. We have alias names for these colors to allow some abstraction. Aliases may point to different colors depending on the alias-set. See `lpzrobots::ColorSchema` for the implementation and `lpzrobots::OsgHandle` for the access.

**Example:**
Assume we have colors: Red, Green, Blue. An alias may be called `groundcolor` pointing to Green. So far so good. Now we can define for each alias a color for different color sets. Let's say we define alias `robot1` that points to Red in set 0 and to Blue in set 1. In the implementation of a robot we simply use the color `robot1`, but we can change the default color set (in the `lpzrobots::OsgHandle`) such that the robot magically becomes blue.

**Files:**

  - For an example palette file check: `ode_robots/osg/data/colors/DefaultColors.gpl`
  - The default color-set file is located at: `ode_robots/osg/data/colors/DefaultColorsSchema.txt`

The format is:

```
# comment
AliasName  ColorName [Set]
...
```

where `ColorName` cannot refer to another alias and `Set` is optional (0 by default).

**BTW:** By providing your own alias files you can tune the colors of your simulation without recompiling it. The files are searched relative to the folders: `data/`, `PREFIX/share/lpzrobots/data`, and if the variable `ODE_ROBOTS_DATA` exists, then also there.

There is a big palette file called `RGB_full.gpl` that contains the colors from a book called "Farbwelten" from www.CleverPrinting.de. It is a great resource for colors but it is unfortunately in German, so most of the color names are in German too.

**Handling:**
If an alias is accessed and there is no color found for the current alias-set then the color referred to in the set 0 is used. If this also does not exist then the default color is used.

If you want to change the colors in your simulation you can load a different color-alias-set. See for instance `UrbanColorSchema.txt`, `UrbanExtraColors.gpl` in the colors folder. You need to register them with your simulation in the constructor with `lpzrobots::Simulation::addPaletteFile` and `lpzrobots::Simulation::addColorAliasFile`. An example is given in `ode_robots/simulations/zoo/main.cpp`. You may load them later as well, but the colors used for the environment will not be affected this way.

### Robot Operators/Manipulators

**Introduction**
In robot simulations there is sometimes the necessity to watch the robots and help them, when they got stuck. For example a legged robot may have fallen over, and you wish to automatically flip it back over again. So in a way we need a simulated human operator.

**Implementation**
The base class/interface is defined by `lpzrobots::Operator` where you also find the inherited Operators (`lpzrobots::LimitOrientationOperator` and `lpzrobots::LiftUpOperator`). An Operator is registered to an `lpzrobots::OdeAgent` and is called every simulation step. In case it takes actions it is also visualized by a yellow sphere. See `ode_robots/simulations/hexapod/main` for an example.

-----

## Deprecated List

  - **Member `OdeAgent::getTraceLength ()`** *Use `TrackRobot` parameters.*
  - **Member `OdeAgent::OdeAgent (const std::list< PlotOption > &plotOptions, double noisefactor=1, const std::string &name="OdeAgent", const std::string &revision="$ID$") __attribute__((deprecated))`** *Obsolete. Provide globaldata, see the other constructors.*
  - **Member `OdeAgent::OdeAgent (const PlotOption &plotOption=PlotOption(NoPlot), double noisefactor=1, const std::string &name="OdeAgent", const std::string &revision="$ID$") __attribute__((deprecated))`** *Obsolete. Provide globaldata, see the other constructors.*
  - **Member `OdeAgent::setTraceLength (int tracelength)`** *Use `TrackRobot` parameters.*
  - **Member `OdeAgent::setTraceThickness (int tracethickness)`** *Use `TrackRobot` parameters.*
  - **Member `OdeRobot::collisionCallback (void *data, dGeomID o1, dGeomID o2)`** *This function will be removed in 0.8. Do not use it anymore, collision control is done automatically. In case of a routine return true (collision will be ignored by other objects and the default routine) else false (collision is passed to other objects and (if not treated) to the default routine).*
  - **Member `Simulation::__attribute__ ((deprecated)) void showParams(const ConfigList &configs)`** *This is handled by simulation itself, do not call this function anymore.*

-----

## Namespace List

Here is a list of all namespaces with brief descriptions:

  - **ASHIGARU**
  - **lpzrobots**: Forward declarations
  - **matrix**: Namespace for the matrix library
  - **osg**
  - **osgShadow**
  - **osgText**
  - **qmp\_internal**: A namespace for internal data structures
  - **quickmp**: A namespace for symbols that are part of the public API
  - **quickprof**: The main namespace that contains everything
  - **std**: Some additions to the standard template library

-----

## Examples

Here is a list of all examples:

  - `directconnect/directconnect.cpp`
  - `integration/main.cpp`
  - `matrix/matrixexample.cpp`
  - `nimm4.cpp`
  - `sphererobot3masses.cpp`
  - `template_sphererobot/main.cpp`
  - `wiredconnect/main.cpp`
