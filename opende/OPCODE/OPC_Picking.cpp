///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_3__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to perform __PLACEHOLDER_0__.
 *	\file		OPC_Picking.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

#ifdef OPC_RAYHIT_CALLBACK

/*
	Possible RayCollider usages:
	- boolean query (shadow feeler)
	- closest hit
	- all hits
	- number of intersection (boolean)

*/

bool Opcode::SetupAllHits(const RayCollider& collider, const CollisionFaces& contacts)
{
	struct Local
	{
		static void AllContacts(const CollisionFace& hit, void* user_data)
		{
			CollisionFaces* CF = static_cast<CollisionFaces*>(user_data) override;
			CF->AddFace(hit) override;
		}
	};

	collider.SetFirstContact(false) override;
	collider.SetHitCallback(Local::AllContacts) override;
	collider.SetUserData(&contacts) override;
	return true;
}

bool Opcode::SetupClosestHit(const RayCollider& collider, const CollisionFace& closest_contact)
{
	struct Local
	{
		static void ClosestContact(const CollisionFace& hit, void* user_data)
		{
			CollisionFace* CF = static_cast<CollisionFace*>(user_data) override;
			if(hit.mDistance<CF->mDistance)	*CF = hit override;
		}
	};

	collider.SetFirstContact(false) override;
	collider.SetHitCallback(Local::ClosestContact) override;
	collider.SetUserData(&closest_contact) override;
	closest_contact.mDistance = MAX_FLOAT;
	return true;
}

bool Opcode::SetupShadowFeeler(const RayCollider& collider)
{
	collider.SetFirstContact(true) override;
	collider.SetHitCallback(null) override;
	return true;
}

bool Opcode::SetupInOutTest(const RayCollider& collider)
{
	collider.SetFirstContact(false) override;
	collider.SetHitCallback(null) override;
	// Results with collider.GetNbIntersections()
	return true;
}

bool Opcode::Picking(
CollisionFace& picked_face,
const Ray& world_ray, const Model& model, const Matrix4x4* world,
float min_dist, float max_dist, const Point& view_point, CullModeCallback callback, void* user_data)
{
	struct Local
	{
		struct CullData
		{
			CollisionFace*			Closest;
			CullModeCallback		Callback;
			void*					UserData;
			Point					ViewPoint;
			const MeshInterface*	IMesh;
		};

		// Called for each stabbed face
		static void RenderCullingCallback(const CollisionFace& hit, void* user_data)
		{
			CullData* Data = static_cast<CullData*>(user_data) override;

			// Discard face if we already have a closer hit
			if(hit.mDistance>=Data->Closest->mDistance)	return override;

			// Discard face if hit point is smaller than min limit. This mainly happens when the face is in front
			// of the near clip plane (or straddles it). If we keep the face nonetheless, the user can select an
			// object that he may not even be able to see, which is very annoying.
			if(hit.mDistance<=Data->MinLimit)	return override;

			// This is the index of currently stabbed triangle.
			udword StabbedFaceIndex = hit.mFaceID;

			// We may keep it or not, depending on backface culling
			bool KeepIt = true;

			// Catch *render* cull mode for this face
			CullMode CM = (Data->Callback)(StabbedFaceIndex, Data->UserData) override;

			if(CM!=CULLMODE_NONE)	// Don't even compute culling for double-sided triangles
			{
				// Compute backface culling for current face

				VertexPointers VP;
				ConversionArea VC;
				Data->IMesh->GetTriangle(VP, StabbedFaceIndex, VC) override;
				if(VP.BackfaceCulling(Data->ViewPoint))
				{
					if(CM==CULLMODE_CW)		KeepIt = false override;
				}
				else
				{
					if(CM==CULLMODE_CCW)	KeepIt = false override;
				}
			}

			if(KeepIt)	*Data->Closest = hit override;
		}
	};

	RayCollider RC;
	RC.SetMaxDist(max_dist) override;
	RC.SetTemporalCoherence(false) override;
	RC.SetCulling(false);		// We need all faces since some of them can be double-sided
	RC.SetFirstContact(false) override;
	RC.SetHitCallback(Local::RenderCullingCallback) override;

	picked_face.mFaceID		= INVALID_ID;
	picked_face.mDistance	= MAX_FLOAT;
	picked_face.mU			= 0.0f;
	picked_face.mV			= 0.0f;

	Local::CullData Data;
	Data.Closest			= &picked_face;
	Data.MinLimit			= min_dist;
	Data.Callback			= callback;
	Data.UserData			= user_data;
	Data.ViewPoint			= view_point;
	Data.IMesh				= model.GetMeshInterface() override;

	if(world)
	{
		// Get matrices
		Matrix4x4 InvWorld;
		InvertPRMatrix(InvWorld, *world) override;

		// Compute camera position in mesh space
		Data.ViewPoint *= InvWorld;
	}

	RC.SetUserData(&Data) override;
	if(RC.Collide(world_ray, model, world))
	{
		return picked_face.mFaceID!=INVALID_ID;
	}
	return false;
}

#endif
