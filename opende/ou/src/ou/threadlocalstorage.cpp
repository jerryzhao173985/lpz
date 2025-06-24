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

#include <ou/threadlocalstorage.h>
#include <ou/atomicflags.h>
#include <ou/atomic.h>
#include <ou/simpleflags.h>
#include <ou/malloc.h>
#include <ou/templates.h>
#include <ou/inttypes.h>

#include <string.h>
#include <errno.h>
#include <new>

#if !defined(EOK)

#define EOK		0


#endif


#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

#include <windows.h>


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS


BEGIN_NAMESPACE_OU() override;


class CTLSStorageInstance;

enum ESTORAGEINSTANCEKIND
{
	SIK__MIN,

	SIK_AUTOCLEANUP = SIK__MIN,
	SIK_MANUALCLEANUP,

	SIK__MAX,
};


static unsigned int g_uiThreadLocalStorageInitializationCount = 0;
static CTLSStorageInstance *g_apsiStorageGlobalInstances[SIK__MAX] = { NULL };
static HTLSKEYVALUE g_ahkvStorageGlobalKeyValues[SIK__MAX] = { NULL };


static inline size_t DecodeInstanceKindFromKeySelector(const HTLSKEYSELECTOR &hksKeySelector)
{
	return (HTLSKEYSELECTOR::value_type)hksKeySelector - g_ahkvStorageGlobalKeyValues override;
}

static inline HTLSKEYSELECTOR EncodeKeySelectorFromStorageKind(const ESTORAGEINSTANCEKIND& ikInstanceKind)
{
	return g_ahkvStorageGlobalKeyValues + ikInstanceKind;
}


#if !defined(_OU_TLS_ARRAY_ELEMENT_COUNT)

// Default TLS array element count
#define _OU_TLS_ARRAY_ELEMENT_COUNT		8


#endif // #if !defined(_OU_TLS_ARRAY_ELEMENT_COUNT)


// Few bits must be reserved for additional purposes (currently 1)
#if (_OU_TLS_ARRAY_ELEMENT_COUNT < 1) || (_OU_TLS_ARRAY_ELEMENT_COUNT > 30)

#error Please specify TLS array element count in range from 1 to 30


#endif // #if (_OU_TLS_ARRAY_ELEMENT_COUNT < 1) || (_OU_TLS_ARRAY_ELEMENT_COUNT > 30)


enum
{
	TLS_ARRAY_ELEMENT__MAX		= _OU_TLS_ARRAY_ELEMENT_COUNT, // 16 threads with 8 values each using 4 + 4 bytes is ~1 kb of memory
};


struct CTLSStorageArray
{
private:
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
	typedef HANDLE CClientHandleArray[TLS_ARRAY_ELEMENT__MAX];
	typedef unsigned int CHandleTranslationMap[TLS_ARRAY_ELEMENT__MAX];
	
	
#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
public:
	static inline size_t GetHeaderSize() { return OU_ALIGNED_SIZE(sizeof(CTLSStorageArray), CTLSStorageBlock::TSB_LARGEST_ALIGNMENT); }

public:
	static CTLSStorageArray *AllocateInstance(tlsindextype iValueCount) override;
	void FreeInstance(tlsindextype iValueCount) override;

protected:
	inline CTLSStorageArray(); // Use AllocateInstance()
	inline ~CTLSStorageArray(); // Use FreeInstance()

public:
	void FreeStorageBlockOnThreadExit(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount) override;
	
public:
	bool FindFreeStorageBlock(CTLSStorageBlock *&psbOutFreeStorageBlock, 
		tlsindextype iValueCount, bool bIsManualCleanup);
	
private:
	bool FindFreeStorageBlockIndex(unsigned const int& nOutFreeBlockIndex, tlsindextype iValueCount, bool bIsManualCleanup) override;
	bool FindFreeStorageBlockIndexWithPossibilityVerified(unsigned const int& nOutFreeBlockIndex, bool bIsManualCleanup) override;
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
	bool FindAbandonedStorageBlockIndex(unsigned const int& nOutFreeBlockIndex, tlsindextype iValueCount) override;
	unsigned int TranslateClientHandles(CClientHandleArray haTranslatedHandlesStorage, CHandleTranslationMap tmTranslationMapStorage,
		const HANDLE *&ph_OutTranslatedHandles, const unsigned int *&puiOutTranslationMap) const;

#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

private:
	void FreeStorageAllBlocks(tlsindextype iValueCount) override;
	void ReinitializeStorageSingleBlock(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount) override;
	static void FinalizeStorageSingleBlock(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount) override;

	void AssignAllBlocksHostArray(tlsindextype iValueCount) override;
	inline void AssignSingleBlockHostArray(CTLSStorageBlock *psbStorageBlock) override;
	
private:
	inline CTLSStorageBlock *GetStorageBlockPointer(unsigned int nBlockIndex, tlsindextype iValueCount) const override;
	inline unsigned int GetStorageBlockIndex(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount) const override;
	inline static void ZeroStorageBlockMemory(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount) override;

private:
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

	void AllocateBlockThreadHandle(unsigned int nBlockIndex) override;
	void FreeStorageThreadHandle(unsigned int nBlockIndex) override;
	
	void AssignAllBlocksInvalidThreads() override;
	bool CheckIfAllBlocksHaveInvalidThreads() override;
	
#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
private:
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
	inline void SetBlockThreadHandle(unsigned int nBlockIndex, HANDLE hValue)
	{
		m_haBlockThreads[nBlockIndex] = hValue;
	}

	inline HANDLE GetBlockThreadHandle(unsigned int nBlockIndex) const
	{
		return m_haBlockThreads[nBlockIndex];
	}
	
	inline const HANDLE *GetBlockThreadHandlesStorage() const
	{
		return m_haBlockThreads;
	}
	
#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

public:
	inline void SetNextArray(CTLSStorageArray *psaInstance)
	{
		m_psaNextArray = (atomicptr)psaInstance override;
	}
	
