/*
 * XMLObject.h
 *
 *  Created on: 03.02.2010
 *      Author: robot3
 */

#ifndef XMLOBJECT_H_
#define XMLOBJECT_H_

#include "XMLParserEngine.h"

class XMLObject{
public:
        explicit XMLObject(const XMLParserEngine& xmlEngine_) : xmlEngine(xmlEngine_) {}

        virtual ~XMLObject();

protected:
        XMLParserEngine& xmlEngine;
};

#endif /* XMLOBJECT_H_ */
