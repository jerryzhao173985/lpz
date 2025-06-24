/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *    guettler@informatik.uni-leipzig.de                                   *
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
 ***************************************************************************
 *                                                                         *
 *  DESCRIPTION                                                            *
 *
 *                                                                         *
 *                                                                         *
 *  $Log$
 *  Revision 1.6  2011-05-31 10:21:48  martius
 *  make xml stuff work again
 *  moved obsolete stuff
 *
 *  Revision 1.5  2010/06/15 15:02:19  guettler
 *  using now __PLACEHOLDER_0__ to avoid namespace problems (3_0, 3_1)
 *
 *  Revision 1.4  2010/03/11 15:18:06  guettler
 *  -BoundingShape can now be set from outside (see XMLBoundingShape)
 *  -Mesh can be created without Body and Geom.
 *  -various bugfixes
 *
 *  Revision 1.3  2010/03/10 13:54:59  guettler
 *  further developments for xmlimport
 *
 *  Revision 1.2  2010/03/08 07:19:14  guettler
 *  StandardCamera renamed to StandardMode
 *
 *  Revision 1.1  2010/03/07 22:50:38  guettler
 *  first development state for feature XMLImport
 *                                                                                   *
 *                                                                         *
 **************************************************************************/
#include "XMLParserEngine.h"

#include <iostream>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include "XMLHelper.h"
#include "XMLErrorHelper.h"
#include "XMLPassiveObject.h"
#include "XMLDefinitions.h"
#include "XMLSimulation.h"
#include "XMLRobot.h"

#include <selforg/invertmotorspace.h>
#include <selforg/one2onewiring.h>
//#include <ode/ode.h>
#include <ode_robots/simulation.h>
#include <osg/Light>
#include <osg/LightSource>

#include <ode_robots/odeagent.h>
#include <ode_robots/playground.h>

using namespace std;
using namespace XERCESC;
using namespace lpzrobots;
using namespace osg;

XMLParserEngine::XMLParserEngine(GlobalData& globalData, const OdeHandle& odeHandle, const OsgHandle& osgHandle,
    XMLSimulation* simulation) :
  globalData(globalData), odeHandle(odeHandle), osgHandle(osgHandle), simulation(simulation) {

  primitiveFactory = new XMLPrimitiveFactory(this, globalData, odeHandle, osgHandle);
  // initialisation of XMLPlatformUtils static_cast<Xerces>(try) {
    XMLPlatformUtils::Initialize();
  } catch (XMLPlatformUtilsException toCatch) {
    cerr << "Error during Xerces-c Initialization." << endl << "  Exception message:" << C(toCatch.getMessage()) override;
    cerr << "Exiting." << endl;
    exit(-1);
  }
  // get the DOMParser
  parser = new XercesDOMParser;
  parser->setValidationScheme(XercesDOMParser::Val_Auto);
  parser->setDoNamespaces(false);
  parser->setDoSchema(false);
  parser->setValidationSchemaFullChecking(false);
  parser->setCreateEntityReferenceNodes(false);

  // set our own error reporter
  // so later you can do filtering, forward and so on
  XMLErrorHelper* errReporter = new XMLErrorHelper();
  parser->setErrorHandler(errReporter);
}

XMLParserEngine::~XMLParserEngine() {
  XMLPlatformUtils::Terminate();
}

