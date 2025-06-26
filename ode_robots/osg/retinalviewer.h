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
#ifndef __RETINALVIEWER_H
#define __RETINALVIEWER_H

#include "lpzviewer.h"
#include <iostream>

namespace lpzrobots {

/**
 * Extended LPZViewer that properly handles high-DPI (Retina) displays.
 * This viewer ensures that the viewport is always set to match the framebuffer
 * dimensions, which on Retina displays is typically 2x the window dimensions.
 */
class RetinaLPZViewer : public LPZViewer {
protected:
    mutable bool viewportCorrected = false;
    mutable int correctedWidth = 0;
    mutable int correctedHeight = 0;
    
public:
    RetinaLPZViewer() : LPZViewer(), viewportCorrected(false), correctedWidth(0), correctedHeight(0) {}
    
    RetinaLPZViewer(osg::ArgumentParser& arguments) 
        : LPZViewer(arguments), viewportCorrected(false), correctedWidth(0), correctedHeight(0) {}
    
    RetinaLPZViewer(const osgViewer::Viewer& viewer, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) 
        : LPZViewer(viewer, copyop), viewportCorrected(false), correctedWidth(0), correctedHeight(0) {}
    
    virtual ~RetinaLPZViewer() {}
    
    /** Override frame to ensure viewport is correct before each frame */
    virtual void frame(double simulationTime) override {
        // Check and correct viewport before rendering
        if (!viewportCorrected) {
            correctViewportForRetina();
        }
        
        // Call parent implementation
        LPZViewer::frame(simulationTime);
    }
    
    /** Support the default frame() call */
    virtual void frame() {
        // Check and correct viewport before rendering
        if (!viewportCorrected) {
            correctViewportForRetina();
        }
        
        // Call parent implementation
        LPZViewer::frame();
    }
    
protected:
    void correctViewportForRetina() const
    {
        osg::Camera* camera = const_cast<osg::Camera*>(getCamera());
        if (!camera) return;
        
        osg::GraphicsContext* gc = camera->getGraphicsContext();
        if (!gc) return;
        
        const osg::GraphicsContext::Traits* traits = gc->getTraits();
        if (!traits) return;
        
        // Get current viewport
        const osg::Viewport* vp = camera->getViewport();
        if (!vp) return;
        
        int fbWidth = traits->width;
        int fbHeight = traits->height;
        
        // Check if viewport matches framebuffer
        if (static_cast<int>(vp->width()) != fbWidth || static_cast<int>(vp->height()) != fbHeight) {
            // Use current viewport dimensions as window size
            int winWidth = static_cast<int>(vp->width());
            int winHeight = static_cast<int>(vp->height());
            
            // Determine if we need manual scaling
            int viewportWidth = fbWidth;
            int viewportHeight = fbHeight;
            
            #ifdef __APPLE__
            // On macOS, if traits don't show high-DPI, apply manual scaling
            // Check if framebuffer dimensions match viewport dimensions
            if (fbWidth == winWidth && fbHeight == winHeight && winWidth > 0) {
                // Apply 2x scaling for Retina displays
                viewportWidth = fbWidth * 2;
                viewportHeight = fbHeight * 2;
                std::cout << "RetinaLPZViewer: Applying manual 2x scaling for Retina display" << std::endl;
            }
            #endif
            
            if (viewportWidth != correctedWidth || viewportHeight != correctedHeight) {
                std::cout << "RetinaLPZViewer: Correcting viewport from " 
                          << vp->width() << "x" << vp->height() 
                          << " to " << viewportWidth << "x" << viewportHeight << std::endl;
                
                const_cast<osg::Camera*>(camera)->setViewport(0, 0, viewportWidth, viewportHeight);
                
                // Update projection matrix aspect ratio
                double aspectRatio = static_cast<double>(viewportWidth) / static_cast<double>(viewportHeight);
                double fovy, aspectRatioOld, zNear, zFar;
                camera->getProjectionMatrixAsPerspective(fovy, aspectRatioOld, zNear, zFar);
                const_cast<osg::Camera*>(camera)->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
                
                correctedWidth = viewportWidth;
                correctedHeight = viewportHeight;
            }
            
            viewportCorrected = true;
        }
    }
};

} // namespace lpzrobots

#endif // __RETINALVIEWER_H