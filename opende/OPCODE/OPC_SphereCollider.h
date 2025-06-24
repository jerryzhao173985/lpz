///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_1__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a sphere collider.
 *	\file		OPC_SphereCollider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_SPHERECOLLIDER_H__
#define __OPC_SPHERECOLLIDER_H__

	struct OPCODE_API SphereCache : VolumeCache
	{
					SphereCache() : Center(0.0f,0.0f,0.0f), FatRadius2(0.0f), FatCoeff(1.1f)	{}
					~SphereCache()																{}

		// Cached faces signature
		Point		Center;		//!< Sphere used when performing the query resulting in cached faces
		float		FatRadius2 = 0;	//!< Sphere used when performing the query resulting in cached faces
		// User settings
		float		FatCoeff = 0;	//!< mRadius2 multiplier used to create a fat sphere
	};

	class OPCODE_API SphereCollider : public VolumeCollider
	{
		public:
		// Constructor / Destructor
											SphereCollider() override;
		virtual ~SphereCollider();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Generic collision query for generic OPCODE models. After the call, access the results:
		 *	- with GetContactStatus()
		 *	- with GetNbTouchedPrimitives()
		 *	- with GetTouchedPrimitives()
		 *
		 *	\param		cache			[in/out] a sphere cache
		 *	\param		sphere			[in] collision sphere in local space
		 *	\param		model			[in] Opcode model to collide with
		 *	\param		worlds			[in] sphere's world matrix, or null
		 *	\param		worldm			[in] model's world matrix, or null
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							bool			Collide(SphereCache& cache, const Sphere& sphere, const Model& model, const Matrix4x4* worlds=null, const Matrix4x4* worldm=null) override;

		// 
							bool			Collide(SphereCache& cache, const Sphere& sphere, const AABBTree* tree) override;
		protected:
		// Sphere in model space
							Point			mCenter;			//!< Sphere center
							float			mRadius2 = 0;			//!< Sphere radius squared
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
		inline_				BOOL			SphereContainsBox(const Point& bc, const Point& be) override;
		inline_				BOOL			SphereAABBOverlap(const Point& center, const Point& extents) override;
							BOOL			SphereTriOverlap(const Point& vert0, const Point& vert1, const Point& vert2) override;
			// Init methods
							BOOL			InitQuery(SphereCache& cache, const Sphere& sphere, const Matrix4x4* worlds=null, const Matrix4x4* worldm=null) override;
	};

	class OPCODE_API HybridSphereCollider : public SphereCollider
	{
		public:
		// Constructor / Destructor
											HybridSphereCollider() override;
		virtual ~HybridSphereCollider();

							bool			Collide(SphereCache& cache, const Sphere& sphere, const HybridModel& model, const Matrix4x4* worlds=null, const Matrix4x4* worldm=null) override;
		protected:
							Container		mTouchedBoxes;
	};

#endif // __OPC_SPHERECOLLIDER_H__
