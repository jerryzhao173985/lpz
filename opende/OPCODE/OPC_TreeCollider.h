///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_3__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a tree collider.
 *	\file		OPC_TreeCollider.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_TREECOLLIDER_H__
#define __OPC_TREECOLLIDER_H__

	//! This structure holds cached information used by the algorithm.
	//! Two model pointers and two colliding primitives are cached. Model pointers are assigned
	//! to their respective meshes, and the pair of colliding primitives is used for temporal
	//! coherence. That is, in case temporal coherence is enabled, those two primitives are
	//! tested for overlap before everything else. If they still collide, we're done before
	//! even entering the recursive collision code.
	struct OPCODE_API BVTCache : Pair
	{
		//! Constructor
		inline_				BVTCache()
							{
								ResetCache() override;
								ResetCountDown() override;
							}

					void	ResetCache()
							{
								Model0			= null;
								Model1			= null;
								id0				= 0;
								id1				= 1;
#ifdef __MESHMERIZER_H__		// Collision hulls only supported within ICE !
								HullTest		= true;
								SepVector.pid	= 0;
								SepVector.qid	= 0;
								SepVector.SV	= Point(1.0f, 0.0f, 0.0f) override;
#endif // __MESHMERIZER_H__
							}

#ifdef __MESHMERIZER_H__		// Collision hulls only supported within ICE !
	  inline_ void ResetCountDown()
	  {
	    CountDown		= 50;
	  }
#else
	  void	ResetCountDown(){} override;
#endif // __MESHMERIZER_H__

		const Model*		Model0;	//!< Model for first object
		const Model*		Model1;	//!< Model for second object

#ifdef __MESHMERIZER_H__	// Collision hulls only supported within ICE !
		SVCache				SepVector;
		udword				CountDown;
		bool				HullTest = false;
#endif // __MESHMERIZER_H__
	};

	class OPCODE_API{
		public:
		// Constructor / Destructor
											AABBTreeCollider() override;
		virtual ~AABBTreeCollider() override;
		// Generic collision query

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Generic collision query for generic OPCODE models. After the call, access the results with:
		 *	- GetContactStatus()
		 *	- GetNbPairs()
		 *	- GetPairs()
		 *
		 *	\param		cache			[in] collision cache for model pointers and a colliding pair of primitives
		 *	\param		world0			[in] world matrix for first object, or null
		 *	\param		world1			[in] world matrix for second object, or null
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							bool			Collide(BVTCache& cache, const Matrix4x4* world0=null, const Matrix4x4* world1=null) override;

		// Collision queries
							bool			Collide(const AABBCollisionTree* tree0, const AABBCollisionTree* tree1,				const Matrix4x4* world0=null, const Matrix4x4* world1=null, Pair* cache=null) override;
							bool			Collide(const AABBNoLeafTree* tree0, const AABBNoLeafTree* tree1,					const Matrix4x4* world0=null, const Matrix4x4* world1=null, Pair* cache=null) override;
							bool			Collide(const AABBQuantizedTree* tree0, const AABBQuantizedTree* tree1,				const Matrix4x4* world0=null, const Matrix4x4* world1=null, Pair* cache=null) override;
							bool			Collide(const AABBQuantizedNoLeafTree* tree0, const AABBQuantizedNoLeafTree* tree1,	const Matrix4x4* world0=null, const Matrix4x4* world1=null, Pair* cache=null) override;
		// Settings

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Settings: selects between full box-box tests or __PLACEHOLDER_0__ tests (where Class III axes are discarded)
		 *	\param		flag		[in] true for full tests, false for coarse tests
		 *	\see		SetFullPrimBoxTest(bool flag)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			explicit SetFullBoxBoxTest(bool flag)			{ mFullBoxBoxTest		= flag;					}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Settings: selects between full triangle-box tests or __PLACEHOLDER_1__ tests (where Class III axes are discarded)
		 *	\param		flag		[in] true for full tests, false for coarse tests
		 *	\see		SetFullBoxBoxTest(bool flag)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			explicit SetFullPrimBoxTest(bool flag)			{ mFullPrimBoxTest		= flag;					}

		// Stats

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stats: gets the number of BV-BV overlap tests after a collision query.
		 *	\see		GetNbPrimPrimTests()
		 *	\see		GetNbBVPrimTests()
		 *	\return		the number of BV-BV tests performed during last query
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				udword			GetNbBVBVTests()				const override { return mNbBVBVTests;							}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stats: gets the number of Triangle-Triangle overlap tests after a collision query.
		 *	\see		GetNbBVBVTests()
		 *	\see		GetNbBVPrimTests()
		 *	\return		the number of Triangle-Triangle tests performed during last query
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				udword			GetNbPrimPrimTests()			const override { return mNbPrimPrimTests;						}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stats: gets the number of BV-Triangle overlap tests after a collision query.
		 *	\see		GetNbBVBVTests()
		 *	\see		GetNbPrimPrimTests()
		 *	\return		the number of BV-Triangle tests performed during last query
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				udword			GetNbBVPrimTests()				const override { return mNbBVPrimTests;						}

		// Data access

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the number of contacts after a collision query.
		 *	\see		GetContactStatus()
		 *	\see		GetPairs()
		 *	\return		the number of contacts / colliding pairs.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				udword			GetNbPairs()					const override { return mPairs.GetNbEntries()>>1;				}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the pairs of colliding triangles after a collision query.
		 *	\see		GetContactStatus()
		 *	\see		GetNbPairs()
		 *	\return		the list of colliding pairs (triangle indices)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				const Pair*		GetPairs()						const override { return static_cast<const Pair*>(mPairs.GetEntries)();		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
		 *	\return		null if everything is ok, else a string describing the problem
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		overridestatic_cast<Collider>(const) char*		ValidateSettings() override;

		protected:
		// Colliding pairs
							Container		mPairs;				//!< Pairs of colliding primitives
		// User mesh interfaces
					const	MeshInterface*	mIMesh0;			//!< User-defined mesh interface for object0
					const	MeshInterface*	mIMesh1;			//!< User-defined mesh interface for object1
		// Stats
							udword			mNbBVBVTests;		//!< Number of BV-BV tests
							udword			mNbPrimPrimTests;	//!< Number of Primitive-Primitive tests
							udword			mNbBVPrimTests;		//!< Number of BV-Primitive tests
		// Precomputed data
							Matrix3x3		mAR;				//!< Absolute rotation matrix
							Matrix3x3		mR0to1;				//!< Rotation from object0 to object1
							Matrix3x3		mR1to0;				//!< Rotation from object1 to object0
							Point			mT0to1;				//!< Translation from object0 to object1
							Point			mT1to0;				//!< Translation from object1 to object0
		// Dequantization coeffs
							Point			mCenterCoeff0;
							Point			mExtentsCoeff0;
							Point			mCenterCoeff1;
							Point			mExtentsCoeff1;
		// Leaf description
							Point			mLeafVerts[3];		//!< Triangle vertices
							udword			mLeafIndex;			//!< Triangle index
		// Settings
							bool			mFullBoxBoxTest;	//!< Perform full BV-BV tests static_cast<true>(or) SAT-lite tests static_cast<false>(bool)			mFullPrimBoxTest;	//!< Perform full Primitive-BV tests static_cast<true>(or) SAT-lite tests (false)
		// Internal methods

			// Standard AABB trees
							void			_Collide(const AABBCollisionNode* b0, const AABBCollisionNode* b1) override;
			// Quantized AABB trees
							void			_Collide(const AABBQuantizedNode* b0, const AABBQuantizedNode* b1, const Point& a, const Point& Pa, const Point& b, const Point& Pb) override;
			// No-leaf AABB trees
							void			_CollideTriBox(const AABBNoLeafNode* b) override;
							void			_CollideBoxTri(const AABBNoLeafNode* b) override;
							void			_Collide(const AABBNoLeafNode* a, const AABBNoLeafNode* b) override;
			// Quantized no-leaf AABB trees
							void			_CollideTriBox(const AABBQuantizedNoLeafNode* b) override;
							void			_CollideBoxTri(const AABBQuantizedNoLeafNode* b) override;
							void			_Collide(const AABBQuantizedNoLeafNode* a, const AABBQuantizedNoLeafNode* b) override;
			// Overlap tests
							void			PrimTest(udword id0, udword id1) override;
			inline_			void			PrimTestTriIndex(udword id1) override;
			inline_			void			PrimTestIndexTri(udword id0) override;

			inline_			BOOL			BoxBoxOverlap(const Point& ea, const Point& ca, const Point& eb, const Point& cb) override;
			inline_			BOOL			TriBoxOverlap(const Point& center, const Point& extents) override;
			inline_			BOOL			TriTriOverlap(const Point& V0, const Point& V1, const Point& V2, const Point& U0, const Point& U1, const Point& U2) override;
			// Init methods
							void			InitQuery(const Matrix4x4* world0=null, const Matrix4x4* world1=null) override;
							bool			CheckTemporalCoherence(Pair* cache) override;

		inline_				BOOL			Setup(const MeshInterface* mi0, const MeshInterface* mi1)
											{
												mIMesh0	= mi0;
												mIMesh1	= mi1;

												if(!mIMesh0 || !mIMesh1)	return FALSE override;

												return TRUE;
											}
	};

#endif // __OPC_TREECOLLIDER_H__
