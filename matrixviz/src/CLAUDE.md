# MatrixViz ARM64 Migration Guide

## Overview

MatrixViz is a Qt4-based visualization tool for matrix and vector data, using OpenGL for rendering. This guide provides comprehensive migration instructions for updating the codebase to modern frameworks compatible with ARM64 macOS.

## Architecture Summary

### Core Components

1. **Main Application** (`MatrixVisualizer`)
   - Qt4 QWidget-based GUI
   - Manages visualization windows and data channels
   - Coordinates pipe reading and rendering

2. **Input Pipeline**
   - `SimplePipeReader`: Threaded stdin reader (QThread)
   - `MatrixPipeFilter`: Data parsing and filtering
   - Reads structured data from stdin pipe

3. **Data Channels** 
   - `AbstractPlotChannel`: Base interface
   - `MatrixPlotChannel`, `VectorPlotChannel`: Data containers
   - Specialized channels for sensors, motors, etc.

4. **Visualizations**
   - `AbstractVisualisation`: QGLWidget-based base class
   - `TextureVisualisation`, `LandscapeVisualisation`, `BarVisualisation`, `VectorPlotVisualisation`
   - Direct OpenGL 1.x/2.x calls

5. **Configuration**
   - XML-based color palettes
   - Config file parsing

## Dependency Graph

```
main.cpp
    └── MatrixVisualizer (QWidget)
        ├── SimplePipeReader (QThread)
        │   └── MatrixPipeFilter
        ├── VisualiserSubWidget (QWidget)
        │   └── AbstractVisualisation (QGLWidget)
        │       ├── TextureVisualisation
        │       ├── LandscapeVisualisation
        │       ├── BarVisualisation
        │       └── VectorPlotVisualisation
        └── Channel System
            ├── MatrixPlotChannel
            ├── VectorPlotChannel
            └── Specialized Channels
```

## Component-Specific Migration Guide

### 1. Qt Framework Migration (Qt4 → Qt6)

**Risk Level**: HIGH  
**Priority**: 1 (Must complete first)

#### Changes Required:

```cpp
// Old Qt4
#include <QtGui>
#include <qgl.h>
QGLWidget

// New Qt6
#include <QtWidgets>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
QOpenGLWidget
```

#### Specific Classes to Update:

1. **AbstractVisualisation.h**
   ```cpp
   // Replace
   class AbstractVisualisation: public QGLWidget
   
   // With
   class AbstractVisualisation: public QOpenGLWidget, protected QOpenGLFunctions
   ```

2. **All visualization classes**
   - Update inheritance from QGLWidget to QOpenGLWidget
   - Add QOpenGLFunctions initialization in initializeGL()

3. **Build System**
   ```qmake
   # Update matrixviz.pro
   QT += core gui widgets opengl
   QT -= qt4support
   
   # Remove
   CONFIG += qt
   
   # Add
   CONFIG += c++17
   ```

### 2. OpenGL Migration (Legacy → Modern)

**Risk Level**: HIGH  
**Priority**: 2

#### Current Issues:
- Uses deprecated immediate mode (glBegin/glEnd)
- Fixed function pipeline
- Legacy GLU dependency

#### Migration Strategy:

1. **Replace Immediate Mode**
   ```cpp
   // Old (TextureVisualisation::paintGL)
   glBegin(GL_QUADS);
   glTexCoord2f(...); glVertex3f(...);
   glEnd();
   
   // New - Use VAO/VBO
   QOpenGLVertexArrayObject vao;
   QOpenGLBuffer vbo;
   // Setup vertex data and draw with glDrawArrays
   ```

2. **Replace GLU Functions**
   ```cpp
   // Old
   gluPerspective(45.0, aspect, 0.1, 100.0);
   
   // New - Use QMatrix4x4
   QMatrix4x4 projection;
   projection.perspective(45.0f, aspect, 0.1f, 100.0f);
   ```

3. **Shader Implementation**
   - Create vertex/fragment shaders for each visualization type
   - Use QOpenGLShaderProgram for shader management

### 3. Threading Model Update

**Risk Level**: MEDIUM  
**Priority**: 3

#### Current Implementation:
- SimplePipeReader extends QThread
- Uses direct run() override
- Signal/slot communication with GUI

#### Required Changes:
1. **Move to Worker Pattern**
   ```cpp
   class PipeReaderWorker : public QObject {
       Q_OBJECT
   public slots:
       void process();
   signals:
       void dataReady(const QString& data);
   };
   
   // In main thread
   QThread* thread = new QThread;
   PipeReaderWorker* worker = new PipeReaderWorker;
   worker->moveToThread(thread);
   ```

2. **Remove sleep/usleep calls**
   - Use QTimer or event-driven approach
   - Implement proper thread synchronization

### 4. Platform-Specific Optimizations

**Risk Level**: LOW  
**Priority**: 4