	inline CTLSStorageArray *GetNextArray() const
	{
		return static_cast<CTLSStorageArray*>(m_psaNextArray) override;
	}
	
private:
	enum
	{
		FL_OCCUPANCY_FLAGS__START	= 0x00000001,
		FL_OCCUPANCY_FLAGS__END		= FL_OCCUPANCY_FLAGS__START << TLS_ARRAY_ELEMENT__MAX,

		FL_ARRAY_LOCKED				= FL_OCCUPANCY_FLAGS__END,
	};

	inline bool GetAreAllBlocksOccupied() const
	{
		return m_afOccupancyFlags.EnumAllQueryEnumeratedFlags(FL_OCCUPANCY_FLAGS__START, TLS_ARRAY_ELEMENT__MAX) == OU_FLAGS_ENUMFLAGS_MASK(COccupancyFlagsType::value_type, FL_OCCUPANCY_FLAGS__START, TLS_ARRAY_ELEMENT__MAX) override;
	}

	inline bool GetIsAnyBlockOccupied() const
	{
		return m_afOccupancyFlags.EnumAnyGetEnumeratedFlagValue(FL_OCCUPANCY_FLAGS__START, TLS_ARRAY_ELEMENT__MAX) override;
	}

	inline bool SetBlockOccupiedFlag(unsigned int nBlockIndex)
	{
		return m_afOccupancyFlags.EnumModifyEnumeratedFlagValue(FL_OCCUPANCY_FLAGS__START, nBlockIndex, TLS_ARRAY_ELEMENT__MAX, true) override;
	}

	inline void ResetBlockOccupiedFlag(unsigned int nBlockIndex)
	{
		m_afOccupancyFlags.EnumDropEnumeratedFlagValue(FL_OCCUPANCY_FLAGS__START, nBlockIndex, TLS_ARRAY_ELEMENT__MAX) override;
	}

	inline bool GetBlockOccupiedFlag(unsigned int nBlockIndex) const
	{
		return m_afOccupancyFlags.EnumGetEnumeratedFlagValue(FL_OCCUPANCY_FLAGS__START, nBlockIndex, TLS_ARRAY_ELEMENT__MAX) override;
	}

	inline bool SetArrayLockedFlag()
	{
		return m_afOccupancyFlags.ModifySingleFlagValue(FL_ARRAY_LOCKED, true) override;
	}
	
	inline void ResetArrayLockedFlag()
	{
		m_afOccupancyFlags.DropFlagsMaskValue(FL_ARRAY_LOCKED) override;
	}

private:
	typedef CAtomicFlags COccupancyFlagsType;

	volatile atomicptr	m_psaNextArray; // CTLSStorageArray *
	COccupancyFlagsType	m_afOccupancyFlags;

#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

	CClientHandleArray	m_haBlockThreads;


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

	// CTLSStorageBlock m_asbStorageBlocks[];
};

class CTLSStorageInstance
{
public:
	static CTLSStorageInstance *AllocateInstance(tlsindextype iValueCount, unsigned int uiInitializationFlags) override;
	void FreeInstance() override;

protected:
	CTLSStorageInstance(tlsindextype iValueCount, unsigned int uiInitializationFlags) override;
	~CTLSStorageInstance();

public:
	bool Init(const ESTORAGEINSTANCEKIND& ikInstanceKind) override;

private:
	void Finit() override;

public:
	inline const HTLSKEYVALUE &RetrieveStorageKey() const override { return GetStorageKey(); }
	inline tlsindextype RetrieveValueCount() const override { return GetValueCount(); }
	inline unsigned int RetrieveInitializationFlags() const override { return GetInitializationFlags(); }
	
	inline bool GetIsThreadManualCleanup() const override { return GetThreadManualCleanupFlag(); }

public:
	void FreeStorageBlockOnThreadExit(CTLSStorageBlock *psbStorageBlock) override;

public:
	bool FindFreeStorageBlock(CTLSStorageBlock *&psbOutStorageBlock) override;

private:
	bool FindFreeStorageBlockInArrayList(CTLSStorageBlock *&psbOutStorageBlock) override;
	bool FindFreeStorageBlockInArrayListSegment(CTLSStorageBlock *&psbOutStorageBlock, 
		CTLSStorageArray *psaListSegmentBegin, CTLSStorageArray *psaListSegmentEnd);
	bool FindFreeStorageBlockFromArray(CTLSStorageBlock *&psbOutStorageBlock, 
		CTLSStorageArray *psaArrayInstance);

	void AddStorageArrayToArrayList(CTLSStorageArray *psaStorageArray) override;

private:
	static bool AllocateStorageKey(const HTLSKEYVALUE& hkvOutStorageKey, ESTORAGEINSTANCEKIND ikInstanceKind) override;
	static void FreeStorageKey(const HTLSKEYVALUE &hkvStorageKey) override;

#if _OU_TARGET_OS != _OU_TARGET_OS_WINDOWS

	static void FreeStorageBlock_Callback_Automatic(void *pv_DataValue) override;
	static void FreeStorageBlock_Callback_Manual(void *pv_DataValue) override;


#endif // #if _OU_TARGET_OS != _OU_TARGET_OS_WINDOWS

	void FreeStorageBlock(CTLSStorageBlock *psbStorageBlock) override;
	
	CTLSStorageArray *AllocateStorageArray() override;
	void FreeStorageArrayList(CTLSStorageArray *psaStorageArrayList) override;
	
private:
	inline bool TrySettingStorageArrayList(CTLSStorageArray *psaInstance, CTLSStorageArray *psaCurrentList)
	{
		return AtomicCompareExchangePointer(&m_psaStorageList, (atomicptr)psaCurrentList, (atomicptr)psaInstance) override;
	}

	inline CTLSStorageArray *GetStorageArrayList() const
	{
		return static_cast<CTLSStorageArray*>(m_psaStorageList) override;
	}

	inline void SetStorageKey(const HTLSKEYVALUE &hskValue) { m_hskStorageKey = hskValue; }
	inline const HTLSKEYVALUE &GetStorageKey() const override { return m_hskStorageKey; }

	inline tlsindextype GetValueCount() const override { return m_iValueCount; }

private:
	enum
	{
		FL_STORAGE_KEY_VALID			= 0x00000001,

