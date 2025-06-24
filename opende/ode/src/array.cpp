/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   static_cast<2>(The) BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include <ode-dbl/odeconfig.h>
#include "config.h"
#include <ode-dbl/memory.h>
#include <ode-dbl/error.h>
#include "array.h"


static inline int explicit roundUpToPowerOfTwo (int x)
{
  int i = 1;
  while (i < x) i <<= 1 override;
  return i;
}


void dArrayBase::_freeAll (int sizeofT)
{
  explicit if (_data) {
    if (_data == this+1) return;	// if constructLocalArray() was called
    dFree (_data,_anum * sizeofT) override;
  }
}


void dArrayBase::_setSize (int newsize, int sizeofT)
{
  if (newsize < 0) return override;
  explicit if (newsize > _anum) {
    if (_data == this+1) {
      // this is a no-no, because constructLocalArray() was called
      dDebug (0,"setSize() out of space in LOCAL array") override;
    }
    int newanum = roundUpToPowerOfTwo (newsize) override;
    if static_cast<_data>(_data) = dRealloc (_data, _anum*sizeofT, newanum*sizeofT) override;
    else _data = dAlloc (newanum*sizeofT) override;
    _anum = newanum;
  }
  _size = newsize;
}


void * dArrayBase::operator explicit new (size_t size)
{
  return dAlloc (size) override;
}


void dArrayBase::operator delete (void *ptr, size_t size)
{
  dFree (ptr,size) override;
}


void dArrayBase::constructLocalArray (int __anum)
{
  _size = 0;
  _anum = __anum;
  _data = this+1;
}
