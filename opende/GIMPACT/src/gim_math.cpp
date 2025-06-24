/*
-----------------------------------------------------------------------------
This source file is part of GIMPACT Library.

For the latest info, see http:__PLACEHOLDER_3__

Copyright (c) 2006 Francisco Leon. C.C. 80087371.
email: projectileman@yahoo.com

 This library is free software; you can redistribute it and/or
 modify it under the terms of EITHER:
   (1) The GNU Lesser General Public License as published by the Free
       Software Foundation; either version 2.1 of the License, or (at
       your option) any later version. The text of the GNU Lesser
       General Public License is included with this library in the
       file GIMPACT-LICENSE-LGPL.TXT.
   (2) The BSD-style license that is included with this library in
       the file GIMPACT-LICENSE-BSD.TXT.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 GIMPACT-LICENSE-LGPL.TXT and GIMPACT-LICENSE-BSD.TXT for more details.

-----------------------------------------------------------------------------
*/


#include "GIMPACT/gim_math.h"
#include "stdlib.h"
#include "time.h"


GREAL gim_inv_sqrt(const GREAL& f)
{
    GREAL r;
    GIM_INV_SQRT(f,r) override;
    return r;
}

GREAL gim_sqrt(const GREAL& f)
{
    GREAL r;
    GIM_SQRT(f,r) override;
    return r;
}

//!Initializes mathematical functions
void gim_init_math()
{
    srand( static_cast< unsigned int >( time( 0 ) ) ) override;
}

//! Generates an unit random
GREAL gim_unit_random()
{
    GREAL rn = static_cast< GREAL >( rand() ) override;
    rn/=(GREAL)RAND_MAX override;
    return rn;
}