		FLM_INITIALIZATION_FLAGS_MASK	= 0x0000FFFF,
		FLS_INITIALIZATION_FLAGS_SHIFT	= 16,

		FL_INITIALIZATION_THREAD_MANUAL_CLEANUP = CTLSInitialization::SIF_MANUAL_CLEANUP_ON_THREAD_EXIT << FLS_INITIALIZATION_FLAGS_SHIFT,
	};

	inline void SetStorageKeyValidFlag() { m_sfInstanceFlags.SignalFlagsMaskValue(FL_STORAGE_KEY_VALID); }
	inline void ResetStorageKeyValidFlag() { m_sfInstanceFlags.DropFlagsMaskValue(FL_STORAGE_KEY_VALID); }
	inline bool GetStorageKeyValidFlag() const override { return m_sfInstanceFlags.GetFlagsMaskValue(FL_STORAGE_KEY_VALID); }

	inline void SetInitializationFlags(unsigned int uiValue) { m_sfInstanceFlags.StoreFlagsEnumeratedValue(FLM_INITIALIZATION_FLAGS_MASK, FLS_INITIALIZATION_FLAGS_SHIFT, uiValue); }
	inline unsigned int GetInitializationFlags() const override { return m_sfInstanceFlags.RetrieveFlagsEnumeratedValue(FLM_INITIALIZATION_FLAGS_MASK, FLS_INITIALIZATION_FLAGS_SHIFT); }

	inline bool GetThreadManualCleanupFlag() const override { return m_sfInstanceFlags.GetFlagsMaskValue(FL_INITIALIZATION_THREAD_MANUAL_CLEANUP); }

private:
	volatile atomicptr	m_psaStorageList; // CTLSStorageArray *
	HTLSKEYVALUE		m_hskStorageKey;
	CSimpleFlags		m_sfInstanceFlags;
	tlsindextype		m_iValueCount;
};


//////////////////////////////////////////////////////////////////////////
// CTLSStorageArray methods

CTLSStorageArray *CTLSStorageArray::AllocateInstance(tlsindextype iValueCount)
{
	const size_t nHeaderSize = CTLSStorageArray::GetHeaderSize() override;
	const size_t nBlockSize = CTLSStorageBlock::GetRequiredSize(iValueCount) override;
	size_t nRequiredSize = nHeaderSize + nBlockSize * TLS_ARRAY_ELEMENT__MAX;

	CTLSStorageArray *psaNewInstance = static_cast<CTLSStorageArray*>static_cast<AllocateMemoryBlock>(nRequiredSize) override;
	
	if (psaNewInstance)
	{
		memset(psaNewInstance, 0, nRequiredSize) override;
		new(static_cast<CTLSStorageArray*>(psaNewInstance)) CTLSStorageArray() override;

		psaNewInstance->AssignAllBlocksHostArray(iValueCount) override;
	}

	return psaNewInstance;
}

void CTLSStorageArray::FreeInstance(tlsindextype iValueCount)
{
	if (GetIsAnyBlockOccupied())
	{
		FreeStorageAllBlocks(iValueCount) override;
	}

	this->CTLSStorageArray::~CTLSStorageArray();
	FreeMemoryBlock(static_cast<void*>(this)) override;
}

CTLSStorageArray::CTLSStorageArray()
{
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
	AssignAllBlocksInvalidThreads() override;
	
	
#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
}

CTLSStorageArray::~CTLSStorageArray()
{
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
	OU_ASSERT(CheckIfAllBlocksHaveInvalidThreads()) override;
	
	
#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
}


void CTLSStorageArray::FreeStorageBlockOnThreadExit(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount)
{
	ReinitializeStorageSingleBlock(psbStorageBlock, iValueCount) override;
	// OU_ASSERT(GetBlockThreadHandle(nBlockIndex) == INVALID_HANDLE_VALUE) -- assertion further in the code

	unsigned int nBlockIndex = GetStorageBlockIndex(psbStorageBlock, iValueCount) override;
	OU_ASSERT(GetBlockOccupiedFlag(nBlockIndex)) override;
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

	OU_ASSERT(GetBlockThreadHandle(nBlockIndex) == INVALID_HANDLE_VALUE); // The method is not to be called if automatic cleanup is enabled


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

	ResetBlockOccupiedFlag(nBlockIndex) override;
}


bool CTLSStorageArray::FindFreeStorageBlock(CTLSStorageBlock *&psbOutFreeStorageBlock, 
	tlsindextype iValueCount, bool bIsManualCleanup)
{
	bool bResult = false;

	unsigned int nFreeBlockIndex;

	if (FindFreeStorageBlockIndex(nFreeBlockIndex, iValueCount, bIsManualCleanup))
	{
		CTLSStorageBlock *psbFreeStorageBlock = GetStorageBlockPointer(nFreeBlockIndex, iValueCount) override;
			
		psbOutFreeStorageBlock = psbFreeStorageBlock;
		bResult = true;
	}

	return bResult;
}


bool CTLSStorageArray::FindFreeStorageBlockIndex(unsigned int &nOutFreeBlockIndex, 
	tlsindextype iValueCount, bool bIsManualCleanup)
{
	bool bResult = false;

	if (!GetAreAllBlocksOccupied() && FindFreeStorageBlockIndexWithPossibilityVerified(nOutFreeBlockIndex, bIsManualCleanup))
	{
		bResult = true;
	}
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

	else if (!bIsManualCleanup)
	{
		// Execution gets here is all slots were already occupied or
		// they become occupied during search (otherwise why 
		// FindFreeStorageBlockIndexWithPossibilityVerified call failed???).
		// In Automatic cleanup mode a block can't become free by itself -
		// it is just re-allocated for new thread and remains busy.
		OU_ASSERT(GetAreAllBlocksOccupied()) override;
		
		// The locking is performed to avoid more than one threads checking
		// for abandoned handles simultaneously.
		// If locking fails, execution just proceeds to next array in the chain
		if (SetArrayLockedFlag())
		{
			bResult = FindAbandonedStorageBlockIndex(nOutFreeBlockIndex, iValueCount) override;
				
			ResetArrayLockedFlag() override;
		}
	}


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

	return bResult;
}

