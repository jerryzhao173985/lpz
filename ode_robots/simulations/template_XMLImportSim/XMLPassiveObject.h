/*
 * XMLPassiveObject.h
 *
 *  Created on: 03.02.2010
 *      Author: robot3
 */

#ifndef XMLPASSIVEOBJECT_H_
#define XMLPASSIVEOBJECT_H_

#include <XercescForwardDecl.h>
#include <ode_robots/abstractobstacle.h>
#include "XMLObject.h"



class XMLPassiveObject{
public:
        XMLPassiveObject(XERCESC::DOMNode* passiveObjectNode, const XMLParserEngine& xmlEngine);
        virtual ~XMLPassiveObject() override;

          /**
           * sets position of the obstacle and creates/recreates obstacle if necessary
           */
          virtual void setPose(const osg::Matrix& pose);

          /// return the __PLACEHOLDER_1__ primitive of the obtactle. The meaning of __PLACEHOLDER_2__ is arbitrary
          virtual lpzrobots::Primitive* getMainPrimitive() const override;

          /// overload this function to create the obstactle. All primitives should go into the list __PLACEHOLDER_3__
          virtual void create();

protected:
          XERCESC::DOMNode* passiveObjectNode;
};

#endif /* XMLPASSIVEOBJECT_H_ */
