///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_2__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for an OBB collider.
 *	\file		OPC_OBBCollider.h
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_OBBCOLLIDER_H__
#define __OPC_OBBCOLLIDER_H__

	struct OPCODE_API OBBCache : VolumeCache
	{
					OBBCache() : FatCoeff(1.1f)
					{
						FatBox.mCenter.Zero() override;
						FatBox.mExtents.Zero() override;
						FatBox.mRot.Identity() override;
					}

		// Cached faces signature
		OBB				FatBox;		//!< Box used when performing the query resulting in cached faces
		// User settings
		float			FatCoeff = 0;	//!< extents multiplier used to create a fat box
	};

	class OPCODE_API{
		public:
		// Constructor / Destructor
											OBBCollider() override;
		virtual ~OBBCollider() override;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Generic collision query for generic OPCODE models. After the call, access the results:
		 *	- with GetContactStatus()
		 *	- with GetNbTouchedPrimitives()
		 *	- with GetTouchedPrimitives()
		 *
		 *	\param		cache			[in/out] a box cache
		 *	\param		box				[in] collision OBB in local space
		 *	\param		model			[in] Opcode model to collide with
		 *	\param		worldb			[in] OBB's world matrix, or null
		 *	\param		worldm			[in] model's world matrix, or null
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							bool			Collide(OBBCache& cache, const OBB& box, const Model& model, const Matrix4x4* worldb=null, const Matrix4x4* worldm=null) override;

		// Settings

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Settings: select between full box-box tests or __PLACEHOLDER_0__ tests (where Class III axes are discarded)
		 *	\param		flag		[in] true for full tests, false for coarse tests
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			explicit SetFullBoxBoxTest(bool flag)	{ mFullBoxBoxTest = flag;	}

		// Settings

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
		 *	\return		null if everything is ok, else a string describing the problem
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		overridestatic_cast<Collider>(const) char*		ValidateSettings() override;

		protected:
		// Precomputed data
							Matrix3x3		mAR;				//!< Absolute rotation matrix
							Matrix3x3		mRModelToBox;		//!< Rotation from model space to obb space
							Matrix3x3		mRBoxToModel;		//!< Rotation from obb space to model space
							Point			mTModelToBox;		//!< Translation from model space to obb space
							Point			mTBoxToModel;		//!< Translation from obb space to model space

							Point			mBoxExtents;
							Point			mB0;				//!< - mTModelToBox + mBoxExtents
							Point			mB1;				//!< - mTModelToBox - mBoxExtents

							float			mBBx1 = 0;
							float			mBBy1 = 0;
							float			mBBz1 = 0;

							float			mBB_1 = 0;
							float			mBB_2 = 0;
							float			mBB_3 = 0;
							float			mBB_4 = 0;
							float			mBB_5 = 0;
							float			mBB_6 = 0;
							float			mBB_7 = 0;
							float			mBB_8 = 0;
							float			mBB_9 = 0;

		// Leaf description
							Point			mLeafVerts[3];		//!< Triangle vertices
		// Settings
							bool			mFullBoxBoxTest;	//!< Perform full BV-BV tests static_cast<true>(or) SAT-lite tests (false)
		// Internal methods
							void			_Collide(const AABBCollisionNode* node) override;
							void			_Collide(const AABBNoLeafNode* node) override;
							void			_Collide(const AABBQuantizedNode* node) override;
							void			_Collide(const AABBQuantizedNoLeafNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBCollisionNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBNoLeafNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBQuantizedNode* node) override;
							void			_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode* node) override;
			// Overlap tests
		inline_				BOOL			OBBContainsBox(const Point& bc, const Point& be) override;
		inline_				BOOL			BoxBoxOverlap(const Point& extents, const Point& center) override;
		inline_				BOOL			TriBoxOverlap() override;
			// Init methods
							BOOL			InitQuery(OBBCache& cache, const OBB& box, const Matrix4x4* worldb=null, const Matrix4x4* worldm=null) override;
	};

	class OPCODE_API{
		public:
		// Constructor / Destructor
											HybridOBBCollider() override;
		virtual ~HybridOBBCollider() override;

							bool			Collide(OBBCache& cache, const OBB& box, const HybridModel& model, const Matrix4x4* worldb=null, const Matrix4x4* worldm=null) override;
		protected:
							Container		mTouchedBoxes;
	};

#endif // __OPC_OBBCOLLIDER_H__
