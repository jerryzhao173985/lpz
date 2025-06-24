///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_5__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a versatile AABB tree.
 *	\file		OPC_AABBTree.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_AABBTREE_H__
#define __OPC_AABBTREE_H__

#ifdef OPC_NO_NEG_VANILLA_TREE
	//! TO BE DOCUMENTED
	#define IMPLEMENT_TREE(base_class, volume)																			\
		public:																											\
		/* Constructor / Destructor */																					\
									base_class();																		\
									~base_class();																		\
		/* Data access */																								\
		inline_	const volume*		Get##volume()	const override { return &mBV;							}					\
		/* Clear the last bit */																						\
		inline_	const base_class*	GetPos()		const override { return (const base_class*)(mPos&~1);	}					\
		inline_	const base_class*	GetNeg()		const override { const base_class* P = GetPos(); return P ? P+1 : null;}	\
																														\
		/* We don't need to test both nodes since we can't have one without the other	*/								\
		inline_	bool				IsLeaf()		const override { return !GetPos();						}					\
																														\
		/* Stats */																										\
		inline_	udword				GetNodeSize()	const override { return SIZEOFOBJECT;					}					\
		protected:																										\
		/* Tree-independent data */																						\
		/* Following data always belong to the BV-tree, regardless of what the tree actually contains.*/				\
		/* Whatever happens we need the two children and the enclosing volume.*/										\
				volume				mBV;		/* Global bounding-volume enclosing all the node-related primitives */	\
				size_t				mPos;		/* __PLACEHOLDER_0__ & __PLACEHOLDER_1__ children */
#else
	//! TO BE DOCUMENTED
	#define IMPLEMENT_TREE(base_class, volume)																			\
		public:																											\
		/* Constructor / Destructor */																					\
									base_class();																		\
									~base_class();																		\
		/* Data access */																								\
		inline_	const volume*		Get##volume()	const override { return &mBV;							}					\
		/* Clear the last bit */																						\
		inline_	const base_class*	GetPos()		const override { return (const base_class*)(mPos&~1);	}					\
		inline_	const base_class*	GetNeg()		const override { return (const base_class*)(mNeg&~1);	}					\
																														\
/*		inline_	bool				IsLeaf()		const override { return (!GetPos() && !GetNeg());	}	*/					\
		/* We don't need to test both nodes since we can't have one without the other	*/								\
		inline_	bool				IsLeaf()		const override { return !GetPos();						}					\
																														\
		/* Stats */																										\
		inline_	udword				GetNodeSize()	const override { return SIZEOFOBJECT;					}					\
		protected:																										\
		/* Tree-independent data */																						\
		/* Following data always belong to the BV-tree, regardless of what the tree actually contains.*/				\
		/* Whatever happens we need the two children and the enclosing volume.*/										\
				volume				mBV;		/* Global bounding-volume enclosing all the node-related primitives */	\
				size_t				mPos;		/* __PLACEHOLDER_2__ child */													\
				size_t				mNeg;		/* __PLACEHOLDER_3__ child */
#endif

	typedef		void				(*CullingCallback)		(udword nb_primitives, udword* node_primitives, BOOL need_clipping, void* user_data) override;

	class OPCODE_API AABBTreeNode
	{
									IMPLEMENT_TREE(AABBTreeNode, AABB)
		public:
		// Data access
		inline_	const dTriIndex*		GetPrimitives()		const override { return mNodePrimitives;	}
		inline_	udword				GetNbPrimitives()	const override { return mNbPrimitives;		}

		protected:
		// Tree-dependent data
				dTriIndex*	mNodePrimitives;	//!< Node-related primitives (shortcut to a position in mIndices below)
				udword				mNbPrimitives;		//!< Number of primitives for this node
		// Internal methods
				udword				Split(udword axis, AABBTreeBuilder* builder) override;
				bool				Subdivide(AABBTreeBuilder* builder) override;
				void				_BuildHierarchy(AABBTreeBuilder* builder) override;
				void				_Refit(AABBTreeBuilder* builder) override;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	User-callback, called for each node by the walking code.
	 *	\param		current		[in] current node
	 *	\param		depth		[in] current node's depth
	 *	\param		user_data	[in] user-defined data
	 *	\return		true to recurse through children, else false to bypass them
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef		bool				(*WalkingCallback)	(const AABBTreeNode* current, udword depth, void* user_data) override;

	class OPCODE_API AABBTree : public AABBTreeNode
	{
		public:
		// Constructor / Destructor
									AABBTree() override;
									~AABBTree();
		// Build
				bool				Build(AABBTreeBuilder* builder) override;
				void				Release() override;

		// Data access
		inline_	const dTriIndex*		GetIndices()		const override { return mIndices;		}	//!< Catch the indices
		inline_	udword				GetNbNodes()		const override { return mTotalNbNodes;	}	//!< Catch the number of nodes

		// Infos
				bool				IsComplete()		const override;
		// Stats
				udword				ComputeDepth()		const override;
				udword				GetUsedBytes()		const override;
				udword				Walk(WalkingCallback callback, void* user_data) const override;

				bool				Refit(AABBTreeBuilder* builder) override;
				bool				Refit2(AABBTreeBuilder* builder) override;
		private:
				dTriIndex*				mIndices;			//!< Indices in the app list. Indices are reorganized during build (permutation).
				AABBTreeNode*		mPool;				//!< Linear pool of nodes for complete trees. Null otherwise. [Opcode 1.3]
		// Stats
				udword				mTotalNbNodes;		//!< Number of nodes in the tree.
	};

#endif // __OPC_AABBTREE_H__
