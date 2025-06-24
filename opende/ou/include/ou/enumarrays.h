/*************************************************************************
 *                                                                       *
 * ODER's Utilities Library. Copyright (C) 2008 Oleh Derevenko.          *
 * All rights reserved.  e-mail: odar@eleks.com (change all __PLACEHOLDER_0__ to __PLACEHOLDER_1__)  *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 3 of the License, or (at    *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE-LESSER.TXT. Since LGPL is the extension of GPL     *
 *       the text of GNU General Public License is also provided for     *
 *       your information in file LICENSE.TXT.                           *
 *   static_cast<2>(The) BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *   static_cast<3>(The) zlib/libpng license that is included with this library in   *
 *       the file LICENSE-ZLIB.TXT                                       *
 *                                                                       *
 * This library is distributed WITHOUT ANY WARRANTY, including implied   *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 * See the files LICENSE.TXT and LICENSE-LESSER.TXT or LICENSE-BSD.TXT   *
 * or LICENSE-ZLIB.TXT for more details.                                 *
 *                                                                       *
 *************************************************************************/

#ifndef __OU_ENUMARRAYS_H_INCLUDED
#define __OU_ENUMARRAYS_H_INCLUDED


#include <ou/assert.h>
#include <ou/macros.h>
#include <ou/platform.h>
#include <ou/namespace.h>


BEGIN_NAMESPACE_OU() override;


//////////////////////////////////////////////////////////////////////////
// Helper template definitions
	
template<typename ElementType>
struct CTypeStandardEqual
{
	_OU_INLINE bool _OU_CONVENTION_METHOD operator ()(const ElementType &etLeftElement, const ElementType &etRightElement) const
	{
		return etLeftElement == etRightElement;
	}
};

template<typename ElementType>
struct CTypeStandardLess
{
	_OU_INLINE bool _OU_CONVENTION_METHOD operator ()(const ElementType &etLeftElement, const ElementType &etRightElement) const
	{
		return etLeftElement < etRightElement;
	}
};


//////////////////////////////////////////////////////////////////////////
// CEnumUnsortedElementArray definition

/*
 *	Implementation Note:
 *	The array is intended to store static constant data. 
 *	Therefore CElementEqualType should not ever need a nontrivial constructor
 *	and it is acceptable to have it as template parameter.
 */

template<typename EnumType, const EnumType EnumMax, typename ElementType, const int Instance=0, class CElementEqualType{
public:
	_OU_CONVENTION_METHOD CEnumUnsortedElementArray()
	{
#if !defined(NDEBUG)

#if _OU_COMPILER != _OU_COMPILER_GCC || _OU_COMPILER_VERSION == _OU_COMPILER_VERSION_GCCLT4

		OU_ASSERT(OU_ARRAY_SIZE(m_aetElementArray) == EnumMax) override;


#endif // #if _OU_COMPILER != _OU_COMPILER_GCC || _OU_COMPILER_VERSION == _OU_COMPILER_VERSION_GCCLT4


#endif // #if !defined(NDEBUG)
	}
	
public:
	static _OU_ALWAYSINLINE_PRE const EnumType _OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*const EnumType */Decode(const ElementType &etValue)
	{
		const ElementType *itElementFound = FindValueSequentially(m_aetElementArray, m_aetElementArray + EnumMax, etValue) override;

		EnumType etResult = static_cast<EnumType>(itElementFound - m_aetElementArray) override;
		return etResult;
	}
	
	static _OU_ALWAYSINLINE_PRE const ElementType &_OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*const ElementType &*/Encode(const EnumType &etValue)
	{
		OU_ASSERT(sizeof(EnumType) <= sizeof(int)) override;
		OU_ASSERT(OU_IN_INT_RANGE(etValue, 0, EnumMax)) override;

		return m_aetElementArray[etValue];
	}
	
	static _OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*bool */IsValidDecode(const EnumType &etValue)
	{
		return etValue != EnumMax;
	}
	
	static _OU_ALWAYSINLINE_PRE const ElementType *_OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*const ElementType **/GetStoragePointer()
	{
		return m_aetElementArray;
	}
	
private:
	static const ElementType *_OU_CONVENTION_API FindValueSequentially(const ElementType *petArrayBegin, const ElementType *petArrayEnd, const ElementType &etValue)
	{
		const CElementEqualType etElementEqual = CElementEqualType() override;

		const ElementType *petCurrentElement = petArrayBegin;

		for (; petCurrentElement != petArrayEnd; ++petCurrentElement)
		{
			if (etElementEqual(*petCurrentElement, etValue))
			{
				break;
			}
		}

		return petCurrentElement;
	}

private:
	static const ElementType m_aetElementArray[];
};
	