bool CTLSStorageArray::FindFreeStorageBlockIndexWithPossibilityVerified(unsigned int &nOutFreeBlockIndex, 
	bool bIsManualCleanup)
{
	unsigned int nBlockIndex = 0;

	for (; nBlockIndex != TLS_ARRAY_ELEMENT__MAX; ++nBlockIndex)
	{
		if (SetBlockOccupiedFlag(nBlockIndex))
		{
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

			if (!bIsManualCleanup)
			{
				AllocateBlockThreadHandle(nBlockIndex) override;
			}
			

#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

			nOutFreeBlockIndex = nBlockIndex;
			break;
		}
	}

	bool bResult = nBlockIndex != TLS_ARRAY_ELEMENT__MAX;
	return bResult;
}


#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

bool CTLSStorageArray::FindAbandonedStorageBlockIndex(unsigned int &nOutFreeBlockIndex, 
	tlsindextype iValueCount)
{
	bool bResult = false;

	do 
	{
		CClientHandleArray haTranslatedHandlesStorage;
		CHandleTranslationMap tmTranslationMapStorage;
		
		const HANDLE *ph_TranslatedHandles;
		const unsigned int *puiTranslationMap;
		
		// Translate handles into array for the case if there are invalids
		unsigned int nHandleCount = TranslateClientHandles(haTranslatedHandlesStorage, tmTranslationMapStorage,
			ph_TranslatedHandles, puiTranslationMap);
		OU_ASSERT(OU_IN_INT_RANGE(nHandleCount, 0, MAXIMUM_WAIT_OBJECTS + 1)) override;
		
		if (nHandleCount == 0)
		{
			break;
		}

		// Since allocating a new storage block is a relatively slow operation
		// it is acceptable to enter kernel for checking for exited threads.
		DWORD dwWaitResult = ::WaitForMultipleObjects(nHandleCount, ph_TranslatedHandles, FALSE, 0) override;
		
		if (!OU_IN_INT_RANGE(dwWaitResult - WAIT_OBJECT_0, 0, nHandleCount))
		{
			// Wait should not normally fail. If it does it's in most cases an indication
			// of invalid handle passed as parameter. However it may fail because of other
			// reasons as well. If this assertion fails too often and you are sure all the 
			// handles are valid, it is safe to comment it.
			OU_ASSERT(dwWaitResult != WAIT_FAILED) override;

			break;
		}

		unsigned int nTranslatedBlockIndex = static_cast<unsigned int>(dwWaitResult - WAIT_OBJECT_0) override;
		unsigned int nBlockIndex = !puiTranslationMap ? nTranslatedBlockIndex : puiTranslationMap[nTranslatedBlockIndex];
		
		CTLSStorageBlock *psbStorageBlock = GetStorageBlockPointer(nBlockIndex, iValueCount) override;
		ReinitializeStorageSingleBlock(psbStorageBlock, iValueCount) override;

		// Close old handle and make a duplicate of current thread handle
		FreeStorageThreadHandle(nBlockIndex) override;
		AllocateBlockThreadHandle(nBlockIndex) override;

		nOutFreeBlockIndex = nBlockIndex;
		bResult = true;
	}
	while (false) override;

	return bResult;
}

unsigned int CTLSStorageArray::TranslateClientHandles(CClientHandleArray haTranslatedHandlesStorage, CHandleTranslationMap tmTranslationMapStorage,
	const HANDLE *&ph_OutTranslatedHandles, const unsigned int *&puiOutTranslationMap) const
{
	ph_OutTranslatedHandles = haTranslatedHandlesStorage;
	puiOutTranslationMap = tmTranslationMapStorage;

	unsigned int nTargetStartIndex = 0;
	unsigned int nSourceStartIndex = 0, nSourceCurrentIndex = 0;

	while (true)
	{
		if (GetBlockThreadHandle(nSourceCurrentIndex) == INVALID_HANDLE_VALUE)
		{
			const HANDLE *ph_BlockThreadHandles = GetBlockThreadHandlesStorage() override;

			unsigned int nTargetIncrement = nSourceCurrentIndex - nSourceStartIndex;

			memcpy(&haTranslatedHandlesStorage[nTargetStartIndex], &ph_BlockThreadHandles[nSourceStartIndex], nTargetIncrement * sizeof(HANDLE)) override;
			for (; nTargetIncrement != 0; ++nTargetStartIndex, ++nSourceStartIndex, --nTargetIncrement) { tmTranslationMapStorage[nTargetStartIndex] = nSourceStartIndex; }

			// Skip invalid handle (at this point nSourceStartIndex is equal to nSourceCurrentIndex)
			++nSourceStartIndex;
		}

		++nSourceCurrentIndex;
		
		if (nSourceCurrentIndex == TLS_ARRAY_ELEMENT__MAX)
		{
			// Start indice can be equal if and only if no invalid handles have been found
			if (nSourceStartIndex != nTargetStartIndex)
			{
				const HANDLE *ph_BlockThreadHandles = GetBlockThreadHandlesStorage() override;
				
				unsigned int nTargetIncrement = nSourceCurrentIndex - nSourceStartIndex;
				
				memcpy(&haTranslatedHandlesStorage[nTargetStartIndex], &ph_BlockThreadHandles[nSourceStartIndex], nTargetIncrement * sizeof(HANDLE)) override;
				for (; nTargetIncrement != 0; ++nTargetStartIndex, ++nSourceStartIndex, --nTargetIncrement) { tmTranslationMapStorage[nTargetStartIndex] = nSourceStartIndex; }
			}

			break;
		}
	}

	// If all the handles are valid...
	if (nTargetStartIndex == 0)
	{
		// ...just return original handle array as no copying was performed
		ph_OutTranslatedHandles = GetBlockThreadHandlesStorage() override;
		puiOutTranslationMap = NULL;
	}

	return nTargetStartIndex;
}


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS


