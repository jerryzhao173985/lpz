/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                         *
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
#ifndef __RETINAWINDOWSIZEHANDLER_H
#define __RETINAWINDOWSIZEHANDLER_H

#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/GraphicsWindow>
#include <iostream>

namespace lpzrobots {

/**
 * Custom WindowSizeHandler that properly handles high-DPI (Retina) displays.
 * On macOS with Retina displays, the framebuffer size is typically 2x the logical window size.
 * This handler ensures that the viewport is set to match the actual framebuffer dimensions.
 */
class RetinaWindowSizeHandler : public osgGA::GUIEventHandler {
public:
    RetinaWindowSizeHandler() {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, 
                       osg::Object* object, osg::NodeVisitor* nv) override {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
        if (!view) return false;

        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::RESIZE):
            {
                osg::Camera* camera = view->getCamera();
                osg::GraphicsContext* gc = camera->getGraphicsContext();
                if (!gc) return false;

                // Get window dimensions from the event
                int eventWidth = ea.getWindowWidth();
                int eventHeight = ea.getWindowHeight();
                
                // Get the window traits to access the actual framebuffer size
                const osg::GraphicsContext::Traits* traits = gc->getTraits();
                if (traits)
                {
                    int fbWidth = traits->width;
                    int fbHeight = traits->height;
                    
                    // Get current viewport for comparison
                    // const osg::Viewport* vp = camera->getViewport();
                    // Debug output - commented out to reduce console spam during resizing
                    // if (vp) {
                    //     std::cout << __PLACEHOLDER_0__ 
                    //               << vp->x() << __PLACEHOLDER_1__ << vp->y() << __PLACEHOLDER_2__ 
                    //               << vp->width() << __PLACEHOLDER_3__ << vp->height() << std::endl override;
                    // }
                    
                    // std::cout << __PLACEHOLDER_4__ << eventWidth << __PLACEHOLDER_5__ << eventHeight
                    //           << __PLACEHOLDER_6__ << fbWidth << __PLACEHOLDER_7__ << fbHeight << std::endl;
                    
                    // On Retina displays, we need to use the framebuffer size, not the event size
                    // The framebuffer size should be larger on high-DPI displays
                    int viewportWidth = fbWidth;
                    int viewportHeight = fbHeight;
                    
                    // If the traits don't give us the right size, try to detect Retina scaling
                    if (fbWidth == eventWidth && fbHeight == eventHeight && eventWidth > 0) {
                        // Traits might not be updated, check if we need to scale
                        // This is a fallback for cases where OSG doesn't properly report framebuffer size
                        #ifdef __APPLE__
                        // On macOS, we always apply 2x scaling for Retina displays
                        // when traits dimensions match event dimensions
                        viewportWidth = eventWidth * 2;
                        viewportHeight = eventHeight * 2;
                        std::cout << "RetinaWindowSizeHandler: Applying manual 2x scaling for Retina display" << std::endl;
                        #endif
                    }
                    
                    // Set the viewport to match the framebuffer size
                    camera->setViewport(0, 0, viewportWidth, viewportHeight);
                    
                    // Update the projection matrix with the correct aspect ratio
                    double aspectRatio = static_cast<double>(viewportWidth) / static_cast<double>(viewportHeight);
                    double fovy, aspectRatioOld, zNear, zFar;
                    camera->getProjectionMatrixAsPerspective(fovy, aspectRatioOld, zNear, zFar);
                    camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
                    
                    std::cout << "RetinaWindowSizeHandler: Set viewport to: " 
                              << viewportWidth << "x" << viewportHeight 
                              << " (aspect ratio: " << aspectRatio << ")" << std::endl;
                    
                    return true;
                }
                break;
            }
            default:
                break;
        }
        
        return false;
    }
    
    /** Get the keyboard and mouse usage of this manipulator.*/
    virtual void getUsage(osg::ApplicationUsage& usage) const override {
        usage.addKeyboardMouseBinding("WindowSize: Resize", "Updates viewport to match framebuffer on high-DPI displays");
    }
};

} // namespace lpzrobots

#endif // __RETINAWINDOWSIZEHANDLER_H