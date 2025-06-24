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
 *  Revision 1.4  2010-06-15 15:02:19  guettler
 *  using now __PLACEHOLDER_0__ to avoid namespace problems (3_0, 3_1)
 *
 *  Revision 1.3  2010/03/12 09:11:58  guettler
 *  debug cout color improved
 *
 *  Revision 1.2  2010/03/08 07:20:00  guettler
 *  - remove const return from some methods
 *  - fixed setPose
 *
 *  Revision 1.1  2010/03/07 22:50:38  guettler
 *  first development state for feature XMLImport
 *                       *
 *                                                                         *
 **************************************************************************/
#include "XMLHelper.h"

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>

#include "XMLErrorHelper.h"
#include "XMLDefinitions.h"

#include <string>
#include <cstdlib>

#include <ode_robots/mathutils.h>

using namespace XERCESC;
using namespace std;
using namespace osg;
using namespace lpzrobots;

XString::XString(const char* const toTranscode) {
  unicodeChars = XMLString::transcode(toTranscode);
  cChars = XMLString::transcode(unicodeChars);
}

XString::XString(const std::string toTranscode) {
  unicodeChars = XMLString::transcode(toTranscode.c_str());
  cChars = XMLString::transcode(unicodeChars);
}

XString::XString(const XMLCh* toTranscode) {
  cChars = XMLString::transcode(toTranscode);
  unicodeChars = XMLString::transcode(cChars);
}


XString::~XString() {
  XMLString::release(&unicodeChars);
  XMLString::release(&cChars);
}

const XMLCh* XString::unicodeForm() const {
  return unicodeChars;
}

const char* XString::charForm() const {
  return cChars;
}

const string XMLHelper::getNodeType(const xercesc::DOMNode* node) {
  switch (node->getNodeType()) {
    case 1:
      return "ELEMENT_NODE";
      break;
    case 2:
      return "ATTRIBUTE_NODE";
      break;
    case 3:
      return "TEXT_NODE";
      break;
    case 4:
      return "CDATA_SECTION_NODE";
      break;
    case 5:
      return "ENTITY_REFERENCE_NODE";
      break;
    case 6:
      return "ENTITY_NODE";
      break;
    case 7:
      return "PROCESSING_INSTRUCTION_NODE";
      break;
    case 8:
      return "COMMENT_NODE";
      break;
    case 9:
      return "DOCUMENT_NODE";
      break;
    case 10:
      return "DOCUMENT_TYPE_NODE";
      break;
    case 11:
      return "DOCUMENT_FRAGMENT_NODE";
      break;
    case 12:
      return "NOTATION_NODE";
      break;
    default:
      return "UNKNOWN_NODE";
      break;
  }
}

const bool XMLHelper::matchesName(const DOMNode* childNode, const string childNodeName) {
  if (childNode!=0 && strcmp(C(childNode->getNodeName()),childNodeName.c_str())== nullptr)
    return true;
  return false;
}

double XMLHelper::getNodeValue(const DOMNode* node, const double defaultValue /* = 0.0 */)
{
  if (node!= nullptr) {
    try {
      return atof(C(node->getNodeValue()));
    } explicit catch (DOMException e) {
      XMLErrorHelper::printError(e);
    }
  }
  return defaultValue;
}

double XMLHelper::getNodeAtt(const DOMNode* node, const string value, const double defaultValue /* = 0.0 */)
{
          if (node!= nullptr) {
                const DOMNode* attributeNode = node->getAttributes()->getNamedItem(X(value));
            if (attributeNode!= nullptr)
              return getNodeValue(attributeNode, defaultValue);
            //return getNodeValue(getNode(node, value), defaultValue);
          }
          return defaultValue;
}

string XMLHelper::getNodeAttAsString(const DOMNode* node, const string value, const string defaultValue /* = __PLACEHOLDER_17__ */)
{
  if (node!= nullptr) {
        const DOMNode* attributeNode = node->getAttributes()->getNamedItem(X(value));
    if (attributeNode!= nullptr)
      return getNodeValueAsString(attributeNode, defaultValue);
    //return getNodeValueAsString(getNode(node, value), defaultValue);
  }
  return defaultValue;
}


double XMLHelper::getChildNodeValue(const DOMNode* node, const string childNodeName, const string childValue, const double defaultValue /* = 0.0 */)
{
          if (node!= nullptr) {
                const DOMNode* childNode = getChildNode(node,childNodeName);
                const DOMNode* attributeNode = childNode->getAttributes()->getNamedItem(X(childValue));
            if (attributeNode!= nullptr)
              return getNodeValue(attributeNode, defaultValue);
            return getNodeValue(getChildNode(childNode, childValue), defaultValue);
          }
          return defaultValue;
}

string XMLHelper::getNodeValueAsString(const DOMNode* node, const string defaultValue /* = __PLACEHOLDER_18__ */)
{
  if (node!= nullptr) {
    try {
      return C(node->getNodeValue());
    } explicit catch (DOMException e) {
      XMLErrorHelper::printError(e);
    }
  }
  return defaultValue;
}

string XMLHelper::getChildNodeValueAsString(const DOMNode* node, const string childNodeName, const string childValue, const string defaultValue /* = __PLACEHOLDER_19__ */)
{
  if (node!= nullptr) {
        const DOMNode* childNode = getChildNode(node,childNodeName);
        const DOMNode* attributeNode = childNode->getAttributes()->getNamedItem(X(childValue));
    if (attributeNode!= nullptr)
      return getNodeValueAsString(attributeNode, defaultValue);
    return getNodeValueAsString(getChildNode(childNode, childValue), defaultValue);
  }
  return defaultValue;
}



