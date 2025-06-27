# LPZRobots GUI Tools Comprehensive Analysis

## Executive Summary

LPZRobots provides three essential GUI tools for robotics research:
- **guilogger**: Real-time data plotting and logging (Qt6 ready, build fails)
- **matrixviz**: Neural network weight visualization (Qt6 ready, build fails)
- **configurator**: Runtime parameter adjustment (needs Qt6 migration)

All tools use pipe-based communication protocols for real-time data exchange with simulations.

## 1. guilogger - Real-time Data Logger and Plotter

### Purpose
Provides real-time visualization of robot sensor values, motor commands, and controller internal states during simulation.

### Architecture

#### Communication Protocol
```
Simulation → stdout pipe → QPipeReader → ChannelData → Gnuplot
```

Data format:
- `#C <channel_names>` - Channel definition line
- `#D <values>` - Data values line
- `#N` - Network description for neural architectures

#### Core Components

1. **QPipeReader** (qpipereader.cpp)
   - Reads from stdin or file in separate thread
   - Handles up to 64MB buffer for large matrices
   - Emits Qt signals for new data

2. **ChannelData** (channeldata.cpp)
   - Manages channel names and time series data
   - Supports channel filtering and selection
   - Maintains data history for plotting

3. **Gnuplot Integration** (gnuplot.cpp)
   - Uses popen() for bidirectional communication
   - Platform-aware: macOS doesn't support -geometry
   - Terminal set to "qt" for cross-platform support

4. **PlotInfo** (plotinfo.cpp)
   - Manages plot windows and channel assignments
   - Supports multiple plot styles per window

### Data Flow

1. **Simulation Side** (selforg/utils/plotoption.cpp):
   ```cpp
   PlotOption opt(GuiLogger, 1);  // Mode and interval
   plotEngine.addPlotOption(opt);
   plotEngine.plot(time);  // Sends data through pipe
   ```

2. **guilogger Side**:
   - QPipeReader reads lines from stdin
   - Parses #C for channel names, #D for data
   - Updates ChannelData storage
   - Triggers plot updates via timer

### Key Features
- Multiple plot windows
- Real-time data streaming
- File logging capability
- Channel filtering
- Configuration save/load
- Cross-platform gnuplot integration

### Build Status
- Qt6 compatible (using Qt5 currently)
- Build error: `reset()` method missing in PlotChannelsTableModel
- Minor warnings: sprintf deprecation, unused parameters

## 2. matrixviz - Matrix and Network Visualizer

### Purpose
Visualizes neural network weights, connection matrices, and sensor/motor mappings as they evolve during learning.

### Architecture

#### Communication Protocol
```
Simulation → stdout pipe → MatrixPipeFilter → PlotChannels → Visualizations
```

Matrix format:
- `#M <matrix_name> <rows> <cols>` - Matrix definition
- `#V <vector_name> <size>` - Vector definition
- Followed by data values

#### Core Components

1. **MatrixPipeFilter** (MatrixPipeFilter.cpp)
   - Parses matrix/vector definitions
   - Creates appropriate PlotChannel objects
   - Routes data to visualizations

2. **Visualization Types**:
   - **TextureVisualisation**: Heatmap display using OpenGL textures
   - **LandscapeVisualisation**: 3D surface plot
   - **BarVisualisation**: Bar chart representation
   - **VectorPlotVisualisation**: Time series plots

3. **AbstractVisualisation** (base class)
   - Inherits from QOpenGLWidget and QOpenGLFunctions
   - Provides OpenGL rendering framework
   - Color palette management

4. **ColorPalette** System
   - XML-based color scheme definitions
   - Gradient interpolation
   - Multiple predefined palettes

### OpenGL Integration
- Migrated from QGLWidget to QOpenGLWidget (Qt5)
- Uses QOpenGLFunctions for platform abstraction
- Shader-based rendering for performance

### Key Features
- Real-time matrix visualization
- Multiple visualization modes
- Interactive scaling and rotation
- Frame capture for videos
- Color palette customization
- Automatic layout adjustment

### Build Status
- Qt6 ready (QOpenGLWidget compatible)
- Build errors: Missing widget includes
- Needs: `#include <QWidget>`, `#include <QPushButton>`, etc.

## 3. configurator - Runtime Parameter Adjustment

### Purpose
Provides GUI for adjusting robot and controller parameters during simulation without recompilation.

### Architecture

#### Communication Mechanism
```
Simulation ← ConfiguratorProxy → QConfigurator → QConfigurableWidget
```

Uses Configurable interface from selforg library for introspection.

#### Core Components

1. **ConfiguratorProxy** (ConfiguratorProxy.cpp)
   - Creates configurator in separate thread
   - Manages lifetime and callbacks
   - Bridge between simulation and Qt GUI

2. **QConfigurator** (QConfigurator.cpp)
   - Main window managing configurable objects
   - Hierarchical parameter display
   - Load/save functionality
   - Auto-save capability

3. **QConfigurableWidget** Types:
   - **QValConfigurableTileWidget**: Float/double parameters
   - **QIntConfigurableTileWidget**: Integer parameters
   - **QBoolConfigurableTileWidget**: Boolean switches
   - Tile-based layout with grouping

4. **Configurable Interface** (selforg/configurable.h)
   - Provides parameter introspection
   - Type information and bounds
   - Hierarchical organization
   - Change notifications

