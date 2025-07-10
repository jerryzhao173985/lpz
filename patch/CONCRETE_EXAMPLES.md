# Modernization Examples

## Traditional For Loops → Range-Based
Found in 541 files. Here are some examples:

**ga_tools/examples/robot_chain_tasked_GA_Simulation/main.cpp:663**
```cpp
// Current
for(int hy=0;hy<mi.size();++hy)
// Could be
for (auto& item : mi) {
```

**ecbrobots/GUIs/SphericalRobotGUI/src/InputReader/SimplePipeReader.cpp:101**
```cpp
// Current
for ( int i = 0;i < string_list.size();++i )
// Could be
for (auto& item : string_list) {
```

**matrixviz/src/ColorPalette.cpp:256**
```cpp
// Current
for(int i = 0; i < stops.size(); ++i)
// Could be
for (auto& item : stops) {
```

**configurator/src/QConfigurator.cpp:503**
```cpp
// Current
for (int index = 0; index < nodeList.size(); ++index)
// Could be
for (auto& item : nodeList) {
```

**ecbrobots/QECBRobotsWindow.cpp:603**
```cpp
// Current
for (int index = 0; index < nodeList.size(); ++index)
// Could be
for (auto& item : nodeList) {
```

**matrixviz/src/InputReader/SimplePipeReader.cpp:103**
```cpp
// Current
for ( int i = 0;i < string_list.size();++i )
// Could be
for (auto& item : string_list) {
```

**ecbrobots/GUIs/SphericalRobotGUI/src/InputReader/SimplePipeReader.cpp:122**
```cpp
// Current
for ( int i = 0;i < string_list.size();++i )
// Could be
for (auto& item : string_list) {
```

**matrixviz/src/ColorPalette.cpp:290**
```cpp
// Current
for(int i = 0; i < stops.size(); ++i)
// Could be
for (auto& item : stops) {
```

**matrixviz/src/InputReader/SimplePipeReader.cpp:119**
```cpp
// Current
for ( int i = 0;i < string_list.size();++i )
// Could be
for (auto& item : string_list) {
```

**ode_robots/simulations/hand/invertnchannelcontrollerhebbhhand.cpp:133**
```cpp
// Current
for (int i=0; i<context_sensors.size(); ++i)
// Could be
for (auto& item : context_sensors) {
```


## typedef → using
Found in 316 files. Here are some examples:

**include/ode_robots/robots/hexabotsensormotordefinition.h:184**
```cpp
// Current
typedef struct {
  //Unit m, kg
  double length;
// Could be
using length = struct {
  //Unit m, kg
  double;
```

**include/selforg/feedbackwiring.h:46**
```cpp
// Current
typedef enum {Motor=1, Context=2, All=3} Mode;
// Could be
using Mode = enum {Motor=1, Context=2, All=3};
```

**selforg/examples/integration/cmdline.h:7**
```cpp
// Current
typedef vector<Configurable*> ConfigList;
// Could be
using ConfigList = vector<Configurable*>;
```

**ode_robots/include/ode-dbl/odecpp.h:1338**
```cpp
// Current
typedef dHinge2JointTemplate<dODECPP_JOINT_TEMPLATE_BASE, dODECPP_WORLD_TEMPLATE_BASE, dODECPP_BODY_TEMPLATE_BASE> dHinge2Joint;
// Could be
using dHinge2Joint = dHinge2JointTemplate<dODECPP_JOINT_TEMPLATE_BASE, dODECPP_WORLD_TEMPLATE_BASE, dODECPP_BODY_TEMPLATE_BASE>;
```

**selforg/simulations/spherical_xbee/commtest/serial_unix.h:38**
```cpp
// Current
typedef unsigned char uint8;
// Could be
using uint8 = unsigned char;
```


## Iterator Declarations → auto
Many opportunities for auto. Here are some examples:

**ga_tools/GenPrototype.cpp:47**
```cpp
// Current
std::map<Generation*,GenContext*>::iterator itr = ...
// Could be
auto itr = ...
```

**ode_robots/utils/quickprof.h:518**
```cpp
// Current
std::map<std::string, ProfileBlock*>::iterator iter = ...
// Could be
auto iter = ...
```

**matrixviz/src/InputFilter/AbstractPipeFilter.h:82**
```cpp
// Current
std::list<double>::iterator i = ...
// Could be
auto i = ...
```

**matrixviz/src/InputFilter/AbstractPipeFilter.h:164**
```cpp
// Current
std::list<std::string>::iterator i = ...
// Could be
auto i = ...
```

**selforg/statistictools/measure/complexmeasure.cpp:69**
```cpp
// Current
std::list<Discretisizer*>::iterator di = ...
// Could be
auto di = ...
```

