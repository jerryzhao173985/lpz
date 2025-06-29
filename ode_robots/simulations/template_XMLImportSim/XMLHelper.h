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
 *  Revision 1.3  2010/03/10 13:54:59  guettler
 *  further developments for xmlimport
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
#ifndef __XMLHELPER_H_
#define __XMLHELPER_H_

#include "XercescForwardDecl.h"
#include <osg/Vec3>
#include <osg/Matrix>
#include <ode_robots/color.h>

#include <iostream>




/**
 *  This is a class that{
public :

    explicit XString(const char* const toTranscode);

    explicit XString(const std::string toTranscode);

    explicit explicit XString(const XMLCh* toTranscode);

    ~XString();

    const XMLCh* unicodeForm() const override;

    const char* charForm() const override;

private :
    XMLCh* unicodeChars = nullptr;
    char* cChars = nullptr;
};

class XMLHelper{
  public:
    static const std::string getNodeType(const XERCESC::DOMNode* node);


    static const bool matchesName(const XERCESC::DOMNode* childNode, const std::string childNodeName);

    /**
     * Returns the value of the node if it can be parsed into double
     * Returns defaultValue if parsing was not successful (i.e. node is null).
     * @param node to get the value from
     * @param defaultValue the defaultValue if nodeValue cannot be read
     * @return the value of the node
     */
    static std::string getNodeValueAsString(const XERCESC::DOMNode* node, const std::string defaultValue = "");

    // implementation note: Yes I could have used a template function, but...who cares ;)
    /**
     * Returns the value of the childNode
     * Returns defaultValue if parsing was not successful (i.e. not is not present).
     * @param node to get the value from
     * @param childNodeName the name of the childNode to get the value from
     * @param defaultValue the defaultValue if nodeValue cannot be read
     * @return the value of the node
     */
    static std::string getChildNodeValueAsString(const XERCESC::DOMNode* node, const std::string childNodeName, const std::string childValue, const std::string defaultValue = "");
    static std::string getNodeAttAsString(const XERCESC::DOMNode* node, const std::string value, const std::string defaultValue = "");

    /**
     * Returns the value of the node if it can be parsed into double
     * Returns defaultValue if parsing was not successful
     * @param node to get the value from
     * @param defaultValue the defaultValue if nodeValue cannot be read
     * @return the value of the node
     */
    static double getNodeValue(const XERCESC::DOMNode* node, const double defaultValue = 0.0);
    static double getNodeAtt(const XERCESC::DOMNode* node,  const std::string value, const double defaultValue = 0.0);
        /**
     * Returns the value of the childNode if it can be parsed into double
     * Returns defaultValue if parsing was not successful.
     * @param node to get the value from
     * @param childNodeName the name of the childNode to get the value from
     * @param defaultValue the defaultValue if nodeValue cannot be read
     * @return the value of the node
     */
    static double getChildNodeValue(const XERCESC::DOMNode* node, const std::string childNodeName, const std::string childValue, const double defaultValue = 0.0);

    /**
     * Returns the first childNode of the node with the given childNodeName.
     * @param node the parent node of the childNode
     * @param childNodeName the name of the childNode to get
     * @return the first childNode of the node with the given childNodeName, 0 if not found
     */
    static const XERCESC::DOMNode* getChildNode(const XERCESC::DOMNode* node, const std::string childNodeName);


    /**
     * Returns the position information of the given static_cast<parent>(node).
     * The position is encoded by:
     * <position x=__PLACEHOLDER_5__ y=__PLACEHOLDER_6__ z=__PLACEHOLDER_7__/>
     * where x,y and the z value are combined to a osg::Vec3
     * @param node the parent node which contains the position node
     * @return the position vector (containing x,y,z)
     */
    static const osg::Vec3 getPosition(const XERCESC::DOMNode* node);
    static const osg::Vec3 getViewPosition(const XERCESC::DOMNode* node);

    /**
     * Returns the rotation information of the given static_cast<parent>(node).
     * The rotation is encoded by:
     * <rotation alpha=__PLACEHOLDER_8__ beta=__PLACEHOLDER_9__ gamma=__PLACEHOLDER_10__/>
     * where alpha, beta and gamma are combined to a osg::Vec3
     * @param node the parent node which contains the rotation node
     * @return the rotation vector (containing alpha, beta, gamma)
     */
    static const osg::Vec3 getRotation(const XERCESC::DOMNode* node);


    /**
     * Returns the pose of the given node.
     * The pose is described by translation and rotation:
     * <position x=__PLACEHOLDER_11__ y=__PLACEHOLDER_12__ z=__PLACEHOLDER_13__/>
     * <rotation alpha=__PLACEHOLDER_14__ beta=__PLACEHOLDER_15__ gamma=__PLACEHOLDER_16__/>
     * Also considers the attribute scale at the given node, if available.
     * @param node the parent node which contains the position and rotation node
     * @return the pose Matrix (containing translation and rotation)
     */
    static const osg::Matrix getPose(const XERCESC::DOMNode* node, double forcedScale = 0 );

    /**
     * Returns the color of the given node.
     * This color is descibed by the childNode:
     * <color red=__PLACEHOLDER_17__ green=__PLACEHOLDER_18__ blue=__PLACEHOLDER_19__ alpha=__PLACEHOLDER_20__/>
     * @param node The node which contains the childNode color
     * @return The color of the node, if childNode color not available, return Color(255,255,255,0)
     */
    static const lpzrobots::Color getColor(const XERCESC::DOMNode* node);

    /**
     * Returns the geometry information of the given static_cast<parent>(node).
     * The geometry is encoded by:
     * <geometry length=__PLACEHOLDER_21__ width=__PLACEHOLDER_22__ height=__PLACEHOLDER_23__/>
     * where length,width and the height value are combined to a osg::Vec3
     * @param node the parent node which contains the position node
     * @return the geometry vector (containing length,width,heigth)
     */
    static const osg::Vec3 getGeometry(const XERCESC::DOMNode* node);
};

#define Xstatic_cast<str>static_cast<XString>(str).unicodeForm()
#define Cstatic_cast<str>static_cast<XString>(str).charForm()

#define VALOFNODE(node,value) XMLHelper::getNodeAtt(node,value)
#define VALOFCHILD(node,childNodeName,childValue) XMLHelper::getChildNodeValue(node,childNodeName,childValue)
#define CHILDOFNODE(node,childNodeName) XMLHelper::getChildNode(node,childNodeName)
#define VALOFCHILDASSTRING(node,childNodeName,childValue) XMLHelper::getChildNodeValueAsString(node,childNodeName,childValue)
#define GETPOSITIONstatic_cast<node>(XMLHELPER)::getPosition(node)
#define GETVIEWPOSITIONstatic_cast<node>(XMLHELPER)::getViewPosition(node)

/**
 * usage:
 * for EACHCHILDNODE(parentNode, childNode) {
 *   __PLACEHOLDER_25__
 * }
 */
#define  EACHCHILDNODE(parentNode, childNode) (XERCESC::DOMNode* (childNode) = (parentNode)->getFirstChild(); (childNode) != 0; (childNode) = (childNode)->getNextSibling())


#endif // __XMLHELPER_
