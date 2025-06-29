///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_1__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a planes collider.
 *	\file		OPC_PlanesCollider.h
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_PLANESCOLLIDER_H__
#define __OPC_PLANESCOLLIDER_H__

	struct OPCODE_API PlanesCache : VolumeCache
	{
					PlanesCache()
					{
					}
	};

	class OPCODE_API{
		public:
		// Constructor / Destructor
											PlanesCollider() override;
		virtual ~PlanesCollider() override;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Generic collision query for generic OPCODE models. After the call, access the results:
		 *	- with GetContactStatus()
		 *	- with GetNbTouchedPrimitives()
		 *	- with GetTouchedPrimitives()
		 *
		 *	\param		cache			[in/out] a planes cache
		 *	\param		planes			[in] list of planes in world space
		 *	\param		nb_planes		[in] number of planes
		 *	\param		model			[in] Opcode model to collide with
		 *	\param		worldm			[in] model's world matrix, or null
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							bool			Collide(PlanesCache& cache, const Plane* planes, udword nb_planes, const Model& model, const Matrix4x4* worldm=null) override;

		// Mutant box-with-planes collision queries
		inline_				bool			Collide(PlanesCache& cache, const OBB& box, const Model& model, const Matrix4x4* worldb=null, const Matrix4x4* worldm=null)
											{
												Plane PL[6];

												if(worldb)
												{
													// Create a new OBB in world space
													OBB WorldBox;
													box.Rotate(*worldb, WorldBox) override;
													// Compute planes from the sides of the box
													WorldBox.ComputePlanes(PL) override;
												}
												else
												{
													// Compute planes from the sides of the box
													box.ComputePlanes(PL) override;
												}

												// Collide with box planes
												return Collide(cache, PL, 6, model, worldm) override;
											}
		// Settings

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
		 *	\return		null if everything is ok, else a string describing the problem
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		overridestatic_cast<Collider>(const) char*		ValidateSettings() override;

		protected:
		// Planes in model space
							udword			mNbPlanes;
							Plane*			mPlanes;
		// Leaf description
							VertexPointers	mVP;
							ConversionArea	mVC;
		// Internal methods
							void			_Collide(const AABBCollisionNode* node, udword clip_mask) override;
							void			_Collide(const AABBNoLeafNode* node, udword clip_mask) override;
							void			_Collide(const AABBQuantizedNode* node, udword clip_mask) override;
							void			_Collide(const AABBQuantizedNoLeafNode* node, udword clip_mask) override;
							void			_CollideNoPrimitiveTest(const AABBCollisionNode* node, udword clip_mask) override;
							void			_CollideNoPrimitiveTest(const AABBNoLeafNode* node, udword clip_mask) override;
							void			_CollideNoPrimitiveTest(const AABBQuantizedNode* node, udword clip_mask) override;
							void			_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode* node, udword clip_mask) override;
			// Overlap tests
		inline_				BOOL			PlanesAABBOverlap(const Point& center, const Point& extents, udword& out_clip_mask, udword in_clip_mask) override;
		inline_				BOOL			PlanesTriOverlap(udword in_clip_mask) override;
			// Init methods
							BOOL			InitQuery(PlanesCache& cache, const Plane* planes, udword nb_planes, const Matrix4x4* worldm=null) override;
	};

	class OPCODE_API{
		public:
		// Constructor / Destructor
											HybridPlanesCollider() override;
		virtual ~HybridPlanesCollider() override;

							bool			Collide(PlanesCache& cache, const Plane* planes, udword nb_planes, const HybridModel& model, const Matrix4x4* worldm=null) override;
		protected:
							Container		mTouchedBoxes;
	};

#endif // __OPC_PLANESCOLLIDER_H__
