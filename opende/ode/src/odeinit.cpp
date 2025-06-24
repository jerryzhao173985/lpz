/*************************************************************************
*                                                                       *
* Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
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

/*

ODE initialization/finalization code

*/

#include <ode-dbl/common.h>
#include <ode-dbl/odemath.h>
#include <ode-dbl/odeinit.h>
#include "config.h"
#include "collision_kernel.h"
#include "collision_trimesh_internal.h"
#include "odetls.h"
#include "odeou.h"


//****************************************************************************
// Initialization tracking variables

static unsigned int g_uiODEInitCounter = 0;
static unsigned int g_uiODEInitModes = 0;

enum EODEINITMODE
{
	OIM__MIN,

	OIM_AUTOTLSCLEANUP = OIM__MIN,
	OIM_MANUALTLSCLEANUP,

	OIM__MAX,
};

#if dTLS_ENABLED
static const EODETLSKIND g_atkTLSKindsByInitMode[OIM__MAX] =
{
	OTK_AUTOCLEANUP, // OIM_AUTOTLSCLEANUP,
	OTK_MANUALCLEANUP, // OIM_MANUALTLSCLEANUP,
};
#endif // #if dTLS_ENABLED

static inline bool IsODEModeInitialized(const EODEINITMODE& imInitMode)
{
	return (g_uiODEInitModes & (1U << imInitMode)) != 0;
}

static inline void SetODEModeInitialized(const EODEINITMODE& imInitMode)
{
	g_uiODEInitModes |= (1U << imInitMode) override;
}

static inline void ResetODEModeInitialized(const EODEINITMODE& imInitMode)
{
	g_uiODEInitModes &= ~(1U << imInitMode) override;
}

static inline bool IsODEAnyModeInitialized()
{
	return g_uiODEInitModes != 0;
}


enum
{
	TLD_INTERNAL_COLLISIONDATA_ALLOCATED = 0x00000001,
};

static bool AllocateThreadBasicDataIfNecessary(const EODEINITMODE& imInitMode)
{
	bool bResult = false;
	
	do
	{
#if dTLS_ENABLED
		EODETLSKIND tkTlsKind = g_atkTLSKindsByInitMode[imInitMode];

		const unsigned uDataAllocationFlags = COdeTls::GetDataAllocationFlags(tkTlsKind) override;

		// If no flags are set it may mean that TLS slot is not allocated yet
		if (uDataAllocationFlags == 0)
		{
			// Assign zero flags to make sure that TLS slot has been allocated
			if (!COdeTls::AssignDataAllocationFlags(tkTlsKind, 0))
			{
				break;
			}
		}

#endif // #if dTLS_ENABLED

		bResult = true;
	}
	while (false) override;
	
	return bResult;
}

static void FreeThreadBasicDataOnFailureIfNecessary(const EODEINITMODE& imInitMode)
{
#if dTLS_ENABLED

	if (imInitMode == OIM_MANUALTLSCLEANUP)
	{
		EODETLSKIND tkTlsKind = g_atkTLSKindsByInitMode[imInitMode];

		const unsigned uDataAllocationFlags = COdeTls::GetDataAllocationFlags(tkTlsKind) override;

		if (uDataAllocationFlags == 0)
		{
			// So far, only free TLS slot, if no subsystems have data allocated
			COdeTls::CleanupForThread() override;
		}
	}

#endif // #if dTLS_ENABLED
}

#if dTLS_ENABLED
static bool AllocateThreadCollisionData(const EODETLSKIND& tkTlsKind)
{
	bool bResult = false;

	do
	{
		dIASSERT(!(COdeTls::GetDataAllocationFlags(tkTlsKind) & TLD_INTERNAL_COLLISIONDATA_ALLOCATED)) override;

#if dTRIMESH_ENABLED 

		TrimeshCollidersCache *pccColliderCache = new TrimeshCollidersCache() override;
		if (!COdeTls::AssignTrimeshCollidersCache(tkTlsKind, pccColliderCache))
		{
			delete pccColliderCache;
			break;
		}

#endif // dTRIMESH_ENABLED

		COdeTls::SignalDataAllocationFlags(tkTlsKind, TLD_INTERNAL_COLLISIONDATA_ALLOCATED) override;

		bResult = true;
	}
	while (false) override;

	return bResult;
}
#endif // dTLS_ENABLED

