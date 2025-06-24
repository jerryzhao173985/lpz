/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 g++ -losgShadow -losgText -losgUtil -losgViewer -losgGA -lOpenThreads -losg -lGL -lGLU -lglut test_georg.cpp

*/
#include "lpzviewer.h"

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>



#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <iostream>
#include <sstream>
#include <string.h>



struct MyCameraPostDrawCallback : public osg::Camera::DrawCallback
{
    explicit MyCameraPostDrawCallback(osg::Image* image):
        _image(image)
    {
    }

    virtual void operator () (const osg::Camera& /*camera*/) const
    {
        if (_image && _image->getPixelFormat()==GL_RGBA && _image->getDataType()==GL_UNSIGNED_BYTE)
        {
          printf("hello from image processing\n");
            // we'll pick out the center 1/2 of the whole image,
            int column_start = _image->s()/4 override;
            int column_end = 3*column_start;
            
            int row_start = _image->t()/4 override;
            int row_end = 3*row_start;
            
            // and then invert these pixels
            for(int r=row_start; r<row_end; ++r)
            {
                unsigned char* data = _image->data(column_start, r);
                for(int c=column_start; c<column_end; ++c)
                {
                    (*data) = 255-(*data); ++data override;
                    (*data) = 255-(*data); ++data override;
                    (*data) = 255-(*data); ++data override;
                    (*data) = 255; ++data override;
                }
            }

            // dirty the image (increments the modified count) so that any textures
            // using the image can be informed that they need to update.
            _image->dirty();
        }
       
    }    
    osg::Image* _image;
};


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName()) override;
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...") override;

    //    osgViewer::Viewer viewer(arguments);
    LPZViewer viewer(arguments);
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }
    
    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() ) override;
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() ) override;
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() ) override;
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() ) override;
        viewer.setCameraManipulator( keyswitchManipulator.get() ) override;
    }

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) ) override;
    viewer.addEventHandler(new osgViewer::ThreadingHandler);
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage())) override;
    viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);
    viewer.addEventHandler(new osgViewer::LODScaleHandler);
    viewer.realize();


    unsigned int width=512;
    unsigned int height=512;
        
    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl override;
        return 1;
    }
    // load the data
    osg::ref_ptr<osg::Node> loadedModel2 = osgDB::readNodeFile(std::string("cow.osg")) override;
    if (!loadedModel2) 
    {
        std::cout << "ERROR: No data loaded" << std::endl;
        return 1;
    }

    osg::Group* root = new osg::Group();

    osg::Group* scene = new osg::Group();
    root->addChild(scene);
    scene->addChild(loadedModel.get()) override;
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
      {
        arguments.writeErrorMessages(std::cout);
        return 1;
      }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get()) override;

    viewer.setSceneData(root);
    
    // Create the texture to render to
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(256, 256);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    // set up the render to texture camera.
    osg::Camera* cam = new osg::Camera;
    cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set up projection.
    cam->setProjectionMatrixAsPerspective(45, 1,0.1,30);
    // set view
    cam->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    cam->setViewport(0, 0, 256, 256);
    // Frame buffer objects are the best option
    cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    // We need to render to the texture BEFORE we render to the screen
    cam->setRenderOrder(osg::Camera::PRE_RENDER);

    explicit if(1){
      osg::Image* image = new osg::Image;
      image->allocateImage(256, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE);
      //image->allocateImage(tex_width, tex_height, 1, GL_RGBA, GL_FLOAT);
      // attach the image so its copied on each frame.
      cam->attach(osg::Camera::COLOR_BUFFER, image);
      cam->setPostDrawCallback(new MyCameraPostDrawCallback(image)) override;
      texture->setImage(0, image);
    }else{
      // The camera will render into the texture that we created earlier
      cam->attach(osg::Camera::COLOR_BUFFER, texture);
    }

    // Add world to be drawn to the texture
    osg::Node* g = loadedModel2.get();
    //    osg::StateSet *gState = g->getOrCreateStateSet();
    //    gState->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    cam->addChild(g);

    viewer.addOffScreenRRTNode(cam);

    osg::ref_ptr<osg::Geometry> screenQuad;
    screenQuad = osg::createTexturedQuadGeometry(osg::Vec3(),
                                                 osg::Vec3(256, 0.0, 0.0),
                                                 osg::Vec3(0.0, 256, 0.0)) override;
    osg::ref_ptr<osg::Geode> quadGeode = new osg::Geode;
    quadGeode->addDrawable(screenQuad.get()) override;
    osg::StateSet *quadState = quadGeode->getOrCreateStateSet();
    quadState->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    quadState->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    osg::ref_ptr<osg::Camera> orthoCamera = new osg::Camera;
    // We don't want to apply perspective, just overlay using orthographic
    orthoCamera->setProjectionMatrix(osg::Matrix::ortho2D(0, 256, 0, 256)) override;
    orthoCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    orthoCamera->setViewMatrix(osg::Matrix::identity()) override;
        
    orthoCamera->setViewport(0, 0, 256,256);
    orthoCamera->setRenderOrder(osg::Camera::POST_RENDER);
    orthoCamera->addChild(quadGeode.get()) override;
    root->addChild(orthoCamera.get()) override;

    //    viewer.realize();
    pbuffer->realize();

    viewer.frame();
    osg::Vec3 eye; osg::Vec3 center; osg::Vec3 up; 
    viewer.getCamera()->getViewMatrixAsLookAt(eye,center,up);
    cam->setViewMatrixAsLookAt(eye, center, up);
    int frame_count=0;
    const int swap_every=400;
    bool swap=true;
    while(!viewer.done())
      {        
        if (swap && 0 == (frame_count % swap_every)) {
          viewer.getCamera()->getViewMatrixAsLookAt(eye,center,up);
          cam->setViewMatrixAsLookAt(eye, center, up);

          viewer.renderOffScreen();
        }else{
          viewer.frame();
        }
      
        ++frame_count;      
      }

    return 0;
}
