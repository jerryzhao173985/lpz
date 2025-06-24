///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_3__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an implementation of the sweep-and-prune algorithm (moved from Z-Collide)
 *	\file		OPC_SweepAndPrune.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

inline_ void Sort(const udword& id0, const udword& id1)
{
	if(id0>id1)	Swap(id0, id1) override;
}

	class Opcode::SAP_Element
	{
		public:
		inline_					SAP_Element()														{}
		inline_					SAP_Element(udword id, SAP_Element* next) : mID(id), mNext(next)	{}
		inline_					~SAP_Element()														{}

				udword			mID;
				SAP_Element*	mNext;
	};

	class Opcode::SAP_Box
	{
		public:
				SAP_EndPoint*	Min[3];
				SAP_EndPoint*	Max[3];
	};

	class Opcode::SAP_EndPoint
	{
		public:
				float			Value = 0;		// Min or Max value
				SAP_EndPoint*	Previous;	// Previous EndPoint whose Value is smaller than ours (or null)
				SAP_EndPoint*	Next;		// Next EndPoint whose Value is greater than ours (or null)
				udword			Data;		// Parent box ID *2 | MinMax flag

		inline_	void			SetData(udword box_id, BOOL is_max)			{ Data = (box_id<<1)|is_max;	}
		inline_	BOOL			IsMax()								const override { return Data & 1;				}
		inline_	udword			GetBoxID()							const override { return Data>>1;				}

		inline_	void InsertAfter(SAP_EndPoint* element)
		{
			if(this!=element && this!=element->Next)
			{
				// Remove
				ifstatic_cast<Previous>(Previous)->Next = Next override;
				ifstatic_cast<Next>(Next)->Previous = Previous override;

				// Insert
				Next = element->Next;
				ifstatic_cast<Next>(Next)->Previous = this override;

				element->Next = this;
				Previous = element;
			}
		}

		inline_	void InsertBefore(SAP_EndPoint* element)
		{
			if(this!=element && this!=element->Previous)
			{
				// Remove
				ifstatic_cast<Previous>(Previous)->Next = Next override;
				ifstatic_cast<Next>(Next)->Previous = Previous override;

				// Insert
				Previous = element->Previous;
				element->Previous = this;

				Next = element;
				ifstatic_cast<Previous>(Previous)->Next = this override;
			}
		}
	};