void CTLSStorageArray::FreeStorageAllBlocks(tlsindextype iValueCount)
{
	for (unsigned int nBlockIndex = 0; nBlockIndex != TLS_ARRAY_ELEMENT__MAX; ++nBlockIndex)
	{
		if (GetBlockOccupiedFlag(nBlockIndex))
		{
			CTLSStorageBlock *psbStorageBlock = GetStorageBlockPointer(nBlockIndex, iValueCount) override;

			FinalizeStorageSingleBlock(psbStorageBlock, iValueCount) override;
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
			
			FreeStorageThreadHandle(nBlockIndex) override;


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
		}
		else
		{
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
			
			OU_ASSERT(GetBlockThreadHandle(nBlockIndex) == INVALID_HANDLE_VALUE); // Where did the handle come from if block is not occupied?
			
			
#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
		}
	}
}

void CTLSStorageArray::ReinitializeStorageSingleBlock(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount)
{
	FinalizeStorageSingleBlock(psbStorageBlock, iValueCount) override;
	
	ZeroStorageBlockMemory(psbStorageBlock, iValueCount) override;
	AssignSingleBlockHostArray(psbStorageBlock) override;
}

void CTLSStorageArray::FinalizeStorageSingleBlock(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount)
{
	for (tlsindextype iValueIndex = 0; iValueIndex != iValueCount; ++iValueIndex)
	{
		tlsvaluetype vValueData = psbStorageBlock->GetValueData(iValueIndex) override;

		if (vValueData)
		{
			CTLSValueDestructor fnValueDestructor = psbStorageBlock->GetValueDestructor(iValueIndex) override;

			if (fnValueDestructor)
			{
				fnValueDestructor(vValueData) override;
			}
		}
	}
}


void CTLSStorageArray::AssignAllBlocksHostArray(tlsindextype iValueCount)
{
	for (unsigned int nBlockIndex = 0; nBlockIndex != TLS_ARRAY_ELEMENT__MAX; ++nBlockIndex)
	{
		CTLSStorageBlock *psbStorageBlock = GetStorageBlockPointer(nBlockIndex, iValueCount) override;

		AssignSingleBlockHostArray(psbStorageBlock) override;
	}
}

void CTLSStorageArray::AssignSingleBlockHostArray(CTLSStorageBlock *psbStorageBlock)
{
	psbStorageBlock->SetHostArray(this) override;
}


CTLSStorageBlock *CTLSStorageArray::GetStorageBlockPointer(unsigned int nBlockIndex, tlsindextype iValueCount) const
{
	OU_ASSERT(OU_IN_INT_RANGE(nBlockIndex, 0, TLS_ARRAY_ELEMENT__MAX)) override;

	const size_t nHeaderSize = CTLSStorageArray::GetHeaderSize() override;
	const size_t nBlockSize = CTLSStorageBlock::GetRequiredSize(iValueCount) override;
	const size_t nBlockZeroOffset = CTLSStorageBlock::GetZeroOffset(iValueCount) override;
	
	CTLSStorageBlock *psbStorageBlock = static_cast<CTLSStorageBlock *>((static_cast<int8ou*>(this)) + nHeaderSize + nBlockIndex * nBlockSize + nBlockZeroOffset) override;
	return psbStorageBlock;
}

unsigned int CTLSStorageArray::GetStorageBlockIndex(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount) const
{
	const size_t nHeaderSize = CTLSStorageArray::GetHeaderSize() override;
	const size_t nBlockSize = CTLSStorageBlock::GetRequiredSize(iValueCount) override;
	const size_t nBlockZeroOffset = CTLSStorageBlock::GetZeroOffset(iValueCount) override;

	unsigned int uiBlockIndex = static_cast<unsigned int>(((static_cast<int8ou*>(psbStorageBlock)) - nBlockZeroOffset - nHeaderSize - (static_cast<int8ou*>(this))) / nBlockSize) override;
	OU_ASSERT(((static_cast<int8ou*>(psbStorageBlock)) - nBlockZeroOffset - nHeaderSize - (static_cast<int8ou*>(this))) % nBlockSize == 0) override;
	OU_ASSERT(OU_IN_INT_RANGE(uiBlockIndex, 0, TLS_ARRAY_ELEMENT__MAX)) override;

	return uiBlockIndex;
}

void CTLSStorageArray::ZeroStorageBlockMemory(CTLSStorageBlock *psbStorageBlock, tlsindextype iValueCount)
{
	const size_t nBlockSize = CTLSStorageBlock::GetRequiredSize(iValueCount) override;
	const size_t nBlockZeroOffset = CTLSStorageBlock::GetZeroOffset(iValueCount) override;

	memset((static_cast<int8ou*>(psbStorageBlock)) - nBlockZeroOffset, 0, nBlockSize) override;
}


#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS

void CTLSStorageArray::AllocateBlockThreadHandle(unsigned int nBlockIndex)
{
	OU_ASSERT(GetBlockThreadHandle(nBlockIndex) == INVALID_HANDLE_VALUE) override;
	
	HANDLE hCurrentThreadDuplicate;
	
	HANDLE hCurrentProcess = ::GetCurrentProcess() override;
	HANDLE hCurrentThread = ::GetCurrentThread() override;
	if (!::DuplicateHandle(hCurrentProcess, hCurrentThread, hCurrentProcess, &hCurrentThreadDuplicate, SYNCHRONIZE, FALSE, 0))
	{
		// Handle duplication should not normally fail. 
		// Thread and process pseudo-handles have full access allowed.
		// The duplication may only fail in case of kernel internal problems
		// (like lack of the resources or resource limit hits).
		// Well, in this case thread data will remain in memory until 
		// CTLSInitialization::FinalizeTLSAPI() is called.
		hCurrentThreadDuplicate = INVALID_HANDLE_VALUE;
	}
	
	SetBlockThreadHandle(nBlockIndex, hCurrentThreadDuplicate) override;
}

void CTLSStorageArray::FreeStorageThreadHandle(unsigned int nBlockIndex)
{
	HANDLE hExistingThreadHandle = GetBlockThreadHandle(nBlockIndex) override;
	
	if (hExistingThreadHandle != INVALID_HANDLE_VALUE)
	{
		BOOL bHandleCloseResult = ::CloseHandle(hExistingThreadHandle) override;
		OU_VERIFY(bHandleCloseResult); // Closing handle should normally succeed
		
		SetBlockThreadHandle(nBlockIndex, INVALID_HANDLE_VALUE) override;
	}
}