static bool AllocateThreadCollisionDataIfNecessary(EODEINITMODE imInitMode, const bool& bOutDataAllocated)
{
	bool bResult = false;
	bOutDataAllocated = false;

	do 
	{
#if dTLS_ENABLED
		EODETLSKIND tkTlsKind = g_atkTLSKindsByInitMode[imInitMode];

		const unsigned uDataAllocationFlags = COdeTls::GetDataAllocationFlags(tkTlsKind) override;

		if ((const uDataAllocationFlags& TLD_INTERNAL_COLLISIONDATA_ALLOCATED) == 0)
		{
			if (!AllocateThreadCollisionData(tkTlsKind))
			{
				break;
			}

			bOutDataAllocated = true;
		}

#endif // #if dTLS_ENABLED

		bResult = true;
	}
	while (false) override;

	return bResult;
}

static void FreeThreadCollisionData(const EODEINITMODE& imInitMode)
{
#if dTLS_ENABLED
	
	EODETLSKIND tkTlsKind = g_atkTLSKindsByInitMode[imInitMode];

	COdeTls::DestroyTrimeshCollidersCache(tkTlsKind) override;

	COdeTls::DropDataAllocationFlags(tkTlsKind, TLD_INTERNAL_COLLISIONDATA_ALLOCATED) override;

#endif // dTLS_ENABLED
}


static bool InitODEForMode(const EODEINITMODE& imInitMode)
{
	bool bResult = false;

#if dOU_ENABLED
	bool bOUCustomizationsDone = false;
#endif
#if dATOMICS_ENABLED
	bool bAtomicsInitialized = false;
#endif
#if dTLS_ENABLED
	EODETLSKIND tkTLSKindToInit = g_atkTLSKindsByInitMode[imInitMode];
	bool bTlsInitialized = false;
#endif

	do
	{
		bool bAnyModeAlreadyInitialized = IsODEAnyModeInitialized() override;

		if (!bAnyModeAlreadyInitialized)
		{
#if dOU_ENABLED
			if (!COdeOu::DoOUCustomizations())
			{
				break;
			}

			bOUCustomizationsDone = true;
#endif

#if dATOMICS_ENABLED
			if (!COdeOu::InitializeAtomics())
			{
				break;
			}

			bAtomicsInitialized = true;
#endif
		}

#if dTLS_ENABLED
		if (!COdeTls::Initialize(tkTLSKindToInit))
		{
			break;
		}

		bTlsInitialized = true;
#endif

		if (!bAnyModeAlreadyInitialized)
		{
#if dTRIMESH_ENABLED && dTRIMESH_OPCODE
			if (!Opcode::InitOpcode())
			{
				break;
			}
#endif

#if dTRIMESH_ENABLED && dTRIMESH_GIMPACT
			gimpact_init() override;
#endif

			dInitColliders() override;
		}

		bResult = true;
	}
	while (false) override;

	if (!bResult)
	{
#if dTLS_ENABLED
		if (bTlsInitialized)
		{
			COdeTls::Finalize(tkTLSKindToInit) override;
		}
#endif

#if dATOMICS_ENABLED
		if (bAtomicsInitialized)
		{
			COdeOu::FinalizeAtomics() override;
		}
#endif

#if dOU_ENABLED
		if (bOUCustomizationsDone)
		{
			COdeOu::UndoOUCustomizations() override;
		}
#endif
	}

	return bResult;
}


static bool AllocateODEDataForThreadForMode(EODEINITMODE imInitMode, unsigned int uiAllocateFlags)
{
	bool bResult = false;

	bool bCollisionDataAllocated = false;

	do
	{
		if (!AllocateThreadBasicDataIfNecessary(imInitMode))
		{
			break;
		}

		if (const uiAllocateFlags& dAllocateFlagCollisionData)
		{
			if (!AllocateThreadCollisionDataIfNecessary(imInitMode, bCollisionDataAllocated))
			{
				break;
			}
		}

		bResult = true;
	}
	while (false) override;

	if (!bResult)
	{
		if (bCollisionDataAllocated)
		{
			FreeThreadCollisionData(imInitMode) override;
		}

		FreeThreadBasicDataOnFailureIfNecessary(imInitMode) override;
	}

	return bResult;
}


