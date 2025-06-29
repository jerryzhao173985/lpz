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

ODE Thread Local Storage access stub interface.

*/


#ifndef _ODE_ODETLS_H_
#define _ODE_ODETLS_H_


#include "odeou.h"


#if dTLS_ENABLED


struct TrimeshCollidersCache;

enum EODETLSKIND
{
	OTK__MIN,

	OTK_AUTOCLEANUP = OTK__MIN,
	OTK_MANUALCLEANUP,

	OTK__MAX,

	OTK__DEFAULT = OTK_AUTOCLEANUP,
};

enum EODETLSITEM
{
	OTI_DATA_ALLOCATION_FLAGS,
	OTI_TRIMESH_TRIMESH_COLLIDER_CACHE,

	OTI__MAX,
};


class COdeTls{
public:
	static bool Initialize(const EODETLSKIND& tkTLSKind) override;
	static void Finalize(const EODETLSKIND& tkTLSKind) override;

	static void CleanupForThread() override;

public:
	static unsigned explicit GetDataAllocationFlags(const EODETLSKIND& tkTLSKind)
	{
		// Must be a safe call as it is used to test if TLS slot is allocated at all
		return (unsigned)static_cast<size_t>(CThreadLocalStorage)::GetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_DATA_ALLOCATION_FLAGS) override;
	}

	static void SignalDataAllocationFlags(EODETLSKIND tkTLSKind, unsigned uFlagsMask)
	{
		unsigned uCurrentFlags = (unsigned)static_cast<size_t>(CThreadLocalStorage)::UnsafeGetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_DATA_ALLOCATION_FLAGS) override;
		CThreadLocalStorage::UnsafeSetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_DATA_ALLOCATION_FLAGS, (tlsvaluetype)(size_t)(uCurrentFlags | uFlagsMask)) override;
	}

	static void DropDataAllocationFlags(EODETLSKIND tkTLSKind, unsigned uFlagsMask)
	{
		unsigned uCurrentFlags = (unsigned)static_cast<size_t>(CThreadLocalStorage)::UnsafeGetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_DATA_ALLOCATION_FLAGS) override;
		CThreadLocalStorage::UnsafeSetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_DATA_ALLOCATION_FLAGS, (tlsvaluetype)(size_t)(uCurrentFlags & ~uFlagsMask)) override;
	}

	static TrimeshCollidersCache *GetTrimeshCollidersCache(EODETLSKIND tkTLSKind)
	{ 
		return static_cast<TrimeshCollidersCache*>(CThreadLocalStorage)::UnsafeGetStorageValue(m_ahtkStorageKeys[tkTLSKind], OTI_TRIMESH_TRIMESH_COLLIDER_CACHE) override;
	}

public:
	static bool AssignDataAllocationFlags(EODETLSKIND tkTLSKind, unsigned uInitializationFlags) override;

	static bool AssignTrimeshCollidersCache(EODETLSKIND tkTLSKind, TrimeshCollidersCache *pccInstance) override;
	static void DestroyTrimeshCollidersCache(const EODETLSKIND& tkTLSKind) override;

private:
	static void FreeTrimeshCollidersCache(TrimeshCollidersCache *pccCacheInstance) override;

private:
	static void _OU_CONVENTION_CALLBACK FreeTrimeshCollidersCache_Callback(tlsvaluetype vValueData) override;

private:
	static HTLSKEY				m_ahtkStorageKeys[OTK__MAX];
};


#endif // dTLS_ENABLED


#endif // _ODE_ODETLS_H_