### Parameter Management
- Real-time parameter updates
- Bounds checking and validation
- Undo/redo functionality
- Parameter search and filtering
- Profile save/load

### Integration with Simulations
```cpp
// In simulation
ConfiguratorProxy configurator(configList);
// Automatically creates GUI thread

// Controllers/robots implement Configurable
class MyController : public AbstractController {
  void addParameterDef("eps", &eps, 0.1, 0.0, 1.0);
  // name, pointer, default, min, max
};
```

### Build Status
- Currently Qt5, needs Qt6 migration
- VERSION.txt conflict resolved
- Builds but needs testing

## 4. Integration with Simulations

### Data Flow Architecture

1. **Simulation → GUI Tools**:
   ```
   Agent::step() → WiredController::step() → PlotOptionEngine::plot()
     ↓
   stdout pipe (text protocol)
     ↓
   guilogger/matrixviz (parse and display)
   ```

2. **PlotOption Modes**:
   ```cpp
   enum PlotMode {
     NoPlot,
     File,           // Log to file
     GuiLogger,      // Plot with guilogger
     GuiLogger_File, // Both plot and log
     MatrixViz,      // Matrix visualization
     ECBRobotGUI     // Special ECB interface
   };
   ```

3. **Typical Usage**:
   ```cpp
   // In main simulation
   PlotOption guilogger(GuiLogger, 1);     // Every timestep
   PlotOption matrixviz(MatrixViz, 10);    // Every 10 steps
   globalData.plotoptions.push_back(guilogger);
   globalData.plotoptions.push_back(matrixviz);
   ```

### Performance Considerations

1. **Data Rate Management**:
   - Interval parameter controls update frequency
   - Channel filtering reduces data volume
   - Buffering in pipe readers

2. **Threading Model**:
   - GUI tools run in separate processes
   - Pipe communication is non-blocking
   - Qt event loop handles updates

3. **Memory Usage**:
   - guilogger: 64MB buffer for large datasets
   - matrixviz: OpenGL texture memory for matrices
   - configurator: Minimal, only parameter metadata

## 5. Modernization Status

### Completed
- ✅ Qt6 framework compatibility (QOpenGLWidget)
- ✅ C++17 standard compliance
- ✅ Platform detection for macOS
- ✅ ARM64 architecture support

### Remaining Issues

1. **guilogger**:
   - Fix `reset()` method in table model
   - Replace sprintf with snprintf
   - Test gnuplot on ARM64 macOS

2. **matrixviz**:
   - Add missing widget includes
   - Verify OpenGL on macOS ARM64
   - Test with large matrices

3. **configurator**:
   - Complete Qt6 migration
   - Update build configuration
   - Test parameter synchronization

### Build Commands
```bash
# guilogger
cd guilogger && qmake && make

# matrixviz (after fixes)
cd matrixviz && qmake && make

# configurator
cd configurator && qmake && make
```

## 6. Protocol Specifications

### guilogger Protocol
```
# Channel definition
#C <channel1> <channel2> ... <channelN>

# Data line
#D <value1> <value2> ... <valueN>

# Comment
# This is a comment

# Network description (optional)
#N neural_net <name>
#N layer <layer_name> <rank>
#N neuron <id> <bias>
#N connection <id> <from> <to>
#N nn_end
```

### matrixviz Protocol
```
# Matrix definition
#M <name> <rows> <cols>
<value11> <value12> ... <value1cols>
<value21> <value22> ... <value2cols>
...

# Vector definition  
#V <name> <size>
<value1> <value2> ... <valueN>

# Update existing matrix/vector
#U <name>
<new values...>
```

### configurator Protocol
- Direct C++ object introspection
- No text protocol, uses Configurable interface
- Parameter changes via method calls

## 7. Research Workflow Integration

### Typical Usage Scenario

1. **Start Simulation with GUI**:
   ```bash
   ./start -g              # Start with guilogger
   ./start -m              # Start with matrixviz
   ./start -c              # Start with configurator
   ./start -g -m -c        # All tools
   ```

2. **Real-time Monitoring**:
   - guilogger: Watch sensor/motor values
   - matrixviz: Observe weight evolution
   - configurator: Tune parameters live

3. **Data Analysis**:
   - Save guilogger data for offline analysis
   - Capture matrixviz frames for videos
   - Export parameter sets from configurator

4. **Reproducibility**:
   - Log files preserve complete data
   - Parameter configurations can be saved
   - Visualization settings are persistent

## 8. Future Enhancements

### Proposed Improvements

1. **Unified GUI Framework**:
   - Single application with dockable tools
   - Shared communication backend
   - Synchronized time controls

2. **Enhanced Protocols**:
   - Binary format option for efficiency
   - Compression for large matrices
   - Bidirectional communication

3. **Modern Features**:
   - GPU acceleration for visualization
   - Web-based interface option
   - Real-time collaboration support

4. **Integration**:
   - Jupyter notebook support
   - Python bindings
   - ROS compatibility

## Conclusion

The LPZRobots GUI tools form an essential part of the research workflow, enabling real-time monitoring, parameter tuning, and data visualization. While they need minor fixes for full Qt6/ARM64 compatibility, the architecture is sound and the tools are well-integrated with the simulation framework. The pipe-based communication provides a clean separation between simulation and visualization, allowing for flexible deployment and easy extension.