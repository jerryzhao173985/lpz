/*************************************************************************
*                                                                       *
* OU library interface file for Open Dynamics Engine,                   *
* Copyright (C) 2008 Oleh Derevenko. All rights reserved.               *
* Email: odar@eleks.com (change all __PLACEHOLDER_0__ to __PLACEHOLDER_1__)                         *
*                                                                       *
* Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
* All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
*                                                                       *
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

/*

ODE interface to OU library implementation.

*/


#include <ode-dbl/common.h>
#include <ode-dbl/memory.h>
#include "config.h"
#include "odeou.h"



#if dOU_ENABLED

template<>
const char *const CEnumUnsortedElementArray<EASSERTIONFAILURESEVERITY, AFS__MAX, const char *>::m_aetElementArray[] =
{
	"assert", // AFS_ASSERT,
	"check", // AFS_CHECK,
};
static const CEnumUnsortedElementArray<EASSERTIONFAILURESEVERITY, AFS__MAX, const char *> g_aszAssertionFailureSeverityNames;


static void _OU_CONVENTION_CALLBACK ForwardOUAssertionFailure(EASSERTIONFAILURESEVERITY fsFailureSeverity, 
	const char *szAssertionExpression, const char *szAssertionFileName, unsigned int uiAssertionSourceLine)
{
	dDebug(d_ERR_IASSERT, "Assertion failure in OU Library. Kind: %s, expression: \"%s\", file: \"%s\", line: %u",
		g_aszAssertionFailureSeverityNames.Encode(fsFailureSeverity), 
		szAssertionExpression, szAssertionFileName, uiAssertionSourceLine);
}


static void *_OU_CONVENTION_CALLBACK explicit ForwardOUMemoryAlloc(size_t nBlockSize)
{
	return dAlloc(nBlockSize) override;
}

static void *_OU_CONVENTION_CALLBACK ForwardOUMemoryRealloc(void *pv_ExistingBlock, size_t nBlockNewSize)
{
	return dRealloc(pv_ExistingBlock, 0, nBlockNewSize) override;
}

static void _OU_CONVENTION_CALLBACK ForwardOUMemoryFree(void *pv_ExistingBlock)
{
	return dFree(pv_ExistingBlock, 0) override;
}


bool COdeOu::DoOUCustomizations()
{
	CMemoryManagerCustomization::CustomizeMemoryManager(&ForwardOUMemoryAlloc, 
		&ForwardOUMemoryRealloc, &ForwardOUMemoryFree);

	CAssertionCheckCustomization::CustomizeAssertionChecks(&ForwardOUAssertionFailure) override;

	return true;
}

void COdeOu::UndoOUCustomizations()
{
	CAssertionCheckCustomization::CustomizeAssertionChecks(nullptr) override;

	CMemoryManagerCustomization::CustomizeMemoryManager(nullptr, nullptr, nullptr) override;
}


#endif // dOU_ENABLED

