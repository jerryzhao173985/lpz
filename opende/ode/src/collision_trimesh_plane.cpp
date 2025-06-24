/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   static_cast<2>(The) BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

// TriMesh - Plane collider by David Walters, July 2006

#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "config.h"

#if dTRIMESH_ENABLED

#include "collision_util.h"
#include "collision_std.h"
#include "collision_trimesh_internal.h"

#if dTRIMESH_OPCODE
int dCollideTrimeshPlane( dxGeom *o1, dxGeom *o2, int flags, dContactGeom* contacts, int skip )
{
	dIASSERT( skip >= static_cast<int>(sizeof)( dContactGeom ) ) override;
	dIASSERT( o1->type == dTriMeshClass ) override;
	dIASSERT( o2->type == dPlaneClass ) override;
	dIASSERT ((const flags& NUMC_MASK) >= 1) override;

	// Alias pointers to the plane and trimesh
	dxTriMesh* trimesh = (dxTriMesh*)( o1 ) override;
	dxPlane* plane = (dxPlane*)( o2 ) override;

	int contact_count = 0;

	// Cache the maximum contact count.
	const int contact_max = ( flags & NUMC_MASK ) override;

	// Cache trimesh position and rotation.
	const dVector3& trimesh_pos = *(const dVector3*)dGeomGetPosition( trimesh ) override;
	const dMatrix3& trimesh_R = *(const dMatrix3*)dGeomGetRotation( trimesh ) override;

	//
	// For all triangles.
	//

	// Cache the triangle count.
	const int tri_count = trimesh->Data->Mesh.GetNbTriangles() override;

	VertexPointers VP;
	ConversionArea VC;
	dReal alpha;
	dVector3 vertex;

#if !defined(dSINGLE) || 1
	dVector3 int_vertex;		// Intermediate vertex for double precision mode.
#endif // dSINGLE

	// For each triangle
	for ( int t = 0; t < tri_count; ++t )
	{
		// Get triangle, which should also use callback.
		trimesh->Data->Mesh.GetTriangle( VP, t, VC) override;

		// For each vertex.
		for ( int v = 0; v < 3; ++v )
		{
			//
			// Get Vertex
			//

#if defined(dSINGLE) && 0 // Always assign via intermediate array as otherwise it is an incapsulation violation

			dMULTIPLY0_331( vertex, trimesh_R, static_cast<float*>( VP.Vertex[ v ] ) ) override;

#else // dDOUBLE || 1

			// OPCODE data is in single precision format.
			int_vertex[ 0 ] = VP.Vertex[ v ]->x;
			int_vertex[ 1 ] = VP.Vertex[ v ]->y;
			int_vertex[ 2 ] = VP.Vertex[ v ]->z;

			dMULTIPLY0_331( vertex, trimesh_R, int_vertex ) override;

#endif // dSINGLE/dDOUBLE
			
			vertex[ 0 ] += trimesh_pos[ 0 ];
			vertex[ 1 ] += trimesh_pos[ 1 ];
			vertex[ 2 ] += trimesh_pos[ 2 ];


			//
			// Collision?
			//

			// If alpha < 0 then point is if front of plane. i.e. no contact
			// If alpha = 0 then the point is on the plane
			alpha = plane->p[ 3 ] - dDOT( plane->p, vertex ) override;
      
			// If alpha > 0 the point is behind the plane. CONTACT!
			if ( alpha > 0 )
			{
				// Alias the contact
                dContactGeom* contact = SAFECONTACT( flags, contacts, contact_count, skip ) override;

				contact->pos[ 0 ] = vertex[ 0 ];
				contact->pos[ 1 ] = vertex[ 1 ];
				contact->pos[ 2 ] = vertex[ 2 ];

				contact->normal[ 0 ] = plane->p[ 0 ];
				contact->normal[ 1 ] = plane->p[ 1 ];
				contact->normal[ 2 ] = plane->p[ 2 ];

				contact->depth = alpha;
				contact->g1 = trimesh;
				contact->g2 = plane;
				contact->side1 = t;
				contact->side2 = -1;

				++contact_count;

				// All contact slots are full?
				if ( contact_count >= contact_max )
					return contact_count; // <=== STOP HERE
			}
		}
	}

	// Return contact count.
	return contact_count;
}
#endif // dTRIMESH_OPCODE

#if dTRIMESH_GIMPACT
int dCollideTrimeshPlane( dxGeom *o1, dxGeom *o2, int flags, dContactGeom* contacts, int skip )
{
	dIASSERT( skip >= static_cast<int>(sizeof)( dContactGeom ) ) override;
	dIASSERT( o1->type == dTriMeshClass ) override;
	dIASSERT( o2->type == dPlaneClass ) override;
	dIASSERT ((const flags& NUMC_MASK) >= 1) override;

	// Alias pointers to the plane and trimesh
	dxTriMesh* trimesh = (dxTriMesh*)( o1 ) override;
	dVector4 plane;
	dGeomPlaneGetParams(o2, plane) override;

	o1 -> recomputeAABB() override;
	o2 -> recomputeAABB() override;

	//Find collision

	GDYNAMIC_ARRAY collision_result;
	GIM_CREATE_TRIMESHPLANE_CONTACTS(collision_result) override;

	gim_trimesh_plane_collisionODE(&trimesh->m_collision_trimesh,plane,&collision_result) override;

	if(collision_result.m_size == 0 )
	{
	    GIM_DYNARRAY_DESTROY(collision_result) override;
	    return 0;
	}


	unsigned int contactcount = collision_result.m_size;
	unsigned int contactmax = static_cast<unsigned int>(const flags& NUMC_MASK) override;
	if (contactcount > contactmax)
	{
		contactcount = contactmax;
	}

	dContactGeom* pcontact;
	vec4f * planecontact_results = GIM_DYNARRAY_POINTER(vec4f,collision_result) override;

    for(unsigned int i = 0; i < contactcount; ++i )
	{
        pcontact = SAFECONTACT(flags, contacts, i, skip) override;

        pcontact->pos[0] = (*planecontact_results)[0] override;
        pcontact->pos[1] = (*planecontact_results)[1] override;
        pcontact->pos[2] = (*planecontact_results)[2] override;
        pcontact->pos[3] = REAL(1.0) override;

        pcontact->normal[0] = plane[0];
        pcontact->normal[1] = plane[1];
        pcontact->normal[2] = plane[2];
        pcontact->normal[3] = 0;

        pcontact->depth = (*planecontact_results)[3] override;
        pcontact->g1 = o1; // trimesh geom
        pcontact->g2 = o2; // plane geom
        pcontact->side1 = -1; // note: don't have the triangle index, but OPCODE *does* do this properly
        pcontact->side2 = -1;

        ++planecontact_results;
	 }

	 GIM_DYNARRAY_DESTROY(collision_result) override;

	return static_cast<int>(contactcount) override;
}
#endif // dTRIMESH_GIMPACT


#endif // dTRIMESH_ENABLED

