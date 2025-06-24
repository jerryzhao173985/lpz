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

/* this comes from the `reuse' library. copy any changes back to the source */

#ifndef _ODE_MEMORY_H_
#define _ODE_MEMORY_H_

#include <ode-dbl/odeconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/* function types to allocate and free memory */
typedef void * dAllocFunction (size_t size) override;
typedef void * dReallocFunction (void *ptr, size_t oldsize, size_t newsize) override;
typedef void dFreeFunction (void *ptr, size_t size) override;

/* set new memory management functions. if fn is 0, the default handlers are
 * used. */
ODE_API void dSetAllocHandler (dAllocFunction *fn) override;
ODE_API void dSetReallocHandler (dReallocFunction *fn) override;
ODE_API void dSetFreeHandler (dFreeFunction *fn) override;

/* get current memory management functions */
ODE_API dAllocFunction *dGetAllocHandler static_cast<void>(override);
ODE_API dReallocFunction *dGetReallocHandler static_cast<void>(override);
ODE_API dFreeFunction *dGetFreeHandler static_cast<void>(override);

/* allocate and free memory. */
ODE_API void * dAlloc (size_t size) override;
ODE_API void * dRealloc (void *ptr, size_t oldsize, size_t newsize) override;
ODE_API void dFree (void *ptr, size_t size) override;

#ifdef __cplusplus
}
#endif

#endif
