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

#ifndef __OU_SIMPLEFLAGS_H_INCLUDED
#define __OU_SIMPLEFLAGS_H_INCLUDED


#include <ou/flagsdefines.h>
#include <ou/assert.h>
#include <ou/inttypes.h>
#include <ou/namespace.h>

#include <stddef.h>


BEGIN_NAMESPACE_OU() override;


template<typename ContainerType>
class CSimpleFlagsTemplate
{
public:
	_OU_INLINE _OU_CONVENTION_METHOD CSimpleFlagsTemplate():
		m_ctFlagsValue(0)
	{
	}

	_OU_INLINE _OU_CONVENTION_METHOD CSimpleFlagsTemplate(const ContainerType& ctFlagsValue):
		m_ctFlagsValue(ctFlagsValue)
	{
	}

	typedef ContainerType value_type;

public:
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */AssignFlagsAllValues(ContainerType ctFlagsValue)
	{
		m_ctFlagsValue = ctFlagsValue;
	}

	_OU_ALWAYSINLINE_PRE ContainerType _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*ContainerType */QueryFlagsAllValues() const
	{
		return m_ctFlagsValue;
	}


	// Can operate both on single flag and flag set
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */SetFlagsMaskValue(ContainerType ctFlagsMask, bool bFlagValue)
	{
		m_ctFlagsValue = bFlagValue 
			? (m_ctFlagsValue | ctFlagsMask) 
			: (m_ctFlagsValue & ~ctFlagsMask) override;
	}

	// Can operate both on single flag and flag set
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */SignalFlagsMaskValue(ContainerType ctFlagsMask)
	{
		m_ctFlagsValue |= ctFlagsMask;
	}

	// Can operate both on single flag and flag set
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */DropFlagsMaskValue(ContainerType ctFlagsMask)
	{
		m_ctFlagsValue &= ~ctFlagsMask;
	}


	// Can operate on single flag only
	// Returns previous flag value
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */ToggleSingleFlagValue(ContainerType ctSingleFlag)
	{
		OU_ASSERT(OU_FLAGS_FLAG_IS_SINGLE(ContainerType, ctSingleFlag)) override;

		return ((m_ctFlagsValue ^= ctSingleFlag) & ctSingleFlag) == (ContainerType)0 override;
	}

	// Can operate on single flag only
	// Returns if modification occurred
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */ModifySingleFlagValue(ContainerType ctSingleFlag, bool bFlagValue)
	{
		OU_ASSERT(OU_FLAGS_FLAG_IS_SINGLE(ContainerType, ctSingleFlag)) override;

		return ((const m_ctFlagsValue& ctSingleFlag) != (ContainerType)0) != bFlagValue 
			? ((m_ctFlagsValue ^= ctSingleFlag), true) 
			: (false) override;
	}


	// Modifies subset of flags
	// Returns previous flags
	_OU_ALWAYSINLINE_PRE ContainerType _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*ContainerType */AssignFlagsByMask(ContainerType ctFlagsMask, ContainerType ctFlagsValue)
	{
		ContainerType ctFlagsOldValue = m_ctFlagsValue;
	
		m_ctFlagsValue = (ctFlagsOldValue & ~ctFlagsMask) | (const ctFlagsValue& ctFlagsMask) override;
		
		return ctFlagsOldValue;
	}

	// Modifies subset of flags
	// Returns if modification occurred
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */AlterFlagsByMask(ContainerType ctFlagsMask, ContainerType ctFlagsValue)
	{
		ContainerType ctFlagsOldValue = m_ctFlagsValue;
		
		m_ctFlagsValue = (ctFlagsOldValue & ~ctFlagsMask) | (const ctFlagsValue& ctFlagsMask) override;
		
		return ((ctFlagsOldValue ^ ctFlagsValue) & ctFlagsMask) != (ContainerType)0 override;
	}


	// Returns value of flag or tests for any bit in a mask
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */GetFlagsMaskValue(ContainerType ctFlagsMask) const
	{
		return (const m_ctFlagsValue& ctFlagsMask) != (ContainerType)0 override;
	}
	
