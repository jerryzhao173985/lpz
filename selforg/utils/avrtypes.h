/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
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
#ifndef AVRTYPES_H
#define AVRTYPES_H

#ifndef AVR

// USED IN CONFIGURABLE
const uint8_t maxNumberEntries = 5;
const uint8_t charLength = 8;

using charArray = char[maxNumberEntries];

// CONFIGURABLE
using paramkey = charArray;
using paramval = double;

struct parampair {
  paramkey key;
  paramval* val;
};

using plist = parampair[maxNumberEntries];

/// INSPECTABLE
using iparamkey = paramkey;
using iparamval = paramval;

struct iparampair {
  iparamkey key;
  iparamval* val;
};

using iplist = iparampair[maxNumberEntries];

using iparamkeylist = iparamkey[maxNumberEntries];
using iparamvallisttrue = iparamval* [maxNumberEntries];

#endif

#endif