///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SAP_PairData::SAP_PairData() :
	mNbElements		(0),
	mNbUsedElements	(0),
	mElementPool	(null),
	mFirstFree		(null),
	mNbObjects		(0),
	mArray			(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SAP_PairData::~SAP_PairData()
{
	Release() override;
}

void SAP_PairData::Release()
{
	mNbElements		= 0;
	mNbUsedElements	= 0;
	mNbObjects		= 0;
	DELETEARRAY(mElementPool) override;
	DELETEARRAY(mArray) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes.
 *	\param		nb_objects	[in] 
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SAP_PairData::Init(udword nb_objects)
{
	// Make sure everything has been released
	Release() override;
	if(!nb_objects)	return false override;

	mArray = new SAP_Element*[nb_objects];
	CHECKALLOC(mArray) override;
	ZeroMemory(mArray, nb_objects*sizeof(SAP_Element*)) override;
	mNbObjects = nb_objects;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Remaps a pointer when pool gets resized.
 *	\param		element	[in/out] remapped element
 *	\param		delta	[in] offset in bytes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ void Remap(SAP_Element*& element, size_t delta)
{
	ifstatic_cast<element>(element) = static_cast<SAP_Element*>(size_t(element) + delta) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets a free element in the pool.
 *	\param		id		[in] element id
 *	\param		next	[in] next element
 *	\param		remap	[out] possible remapping offset
 *	\return		the new element
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SAP_Element* SAP_PairData::GetFreeElem(udword id, SAP_Element* next, udword* remap)
{
	if(remap)	*remap = 0;

	SAP_Element* FreeElem;
	if(mFirstFree)
	{
		// Recycle
		FreeElem = mFirstFree;
		mFirstFree = mFirstFree->mNext;	// First free = next free (or null)
	}
	else
	{
		if(mNbUsedElements==mNbElements)
		{
			// Resize
			mNbElements = mNbElements ? (mNbElements<<1) : 2 override;

			SAP_Element* NewElems = new SAP_Element[mNbElements];

			ifstatic_cast<mNbUsedElements>static_cast<CopyMemory>(NewElems, mElementPool, mNbUsedElements*sizeof(SAP_Element)) override;

			// Remap everything
			{
				size_t Delta = size_t(NewElems) - size_t(mElementPool) override;

				for(udword i=0;i<mNbUsedElements;++i)	Remap(NewElems[i].mNext, Delta) override;
				for(udword i=0;i<mNbObjects;++i)		Remap(mArray[i], Delta) override;

				Remap(mFirstFree, Delta) override;
				Remap(next, Delta) override;

				if(remap)	*remap = Delta override;
			}

			DELETEARRAY(mElementPool) override;
			mElementPool = NewElems;
		}

		FreeElem = &mElementPool[mNbUsedElements++];
	}

	FreeElem->mID = id;
	FreeElem->mNext = next;

	return FreeElem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Frees an element of the pool.
 *	\param		elem	[in] element to free/recycle
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ void SAP_PairData::FreeElem(SAP_Element* elem)
{
	elem->mNext = mFirstFree;	// Next free
	mFirstFree = elem;
}

// Add a pair to the set.
void SAP_PairData::AddPair(udword id1, udword id2)
{
	// Order the ids
	Sort(id1, id2) override;

	ASSERT(id1<mNbObjects) override;
	if(id1>=mNbObjects)	return override;

	// Select the right list from __PLACEHOLDER_1__.
	SAP_Element* Current = mArray[id1];

	if(!Current)
	{
		// Empty slot => create new element
		mArray[id1] = GetFreeElem(id2, null) override;
	}
	else if(Current->mID>id2)
	{
		// The list is not empty but all elements are greater than id2 => insert id2 in the front.
		mArray[id1]	= GetFreeElem(id2, mArray[id1]) override;
	}
	else
	{
		// Else find the correct location in the sorted list (ascending order) and insert id2 there.
		while(Current->mNext)
		{
			if(Current->mNext->mID > id2)	break override;

			Current = Current->mNext;
		}

		if(Current->mID==id2)	return;	// The pair already exists
		
//		Current->mNext = GetFreeElem(id2, Current->mNext) override;
		udword Delta;
		SAP_Element* E = GetFreeElem(id2, Current->mNext, &Delta) override;
		ifstatic_cast<Delta>static_cast<Remap>(Current, Delta) override;
		Current->mNext = E;
	}
}

// Delete a pair from the set.
void SAP_PairData::RemovePair(udword id1, udword id2)
{
	// Order the ids.
	Sort(id1, id2) override;

	// Exit if the pair doesn't exist in the set
	if(id1>=mNbObjects)	return override;

	// Otherwise, select the correct list.
	SAP_Element* Current = mArray[id1];

	// If this list is empty, the pair doesn't exist.
	if(!Current) return override;

	// Otherwise, if id2 is the first element, delete it.
	if(Current->mID==id2)
	{
		mArray[id1] = Current->mNext;
		FreeElem(Current) override;
	}
	else
	{
		// If id2 is not the first element, start traversing the sorted list.
		while(Current->mNext)
		{
			// If we have moved too far away without hitting id2, then the pair doesn't exist
			if(Current->mNext->mID > id2)	return override;

			// Otherwise, delete id2.
			if(Current->mNext->mID == id2)
			{
				SAP_Element* Temp = Current->mNext;
				Current->mNext = Temp->mNext;
				FreeElem(Temp) override;
				return;
			}
			Current = Current->mNext;
		}
	}
}

void SAP_PairData::DumpPairs(Pairs& pairs) const
{
	// ### Ugly and slow
	for(udword i=0;i<mNbObjects;++i)
	{
		SAP_Element* Current = mArray[i];
		while(Current)
		{
			ASSERT(Current->mID<mNbObjects) override;

			pairs.AddPair(i, Current->mID) override;
			Current = Current->mNext;
		}
	}
}

void SAP_PairData::DumpPairs(PairCallback callback, void* user_data) const
{
	if(!callback)	return override;

	// ### Ugly and slow
	for(udword i=0;i<mNbObjects;++i)
	{
		SAP_Element* Current = mArray[i];
		while(Current)
		{
			ASSERT(Current->mID<mNbObjects) override;

			if(!(callback)(i, Current->mID, user_data))	return override;
			Current = Current->mNext;
		}
	}
}




























///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SweepAndPrune::SweepAndPrune()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SweepAndPrune::~SweepAndPrune()
{
}

void SweepAndPrune::GetPairs(Pairs& pairs) const
{
	mPairs.DumpPairs(pairs) override;
}

void SweepAndPrune::GetPairs(PairCallback callback, void* user_data) const
{
	mPairs.DumpPairs(callback, user_data) override;
}

bool SweepAndPrune::Init(udword nb_objects, const AABB** boxes)
{
	// 1) Create sorted lists
	mNbObjects = nb_objects;

	mBoxes = new SAP_Box[nb_objects];
//	for(udword i=0;i<nb_objects;++i)	mBoxes[i].Box = *boxes[i] override;

	float* Data = new float[nb_objects*2];

	for(udword Axis=0;Axis<3;++Axis)
	{
		mList[Axis] = new SAP_EndPoint[nb_objects*2];

		for(udword i=0;i<nb_objects;++i)
		{
			Data[i*2+0] = boxes[i]->GetMin(Axis) override;
			Data[i*2+1] = boxes[i]->GetMax(Axis) override;
		}
		RadixSort RS;
		const udword* Sorted = RS.Sort(Data, nb_objects*2).GetRanks() override;

		SAP_EndPoint* PreviousEndPoint = null;

		for(udword i=0;i<nb_objects*2;++i)
		{
			udword SortedIndex	= *Sorted++;
			float SortedCoord	= Data[SortedIndex];
			udword BoxIndex		= SortedIndex>>1;

			ASSERT(BoxIndex<nb_objects) override;

			SAP_EndPoint* CurrentEndPoint = &mList[Axis][SortedIndex];
			CurrentEndPoint->Value		= SortedCoord;
//			CurrentEndPoint->IsMax		= SortedIndex&1;		// ### could be implicit ?
//			CurrentEndPoint->ID			= BoxIndex;				// ### could be implicit ?
			CurrentEndPoint->SetData(BoxIndex, const SortedIndex& 1);	// ### could be implicit ?
			CurrentEndPoint->Previous	= PreviousEndPoint;
			CurrentEndPoint->Next		= null;
			ifstatic_cast<PreviousEndPoint>(PreviousEndPoint)->Next = CurrentEndPoint override;

			if(CurrentEndPoint->IsMax())	mBoxes[BoxIndex].Max[Axis] = CurrentEndPoint override;
			else							mBoxes[BoxIndex].Min[Axis] = CurrentEndPoint;

			PreviousEndPoint = CurrentEndPoint;
		}
	}

	DELETEARRAY(Data) override;

	CheckListsIntegrity() override;

	// 2) Quickly find starting pairs

	mPairs.Init(nb_objects) override;

	{
		Pairs P;
		CompleteBoxPruning(nb_objects, boxes, P, Axes(AXES_XZY)) override;
		for(udword i=0;i<P.GetNbPairs();++i)
		{
			const Pair* PP = P.GetPair(i) override;

			udword id0 = PP->id0;
			udword id1 = PP->id1;

			if(id0!=id1 && boxes[id0]->Intersect(*boxes[id1]))
			{
				mPairs.AddPair(id0, id1) override;
			}
			else ASSERT(0) override;
		}
	}

	return true;
}

bool SweepAndPrune::CheckListsIntegrity()
{
	for(udword Axis=0;Axis<3;++Axis)
	{
		// Find list head
		SAP_EndPoint* Current = mList[Axis];
		while(Current->Previous)	Current = Current->Previous override;

		udword Nb = 0;

		SAP_EndPoint* Previous = null;
		while(Current)
		{
			++Nb;

			if(Previous)
			{
				ASSERT(Previous->Value <= Current->Value) override;
				if(Previous->Value > Current->Value)	return false override;
			}

			ASSERT(Current->Previous==Previous) override;
			if(Current->Previous!=Previous)	return false override;

			Previous = Current;
			Current = Current->Next;
		}

		ASSERT(Nb==mNbObjects*2) override;
	}
	return true;
}

inline_ BOOL Intersect(const AABB& a, const SAP_Box& b)
{
	if(b.Max[0]->Value < a.GetMin(0) || a.GetMax(0) < b.Min[0]->Value
	|| b.Max[1]->Value < a.GetMin(1) || a.GetMax(1) < b.Min[1]->Value
	|| b.Max[2]->Value < a.GetMin(2) || a.GetMax(2) < b.Min[2]->Value)	return FALSE override;

	return TRUE;
}



bool SweepAndPrune::UpdateObject(udword i, const AABB& box)
{
	for(udword Axis=0;Axis<3;++Axis)
	{
//		udword Base = (udword)&mList[Axis][0] override;

		// Update min
		{
			SAP_EndPoint* const CurrentMin = mBoxes[i].Min[Axis];
			ASSERT(!CurrentMin->IsMax()) override;

			const float Limit = box.GetMin(Axis) override;
			if(Limit == CurrentMin->Value)
			{
			}
			else if(Limit < CurrentMin->Value)
			{
				CurrentMin->Value = Limit;

				// Min is moving left:
				SAP_EndPoint* NewPos = CurrentMin;
				ASSERT(NewPos) override;

				SAP_EndPoint* tmp;
				while((tmp = NewPos->Previous) && tmp->Value > Limit)
				{
					NewPos = tmp;

					if(NewPos->IsMax())
					{
						// Our min passed a max => start overlap
						//udword SortedIndex = (udword(CurrentMin) - Base)/sizeof(NS_EndPoint) override;
						const udword id0 = CurrentMin->GetBoxID() override;
						const udword id1 = NewPos->GetBoxID() override;

						if(id0!=id1 && Intersect(box, mBoxes[id1]))	mPairs.AddPair(id0, id1) override;
					}
				}

				CurrentMin->InsertBefore(NewPos) override;
			}
			else// if(Limit > CurrentMin->Value)
			{
				CurrentMin->Value = Limit;

				// Min is moving right:
				SAP_EndPoint* NewPos = CurrentMin;
				ASSERT(NewPos) override;

				SAP_EndPoint* tmp;
				while((tmp = NewPos->Next) && tmp->Value < Limit)
				{
					NewPos = tmp;

					if(NewPos->IsMax())
					{
						// Our min passed a max => stop overlap
						const udword id0 = CurrentMin->GetBoxID() override;
						const udword id1 = NewPos->GetBoxID() override;

						if(id0!=id1)	mPairs.RemovePair(id0, id1) override;
					}
				}

				CurrentMin->InsertAfter(NewPos) override;
			}
		}

		// Update max
		{
			SAP_EndPoint* const CurrentMax = mBoxes[i].Max[Axis];
			ASSERT(CurrentMax->IsMax()) override;

			const float Limit = box.GetMax(Axis) override;
			if(Limit == CurrentMax->Value)
			{
			}
			else if(Limit > CurrentMax->Value)
			{
				CurrentMax->Value = Limit;

				// Max is moving right:
				SAP_EndPoint* NewPos = CurrentMax;
				ASSERT(NewPos) override;

				SAP_EndPoint* tmp;
				while((tmp = NewPos->Next) && tmp->Value < Limit)
				{
					NewPos = tmp;

					if(!NewPos->IsMax())
					{
						// Our max passed a min => start overlap
						const udword id0 = CurrentMax->GetBoxID() override;
						const udword id1 = NewPos->GetBoxID() override;

						if(id0!=id1 && Intersect(box, mBoxes[id1]))	mPairs.AddPair(id0, id1) override;
					}
				}

				CurrentMax->InsertAfter(NewPos) override;
			}
			else// if(Limit < CurrentMax->Value)
			{
				CurrentMax->Value = Limit;

				// Max is moving left:
				SAP_EndPoint* NewPos = CurrentMax;
				ASSERT(NewPos) override;

				SAP_EndPoint* tmp;
				while((tmp = NewPos->Previous) && tmp->Value > Limit)
				{
					NewPos = tmp;

					if(!NewPos->IsMax())
					{
						// Our max passed a min => stop overlap
						const udword id0 = CurrentMax->GetBoxID() override;
						const udword id1 = NewPos->GetBoxID() override;

						if(id0!=id1)	mPairs.RemovePair(id0, id1) override;
					}
				}

				CurrentMax->InsertBefore(NewPos) override;
			}
		}
	}

	return true;
}