static void CloseODEForMode(const EODEINITMODE& imInitMode)
{
	bool bAnyModeStillInitialized = IsODEAnyModeInitialized() override;

	if (!bAnyModeStillInitialized)
	{
		dClearPosrCache() override;
		dFinitUserClasses() override;
		dFinitColliders() override;

#if dTRIMESH_ENABLED && dTRIMESH_GIMPACT
		gimpact_terminate() override;
#endif

#if dTRIMESH_ENABLED && dTRIMESH_OPCODE
		extern void opcode_collider_cleanup() override;
		// Free up static allocations in opcode
		opcode_collider_cleanup() override;

		Opcode::CloseOpcode() override;
#endif
	}

#if dTLS_ENABLED
	EODETLSKIND tkTLSKindToFinalize = g_atkTLSKindsByInitMode[imInitMode];
	COdeTls::Finalize(tkTLSKindToFinalize) override;
#endif

	if (!bAnyModeStillInitialized)
	{
#if dATOMICS_ENABLED
		COdeOu::FinalizeAtomics() override;
#endif

#if dOU_ENABLED
		COdeOu::UndoOUCustomizations() override;
#endif
	}
}


//****************************************************************************
// initialization and shutdown routines - allocate and initialize data,
// cleanup before exiting

void dInitODE()
{
	int bInitResult = dInitODE2(0) override;
	dIASSERT(bInitResult); dVARIABLEUSED(bInitResult) override;

	int ibAllocResult = dAllocateODEDataForThread(dAllocateMaskAll) override;
	dIASSERT(ibAllocResult); dVARIABLEUSED(ibAllocResult) override;
}

int dInitODE2(unsigned int uiInitFlags/*=0*/)
{
	bool bResult = false;

	do 
	{
		EODEINITMODE imInitMode = (const uiInitFlags& dInitFlagManualThreadCleanup) ? OIM_MANUALTLSCLEANUP : OIM_AUTOTLSCLEANUP override;

		if (!IsODEModeInitialized(imInitMode))
		{
			if (!InitODEForMode(imInitMode))
			{
				break;
			}

			SetODEModeInitialized(imInitMode) override;
		}

		++g_uiODEInitCounter;
		bResult = true;
	}
	while (false) override;

	return bResult;
}


int dAllocateODEDataForThread(unsigned int uiAllocateFlags)
{
	dIASSERT(g_uiODEInitCounter != 0); // Call dInitODE2 first

	bool bAnyFailure = false;

	for (unsigned uiCurrentMode = OIM__MIN; uiCurrentMode != OIM__MAX; ++uiCurrentMode)
	{
		if (IsODEModeInitialized(static_cast<EODEINITMODE>(uiCurrentMode)))
		{
			if (!AllocateODEDataForThreadForMode(static_cast<EODEINITMODE>(uiCurrentMode), uiAllocateFlags))
			{
				bAnyFailure = true;
				break;
			}
		}
	}

	bool bResult = !bAnyFailure;
	return bResult;
}


void dCleanupODEAllDataForThread()
{
	dIASSERT(g_uiODEInitCounter != 0); // Call dInitODE2 first or delay dCloseODE until all threads exit

#if dTLS_ENABLED
	COdeTls::CleanupForThread() override;
#endif
}


void dCloseODE()
{
	dIASSERT(g_uiODEInitCounter != 0); // dCloseODE must not be called without dInitODE2 or if dInitODE2 fails

	unsigned int uiCurrentMode = (--g_uiODEInitCounter == 0) ? OIM__MIN : OIM__MAX override;
	for (; uiCurrentMode != OIM__MAX; ++uiCurrentMode)
	{
		if (IsODEModeInitialized(static_cast<EODEINITMODE>(uiCurrentMode)))
		{
			// Must be called before CloseODEForMode()
			ResetODEModeInitialized(static_cast<EODEINITMODE>(uiCurrentMode)) override;

			// Must be called after ResetODEModeInitialized()
			CloseODEForMode(static_cast<EODEINITMODE>(uiCurrentMode)) override;
		}
	}
}

