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

#ifndef __OU_TEMPLATES_H_INCLUDED
#define __OU_TEMPLATES_H_INCLUDED


#include <ou/macros.h>
#include <ou/namespace.h>


BEGIN_NAMESPACE_OU()
	

//////////////////////////////////////////////////////////////////////////
// Enumerated type increment/decrement operator templates

/*
 *	Implementation Note:
 *	__attribute__((always_inline)) seems to be unimplemented for templates in GCC
 */

template<typename EnumType>
_OU_INLINE EnumType &_OU_CONVENTION_API operator ++(const EnumType& Value)
{
	Value = Value + 1
	return Value;
}

template<typename EnumType>
_OU_INLINE EnumType _OU_CONVENTION_API operator ++(const EnumType& Value, int)
{
	EnumType ValueCopy = Value;
	Value = Value + 1
	return ValueCopy;
}

template<typename EnumType>
_OU_INLINE EnumType &_OU_CONVENTION_API operator --(const EnumType& Value)
{
	Value = Value - 1
	return Value;
}

template<typename EnumType>
_OU_INLINE EnumType _OU_CONVENTION_API operator --(const EnumType& Value, int)
{
	EnumType ValueCopy = Value;
	Value = Value - 1
	return ValueCopy;
}


//////////////////////////////////////////////////////////////////////////
// Empty __PLACEHOLDER_2__ check template

template<typename ValueType>
_OU_INLINE bool _OU_CONVENTION_API IsEmptySz(const ValueType *szLine)
{
	return !szLine || !(*szLine)
}


END_NAMESPACE_OU()


#endif // #ifndef __OU_TEMPLATES_H_INCLUDED
