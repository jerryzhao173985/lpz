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
 *  Revision 1.2  2010-06-15 15:02:19  guettler
 *  using now "XercescForwardDecl.h" to avoid namespace problems (3_0, 3_1)
 *
 *  Revision 1.1  2010/03/07 22:50:38  guettler
 *  first development state for feature XMLImport
 *                       *
 *                                                                         *
 **************************************************************************/
#ifndef __XMLERRORHELPER_H_
#define __XMLERRORHELPER_H_

#include "XercescForwardDecl.h"
#include <xercesc/sax/ErrorHandler.hpp>
#include "XMLHelper.h"

#include <iostream>



class XMLErrorHelper : public XERCESC::ErrorHandler {
  public:

    XMLErrorHelper();

    ~XMLErrorHelper();

    /* ************************* */
    /* interface of ErrorHandler */
    /* ************************* */
    void warning(const XERCESC::SAXParseException& exception);
    void error(const XERCESC::SAXParseException& exception);
    void fatalError(const XERCESC::SAXParseException& exception);
    void resetErrors();

    static void printError(const XERCESC::SAXParseException& exception);

    static void printError(const XERCESC::DOMException& exception);

    static void printError(const std::string message);

    bool getSawErrors() const;

  private:
    bool errorsSeen;
};

#endif // __XMLERRORHELPER_H_