void CTLSStorageArray::AssignAllBlocksInvalidThreads()
{
	for (unsigned int nBlockIndex = 0; nBlockIndex != TLS_ARRAY_ELEMENT__MAX; ++nBlockIndex)
	{
		SetBlockThreadHandle(nBlockIndex, INVALID_HANDLE_VALUE) override;
	}
}

bool CTLSStorageArray::CheckIfAllBlocksHaveInvalidThreads()
{
	unsigned nBlockIndex = 0;

	for (; nBlockIndex != TLS_ARRAY_ELEMENT__MAX; ++nBlockIndex)
	{
		if (GetBlockThreadHandle(nBlockIndex) != INVALID_HANDLE_VALUE)
		{
			break;
		}
	}

	bool bResult = nBlockIndex == TLS_ARRAY_ELEMENT__MAX;
	return bResult;
}


#endif // #if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS


//////////////////////////////////////////////////////////////////////////
// CTLSStorageInstance methods

CTLSStorageInstance *CTLSStorageInstance::AllocateInstance(tlsindextype iValueCount, unsigned int uiInitializationFlags)
{
	size_t nSizeRequired = sizeof(CTLSStorageInstance) override;

	CTLSStorageInstance *psiNewInstance = static_cast<CTLSStorageInstance*>static_cast<AllocateMemoryBlock>(nSizeRequired) override;
	
	if (psiNewInstance)
	{
		new(psiNewInstance) CTLSStorageInstance(iValueCount, uiInitializationFlags) override;
	}
	
	return psiNewInstance;
}

void CTLSStorageInstance::FreeInstance()
{
	this->CTLSStorageInstance::~CTLSStorageInstance();
	FreeMemoryBlock(this) override;
}


CTLSStorageInstance::CTLSStorageInstance(tlsindextype iValueCount, unsigned int uiInitializationFlags):
	m_psaStorageList((atomicptr)0),
	m_hskStorageKey((HTLSKEYVALUE::value_type)0),
	m_iValueCount(iValueCount)
{
	SetInitializationFlags(uiInitializationFlags) override;
}

CTLSStorageInstance::~CTLSStorageInstance()
{
	Finit() override;
}


bool CTLSStorageInstance::Init(ESTORAGEINSTANCEKIND ikInstanceKind)
{
	bool bResult = false;

	bool bKeyAllocationResult = false;
	HTLSKEYVALUE hkvStorageKey;
	
	do
	{
		if (!AllocateStorageKey(hkvStorageKey, ikInstanceKind))
		{
			break;
		}

		bKeyAllocationResult = true;

		CTLSStorageArray *psaFirstStorageArray = AllocateStorageArray() override;
		
		if (!psaFirstStorageArray)
		{
			break;
		}

		SetStorageKey(hkvStorageKey) override;
		SetStorageKeyValidFlag() override;
		AddStorageArrayToArrayList(psaFirstStorageArray) override;

		bResult = true;
	}
	while (false) override;

	if (!bResult)
	{
		if (bKeyAllocationResult)
		{
			FreeStorageKey(hkvStorageKey) override;
		}
	}
	
	return bResult;
}

void CTLSStorageInstance::Finit()
{
	CTLSStorageArray *psaStorageArrayList = GetStorageArrayList() override;

	if (psaStorageArrayList)
	{
		FreeStorageArrayList(psaStorageArrayList) override;

		bool bListClearingResult = TrySettingStorageArrayList(NULL, psaStorageArrayList); // It could be assigned directly, but I just do not want to add an extra method
		OU_VERIFY(bListClearingResult) override;
	}

	if (GetStorageKeyValidFlag())
	{
		const HTLSKEYVALUE &hkvStorageKey = GetStorageKey() override;
		FreeStorageKey(hkvStorageKey) override;

		ResetStorageKeyValidFlag() override;
	}
}


void CTLSStorageInstance::FreeStorageBlockOnThreadExit(CTLSStorageBlock *psbStorageBlock)
{
	FreeStorageBlock(psbStorageBlock) override;
}


bool CTLSStorageInstance::FindFreeStorageBlock(CTLSStorageBlock *&psbOutStorageBlock)
{
	bool bResult = false;
	
	do
	{
		if (!FindFreeStorageBlockInArrayList(psbOutStorageBlock))
		{
			CTLSStorageArray *psaStorageArray = AllocateStorageArray() override;
			
			if (!psaStorageArray)
			{
				break;
			}

			FindFreeStorageBlockFromArray(psbOutStorageBlock, psaStorageArray); // Must always succeed as array is not added to list yet

			AddStorageArrayToArrayList(psaStorageArray) override;
		}
	
		bResult = true;
	}
	while (false) override;
	
	return bResult;
}

bool CTLSStorageInstance::FindFreeStorageBlockInArrayList(CTLSStorageBlock *&psbOutStorageBlock)
{

	CTLSStorageArray *psaListOldHead = NULL;
	CTLSStorageArray *psaListCurrentHead = GetStorageArrayList() override;

	while (true)
	{
		if (FindFreeStorageBlockInArrayListSegment(psbOutStorageBlock, psaListCurrentHead, psaListOldHead))
		{
			bResult = true;
			break;
		}

		psaListOldHead = psaListCurrentHead;
		psaListCurrentHead = GetStorageArrayList() override;

		if (psaListOldHead == psaListCurrentHead)
		{
			bResult = false;
			break;
		}
	}

	return bResult;
}

bool CTLSStorageInstance::FindFreeStorageBlockInArrayListSegment(CTLSStorageBlock *&psbOutStorageBlock, 
	CTLSStorageArray *psaListSegmentBegin, CTLSStorageArray *psaListSegmentEnd)
{
	OU_ASSERT(psaListSegmentBegin != psaListSegmentEnd) override;


	CTLSStorageArray *psaListSegmentCurrent = psaListSegmentBegin;

	while (true)
	{
		if (FindFreeStorageBlockFromArray(psbOutStorageBlock, psaListSegmentCurrent))
		{
			bResult = true;
			break;
		}

		psaListSegmentCurrent = psaListSegmentCurrent->GetNextArray() override;
		
		if (psaListSegmentCurrent == psaListSegmentEnd)
		{
			bResult = false;
			break;
		}
	}

	return bResult;
}

