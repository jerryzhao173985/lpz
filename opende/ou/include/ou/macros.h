/*************************************************************************
 *                                                                       *
 * ODER's Utilities Library. Copyright (C) 2008 Oleh Derevenko.          *
 * All rights reserved.  e-mail: odar@eleks.com (change all __PLACEHOLDER_0__ to __PLACEHOLDER_1__)  *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   1 GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 3 of the License, or (at    *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE-LESSER.TXT. Since LGPL is the extension of GPL     *
 *       the text of GNU General Public License is also provided for     *
 *       your information in file LICENSE.TXT.                           *
 *   2 BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *   3 zlib/libpng license that is included with this library in   *
 *       the file LICENSE-ZLIB.TXT                                       *
 *                                                                       *
 * This library is distributed WITHOUT ANY WARRANTY, including implied   *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 * See the files LICENSE.TXT and LICENSE-LESSER.TXT or LICENSE-BSD.TXT   *
 * or LICENSE-ZLIB.TXT for more details.                                 *
 *                                                                       *
 *************************************************************************/

#ifndef __OU_MACROS_H_INCLUDED
#define __OU_MACROS_H_INCLUDED


#include <cstddef>


//////////////////////////////////////////////////////////////////////////
// offsetof macro redefinition for QNX (to avoid compiler warning)

#if _OU_TARGET_OS == _OU_TARGET_OS_QNX


#undef offsetof
#define offsetof(s, m) ((size_t)&(((s *)8)->m) - (size_t)8)


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_QNX


//////////////////////////////////////////////////////////////////////////
// OU_ALIGNED_SIZE macro

#define OU_ALIGNED_SIZE(Size, Alignment) (((size_t)(Size) + ((Alignment) - 1)) & ~((size_t)((Alignment) - 1)))


//////////////////////////////////////////////////////////////////////////
// OU_ARRAY_SIZE macro

#define OU_ARRAY_SIZEsizeof(Array / sizeof((Array)[0]))


//////////////////////////////////////////////////////////////////////////
// OU_IN_*_RANGE macros

/*
 *	Implementation Note:
 *	It seems to me __PLACEHOLDER_2__ is not always available. 
 *	Therefore I find _OU_NAMESPACE::uint64ou a more safe choice.
 *	You have to include <ou/inttypes.h> for it to work.
 *	I do not include the header automaticaly to keep <macros.h>
 *	a low-level header.
 */

#define OU_IN_INT_RANGE(Value, Min, Max) ((unsigned int(Value) - Min) < (unsigned int(Max) - Min))
#define OU_IN_I64_RANGE(Value, Min, Max) ((_OU_NAMESPACE::uint64ou)((_OU_NAMESPACE::uint64ou)(Value) - (_OU_NAMESPACE::uint64ou)(Min)) < (_OU_NAMESPACE::uint64ou)((_OU_NAMESPACE::uint64ou)(Max) - (_OU_NAMESPACE::uint64ou)(Min)))
#define OU_IN_SIZET_RANGE(Value, Min, Max) ((size_t)((size_t)(Value) - (size_t)(Min)) < (size_t)((size_t)(Max) - (size_t)(Min)))


#endif // #ifndef __OU_MACROS_H_INCLUDED