#### ARM64 SIMD Opportunities:

1. **Matrix Operations** (MatrixPlotChannel)
   ```cpp
   // Add ARM NEON optimizations
   #ifdef __ARM_NEON
   #include <arm_neon.h>
   // Vectorized matrix operations
   #endif
   ```

2. **Color Conversions** (ColorPalette)
   - Use Accelerate framework for color space conversions
   - Optimize palette lookups with SIMD

3. **Texture Updates** (TextureVisualisation)
   - Use Metal Performance Shaders for texture processing
   - Implement double buffering for smooth updates

## Risk Assessment

### Critical Path Components (Must Migrate):
1. **Qt4 → Qt6**: Blocking all other work
2. **QGLWidget → QOpenGLWidget**: Required for Qt6
3. **OpenGL Legacy → Modern**: Required for macOS compatibility

### High Risk Components:
1. **SimplePipeReader**: Threading model outdated
2. **Direct OpenGL calls**: May fail on modern macOS
3. **GLU dependency**: Not available on ARM64

### Medium Risk Components:
1. **Signal/slot connections**: Qt4 syntax deprecated
2. **XML parsing**: Minor API changes
3. **File I/O**: Path handling differences

### Low Risk Components:
1. **Data structures**: Mostly compatible
2. **Color palette system**: Minimal changes needed
3. **Configuration files**: Format unchanged

## Testing Strategy

### Unit Testing Requirements:

1. **Input Pipeline Tests**
   ```cpp
   // Test data parsing
   void testMatrixParsing() {
       MatrixPipeFilter filter;
       QString testData = "1.0 2.0 3.0";
       auto result = filter.parse(testData);
       // Verify matrix dimensions and values
   }
   ```

2. **Visualization Tests**
   - Render to offscreen buffer
   - Verify pixel output
   - Test coordinate transformations

3. **Threading Tests**
   - Data race detection with ThreadSanitizer
   - Stress test with rapid data updates

### Integration Testing:
1. Pipe communication with parent process
2. Real-time data visualization performance
3. Window management and resizing
4. Video frame capture functionality

## Fallback Plans

### If Qt6 Migration Fails:
1. **Option A**: Use Qt5.15 (last LTS with QGLWidget)
   - Still supports modern C++
   - Bridge to eventual Qt6 migration
   
2. **Option B**: Separate Visualization Backend
   - Keep Qt for GUI only
   - Use native Metal/OpenGL directly
   - More complex but maximum performance

### If OpenGL Modern Migration Fails:
1. **Option A**: Use Qt Quick Scene Graph
   - Higher level abstraction
   - Automatic optimization
   
2. **Option B**: Metal Backend
   - Native Apple graphics API
   - Best performance on Apple Silicon
   - Requires Objective-C++ wrapper

### If Threading Issues Persist:
1. **Option A**: Single-threaded with async I/O
   - Use Qt's event loop
   - Non-blocking reads
   
2. **Option B**: Process separation
   - Reader as separate process
   - IPC via shared memory

## Implementation Roadmap

### Phase 1: Foundation (Week 1-2)
1. Set up Qt6 development environment
2. Update build system (CMake recommended)
3. Basic Qt6 compatibility changes
4. Fix compilation errors

### Phase 2: Core Migration (Week 3-4)
1. Replace QGLWidget with QOpenGLWidget
2. Implement basic shader pipeline
3. Update threading model
4. Ensure data flow works

### Phase 3: Optimization (Week 5-6)
1. Modern OpenGL implementation
2. ARM64 optimizations
3. Performance profiling
4. Memory leak fixes

### Phase 4: Testing & Polish (Week 7-8)
1. Comprehensive testing
2. Bug fixes
3. Documentation updates
4. Performance validation

## Performance Optimization Opportunities

### 1. Data Pipeline
- Use memory-mapped files for large datasets
- Implement ring buffer for streaming data
- Batch updates to reduce draw calls

### 2. Rendering Pipeline
- Implement frustum culling
- Use instanced rendering for repeated elements
- Texture atlasing for multiple visualizations

### 3. ARM64 Specific
- Use Accelerate.framework for matrix operations
- Implement NEON optimizations for color conversions
- Profile with Instruments for ARM64

## Critical Success Factors

1. **Maintain Compatibility**: Ensure pipe protocol remains unchanged
2. **Preserve Features**: All visualization modes must work
3. **Performance Target**: 60 FPS for typical datasets
4. **Memory Efficiency**: Handle large matrices without leaks
5. **Error Handling**: Graceful degradation on failures

## Next Steps

1. Review and approve migration plan
2. Set up development environment with Qt6
3. Create feature branch for migration
4. Begin with Qt framework updates
5. Implement incremental changes with testing

This migration will modernize MatrixViz for current and future macOS versions while improving performance on Apple Silicon systems.