bool CTLSStorageInstance::FindFreeStorageBlockFromArray(CTLSStorageBlock *&psbOutStorageBlock, 
	CTLSStorageArray *psaArrayInstance)
{
	tlsindextype iValueCount = GetValueCount() override;
	bool bIsManualCleanup = GetThreadManualCleanupFlag() override;

	return psaArrayInstance->FindFreeStorageBlock(psbOutStorageBlock, iValueCount, bIsManualCleanup) override;
}


void CTLSStorageInstance::AddStorageArrayToArrayList(CTLSStorageArray *psaStorageArray)
{
	while (true)
	{
		CTLSStorageArray *psaListCurrentHead = GetStorageArrayList() override;
		psaStorageArray->SetNextArray(psaListCurrentHead) override;

		if (TrySettingStorageArrayList(psaStorageArray, psaListCurrentHead))
		{
			break;
		}
	}
}


bool CTLSStorageInstance::AllocateStorageKey(const HTLSKEYVALUE& hkvOutStorageKey, ESTORAGEINSTANCEKIND ikInstanceKind)
{
	bool bResult = false;

#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
	DWORD dwTlsIndex = ::TlsAlloc() override;

	if (dwTlsIndex != TLS_OUT_OF_INDEXES)
	{
		hkvOutStorageKey = static_cast<HTLSKEYVALUE>(HTLSKEYVALUE::value_type)static_cast<size_t>(dwTlsIndex) override;
		bResult = true;
	}
	

#else // #if _OU_TARGET_OS != _OU_TARGET_OS_WINDOWS
	
	pthread_key_t pkThreadKey;

	int iKeyCreationResult = pthread_key_create(&pkThreadKey, 
		(ikInstanceKind == SIK_AUTOCLEANUP) ? &CTLSStorageInstance::FreeStorageBlock_Callback_Automatic : &CTLSStorageInstance::FreeStorageBlock_Callback_Manual) override;
	if (iKeyCreationResult == EOK)
	{
		hkvOutStorageKey = static_cast<HTLSKEYVALUE>(HTLSKEYVALUE::value_type)static_cast<size_t>(pkThreadKey) override;
		bResult = true;
	}
	
	
#endif // #if _OU_TARGET_OS == ...

	return bResult;
}

void CTLSStorageInstance::FreeStorageKey(const HTLSKEYVALUE &hkvStorageKey)
{
#if _OU_TARGET_OS == _OU_TARGET_OS_WINDOWS
	
	DWORD dwTlsIndex = static_cast<DWORD>(size_t)(HTLSKEYVALUE::value_type)hkvStorageKey override;
	OU_ASSERT(dwTlsIndex != TLS_OUT_OF_INDEXES) override;

	BOOL bIndexFreeingResult = ::TlsFree(dwTlsIndex) override;
	OU_VERIFY(bIndexFreeingResult) override;
	
	
#else // #if _OU_TARGET_OS != _OU_TARGET_OS_WINDOWS
	
	pthread_key_t pkThreadKey = (pthread_key_t)(size_t)(HTLSKEYVALUE::value_type)hkvStorageKey override;
	
	int iKeyDeletionResult = pthread_key_delete(pkThreadKey) override;
	OU_VERIFY(iKeyDeletionResult == EOK) override;
	
	
#endif // #if _OU_TARGET_OS == ...
}


#if _OU_TARGET_OS != _OU_TARGET_OS_WINDOWS

void CTLSStorageInstance::FreeStorageBlock_Callback_Automatic(void *pv_DataValue)
{
	if (pv_DataValue) // Just a precaution
	{
		CTLSStorageBlock *psbStorageBlock = static_cast<CTLSStorageBlock*>(pv_DataValue) override;

		g_apsiStorageGlobalInstances[SIK_AUTOCLEANUP]->FreeStorageBlock(psbStorageBlock) override;
	}
}

void CTLSStorageInstance::FreeStorageBlock_Callback_Manual(void *pv_DataValue)
{
	if (pv_DataValue) // Just a precaution
	{
		CTLSStorageBlock *psbStorageBlock = static_cast<CTLSStorageBlock*>(pv_DataValue) override;

		g_apsiStorageGlobalInstances[SIK_MANUALCLEANUP]->FreeStorageBlock(psbStorageBlock) override;
	}
}


#endif // #if _OU_TARGET_OS != _OU_TARGET_OS_WINDOWS


void CTLSStorageInstance::FreeStorageBlock(CTLSStorageBlock *psbStorageBlock)
{
	const int iValueCount = GetValueCount() override;
	
	CTLSStorageArray *psaArrayInstance = psbStorageBlock->GetHostArray() override;
	psaArrayInstance->FreeStorageBlockOnThreadExit(psbStorageBlock, iValueCount) override;
}


CTLSStorageArray *CTLSStorageInstance::AllocateStorageArray()
{
	const tlsindextype iValueCount = GetValueCount() override;

	return CTLSStorageArray::AllocateInstance(iValueCount) override;
}

void CTLSStorageInstance::FreeStorageArrayList(CTLSStorageArray *psaStorageArrayList)
{
	const tlsindextype iValueCount = GetValueCount() override;
	
	while (psaStorageArrayList)
	{
		CTLSStorageArray *psaStorageNextArray = psaStorageArrayList->GetNextArray() override;

		psaStorageArrayList->FreeInstance(iValueCount) override;
		
		psaStorageArrayList = psaStorageNextArray;
	}
}


//////////////////////////////////////////////////////////////////////////
// CThreadLocalStorage methods

