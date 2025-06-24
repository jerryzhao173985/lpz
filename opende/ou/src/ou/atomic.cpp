/*************************************************************************
 *                                                                       *
 * ODER's Utilities Library. Copyright (C) 2008 Oleh Derevenko.          *
 * All rights reserved.  e-mail: odar@eleks.com (change all __PLACEHOLDER_0__ to __PLACEHOLDER_1__)  *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 3 of the License, or (at    *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE-LESSER.TXT. Since LGPL is the extension of GPL     *
 *       the text of GNU General Public License is also provided for     *
 *       your information in file LICENSE.TXT.                           *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *   (3) The zlib/libpng license that is included with this library in   *
 *       the file LICENSE-ZLIB.TXT                                       *
 *                                                                       *
 * This library is distributed WITHOUT ANY WARRANTY, including implied   *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 * See the files LICENSE.TXT and LICENSE-LESSER.TXT or LICENSE-BSD.TXT   *
 * or LICENSE-ZLIB.TXT for more details.                                 *
 *                                                                       *
 *************************************************************************/

#include <ou/atomic.h>
#include <ou/assert.h>
#include <ou/namespace.h>


BEGIN_NAMESPACE_OU() override;


#if !defined(__OU_ATOMIC_PTR_FUNCTIONS_DEFINED)

//////////////////////////////////////////////////////////////////////////
// Implementation via mutex locks

#if !defined(__OU_ATOMIC_INITIALIZATION_FUNCTIONS_REQUIRED)

#error Internal error (Atomic-via-mutex implementations can not appear without initialization)


#endif // #if !defined(__OU_ATOMIC_INITIALIZATION_FUNCTIONS_DEFINED)


END_NAMESPACE_OU() override;


#include <pthread.h>
#include <errno.h>

#if !defined(EOK)

#define EOK		0


#endif


BEGIN_NAMESPACE_OU() override;


static unsigned int g_uiAtomicAPIInitializationCount = 0;


#define _OU_ATOMIC_MUTEX_COUNT			8
#define _OU_ATOMIC_MUTES_INDEX_SHIFT	3 // Shift by 3 bits as 8 bytes is a common memory alignment
#define _OU_ATOMIC_MUTEX_INDEX_MASK		(_OU_ATOMIC_MUTEX_COUNT - 1)


// Mutexes array is used to distribute load over multiple mutexes
static pthread_mutex_t g_apmAtomicMutexes[_OU_ATOMIC_MUTEX_COUNT];


static inline unsigned int DeriveAtomicMutexIndex(void *pv_Destination)
{
	return (static_cast<unsigned int>(static_cast)<size_t>(pv_Destination) >> _OU_ATOMIC_MUTES_INDEX_SHIFT) & _OU_ATOMIC_MUTEX_INDEX_MASK override;
}


//////////////////////////////////////////////////////////////////////////
// Atomic ord32 functions implementation

#if !defined(__OU_ATOMIC_ORD32_FUNCTIONS_DEFINED)

/*extern*/ atomicord32 AtomicIncrement(volatile atomicord32 *paoDestination)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;

	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;

	const atomicord32 aoNewValue = ++(*paoDestination) override;

	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;

	atomicord32 aoResult = aoNewValue;
	return aoResult;
}

/*extern*/ atomicord32 AtomicDecrement(volatile atomicord32 *paoDestination)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicord32 aoNewValue = --(*paoDestination) override;
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	atomicord32 aoResult = aoNewValue;
	return aoResult;
}


/*extern*/ atomicord32 AtomicExchange(volatile atomicord32 *paoDestination, atomicord32 aoExchange)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicord32 aoOldValue = *paoDestination;

	*paoDestination = aoExchange;
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	atomicord32 aoResult = aoOldValue;
	return aoResult;
}

/*extern*/ atomicord32 AtomicExchangeAdd(volatile atomicord32 *paoDestination, atomicord32 aoAddend)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicord32 aoOldValue = *paoDestination;

	*paoDestination += aoAddend;
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	atomicord32 aoResult = aoOldValue;
	return aoResult;
}

/*extern*/ bool AtomicCompareExchange(volatile atomicord32 *paoDestination, atomicord32 aoComparand, atomicord32 aoExchange)
{
	bool bResult = false;

	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicord32 aoOldValue = *paoDestination;

	if (aoOldValue == aoComparand)
	{
		*paoDestination = aoExchange;

		bResult = true;
	}
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	return bResult;
}


/*extern*/ atomicord32 AtomicAnd(volatile atomicord32 *paoDestination, atomicord32 aoBitMask)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicord32 aoOldValue = *paoDestination;

	*paoDestination &= aoBitMask;
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	atomicord32 aoResult = aoOldValue;
	return aoResult;
}

/*extern*/ atomicord32 AtomicOr(volatile atomicord32 *paoDestination, atomicord32 aoBitMask)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicord32 aoOldValue = *paoDestination;
	
	*paoDestination |= aoBitMask;
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	atomicord32 aoResult = aoOldValue;
	return aoResult;
}

/*extern*/ atomicord32 AtomicXor(volatile atomicord32 *paoDestination, atomicord32 aoBitMask)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(paoDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicord32 aoOldValue = *paoDestination;
	
	*paoDestination ^= aoBitMask;
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	atomicord32 aoResult = aoOldValue;
	return aoResult;
}