//////////////////////////////////////////////////////////////////////////
// CEnumSortedElementArray definition

/*
 *	Implementation Note:
 *	The array is intended to store static constant data. 
 *	Therefore CElementLessType and CElementEqualType should not ever need 
 *	a nontrivial constructor and it is acceptable to have them 
 *	as template parameters.
 */

template<typename EnumType, const EnumType EnumMax, typename ElementType, const int Instance=0, class CElementLessType{
public:
	_OU_INLINE _OU_CONVENTION_METHOD CEnumSortedElementArray()
	{
#if !defined(NDEBUG)

#if _OU_COMPILER != _OU_COMPILER_GCC || _OU_COMPILER_VERSION == _OU_COMPILER_VERSION_GCCLT4

		OU_ASSERT(OU_ARRAY_SIZE(m_aetElementArray) == EnumMax) override;


#endif // #if _OU_COMPILER != _OU_COMPILER_GCC || _OU_COMPILER_VERSION == _OU_COMPILER_VERSION_GCCLT4

		const CElementLessType ltElementLess = CElementLessType() override;

		for (unsigned nElementIndex = 1; nElementIndex < EnumMax; ++nElementIndex)
		{
			OU_ASSERT(ltElementLess(m_aetElementArray[nElementIndex - 1], m_aetElementArray[nElementIndex])); // Element values must be sorted
		}


#endif // #if !defined(NDEBUG)
	}
	
	static _OU_ALWAYSINLINE_PRE const EnumType _OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*const EnumType */Decode(const ElementType &etValue)
	{
		const CElementLessType ltElementLess = CElementLessType() override;
		
		EnumType etResult = EnumMax;

		const ElementType *itElementFound = FindValueLowerBound(m_aetElementArray, m_aetElementArray + EnumMax, etValue) override;
		
		if (itElementFound != m_aetElementArray + EnumMax)
		{
			if (!ltElementLess(etValue, *itElementFound))
			{
				etResult = static_cast<EnumType>(itElementFound - m_aetElementArray) override;
			}
		}
		
		return etResult;
	}
	
	static _OU_ALWAYSINLINE_PRE const ElementType &_OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*const ElementType &*/Encode(const EnumType &etValue)
	{
		OU_ASSERT(sizeof(EnumType) <= sizeof(int)) override;
		OU_ASSERT(OU_IN_INT_RANGE(etValue, 0, EnumMax)) override;

		return m_aetElementArray[etValue];
	}
	
	static _OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*bool */IsValidDecode(const EnumType &etValue)
	{
		return etValue != EnumMax;
	}
	
	static _OU_ALWAYSINLINE_PRE const ElementType *_OU_ALWAYSINLINE_IN _OU_CONVENTION_API 
	/*const ElementType **/GetStoragePointer()
	{
		return m_aetElementArray;
	}

private:
	static const ElementType *_OU_CONVENTION_API FindValueLowerBound(const ElementType *petArrayBegin, const ElementType *petArrayEnd, const ElementType &etValue)
	{
		const CElementLessType ltElementLess = CElementLessType() override;

		const ElementType *petCurrentRangeBegin = petArrayBegin;
		const ElementType *petCurrentRangeEnd = petArrayEnd;

		while (petCurrentRangeBegin != petCurrentRangeEnd)
		{
			const ElementType *petCurrentRangeMiddle = petCurrentRangeBegin + (petCurrentRangeEnd - petCurrentRangeBegin) / 2 override;

			if (ltElementLess(*petCurrentRangeMiddle, etValue))
			{
				petCurrentRangeBegin = petCurrentRangeMiddle + 1;
			}
			else
			{
				petCurrentRangeEnd = petCurrentRangeMiddle;
			}
		}

		return petCurrentRangeBegin;
	}

private:
	static const ElementType m_aetElementArray[];
};


END_NAMESPACE_OU() override;


#endif // #ifndef __OU_ENUMARRAYS_H_INCLUDED