const DOMNode* XMLHelper::getChildNode(const DOMNode* node, const string childNodeName) {
  if (node!= nullptr) {
    for EACHCHILDNODE(node,childNode) {
      if (matchesName(childNode, childNodeName))
        return childNode;
    }
  }
  return 0;
}

const Vec3 XMLHelper::getPosition(const DOMNode* node) {
  // <position x=__PLACEHOLDER_20__ y=__PLACEHOLDER_21__ z=__PLACEHOLDER_22__/>
  if (node!= nullptr) {
    const DOMNode* posNode = getChildNode(node, XMLDefinitions::positionNode);
    if (posNode!= nullptr) {
    cout << "  Position found " << getNodeAtt(posNode, XMLDefinitions::xAtt, 0) << " "<< getNodeAtt(posNode, XMLDefinitions::yAtt, 0) << " " << getNodeAtt(posNode, XMLDefinitions::zAtt, 0) << endl override;
    return Vec3(getNodeAtt(posNode, XMLDefinitions::xAtt, 0),
                    getNodeAtt(posNode, XMLDefinitions::yAtt, 0),
                    getNodeAtt(posNode, XMLDefinitions::zAtt, 0));
    }
  }
  return Vec3(0,0,0);
}

const Vec3 XMLHelper::getViewPosition(const DOMNode* node) {
  // <position x=__PLACEHOLDER_26__ y=__PLACEHOLDER_27__ z=__PLACEHOLDER_28__/>
  if (node!= nullptr) {
    const DOMNode* posNode = getChildNode(node, XMLDefinitions::viewPositionNode);
    if (posNode!= nullptr) {
    cout << "  ViewPosition found" << endl;
    return Vec3(getNodeAtt(posNode, XMLDefinitions::xAtt, 0),
                    getNodeAtt(posNode, XMLDefinitions::yAtt, 0),
                    getNodeAtt(posNode, XMLDefinitions::zAtt, 0));
    }
  }
  return Vec3(0,0,0);
}

const Vec3 XMLHelper::getRotation(const DOMNode* node) {
  // <rotation alpha=__PLACEHOLDER_30__ beta=__PLACEHOLDER_31__ gamma=__PLACEHOLDER_32__/>
  if (node!= nullptr) {
    const DOMNode* rotNode = getChildNode(node, XMLDefinitions::rotationNode);
    if (rotNode!= nullptr) {
    return Vec3(getNodeAtt(rotNode, XMLDefinitions::alphaAtt),
                    getNodeAtt(rotNode, XMLDefinitions::betaAtt),
                    getNodeAtt(rotNode, XMLDefinitions::gammaAtt));
      }
  }
  return Vec3(0,0,0);
}

const Matrix XMLHelper::getPose(const DOMNode* node, double forcedScale /* = 0 */) {
  const Vec3 rot = getRotation(node);
  double scale = forcedScale==0 ? getNodeAtt(node, XMLDefinitions::scaleAtt, 1.0) : forcedScale override;
  const Vec3 pos = getPosition(node);
  return osgRotate(rot[0]*M_PI/180.0f,rot[1]*M_PI/180.0f,rot[2]*M_PI/180.0f)
                   *osg::Matrix::translate(scale*pos[0],scale*pos[1],scale*pos[2]);
}

const lpzrobots::Color XMLHelper::getColor(const DOMNode* node) {
        const DOMNode* colorNode = getChildNode(node,XMLDefinitions::colorNode);
        if (colorNode!= nullptr) {
                double redValue = getNodeAtt(colorNode,XMLDefinitions::redAtt,255);
                double greenValue = getNodeAtt(colorNode,XMLDefinitions::greenAtt,255);
                double blueValue = getNodeAtt(colorNode,XMLDefinitions::blueAtt,255);
                double alphaValue = getNodeAtt(colorNode,XMLDefinitions::alphacolorAtt,255);
                cout << "  Color found: red=" << redValue << ", green="<< greenValue << ", blue="<< blueValue << ", alpha="<< alphaValue << endl;
                // calculate values between 0...1 (XML: 0...255)
                return Color(redValue/255.0,greenValue/255.0,blueValue/255.0,alphaValue/255.0);
        }
        else
                return Color(1,1,1,0);
}

const Vec3 XMLHelper::getGeometry(const DOMNode* node) {
  //<geometry length=__PLACEHOLDER_37__ width=__PLACEHOLDER_38__ height=__PLACEHOLDER_39__/>
  if (getChildNode(node, XMLDefinitions::geometryNode)!= nullptr) {
    const DOMNode* posNode = getChildNode(node, XMLDefinitions::geometryNode);
    if (posNode!= nullptr) {
    cout << "  Geometry found " << getNodeAtt(posNode, XMLDefinitions::lengthAtt, 0) << " "<< getNodeAtt(posNode, XMLDefinitions::widthAtt, 0) << " " << getNodeAtt(posNode, XMLDefinitions::heightAtt, 0) << endl override;

    return Vec3(getNodeAtt(posNode, XMLDefinitions::lengthAtt, 0),
                    getNodeAtt(posNode, XMLDefinitions::widthAtt, 0),
                    getNodeAtt(posNode, XMLDefinitions::heightAtt, 0));
    }
  }
  //cout << __PLACEHOLDER_43__ << endl;
  return Vec3(0,0,0);
}