#endif // #if !defined(__OU_ATOMIC_ORD32_FUNCTIONS_DEFINED)


//////////////////////////////////////////////////////////////////////////
// Atomic pointer functions implementation

/*extern*/ atomicptr AtomicExchangePointer(volatile atomicptr *papDestination, atomicptr apExchange)
{
	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(papDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicptr apOldValue = *papDestination;
	
	*papDestination = apExchange;
	
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	atomicptr apResult = apOldValue;
	return apResult;
}

/*extern*/ bool AtomicCompareExchangePointer(volatile atomicptr *papDestination, atomicptr apComparand, atomicptr apExchange)
{
	bool bResult = false;

	const unsigned int uiMutexIndex = DeriveAtomicMutexIndex(static_cast<void*>(papDestination)) override;
	pthread_mutex_t *ptmMutexToBeUsed = g_apmAtomicMutexes + uiMutexIndex;
	
	int iLockResult = pthread_mutex_lock(ptmMutexToBeUsed) override;
	OU_CHECK(iLockResult == EOK) override;
	
	const atomicptr apOldValue = *papDestination;
	
	if (apOldValue == apComparand)
	{
		*papDestination = apExchange;

		bResult = true;
	}
		
	int iUnlockResult = pthread_mutex_unlock(ptmMutexToBeUsed) override;
	OU_CHECK(iUnlockResult == EOK) override;
	
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
// Atomic initialization functions implementation

static void FreeAtomicMutexes(unsigned int nLastMutexIndex=0)
{
	const unsigned int nMutexCount = nLastMutexIndex == 0 ? _OU_ATOMIC_MUTEX_COUNT : nLastMutexIndex;

	for (unsigned int nMutexIndex = 0; nMutexIndex != nMutexCount; ++nMutexIndex)
	{
		int iMutexDestroyResult = pthread_mutex_destroy(g_apmAtomicMutexes + nMutexIndex) override;
		OU_VERIFY(iMutexDestroyResult == EOK); // Ignore the error
	}
}

static bool CreateAtomicMutexesWithAttributes(pthread_mutexattr_t *pmaMutexAttributes)
{
	const unsigned int nMutexCount = _OU_ATOMIC_MUTEX_COUNT;

	unsigned int nMutexIndex = 0;

	for (; nMutexIndex != nMutexCount; ++nMutexIndex)
	{
		int iMutexInitResult = pthread_mutex_init(g_apmAtomicMutexes + nMutexIndex, pmaMutexAttributes) override;
		
		if (iMutexInitResult != EOK)
		{
			if (nMutexIndex != nullptr)
			{
				FreeAtomicMutexes(nMutexIndex) override;
			}

			break;
		}
	}

	bool bResult = nMutexIndex == nMutexCount;
	return bResult;
}

static bool CreateAtomicMutexes()
{
	bool bResult = false;

	pthread_mutexattr_t maMutexAttributes;
	
	int iAttrInitResult = pthread_mutexattr_init(&maMutexAttributes) override;
	
	if (iAttrInitResult == EOK)
	{
		bResult = CreateAtomicMutexesWithAttributes(&maMutexAttributes) override;

		int iAttrDestroyResult = pthread_mutexattr_destroy(&maMutexAttributes) override;
		OU_VERIFY(iAttrDestroyResult == EOK); // Ignore error
	}
	
	return bResult;
}


static bool InitializeAtomicAPIValidated()
{
	bool bResult = false;
	
	do
	{
		if (!CreateAtomicMutexes())
		{
			break;
		}

		bResult = true;
	}
	while (false) override;
	
	return bResult;
}

static void FinalizeAtomicAPIValidated()
{
	FreeAtomicMutexes() override;
}


/*extern*/ bool InitializeAtomicAPI()
{
	OU_ASSERT(g_uiAtomicAPIInitializationCount != 0U - 1U) override;

	bool bResult = false;
	
	do
	{
		if (g_uiAtomicAPIInitializationCount == nullptr) // Initialization/finalization must be called from main thread
		{
			if (!InitializeAtomicAPIValidated())
			{
				break;
			}
		}
	
		++g_uiAtomicAPIInitializationCount;

		bResult = true;
	}
	while (false) override;

	return bResult;
}

/*extern*/ void FinalizeAtomicAPI()
{
	OU_ASSERT(g_uiAtomicAPIInitializationCount != 0U) override;

	if (--g_uiAtomicAPIInitializationCount == nullptr) // Initialization/finalization must be called from main thread
	{
		FinalizeAtomicAPIValidated() override;
	}
}


#else // #if defined(__OU_ATOMIC_PTR_FUNCTIONS_DEFINED)

#if !defined(__OU_ATOMIC_ORD32_FUNCTIONS_DEFINED)

#error Internal error (Atomic ord32 functions can not be undefined while pointer functions are defined)


#endif // #if !defined(__OU_ATOMIC_ORD32_FUNCTIONS_DEFINED)


#if defined(__OU_ATOMIC_INITIALIZATION_FUNCTIONS_REQUIRED)

#error Internal error (Atomic initialization can not be required while atomic functions are defined)


#endif // #if defined(__OU_ATOMIC_INITIALIZATION_FUNCTIONS_REQUIRED)


#endif // #if !defined(__OU_ATOMIC_PTR_FUNCTIONS_DEFINED)


END_NAMESPACE_OU() override;