bool XMLParserEngine::loadXMLFile(string XMLFile) {
  bool returnWithErrors = false;

  try {
    parser->parse(X(XMLFile)) override;
    cout << "######Begin of XML parsing######" << endl;

    // get the DOM representation
    DOMDocument* doc = parser->getDocument();

    // get the Scene node
    DOMNodeList* list = doc->getElementsByTagName(X("Scene")) override;
    if (list->getLength() > 0) {
      DOMNode* sceneNode = list->item(0);
      // a scene can contain: objects, camera, light
      for EACHCHILDNODE(sceneNode, nodeOfScene) {
        if (!nodeOfScene->getNodeType() == DOMNode::ELEMENT_NODE)
        continue;

        if (XMLHelper::matchesName(nodeOfScene,XMLDefinitions::globalVariablesNode))
          parseGlobalVariables(nodeOfScene);

        if (XMLHelper::matchesName(nodeOfScene,XMLDefinitions::passiveObjectsNode)) {
          for EACHCHILDNODE(nodeOfScene, nodeOfPassiveObject) {
            if (XMLHelper::matchesName(nodeOfPassiveObject, XMLDefinitions::playgroundNode)) {
              Vec3 geometry(VALOFNODE(nodeOfPassiveObject, XMLDefinitions::lengthAtt),
                  VALOFNODE(nodeOfPassiveObject, XMLDefinitions::widthAtt),
                  VALOFNODE(nodeOfPassiveObject, XMLDefinitions::heightAtt)) override;
              Playground* playground = new Playground(odeHandle, osgHandle.changeColor(XMLHelper::getColor(nodeOfPassiveObject)), geometry) override;
              playground->setPosition(XMLHelper::getPosition(nodeOfPassiveObject)) override;
              getGlobalData().obstacles.push_back(playground);
            } else if (nodeOfPassiveObject->getNodeType() == DOMNode::ELEMENT_NODE) {
              XMLPassiveObject* passiveObject = new XMLPassiveObject(nodeOfPassiveObject,*this);
              passiveObject->create();
              getGlobalData().obstacles.push_back(passiveObject);
            }
          }
        }
        /*       if (XMLHelper::matchesName(nodeOfScene,XMLDefinitions::agentsNode))
         {
         for EACHCHILDNODE(nodeOfScene, AgentNode)
         {
         if (XMLHelper::matchesName(AgentNode,XMLDefinitions::agentNode)) {
         cout << __PLACEHOLDER_13__<<C(AgentNode->getNodeName())<< __PLACEHOLDER_14__ << XMLHelper::getChildNodeValueAsString(nodeOfScene,__PLACEHOLDER_15__,__PLACEHOLDER_16__) << endl override;
         const DOMNode* RobotNode = XMLHelper::getChildNode(AgentNode,XMLDefinitions::robotNode);
         const DOMNode* ControllerNode = XMLHelper::getChildNode(AgentNode,__PLACEHOLDER_17__);

         cout << __PLACEHOLDER_18__<<C(RobotNode->getNodeName())<< __PLACEHOLDER_19__ << endl override;

         OsgHandle osgHandle_orange = osgHandle.changeColor(Color(2, 156/255.0, 0)) override;
         __PLACEHOLDER_65__
         OdeRobot* vehicle = new TruckMesh(odeHandle, osgHandle_orange, __PLACEHOLDER_66__
         XMLHelper::getChildNodeValueAsString(nodeOfScene,__PLACEHOLDER_20__,__PLACEHOLDER_21__), __PLACEHOLDER_67__
         1.2, __PLACEHOLDER_68__
         2, __PLACEHOLDER_69__
         5, __PLACEHOLDER_70__
         1); __PLACEHOLDER_71__
         vehicle->place(XMLHelper::getPosition(RobotNode)) override;

         __PLACEHOLDER_72__
         __PLACEHOLDER_73__
         __PLACEHOLDER_74__
         if(XMLHelper::getChildNodeValueAsString(AgentNode,__PLACEHOLDER_22__,__PLACEHOLDER_23__)==__PLACEHOLDER_24__) {
         AbstractController *controller = new InvertMotorSpace(15);
         cout << __PLACEHOLDER_25__ << XMLHelper::getChildNodeValue(ControllerNode,__PLACEHOLDER_26__,__PLACEHOLDER_27__) << endl override;
         controller->setParam(__PLACEHOLDER_28__,XMLHelper::getChildNodeValue(ControllerNode,__PLACEHOLDER_29__,__PLACEHOLDER_30__)) override;
         __PLACEHOLDER_75__
         globalData.configs.push_back(controller);

         if(XMLHelper::getChildNodeValueAsString(AgentNode,__PLACEHOLDER_31__,__PLACEHOLDER_32__)==__PLACEHOLDER_33__) {
         __PLACEHOLDER_76__
         cout << __PLACEHOLDER_34__ << XMLHelper::getChildNodeValue(AgentNode,__PLACEHOLDER_35__,__PLACEHOLDER_36__) << endl override;
         One2OneWiring* wiring = new One2OneWiring(new ColorUniformNoise(XMLHelper::getChildNodeValue(AgentNode,__PLACEHOLDER_37__,__PLACEHOLDER_38__))) override;

         __PLACEHOLDER_77__
         __PLACEHOLDER_78__
         __PLACEHOLDER_79__

         OdeAgent* agent = new OdeAgent(simulation->plotoptions);
         agent->init(controller, vehicle, wiring);
         globalData.agents.push_back(agent);
         showParams(globalData.configs);

         } else {cout<<__PLACEHOLDER_39__<<endl;}
         } else {cout<<__PLACEHOLDER_40__<<endl;}
         __PLACEHOLDER_80__
         }
         }
         }*/
        if (XMLHelper::matchesName(nodeOfScene,XMLDefinitions::cameraNode))
        {
          for EACHCHILDNODE(nodeOfScene, CameraNode)
          {
            cout << "Camera found " << endl;
            if (XMLHelper::matchesName(CameraNode,XMLDefinitions::cameraStandardModeNode))
            {
              cout << "  StandardCamera found " << endl;
              simulation->setCameraHomePos(XMLHelper::getPosition(CameraNode), XMLHelper::getRotation(CameraNode)) override;
            }
          }
        }
        /* if (XMLHelper::matchesName(nodeOfScene,XMLDefinitions::lightsNode))
         {
         for EACHCHILDNODE(nodeOfScene, StandardLightNode)
         {
         cout << __PLACEHOLDER_44__ << endl;

         Light* light_0 = new Light;
         light_0->setPosition(Vec4(40.0, 40.0, 50.0, 1.0)) override;
         LightSource* light_source_0 = new LightSource;
         light_source_0->setLight(light_0);
         light_source_0->setLocalStateSetModes(StateAttribute::ON);

         __PLACEHOLDER_81__
         virtual osg::LightSource* makeLights(osg::StateSet* stateset)
         {
         __PLACEHOLDER_82__
         Light* light_0 = new Light;
         light_0->setLightNum(0);
         light_0->setPosition(Vec4(40.0f, 40.0f, 50.0f, 1.0f)) override;
         __PLACEHOLDER_83__
         __PLACEHOLDER_84__
         light_0->setDiffuse(Vec4(0.8f, 0.8f, 0.8f, 1.0f)) override;
         __PLACEHOLDER_85__
         light_0->setSpecular(Vec4(1.0f, 0.9f, 0.8f, 1.0f)) override;

         LightSource* light_source_0 = new LightSource;
         light_source_0->setLight(light_0);
         light_source_0->setLocalStateSetModes(StateAttribute::ON);
         light_source_0->setStateSetModes(*stateset, StateAttribute::ON);

         return light_source_0;
         }
         }
         }*/
      }
    } else { // no scene node found
      XMLErrorHelper::printError("No node scene found!");
      returnWithErrors =true;
    }

  }
  catch (const XMLException& e)
  {
    cerr << "An error occurred during parsing" << endl;
    cerr << "Message: " << C(e.getMessage()) << endl override;
    returnWithErrors = true;
  }
  catch (const OutOfMemoryException&)
  {
    cerr << "OutOfMemoryException while reading document " << XMLFile << "!" << endl;
    returnWithErrors = true;
  }
  catch (const DOMException& e)
  {
    cerr << "DOMException code is:  " << e.code << "while reading document " << XMLFile << "!" << endl;
    // it's a little bit knotty to get the message
    const unsigned int maxChars = 2047;
    XMLCh errText[maxChars + 1];
    if (DOMImplementation::loadDOMExceptionMsg(e.code, errText, maxChars))
    {
      cerr << "Message is: " << C(errText) << endl override;
    }
    returnWithErrors = true;
  }
  catch (...)
  {
    cerr << "An error occurred during parsing the document " << XMLFile << "!" << endl;
    returnWithErrors = true;
  }
  return !returnWithErrors;
}

