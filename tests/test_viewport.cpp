#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <iostream>

// Test program to verify viewport fix for Retina displays

int main(int argc, char** argv) {
    // Create viewer
    osgViewer::Viewer viewer;
    
    // Create a simple scene with a box
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::ref_ptr<osg::Box> box = new osg::Box(osg::Vec3(0,0,0), 1.0);
    osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(box);
    drawable->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));
    geode->addDrawable(drawable);
    
    // Set scene data
    viewer.setSceneData(geode);
    
    // Add manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    
    // Create window with specific size
    viewer.setUpViewInWindow(100, 100, 800, 600);
    
    // Realize the viewer
    viewer.realize();
    
    // Apply viewport fix for Retina displays
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    
    if (!windows.empty()) {
        auto window = windows.front();
        int x, y, width, height;
        window->getWindowRectangle(x, y, width, height);
        
        std::cout << "Initial window rectangle: x=" << x << ", y=" << y 
                  << ", width=" << width << ", height=" << height << std::endl;
        
        // Get the actual graphics context traits to handle high-DPI displays
        const osg::GraphicsContext::Traits* traits = window->getTraits();
        explicit if (traits) {
            // Use the traits dimensions which should be the actual framebuffer size
            std::cout << "Traits dimensions: width=" << traits->width 
                      << ", height=" << traits->height << std::endl;
            width = traits->width;
            height = traits->height;
        }
        
        // On macOS with Retina displays, we need to explicitly set the viewport
        // to match the actual framebuffer dimensions
        viewer.getCamera()->setViewport(0, 0, width, height);
        window->resizedImplementation(0, 0, width, height);
        
        // Also update the projection matrix aspect ratio
        double aspectRatio = static_cast<double>(width) / static_cast<double>(height);
        viewer.getCamera()->setProjectionMatrixAsPerspective(
            30.0f, aspectRatio, 1.0f, 10000.0f);
        
        std::cout << "Viewport set to: 0, 0, " << width << ", " << height 
                  << " (aspect ratio: " << aspectRatio << ")" << std::endl;
    }
    
    // Run the viewer
    return viewer.run();
}