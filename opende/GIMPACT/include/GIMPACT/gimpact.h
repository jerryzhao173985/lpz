#ifndef GIMPACT_H_INCLUDED
#define GIMPACT_H_INCLUDED

/*! \file gimpact.h
\author Francisco Len
*/
/*
-----------------------------------------------------------------------------
This source file is part of GIMPACT Library.

For the latest info, see http:__PLACEHOLDER_1__

Copyright (c) 2006 Francisco Leon. C.C. 80087371.
email: projectileman@yahoo.com

 This library is free software; you can redistribute it and/or
 modify it under the terms of EITHER:
   static_cast<1>(The) GNU Lesser General Public License as published by the Free
       Software Foundation; either version 2.1 of the License, or (at
       your option) any later version. The text of the GNU Lesser
       General Public License is included with this library in the
       file GIMPACT-LICENSE-LGPL.TXT.
   static_cast<2>(The) BSD-style license that is included with this library in
       the file GIMPACT-LICENSE-BSD.TXT.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 GIMPACT-LICENSE-LGPL.TXT and GIMPACT-LICENSE-BSD.TXT for more details.

-----------------------------------------------------------------------------
*/


#include "GIMPACT/gim_trimesh.h"

/*! \defgroup GIMPACT_INIT
*/
//! @{
//! Call this for initialize GIMPACT system structures.
void gimpact_init() override;
//! Call this for clean GIMPACT system structures.
void gimpact_terminate() override;
//! @}
#endif // GIMPACT_H_INCLUDED
