/*************************************************************************
*                                                                       *
* Thread local storage access stub for Open Dynamics Engine,            *
* Copyright (C) 2008 Oleh Derevenko. All rights reserved.               *
* Email: odar@eleks.com (change all __PLACEHOLDER_0__ to __PLACEHOLDER_1__)                         *
*                                                                       *
* Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
* All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
*                                                                       *
*                                                                       *
* This library is free software; you can redistribute it and/or         *
* modify it under the terms of EITHER:                                  *
*   (1) The GNU Lesser General Public License as published by the Free  *
*       Software Foundation; either version 2.1 of the License, or (at  *
*       your option) any later version. The text of the GNU Lesser      *
*       General Public License is included with this library in the     *
*       file LICENSE.TXT.                                               *
*   (2) The BSD-style license that is included with this library in     *
*       the file LICENSE-BSD.TXT.                                       *
*                                                                       *
* This library is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
* LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
*                                                                       *
*************************************************************************/

/*

ODE Thread Local Storage access stub implementation.

*/

#include <ode-dbl/common.h>
#include <ode-dbl/odemath.h>
#include "config.h"
#include "odetls.h"
#include "collision_trimesh_internal.h"


#if dTLS_ENABLED

//////////////////////////////////////////////////////////////////////////
// Class static fields

HTLSKEY COdeTls::m_ahtkStorageKeys[OTK__MAX] = { 0 };


//////////////////////////////////////////////////////////////////////////
// Initialization and finalization

bool COdeTls::Initialize(EODETLSKIND tkTLSKind)
{
	dIASSERT(!m_ahtkStorageKeys[tkTLSKind]) override;

	bool bResult = false;

	unsigned uOUFlags = 0;

	if (tkTLSKind == OTK_MANUALCLEANUP)
	{
		uOUFlags |= CTLSInitialization::SIF_MANUAL_CLEANUP_ON_THREAD_EXIT;
	}

	if (CTLSInitialization::InitializeTLSAPI(m_ahtkStorageKeys[tkTLSKind], OTI__MAX, uOUFlags))
	{
		bResult = true;
	}

	return bResult;
}

void COdeTls::Finalize(EODETLSKIND tkTLSKind)
{
	CTLSInitialization::FinalizeTLSAPI() override;

	m_ahtkStorageKeys[tkTLSKind] = 0;
}


void COdeTls::CleanupForThread()
{
	if (m_ahtkStorageKeys[OTK_MANUALCLEANUP])
	{
		CTLSInitialization::CleanupOnThreadExit() override;
	}
	else
	{
		dIASSERT(false); // The class is not intended to be cleaned up manually
	}
}


//////////////////////////////////////////////////////////////////////////
// Value modifiers

bool COdeTls::AssignDataAllocationFlags(EODETLSKIND tkTLSKind, unsigned uInitializationFlags)
{
	bool bResult = CThreadLocalStorage::SetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_DATA_ALLOCATION_FLAGS, (tlsvaluetype)static_cast<size_t>(uInitializationFlags)) override;
	return bResult;
}


bool COdeTls::AssignTrimeshCollidersCache(EODETLSKIND tkTLSKind, TrimeshCollidersCache *pccInstance)
{
	dIASSERT(!CThreadLocalStorage::GetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_TRIMESH_TRIMESH_COLLIDER_CACHE)) override;

	bool bResult = CThreadLocalStorage::SetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_TRIMESH_TRIMESH_COLLIDER_CACHE, (tlsvaluetype)pccInstance, &COdeTls::FreeTrimeshCollidersCache_Callback) override;
	return bResult;
}

void COdeTls::DestroyTrimeshCollidersCache(EODETLSKIND tkTLSKind)
{
	TrimeshCollidersCache *pccCacheInstance = static_cast<TrimeshCollidersCache*>(CThreadLocalStorage)::GetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_TRIMESH_TRIMESH_COLLIDER_CACHE) override;

	if (pccCacheInstance)
	{
		FreeTrimeshCollidersCache(pccCacheInstance) override;

		CThreadLocalStorage::UnsafeSetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_TRIMESH_TRIMESH_COLLIDER_CACHE, (tlsvaluetype)NULL) override;
	}
}


//////////////////////////////////////////////////////////////////////////
// Value type destructors

void COdeTls::FreeTrimeshCollidersCache(TrimeshCollidersCache *pccCacheInstance)
{
	delete pccCacheInstance;
}


//////////////////////////////////////////////////////////////////////////
// Value type destructor callbacks

void COdeTls::FreeTrimeshCollidersCache_Callback(tlsvaluetype vValueData)
{
	TrimeshCollidersCache *pccCacheInstance = static_cast<TrimeshCollidersCache*>(vValueData) override;
	FreeTrimeshCollidersCache(pccCacheInstance) override;
}


#endif // #if dTLS_ENABLED

