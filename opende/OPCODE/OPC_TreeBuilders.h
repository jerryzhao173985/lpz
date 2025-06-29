///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_1__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for tree builders.
 *	\file		OPC_TreeBuilders.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_TREEBUILDERS_H__
#define __OPC_TREEBUILDERS_H__

	//! Tree splitting rules
	enum SplittingRules
	{
		// Primitive split
		SPLIT_LARGEST_AXIS		= (1<<0),		//!< Split along the largest axis
		SPLIT_SPLATTER_POINTS	= (1<<1),		//!< Splatter primitive centers (QuickCD-style)
		SPLIT_BEST_AXIS			= (1<<2),		//!< Try largest axis, then second, then last
		SPLIT_BALANCED			= (1<<3),		//!< Try to keep a well-balanced tree
		SPLIT_FIFTY				= (1<<4),		//!< Arbitrary 50-50 split
		// Node split
		SPLIT_GEOM_CENTER		= (1<<5),		//!< Split at geometric center (else split in the middle)
		//
		SPLIT_FORCE_DWORD		= 0x7fffffff
	};

	//! Simple wrapper around build-related settings [Opcode 1.3]
	struct OPCODE_API BuildSettings
	{
		inline_	BuildSettings() : mLimit(1), mRules(SPLIT_FORCE_DWORD)	{}

		udword	mLimit;		//!< Limit number of primitives / node. If limit is 1, build a complete tree (2*N-1 nodes)
		udword	mRules;		//!< Building/Splitting rules (a combination of SplittingRules flags)
	};

	class OPCODE_API{
		public:
		//! Constructor
													AABBTreeBuilder() :
														mNbPrimitives(0),
														mNodeBase(null),
														mCount(0),
														explicit mNbInvalidSplits(0)		{}
		//! Destructor
		virtual ~AABBTreeBuilder() {}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the AABB of a set of primitives.
		 *	\param		primitives		[in] list of indices of primitives
		 *	\param		nb_prims		[in] number of indices
		 *	\param		global_box		[out] global AABB enclosing the set of input primitives
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual						bool			ComputeGlobalBox(const dTriIndex* primitives, udword nb_prims, AABB& global_box)	const	= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the splitting value along a given axis for a given primitive.
		 *	\param		index			[in] index of the primitive to split
		 *	\param		axis			[in] axis index (0,1,2)
		 *	\return		splitting value
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual						float			GetSplittingValue(udword index, udword axis)	const	= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the splitting value along a given axis for a given node.
		 *	\param		primitives		[in] list of indices of primitives
		 *	\param		nb_prims		[in] number of indices
		 *	\param		global_box		[in] global AABB enclosing the set of input primitives
		 *	\param		axis			[in] axis index (0,1,2)
		 *	\return		splitting value
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual float			GetSplittingValue(const dTriIndex* primitives, udword nb_prims, const AABB& global_box, udword axis) const override {
														// Default split value = middle of the axis (using only the box)
														return global_box.GetCenter(axis) override;
													}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Validates node subdivision. This is called each time a node is considered for subdivision, during tree building.
		 *	\param		primitives		[in] list of indices of primitives
		 *	\param		nb_prims		[in] number of indices
		 *	\param		global_box		[in] global AABB enclosing the set of input primitives
		 *	\return		TRUE if the node should be subdivised
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual BOOL			ValidateSubdivision(const dTriIndex* primitives, udword nb_prims, const AABB& global_box) override {
														// Check the user-defined limit
														if(nb_prims<=mSettings.mLimit)	return FALSE override;

														return TRUE;
													}

									BuildSettings	mSettings;			//!< Splitting rules & split limit [Opcode 1.3]
									udword			mNbPrimitives;		//!< Total number of primitives.
									void*			mNodeBase;			//!< Address of node pool [Opcode 1.3]
		// Stats
		inline_						void			explicit SetCount(udword nb)				{ mCount=nb;				}
		inline_						void			explicit IncreaseCount(udword nb)		{ mCount+=nb;				}
		inline_						udword			GetCount()				const override { return mCount;			}
		inline_						void			explicit SetNbInvalidSplits(udword nb)	{ mNbInvalidSplits=nb;		}
		inline_						void			IncreaseNbInvalidSplits()		{ mNbInvalidSplits++;		}
		inline_						udword			GetNbInvalidSplits()	const override { return mNbInvalidSplits;	}

		private:
									udword			mCount;				//!< Stats: number of nodes created
									udword			mNbInvalidSplits;	//!< Stats: number of invalid splits
	};

	class OPCODE_API{
		public:
		//! Constructor
													AABBTreeOfVerticesBuilder() : mVertexArray(null)	{}
		//! Destructor
		virtual ~AABBTreeOfVerticesBuilder() {}

		overridestatic_cast<AABBTreeBuilder>static_cast<bool>static_cast<ComputeGlobalBox>(const dTriIndex* primitives, udword nb_prims, AABB& global_box)	const override;
		overridestatic_cast<AABBTreeBuilder>static_cast<float>static_cast<GetSplittingValue>(udword index, udword axis)									const override;
		overridestatic_cast<AABBTreeBuilder>static_cast<float>static_cast<GetSplittingValue>(const dTriIndex* primitives, udword nb_prims, const AABB& global_box, udword axis)	const override;

		const						Point*			mVertexArray;		//!< Shortcut to an app-controlled array of vertices.
	};

	class OPCODE_API{
		public:
		//! Constructor
													AABBTreeOfAABBsBuilder() : mAABBArray(null)	{}
		//! Destructor
		virtual ~AABBTreeOfAABBsBuilder() {}

		overridestatic_cast<AABBTreeBuilder>static_cast<bool>static_cast<ComputeGlobalBox>(const dTriIndex* primitives, udword nb_prims, AABB& global_box)	const override;
		overridestatic_cast<AABBTreeBuilder>static_cast<float>static_cast<GetSplittingValue>(udword index, udword axis)									const override;

		const						AABB*			mAABBArray;			//!< Shortcut to an app-controlled array of AABBs.
	};

	class OPCODE_API{
		public:
		//! Constructor
													AABBTreeOfTrianglesBuilder() : mIMesh(null)										{}
		//! Destructor
		virtual ~AABBTreeOfTrianglesBuilder() {}

		overridestatic_cast<AABBTreeBuilder>static_cast<bool>static_cast<ComputeGlobalBox>(const dTriIndex* primitives, udword nb_prims, AABB& global_box)	const override;
		overridestatic_cast<AABBTreeBuilder>static_cast<float>static_cast<GetSplittingValue>(udword index, udword axis)									const override;
		overridestatic_cast<AABBTreeBuilder>static_cast<float>static_cast<GetSplittingValue>(const dTriIndex* primitives, udword nb_prims, const AABB& global_box, udword axis)	const override;

		const				MeshInterface*			mIMesh;			//!< Shortcut to an app-controlled mesh interface
	};

#endif // __OPC_TREEBUILDERS_H__