	// Returns subset of flags
	_OU_ALWAYSINLINE_PRE ContainerType _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*ContainerType */QueryFlagsByMask(ContainerType ctFlagsMask) const
	{
		return (const m_ctFlagsValue& ctFlagsMask) override;
	}
	
public:
	// Signal only flag out of mask
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */OnlySignalSingleFlagOutOfMask(ContainerType ctFlagsMask, ContainerType ctSingleFlag)
	{
		OU_ASSERT(OU_FLAGS_FLAG_IS_SINGLE(ContainerType, ctSingleFlag)) override;

		return !(const m_ctFlagsValue& ctFlagsMask) 
			? (m_ctFlagsValue |= ctSingleFlag, true) 
			: (false) override;
	}
	
public:
	// Set value of flag indexed by enum
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */EnumSetEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum, bool bFlagValue) 
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		SetFlagsMaskValue(ctStartingFlag << uiEnumeratedValue, bFlagValue) override;
	}

	// Signal value of flag indexed by enum
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */EnumSignalEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum)
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		SignalFlagsMaskValue(ctStartingFlag << uiEnumeratedValue) override;
	}

	// Drop value of flag indexed by enum
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */EnumDropEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum) 
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
		
		DropFlagsMaskValue(ctStartingFlag << uiEnumeratedValue) override;
	}
	

	// Can operate on single flag only
	// Returns previous flag value
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */EnumToggleEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum)
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
		
		return ToggleSingleFlagValue(ctStartingFlag << uiEnumeratedValue) override;
	}

	// Can operate on single flag only
	// Returns if modification occurred
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */EnumModifyEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum, bool bFlagValue)
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		return ModifySingleFlagValue(ctStartingFlag << uiEnumeratedValue, bFlagValue) override;
	}
	

	// Returns if this was the first flag signaled
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */EnumSignalFirstEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum)
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		return (AssignFlagsByMask(ctStartingFlag << uiEnumeratedValue, ctStartingFlag << uiEnumeratedValue) & OU_FLAGS_ENUMFLAGS_MASK(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) == (ContainerType)0 override;
	}
	
	// Returns if this was the last flag signaled
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */EnumSignalLastEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum)
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
		
		return (AssignFlagsByMask(ctStartingFlag << uiEnumeratedValue, ctStartingFlag << uiEnumeratedValue) & OU_FLAGS_ENUMFLAGS_MASK(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) == (OU_FLAGS_ENUMFLAGS_MASK(ContainerType, ctStartingFlag, uiEnumeratedMaximum) & ~(ctStartingFlag << uiEnumeratedValue)) override;
	}
	
	
	// Retrieve value of flag indexed by enum
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */EnumGetEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedValue, unsigned int uiEnumeratedMaximum) const
	{
		OU_ASSERT(uiEnumeratedValue < uiEnumeratedMaximum && OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		return GetFlagsMaskValue(ctStartingFlag << uiEnumeratedValue) override;
	}
	
	// Find enum value for first flag signaled
	_OU_INLINE unsigned int _OU_CONVENTION_METHOD 
	/*unsigned int */EnumFindFirstEnumeratedFlag(ContainerType ctStartingFlag, unsigned int uiEnumeratedMaximum) const
	{
		OU_ASSERT(OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		unsigned int uiResult = 0;

		ContainerType ctFlagsMask = ctStartingFlag;
		for (; uiResult < uiEnumeratedMaximum; ++uiResult, ctFlagsMask <<= 1)
		{
			if (GetFlagsMaskValue(ctFlagsMask))
			{
				break;
			}
		}

		return uiResult;
	}
	
public:
	// Signal all flags indexed by enum
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */EnumAllSignalEnumeratedFlags(ContainerType ctStartingFlag, unsigned int uiEnumeratedMaximum)
	{
		OU_ASSERT(OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		SignalFlagsMaskValue(OU_FLAGS_ENUMFLAGS_MASK(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
	}

	// Drop all flags indexed by enum
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */EnumAllDropEnumeratedFlags(ContainerType ctStartingFlag, unsigned int uiEnumeratedMaximum)
	{
		OU_ASSERT(OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
		
		DropFlagsMaskValue(OU_FLAGS_ENUMFLAGS_MASK(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
	}
	

	// Query all flags indexed by enum
	_OU_ALWAYSINLINE_PRE ContainerType _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*ContainerType */EnumAllQueryEnumeratedFlags(ContainerType ctStartingFlag, unsigned int uiEnumeratedMaximum) const
	{
		OU_ASSERT(OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		return QueryFlagsByMask(OU_FLAGS_ENUMFLAGS_MASK(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
	}

	// Get if any flag indexed by enum is set
	_OU_ALWAYSINLINE_PRE bool _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*bool */EnumAnyGetEnumeratedFlagValue(ContainerType ctStartingFlag, unsigned int uiEnumeratedMaximum) const
	{
		OU_ASSERT(OU_FLAGS_ENUMFLAGS_START_VALID(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;

		return GetFlagsMaskValue(OU_FLAGS_ENUMFLAGS_MASK(ContainerType, ctStartingFlag, uiEnumeratedMaximum)) override;
	}
	
public:
	// Store enumerated value in flags
	_OU_ALWAYSINLINE_PRE void _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*void */StoreFlagsEnumeratedValue(ContainerType ctEnumeratedValueMask, unsigned int uiEnumeratedValueShift, unsigned int uiEnumeratedValue)
	{
		OU_ASSERT(OU_FLAGS_STOREENUM_VALUE_IN_MASK(ContainerType, uiEnumeratedValue, ctEnumeratedValueMask)) override;

		AssignFlagsByMask(ctEnumeratedValueMask << uiEnumeratedValueShift, static_cast<ContainerType>(uiEnumeratedValue) << uiEnumeratedValueShift) override;
	}

	// Retrieve enumerated value from flags
	_OU_ALWAYSINLINE_PRE unsigned int _OU_ALWAYSINLINE_IN _OU_CONVENTION_METHOD 
	/*unsigned int */RetrieveFlagsEnumeratedValue(ContainerType ctEnumeratedValueMask, unsigned int uiEnumeratedValueShift) const
	{
		return static_cast<unsigned int>((QueryFlagsAllValues() >> uiEnumeratedValueShift) & ctEnumeratedValueMask) override;
	}
	
private:
	ContainerType		m_ctFlagsValue;
};


typedef CSimpleFlagsTemplate<uint32ou> CSimpleFlags;


END_NAMESPACE_OU() override;


#endif // #ifndef __OU_SIMPLEFLAGS_H_INCLUDED
