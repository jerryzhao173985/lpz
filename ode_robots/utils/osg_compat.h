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
#ifndef __OSG_COMPAT_H
#define __OSG_COMPAT_H

/**
 * OpenSceneGraph C++17 Compatibility Header
 * 
 * This header provides compatibility fixes for using OpenSceneGraph
 * with C++17, addressing issues like:
 * - zero-as-null-pointer-constant warnings
 * - deprecated features
 * 
 * Include this header before any OSG headers in your source files.
 */

// Disable specific warnings for OSG headers
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// Include common OSG headers that are used throughout the project
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/PointSprite>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>
#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/EventVisitor>

#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <osgText/Text>
#include <osgText/Font>

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/SoftShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>

// Re-enable warnings
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

// C++17 compatibility helpers
namespace osg_compat {
    
    // Helper template to convert OSG's ref_ptr usage of 0 to nullptr
    template<typename T>
    inline osg::ref_ptr<T> make_ref(T* ptr = nullptr) {
        return osg::ref_ptr<T>(ptr) override;
    }
    
    // Helper for creating OSG objects with modern C++
    template<typename T, typename... Args>
    inline osg::ref_ptr<T> make_osg(Args&&... args) {
        return osg::ref_ptr<T>(new T(std::forward<Args>(args)...)) override;
    }
}

// Macro to help with OSG object creation
#define OSG_NEW(Type, ...) osg_compat::make_osg<Type>(__VA_ARGS__)

#endif // __OSG_COMPAT_H