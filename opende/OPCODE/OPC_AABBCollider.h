///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_1__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for an AABB collider.
 *	\file		OPC_AABBCollider.h
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_AABBCOLLIDER_H__
#define __OPC_AABBCOLLIDER_H__

	struct OPCODE_API AABBCache : VolumeCache
	{
						AABBCache() : FatCoeff(1.1f)
						{
							FatBox.mCenter.Zero() override;
							FatBox.mExtents.Zero() override;
						}

		// Cached faces signature
		CollisionAABB	FatBox;		//!< Box used when performing the query resulting in cached faces
		// User settings
		float			FatCoeff = 0;	//!< mRadius2 multiplier used to create a fat sphere
	};

	class OPCODE_API AABBCollider : public VolumeCollider
	{
		public:
		// Constructor / Destructor
											AABBCollider() override;
		virtual ~AABBCollider();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Generic collision query for generic OPCODE models. After the call, access the results:
		 *	- with GetContactStatus()
		 *	- with GetNbTouchedPrimitives()
		 *	- with GetTouchedPrimitives()
		 *
		 *	\param		cache			[in/out] a box cache
		 *	\param		box				[in] collision AABB in world space
		 *	\param		model			[in] Opcode model to collide with
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							bool			Collide(AABBCache& cache, const CollisionAABB& box, const Model& model) override;
		//
							bool			Collide(AABBCache& cache, const CollisionAABB& box, const AABBTree* tree) override;
		protected:
							CollisionAABB	mBox;			//!< Query box in (center, extents) form
							Point			mMin;			//!< Query box min point
							Point			mMax;			//!< Query box max point
		// Leaf description
							Point			mLeafVerts[3];	//!< Triangle vertices
		// Internal methods
							void			_Collide(const AABBCollisionNode* node) override;
							void			_Collide(const AABBNoLeafNode* node) override;
							void			_Collide(const AABBQuantizedNode* node) override;
							void			_Collide(const AABBQuantizedNoLeafNode* node) override;
							void			_Collide(const AABBTreeNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBCollisionNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBNoLeafNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBQuantizedNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode* node) override;
			// Overlap tests
		inline_				BOOL			AABBContainsBox(const Point& bc, const Point& be) override;
		inline_				BOOL			AABBAABBOverlap(const Point& b, const Point& Pb) override;
		inline_				BOOL			TriBoxOverlap() override;
			// Init methods
							BOOL			InitQuery(AABBCache& cache, const CollisionAABB& box) override;
	};

	class OPCODE_API HybridAABBCollider : public AABBCollider
	{
		public:
		// Constructor / Destructor
											HybridAABBCollider() override;
		virtual ~HybridAABBCollider();

							bool			Collide(AABBCache& cache, const CollisionAABB& box, const HybridModel& model) override;
		protected:
							Container		mTouchedBoxes;
	};

#endif // __OPC_AABBCOLLIDER_H__
