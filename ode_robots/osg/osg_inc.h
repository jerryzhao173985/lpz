/***************************************************************************
 *   Copyright (C) 2005-2025 LpzRobots development team                    *
 *                                                                         *
 *   This header provides warning suppression for OpenSceneGraph includes  *
 *   to keep our build output clean while using external libraries.        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OSG_WARNINGS_SUPPRESSED_H
#define OSG_WARNINGS_SUPPRESSED_H

// Suppress warnings from OpenSceneGraph headers
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  #pragma clang diagnostic ignored "-Wunused-parameter"
  #pragma clang diagnostic ignored "-Wshadow"
  #pragma clang diagnostic ignored "-Wconversion"
  #pragma clang diagnostic ignored "-Wsign-conversion"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wshadow"
  #pragma GCC diagnostic ignored "-Wconversion"
  #pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

// Include commonly used OSG headers
// Add more as needed, but keep them centralized here
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Camera>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/GUIEventHandler>
#include <osgGA/GUIActionAdapter>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

// Restore warning settings
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif

#endif // OSG_WARNINGS_SUPPRESSED_H