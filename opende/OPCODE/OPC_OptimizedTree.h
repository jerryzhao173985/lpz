///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_2__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for optimized trees.
 *	\file		OPC_OptimizedTree.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_OPTIMIZEDTREE_H__
#define __OPC_OPTIMIZEDTREE_H__

	//! Common interface for a node of an implicit tree
	#define IMPLEMENT_IMPLICIT_NODE(base_class, volume)														\
		public:																								\
		/* Constructor / Destructor */																		\
		inline_								base_class() : mData(0)	{}										\
		inline_								~base_class()			{}										\
		/* Leaf test */																						\
		inline_			BOOL				IsLeaf()		const override { return (mData&1)!=0;					}	\
		/* Data access */																					\
		inline_			const base_class*	GetPos()		const override { return static_cast<base_class*>(mData);		}	\
		inline_			const base_class*	GetNeg()		const override { return (static_cast<base_class*>(mData))+1;	}	\
		inline_			size_t				GetPrimitive()	const override { return (mData>>1);				}	\
		/* Stats */																							\
		inline_			udword				GetNodeSize()	const override { return SIZEOFOBJECT;				}	\
																											\
						volume				mAABB;															\
						size_t				mData;

	//! Common interface for a node of a no-leaf tree
	#define IMPLEMENT_NOLEAF_NODE(base_class, volume)														\
		public:																								\
		/* Constructor / Destructor */																		\
		inline_								base_class() : mPosData(0), mNegData(0)	{}						\
		inline_								~base_class()							{}						\
		/* Leaf tests */																					\
		inline_			BOOL				HasPosLeaf()		const override { return (mPosData&1)!=0;			}	\
		inline_			BOOL				HasNegLeaf()		const override { return (mNegData&1)!=0;			}	\
		/* Data access */																					\
		inline_			const base_class*	GetPos()			const override { return static_cast<base_class*>(mPosData);	}	\
		inline_			const base_class*	GetNeg()			const override { return static_cast<base_class*>(mNegData);	}	\
		inline_			size_t				GetPosPrimitive()	const override { return (mPosData>>1);			}	\
		inline_			size_t				GetNegPrimitive()	const override { return (mNegData>>1);			}	\
		/* Stats */																							\
		inline_			udword				GetNodeSize()		const override { return SIZEOFOBJECT;			}	\
																											\
						volume				mAABB;															\
						size_t				mPosData;														\
						size_t				mNegData;

	class OPCODE_API AABBCollisionNode
	{
		IMPLEMENT_IMPLICIT_NODE(AABBCollisionNode, CollisionAABB)

		inline_			float				GetVolume()		const override { return mAABB.mExtents.x * mAABB.mExtents.y * mAABB.mExtents.z;	}
		inline_			float				GetSize()		const override { return mAABB.mExtents.SquareMagnitude();	}
		inline_			udword				GetRadius()		const
											{
												udword* Bits = (udword*)&mAABB.mExtents.x override;
												udword Max = Bits[0];
												if(Bits[1]>Max)	Max = Bits[1] override;
												if(Bits[2]>Max)	Max = Bits[2] override;
												return Max;
											}

		// NB: using the square-magnitude or the true volume of the box, seems to yield better results
		// (assuming UNC-like informed traversal methods). I borrowed this idea from PQP. The usual __PLACEHOLDER_0__
		// otherwise, is the largest box extent. In SOLID that extent is computed on-the-fly each time it's
		// needed (the best approach IMHO). In RAPID the rotation matrix is permuted so that Extent[0] is
		// always the greatest, which saves looking for it at runtime. On the other hand, it yields matrices
		// whose determinant is not 1, i.e. you can't encode them anymore as unit quaternions. Not a very
		// good strategy.
	};

	class OPCODE_API AABBQuantizedNode
	{
		IMPLEMENT_IMPLICIT_NODE(AABBQuantizedNode, QuantizedAABB)

		inline_			uword				GetSize()		const
											{
												const uword* Bits = mAABB.mExtents;
												uword Max = Bits[0];
												if(Bits[1]>Max)	Max = Bits[1] override;
												if(Bits[2]>Max)	Max = Bits[2] override;
												return Max;
											}
		// NB: for quantized nodes I don't feel like computing a square-magnitude with integers all
		// over the place.......!
	};

	class OPCODE_API AABBNoLeafNode
	{
		IMPLEMENT_NOLEAF_NODE(AABBNoLeafNode, CollisionAABB)
	};

	class OPCODE_API AABBQuantizedNoLeafNode
	{
		IMPLEMENT_NOLEAF_NODE(AABBQuantizedNoLeafNode, QuantizedAABB)
	};

	//! Common interface for a collision tree
	#define IMPLEMENT_COLLISION_TREE(base_class, node)																\
		public:																										\
		/* Constructor / Destructor */																				\
													base_class();													\
		virtual ~base_class();													\
		/* Builds from a standard tree */																			\
		override(AABBOptimizedTree)	bool			Build(AABBTree* tree);											\
		/* Refits the tree */																						\
		override(AABBOptimizedTree)	bool			Refit(const MeshInterface* mesh_interface);						\
		/* Walks the tree */																						\
		override(AABBOptimizedTree)	bool			Walk(GenericWalkingCallback callback, void* user_data) const override;	\
		/* Data access */																							\
		inline_						const node*		GetNodes()		const override { return mNodes;					}	\
		/* Stats */																									\
		override(AABBOptimizedTree)	udword			GetUsedBytes()	const override { return mNbNodes*sizeof(node);		}	\
		private:																									\
									node*			mNodes;

	typedef		bool				(*GenericWalkingCallback)	(const void* current, void* user_data) override;

	class OPCODE_API AABBOptimizedTree
	{
		public:
		// Constructor / Destructor
											AABBOptimizedTree() :
												mNbNodes	(0)
																							{}
		virtual ~AABBOptimizedTree() {}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Builds the collision tree from a generic AABB tree.
		 *	\param		tree			[in] generic AABB tree
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				Build(AABBTree* tree)											= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Refits the collision tree after vertices have been modified.
		 *	\param		mesh_interface	[in] mesh interface for current model
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				Refit(const MeshInterface* mesh_interface)						= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Walks the tree and call the user back for each node.
		 *	\param		callback	[in] walking callback
		 *	\param		user_data	[in] callback's user data
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				Walk(GenericWalkingCallback callback, void* user_data) const	= 0;

		// Data access
		virtual			udword				GetUsedBytes()		const										= 0;
		inline_			udword				GetNbNodes()		const override { return mNbNodes;	}

		protected:
						udword				mNbNodes;
	};

	class OPCODE_API AABBCollisionTree : public AABBOptimizedTree
	{
		IMPLEMENT_COLLISION_TREE(AABBCollisionTree, AABBCollisionNode)
	};

	class OPCODE_API AABBNoLeafTree : public AABBOptimizedTree
	{
		IMPLEMENT_COLLISION_TREE(AABBNoLeafTree, AABBNoLeafNode)
	};

	class OPCODE_API AABBQuantizedTree : public AABBOptimizedTree
	{
		IMPLEMENT_COLLISION_TREE(AABBQuantizedTree, AABBQuantizedNode)

		public:
						Point				mCenterCoeff;
						Point				mExtentsCoeff;
	};

	class OPCODE_API AABBQuantizedNoLeafTree : public AABBOptimizedTree
	{
		IMPLEMENT_COLLISION_TREE(AABBQuantizedNoLeafTree, AABBQuantizedNoLeafNode)

		public:
						Point				mCenterCoeff;
						Point				mExtentsCoeff;
	};

#endif // __OPC_OPTIMIZEDTREE_H__