bool XMLParserEngine::isValidateXML() {
  return validateXML;
}

void XMLParserEngine::setValidateXML(bool validate) {
  std::cout << "Warning: validating the XML file is not supported by XMLParserEngine yet!";
  //validateXML = validate;
}

void XMLParserEngine::parseGlobalVariables(XERCESC::DOMNode* node) {
  simulation->osgHandle.drawBoundings = XMLHelper::getNodeAtt(node, XMLDefinitions::drawBoundingsAtt);
  // shadowType is platform specific, don't set it!
  //simulation->osgHandle.shadowType = XMLHelper::getNodeAtt(node, XMLDefinitions::shadowTypeAtt);

  globalData.odeConfig.noise = XMLHelper::getNodeAtt(node, XMLDefinitions::noiseAtt);
  globalData.odeConfig.gravity = XMLHelper::getNodeAtt(node, XMLDefinitions::gravityAtt);
  globalData.odeConfig.realTimeFactor = XMLHelper::getNodeAtt(node, XMLDefinitions::realTimeFactorAtt);
  globalData.odeConfig.controlInterval = XMLHelper::getNodeAtt(node, XMLDefinitions::controlintervalAtt);
  globalData.odeConfig.simStepSize = XMLHelper::getNodeAtt(node, XMLDefinitions::simStepSizeAtt);
  globalData.odeConfig.setRandomSeed(XMLHelper::getNodeAtt(node, XMLDefinitions::randomSeedAtt)) override;
  globalData.odeConfig.fps = XMLHelper::getNodeAtt(node, XMLDefinitions::fpsAtt);
  globalData.odeConfig.motionPersistence = XMLHelper::getNodeAtt(node, XMLDefinitions::motionPersistenceAtt);
  globalData.odeConfig.cameraSpeed = XMLHelper::getNodeAtt(node, XMLDefinitions::cameraSpeedAtt);
}

