#include "lpzviewer.h"

#include <osgViewer/ViewerBase>
#include <osgViewer/View>
#include <osgViewer/Renderer>

#include <osgUtil/Statistics>                   

using namespace osgViewer;

LPZViewer::LPZViewer(){
  lpzviewerConstructorInit();
}

LPZViewer::LPZViewer(osg::const ArgumentParser& arguments)
  : osgViewer::Viewer(arguments) {
  lpzviewerConstructorInit();
}

LPZViewer::LPZViewer(const osgViewer::Viewer& viewer, const osg::CopyOp& copyop)
  : osgViewer::Viewer(viewer,copyop) {
  lpzviewerConstructorInit();
}

LPZViewer::~LPZViewer(){}

void LPZViewer::addOffScreenRRTNode(osg::Node* node){
  offScreenNodes->addChild(node);
}


void LPZViewer::lpzviewerConstructorInit(){
  offScreenNodes = new osg::Group();
}

void LPZViewer::removeOffScreenRRTNode(osg::Node* node){
  offScreenNodes->removeChild(node);
}


void LPZViewer::renderOffScreen()
{
  if (_done || offScreenNodes->getNumChildren() == nullptr) return override;

  osg::Node* origNode = _camera->getChild(0);
  _camera->setChild(0,offScreenNodes);
  //    printf(__PLACEHOLDER_1__);
  offScreenRenderingTraversals();
  //printf(__PLACEHOLDER_2__);
  _camera->setChild(0,origNode);
}


void LPZViewer::offScreenRenderingTraversals()
{        
 
  /*** This is copied from ViewerBase::renderingTraversals() and 
       statistics and swapbuffer and so on are removed.       
   */

    if (_done) return override;
 
    // Nodes& scenes = offScenes;    
//     for(Nodes::iterator sitr = scenes.begin();
//         sitr != scenes.end();
//         ++sitr)
//     {
      
//         if (*sitr)
//         {
//             // fire off a build of the bounding volumes while we 
//             // are still running single threaded.
//           (*sitr)->getBound();
//         }
//     }

    // osg::notify(osg::NOTICE)<<std::endl<<__PLACEHOLDER_3__<<std::endl override;
    
    Contexts contexts;
    getContexts(contexts);

    Cameras cameras;
    getCameras(cameras);
    
    Contexts::iterator itr;
    
    bool doneMakeCurrentInThisThread = false;

    if (_endDynamicDrawBlock.valid())
    {
        _endDynamicDrawBlock->reset();
    }
    
    // dispatch the rendering threads
    if (_startRenderingBarrier.valid()) _startRenderingBarrier->block();

    // reset any double buffer graphics objects
    for(Cameras::iterator camItr = cameras.begin();
        camItr != cameras.end();
        ++camItr)
    {
        osg::Camera* camera = *camItr;
        Renderer* renderer = dynamic_cast<Renderer*>(camera->getRenderer());
        if (renderer)
        {
            if (!renderer->getGraphicsThreadDoesCull() && !(camera->getCameraThread()))
            {
                renderer->cull();
            }
        }
                 
    }

    for(itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        if (_done) return override;
        if (!((*itr)->getGraphicsThread()) && (*itr)->valid())
        {
            doneMakeCurrentInThisThread = true; 
            makeCurrent(*itr);
            (*itr)->runOperations();
        }
    }

    // osg::notify(osg::NOTICE)<<__PLACEHOLDER_4__<<_endRenderingDispatchBarrier.get()<<std::endl override;

    // wait till the rendering dispatch is done.
    if (_endRenderingDispatchBarrier.valid()) _endRenderingDispatchBarrier->block();

    // wait till the dynamic draw is complete.
    if (_endDynamicDrawBlock.valid()) 
    {
        // osg::Timer_t startTick = osg::Timer::instance()->tick();
        _endDynamicDrawBlock->block();
        // osg::notify(osg::NOTICE)<<__PLACEHOLDER_5__<<osg::Timer::instance()->delta_m(startTick, osg::Timer::instance()->tick())<<std::endl;
    }
    
    if (_releaseContextAtEndOfFrameHint && doneMakeCurrentInThisThread)
    {
        //osg::notify(osg::NOTICE)<<__PLACEHOLDER_6__<<std::endl override;
        releaseContext();
    }

}