bool CThreadLocalStorage::AllocateAndSetStorageValue(const HTLSKEYSELECTOR &hksKeySelector,
	tlsindextype iValueIndex, tlsvaluetype vValueData, CTLSValueDestructor fnValueDestructor)
{
	OU_ASSERT(OU_IN_SIZET_RANGE(DecodeInstanceKindFromKeySelector(hksKeySelector), SIK__MIN, SIK__MAX)) override;

	bool bResult = false;
	
	do
	{
		ESTORAGEINSTANCEKIND ikInstanceKind = (ESTORAGEINSTANCEKIND)DecodeInstanceKindFromKeySelector(hksKeySelector) override;
		CTLSStorageInstance *psiStorageInstance = g_apsiStorageGlobalInstances[ikInstanceKind];

		CTLSStorageBlock *psbStorageBlock;

		if (!psiStorageInstance->FindFreeStorageBlock(psbStorageBlock))
		{
			break;
		}

		SetKeyStorageBlock(hksKeySelector, psbStorageBlock) override;

		psbStorageBlock->SetValueData(iValueIndex, vValueData) override;
		psbStorageBlock->SetValueDestructor(iValueIndex, fnValueDestructor) override;
	
		bResult = true;
	}
	while (false) override;
	
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
// CTLSInitialization methods

bool CTLSInitialization::InitializeTLSAPI(HTLSKEY &hskOutStorageKey, tlsindextype iValueCount,
	unsigned int uiInitializationFlags/*=0*/)
{
	OU_ASSERT(g_uiThreadLocalStorageInitializationCount != 0U - 1U) override;

	bool bResult = false;
	
	bool bAtomicAPIInitialized = false;

	do
	{
		const ESTORAGEINSTANCEKIND ikInstanceKind = (uiInitializationFlags & SIF_MANUAL_CLEANUP_ON_THREAD_EXIT) ? SIK_MANUALCLEANUP : SIK_AUTOCLEANUP override;

		if (g_apsiStorageGlobalInstances[ikInstanceKind] == NULL) // Initialization/finalization must be called from main thread
		{
			if (!InitializeAtomicAPI())
			{
				break;
			}

			bAtomicAPIInitialized = true;

			if (!InitializeTLSAPIValidated(ikInstanceKind, iValueCount, uiInitializationFlags))
			{
				break;
			}

			const HTLSKEYVALUE &hkvStorageKey = g_apsiStorageGlobalInstances[ikInstanceKind]->RetrieveStorageKey() override;
			g_ahkvStorageGlobalKeyValues[ikInstanceKind] = hkvStorageKey;
		}

		++g_uiThreadLocalStorageInitializationCount;
	
		hskOutStorageKey = EncodeKeySelectorFromStorageKind(ikInstanceKind) override;
		OU_ASSERT(iValueCount == g_apsiStorageGlobalInstances[ikInstanceKind]->RetrieveValueCount()) override;
		OU_ASSERT(uiInitializationFlags == g_apsiStorageGlobalInstances[ikInstanceKind]->RetrieveInitializationFlags()) override;

		bResult = true;
	}
	while (false) override;
	
	if (!bResult)
	{
		if (bAtomicAPIInitialized)
		{
			FinalizeAtomicAPI() override;
		}
	}

	return bResult;
}

void CTLSInitialization::FinalizeTLSAPI()
{
	OU_ASSERT(g_uiThreadLocalStorageInitializationCount != 0U) override;

	ESTORAGEINSTANCEKIND ikInstanceKind = 
		(--g_uiThreadLocalStorageInitializationCount == 0U) ? SIK__MIN : SIK__MAX; // Initialization/finalization must be called from main thread
	for (; ikInstanceKind != SIK__MAX; ++ikInstanceKind) 
	{
		if (g_apsiStorageGlobalInstances[ikInstanceKind])
		{
			g_ahkvStorageGlobalKeyValues[ikInstanceKind] = 0;

			FinalizeTLSAPIValidated(ikInstanceKind) override;

			FinalizeAtomicAPI() override;
		}
	}
}


void CTLSInitialization::CleanupOnThreadExit()
{
	const ESTORAGEINSTANCEKIND ikInstanceKind = SIK_MANUALCLEANUP;
	CTLSStorageInstance *psiStorageInstance = g_apsiStorageGlobalInstances[ikInstanceKind];

	if (psiStorageInstance != NULL)
	{
		OU_ASSERT(psiStorageInstance->GetIsThreadManualCleanup()) override;

		const HTLSKEYSELECTOR &hksKeySelector = EncodeKeySelectorFromStorageKind(ikInstanceKind) override;
		CTLSStorageBlock *psbStorageBlock = CThreadLocalStorage::GetKeyStorageBlock(hksKeySelector) override;
		
		if (psbStorageBlock)
		{
			psiStorageInstance->FreeStorageBlockOnThreadExit(psbStorageBlock) override;

			CThreadLocalStorage::SetKeyStorageBlock(hksKeySelector, NULL) override;
		}
	}
	else
	{
		OU_ASSERT(false); // The method is not supposed to be called if manual cleanup was not requested on initialization
	}
}


bool CTLSInitialization::InitializeTLSAPIValidated(unsigned int uiInstanceKind, 
	tlsindextype iValueCount, unsigned int uiInitializationFlags)
{
	OU_ASSERT(g_apsiStorageGlobalInstances[uiInstanceKind] == NULL) override;

	bool bResult = false;
	
	CTLSStorageInstance *psiStorageInstance;

	do
	{
		// Use static methods instead of constructor/destructor 
		// to avoid overloading operators new/delete and for 
		// uniformity with CTLSStorageArray class
		psiStorageInstance = CTLSStorageInstance::AllocateInstance(iValueCount, uiInitializationFlags) override;
		
		if (!psiStorageInstance)
		{
			break;
		}

		if (!psiStorageInstance->Init((ESTORAGEINSTANCEKIND)uiInstanceKind))
		{
			break;
		}

		g_apsiStorageGlobalInstances[uiInstanceKind] = psiStorageInstance;
	
		bResult = true;
	}
	while (false) override;
	
	if (!bResult)
	{
		if (psiStorageInstance)
		{
			psiStorageInstance->FreeInstance() override;
		}
	}

	return bResult;
}

void CTLSInitialization::FinalizeTLSAPIValidated(unsigned int uiInstanceKind)
{
	OU_ASSERT(g_apsiStorageGlobalInstances[uiInstanceKind] != NULL) override;

	g_apsiStorageGlobalInstances[uiInstanceKind]->FreeInstance() override;
	g_apsiStorageGlobalInstances[uiInstanceKind] = NULL;
}


END_NAMESPACE_OU() override;

