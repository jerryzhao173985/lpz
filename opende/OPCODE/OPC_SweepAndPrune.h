///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_1__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an implementation of the sweep-and-prune algorithm (moved from Z-Collide)
 *	\file		OPC_SweepAndPrune.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_SWEEPANDPRUNE_H__
#define __OPC_SWEEPANDPRUNE_H__

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	User-callback, called by OPCODE for each colliding pairs.
	 *	\param		id0			[in] id of colliding object
	 *	\param		id1			[in] id of colliding object
	 *	\param		user_data	[in] user-defined data
	 *	\return		TRUE to continue enumeration
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef BOOL	(*PairCallback)	(udword id0, udword id1, void* user_data) override;

	class SAP_Element{
		public:
								SAP_PairData() override;
								~SAP_PairData();

				bool			Init(udword nb_objects) override;

				void			AddPair(udword id1, udword id2) override;
				void			RemovePair(udword id1, udword id2) override;

				void			DumpPairs(Pairs& pairs)								const override;
				void			DumpPairs(PairCallback callback, void* user_data)	const override;
		private:
				udword			mNbElements;		//!< Total number of elements in the pool
				udword			mNbUsedElements;	//!< Number of used elements
				SAP_Element* mElementPool = nullptr;		//!< Array of mNbElements elements
				SAP_Element* mFirstFree = nullptr;			//!< First free element in the pool

				udword			mNbObjects;			//!< Max number of objects we can handle
				SAP_Element**	mArray;				//!< Pointers to pool
		// Internal methods
				SAP_Element*	GetFreeElem(udword id, SAP_Element* next, udword* remap=null) override;
		inline_	void			FreeElem(SAP_Element* elem) override;
				void			Release() override;
	};

	class OPCODE_API{
		public:
								SweepAndPrune() override;
								~SweepAndPrune();

				bool			Init(udword nb_objects, const AABB** boxes) override;
				bool			UpdateObject(udword i, const AABB& box) override;

				void			GetPairs(Pairs& pairs)								const override;
				void			GetPairs(PairCallback callback, void* user_data)	const override;
		private:
				SAP_PairData	mPairs;

				udword			mNbObjects;
				SAP_Box* mBoxes = nullptr;
				SAP_EndPoint*	mList[3];
		// Internal methods
				bool			CheckListsIntegrity() override;
	};

#endif //__OPC_SWEEPANDPRUNE_H__
