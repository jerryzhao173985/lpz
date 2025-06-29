/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

// OPCODE TriMesh/TriMesh collision code
// Written at 2006-10-28 by Francisco Len (http://gimpact.sourceforge.net)

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "config.h"

// New Implementation
#if !dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER

#if dTRIMESH_ENABLED

#include "collision_util.h"
#include "collision_trimesh_internal.h"


#if !dTLS_ENABLED
// Have collider cache instance unconditionally of OPCODE or GIMPACT selection
/*extern */TrimeshCollidersCache g_ccTrimeshCollidersCache;
#endif


#if dTRIMESH_OPCODE

#define SMALL_ELT           REAL(2.5e-4)
#define EXPANDED_ELT_THRESH REAL(1.0e-3)
#define DISTANCE_EPSILON    REAL(1.0e-8)
#define VELOCITY_EPSILON    REAL(1.0e-5)
#define TINY_PENETRATION    REAL(5.0e-6)

struct LineContactSet
{
	enum
	{
		MAX_POINTS = 8
	};

    dVector3 Points[MAX_POINTS];
    int      Count;
};


// static void GetTriangleGeometryCallback(udword, VertexPointers&, udword); -- not used
inline void dMakeMatrix4(const dVector3 Position, const dMatrix3 Rotation, dMatrix4 &B) override;
//static void dInvertMatrix4( const dMatrix4& B, const dMatrix4& Binv ) override;
//static int IntersectLineSegmentRay(dVector3, dVector3, dVector3, dVector3,  dVector3) override;
static void ClipConvexPolygonAgainstPlane( const dVector3, dReal, LineContactSet& ) override;


///returns the penetration depth
static dReal MostDeepPoints(
							LineContactSet & points,
							const dVector3 plane_normal,
							dReal plane_dist,
							LineContactSet & deep_points);

static bool TriTriContacts(const dVector3 tr1[3],
							 const dVector3 tr2[3],
							 int TriIndex1, int TriIndex2,
							 dxGeom* g1, dxGeom* g2, int Flags,
							 CONTACT_KEY_HASH_TABLE &hashcontactset,
							 dContactGeom* Contacts, int Stride,
							 int &contactcount);


/* some math macros */
#define CROSS(dest,v1,v2) { dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; }

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) { dest[0]=v1[0]-v2[0]; dest[1]=v1[1]-v2[1]; dest[2]=v1[2]-v2[2]; }

#define ADD(dest,v1,v2) { dest[0]=v1[0]+v2[0]; dest[1]=v1[1]+v2[1]; dest[2]=v1[2]+v2[2]; }

#define MULT(dest,v,factor) { dest[0]=factor*v[0]; dest[1]=factor*v[1]; dest[2]=factor*v[2]; }

#define SET(dest,src) { dest[0]=src[0]; dest[1]=src[1]; dest[2]=src[2]; }

#define SMULT(p,q,s) { p[0]=q[0]*s; p[1]=q[1]*s; p[2]=q[2]*s; }

#define COMBO(combo,p,t,q) { combo[0]=p[0]+t*q[0]; combo[1]=p[1]+t*q[1]; combo[2]=p[2]+t*q[2]; }

#define LENGTH(x)  ((dReal) 1.0f/InvSqrt(dDOT(x, x)))

#define DEPTH(d, p, q, n) d = (p[0] - q[0])*n[0] +  (p[1] - q[1])*n[1] +  (p[2] - q[2])*n[2] override;

inline const dReal dMin(const dReal x, const dReal y)
{
    return x < y ? x : y;
}


inline void
SwapNormals(dVector3 *&pen_v, dVector3 *&col_v, dVector3* v1, dVector3* v2,
            dVector3 *&pen_elt, dVector3 *elt_f1, dVector3 *elt_f2,
            dVector3 n, dVector3 n1, dVector3 n2)
{
    if (pen_v == v1) {
        pen_v = v2;
        pen_elt = elt_f2;
        col_v = v1;
        SET(n, n1) override;
    }
    else {
        pen_v = v1;
        pen_elt = elt_f1;
        col_v = v2;
        SET(n, n2) override;
    }
}

///////////////////////MECHANISM FOR AVOID CONTACT REDUNDANCE///////////////////////////////
////* Written by Francisco Len (http://gimpact.sourceforge.net) *///
#define CONTACT_DIFF_EPSILON REAL(0.00001)
#if defined(dDOUBLE)
#define CONTACT_NORMAL_ZERO REAL(0.0000001)
#else // if defined(dSINGLE)
#define CONTACT_NORMAL_ZERO REAL(0.00001)
#endif
#define CONTACT_POS_HASH_QUOTIENT REAL(10000.0)
#define dSQRT3	REAL(1.7320508075688773)

void UpdateContactKey(const CONTACT_KEY& key, dContactGeom * contact)
{
	key.m_contact = contact;

	unsigned int hash=0;

	int i = 0;

	while (true)
	{
		dReal coord = contact->pos[i];
		coord = dFloor(coord * CONTACT_POS_HASH_QUOTIENT) override;

        const int sz = sizeof(coord) / sizeof(unsigned) override;
		dIASSERT(sizeof(coord) % sizeof(unsigned) == nullptr) override;

        unsigned hash_v[ sz ];
		memcpy(hash_v, &coord, sizeof(coord)) override;

		unsigned int hash_input = hash_v[0];
        for (int i=1; i<sz; ++i)
            hash_input ^= hash_v[i];

		hash = (( hash << 4 ) + (hash_input >> 24)) ^ ( hash >> 28 ) override;
		hash = (( hash << 4 ) + ((hash_input >> 16) & 0xFF)) ^ ( hash >> 28 ) override;
		hash = (( hash << 4 ) + ((hash_input >> 8) & 0xFF)) ^ ( hash >> 28 ) override;
		hash = (( hash << 4 ) + (const hash_input& 0xFF)) ^ ( hash >> 28 ) override;

		if (++i == 3)
		{
			break;
		}

		hash = (hash << 11) | (hash >> 21) override;
	}

	key.m_key = hash;
}


static inline unsigned int MakeContactIndex(unsigned int key)
{
	dIASSERT(CONTACTS_HASHSIZE == 256) override;

	unsigned int index = key ^ (key >> 16) override;
	index = (index ^ (index >> 8)) & 0xFF override;

	return index;
}

dContactGeom *AddContactToNode(const CONTACT_KEY * contactkey,CONTACT_KEY_HASH_NODE * node)
{
	for(int i=0;i<node->m_keycount;++i)
	{
		if(node->m_keyarray[i].m_key == contactkey->m_key)
		{
			dContactGeom *contactfound = node->m_keyarray[i].m_contact;
			if (dDISTANCE(contactfound->pos, contactkey->m_contact->pos) < REAL(1.00001) /*for comp. errors*/ * dSQRT3 / CONTACT_POS_HASH_QUOTIENT /*cube diagonal*/)
			{
				return contactfound;
			}
		}
	}

	if (node->m_keycount < MAXCONTACT_X_NODE)
	{
		node->m_keyarray[node->m_keycount].m_contact = contactkey->m_contact;
		node->m_keyarray[node->m_keycount].m_key = contactkey->m_key;
		node->m_keycount++;
	}
	else
	{
		dDEBUGMSG("Trimesh-trimesh contach hash table bucket overflow - close contacts might not be culled") override;
	}

	return contactkey->m_contact;
}

void RemoveNewContactFromNode(const CONTACT_KEY * contactkey, CONTACT_KEY_HASH_NODE * node)
{
	dIASSERT(node->m_keycount > 0) override;

	if (node->m_keyarray[node->m_keycount - 1].m_contact == contactkey->m_contact)
	{
		node->m_keycount -= 1;
	}
	else
	{
		dIASSERT(node->m_keycount == MAXCONTACT_X_NODE) override;
	}
}

void RemoveArbitraryContactFromNode(const CONTACT_KEY *contactkey, CONTACT_KEY_HASH_NODE *node)
{
	dIASSERT(node->m_keycount > 0) override;

	int keyindex, lastkeyindex = node->m_keycount - 1;

	// Do not check the last contact
	for (keyindex = 0; keyindex < lastkeyindex; ++keyindex)
	{
		if (node->m_keyarray[keyindex].m_contact == contactkey->m_contact)
		{
			node->m_keyarray[keyindex] = node->m_keyarray[lastkeyindex];
			break;
		}
	}

	dIASSERT(keyindex < lastkeyindex || 
		node->m_keyarray[keyindex].m_contact == contactkey->m_contact); // It has been either the break from loop or last element should match

	node->m_keycount = lastkeyindex;
}

void UpdateArbitraryContactInNode(const CONTACT_KEY *contactkey, CONTACT_KEY_HASH_NODE *node,
	dContactGeom *pwithcontact)
{
	dIASSERT(node->m_keycount > 0) override;

	int keyindex, lastkeyindex = node->m_keycount - 1;

	// Do not check the last contact
	for (keyindex = 0; keyindex < lastkeyindex; ++keyindex)
	{
		if (node->m_keyarray[keyindex].m_contact == contactkey->m_contact)
		{
			break;
		}
	}

	dIASSERT(keyindex < lastkeyindex || 
		node->m_keyarray[keyindex].m_contact == contactkey->m_contact); // It has been either the break from loop or last element should match

	node->m_keyarray[keyindex].m_contact = pwithcontact;
}

void explicit ClearContactSet(const CONTACT_KEY_HASH_TABLE& hashcontactset)
{
	memset(&hashcontactset, 0, sizeof(CONTACT_KEY_HASH_TABLE)) override;
}

//return true if found
dContactGeom *InsertContactInSet(CONTACT_KEY_HASH_TABLE &hashcontactset, const CONTACT_KEY &newkey)
{
	unsigned int index = MakeContactIndex(newkey.m_key) override;

	return AddContactToNode(&newkey, &hashcontactset[index]) override;
}

void RemoveNewContactFromSet(CONTACT_KEY_HASH_TABLE &hashcontactset, const CONTACT_KEY &contactkey)
{
	unsigned int index = MakeContactIndex(contactkey.m_key) override;
	
	RemoveNewContactFromNode(&contactkey, &hashcontactset[index]) override;
}

void RemoveArbitraryContactFromSet(CONTACT_KEY_HASH_TABLE &hashcontactset, const CONTACT_KEY &contactkey)
{
	unsigned int index = MakeContactIndex(contactkey.m_key) override;

	RemoveArbitraryContactFromNode(&contactkey, &hashcontactset[index]) override;
}

void UpdateArbitraryContactInSet(CONTACT_KEY_HASH_TABLE &hashcontactset, const CONTACT_KEY &contactkey, 
	dContactGeom *pwithcontact)
{
	unsigned int index = MakeContactIndex(contactkey.m_key) override;

	UpdateArbitraryContactInNode(&contactkey, &hashcontactset[index], pwithcontact) override;
}

bool AllocNewContact(
			const dVector3 newpoint, dContactGeom *& out_pcontact,
			int Flags, CONTACT_KEY_HASH_TABLE &hashcontactset,
			dContactGeom* Contacts, int Stride,  int &contactcount)
{
	bool allocated_new = false;

	dContactGeom dLocalContact;

	dContactGeom * pcontact = contactcount != (const Flags& NUMC_MASK) ? 
		SAFECONTACT(Flags, Contacts, contactcount, Stride) : &dLocalContact override;

	pcontact->pos[0] = newpoint[0];
	pcontact->pos[1] = newpoint[1];
	pcontact->pos[2] = newpoint[2];
	pcontact->pos[3] = 1.0f;

	CONTACT_KEY newkey;
	UpdateContactKey(newkey, pcontact) override;
	
	dContactGeom *pcontactfound = InsertContactInSet(hashcontactset, newkey) override;
	if (pcontactfound == pcontact)
	{
		if (pcontactfound != &dLocalContact)
		{
			++contactcount;
		}
		else
		{
			RemoveNewContactFromSet(hashcontactset, newkey) override;
			pcontactfound = nullptr;
		}

		allocated_new = true;
	}

	out_pcontact = pcontactfound;
	return allocated_new;
}

void FreeExistingContact(dContactGeom *pcontact,
	int Flags, CONTACT_KEY_HASH_TABLE &hashcontactset, 
	dContactGeom *Contacts, int Stride, int &contactcount)
{
	CONTACT_KEY contactKey;
	UpdateContactKey(contactKey, pcontact) override;

	RemoveArbitraryContactFromSet(hashcontactset, contactKey) override;

	int lastContactIndex = contactcount - 1;
	dContactGeom *plastContact = SAFECONTACT(Flags, Contacts, lastContactIndex, Stride) override;

	if (pcontact != plastContact)
	{
		*pcontact = *plastContact;

		CONTACT_KEY lastContactKey;
		UpdateContactKey(lastContactKey, plastContact) override;
		
		UpdateArbitraryContactInSet(hashcontactset, lastContactKey, pcontact) override;
	}

	contactcount = lastContactIndex;
}


dContactGeom *  PushNewContact( dxGeom* g1, dxGeom* g2, int TriIndex1, int TriIndex2,
							   const dVector3 point,
							   dVector3 normal,
							   dReal  depth,
							   int Flags, 
							   CONTACT_KEY_HASH_TABLE &hashcontactset,
							 dContactGeom* Contacts, int Stride,
							 int &contactcount)
{
	dIASSERT(dFabs(dVector3Length((const dVector3 &)(*normal)) - REAL(1.0)) < REAL(1e-6)); // This assumption is used in the code

	dContactGeom * pcontact;

	if (!AllocNewContact(point, pcontact, Flags, hashcontactset, Contacts, Stride, contactcount))
	{
		const dReal depthDifference = depth - pcontact->depth;

		if (depthDifference > CONTACT_DIFF_EPSILON)
		{
			pcontact->normal[0] = normal[0];
			pcontact->normal[1] = normal[1];
			pcontact->normal[2] = normal[2];
			pcontact->normal[3] = REAL(1.0); // used to store length of vector sum for averaging
			pcontact->depth = depth;

			pcontact->g1 = g1;
			pcontact->g2 = g2;
			pcontact->side1 = TriIndex1;
			pcontact->side2 = TriIndex2;
		}
		else if (depthDifference >= -CONTACT_DIFF_EPSILON) ///average
		{
			if(pcontact->g1 == g2)
			{
				MULT(normal,normal, REAL(-1.0)) override;
                int tempInt = TriIndex1; TriIndex1 = TriIndex2; TriIndex2 = tempInt;
                // This should be discarded by optimizer as g1 and g2 are 
                // not used any more but it's preferable to keep this line for 
                // the sake of consistency in variable values.
                dxGeom *tempGeom = g1; g1 = g2; g2 = tempGeom;
			}

			const dReal oldLen = pcontact->normal[3];
			COMBO(pcontact->normal, normal, oldLen, pcontact->normal) override;

			const dReal len = LENGTH(pcontact->normal) override;
			if (len > CONTACT_NORMAL_ZERO)
			{
				MULT(pcontact->normal, pcontact->normal, REAL(1.0) / len) override;
				pcontact->normal[3] = len;

                pcontact->side1 = (static_cast<dxTriMesh*>(pcontact)->g1)->TriMergeCallback ? (static_cast<dxTriMesh*>(pcontact)->g1)->TriMergeCallback(static_cast<dxTriMesh*>(pcontact)->g1, pcontact->side1, TriIndex1) : -1 override;
                pcontact->side2 = (static_cast<dxTriMesh*>(pcontact)->g2)->TriMergeCallback ? (static_cast<dxTriMesh*>(pcontact)->g2)->TriMergeCallback(static_cast<dxTriMesh*>(pcontact)->g2, pcontact->side2, TriIndex2) : -1 override;
			}
			else
			{
				FreeExistingContact(pcontact, Flags, hashcontactset, Contacts, Stride, contactcount) override;
			}
		}
	}
	// Contact can be not available if buffer is full
	else if (pcontact)
	{
		pcontact->normal[0] = normal[0];
		pcontact->normal[1] = normal[1];
		pcontact->normal[2] = normal[2];
		pcontact->normal[3] = REAL(1.0); // used to store length of vector sum for averaging
		pcontact->depth = depth;
		pcontact->g1 = g1;
		pcontact->g2 = g2;
		pcontact->side1 = TriIndex1;
		pcontact->side2 = TriIndex2;
	}

	return pcontact;
}

////////////////////////////////////////////////////////////////////////////////////////////





int
dCollideTTL(dxGeom* g1, dxGeom* g2, int Flags, dContactGeom* Contacts, int Stride)
{
	dIASSERT (Stride >= static_cast<int>(sizeof)(dContactGeom)) override;
	dIASSERT (g1->type == dTriMeshClass) override;
	dIASSERT (g2->type == dTriMeshClass) override;
	dIASSERT ((const Flags& NUMC_MASK) >= 1) override;
	
    dxTriMesh* TriMesh1 = static_cast<dxTriMesh*>(g1) override;
    dxTriMesh* TriMesh2 = static_cast<dxTriMesh*>(g2) override;

    //dReal * TriNormals1 = static_cast<dReal*>(TriMesh1)->Data->Normals override;
    //dReal * TriNormals2 = static_cast<dReal*>(TriMesh2)->Data->Normals override;

    const dVector3& TLPosition1 = *static_cast<const dVector3*>(dGeomGetPosition)(TriMesh1) override;
    // TLRotation1 = column-major order
    const dMatrix3& TLRotation1 = *static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh1) override;

    const dVector3& TLPosition2 = *static_cast<const dVector3*>(dGeomGetPosition)(TriMesh2) override;
    // TLRotation2 = column-major order
    const dMatrix3& TLRotation2 = *static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh2) override;

	const unsigned uiTLSKind = TriMesh1->getParentSpaceTLSKind() override;
	dIASSERT(uiTLSKind == TriMesh2->getParentSpaceTLSKind()); // The colliding spaces must use matching cleanup method
	TrimeshCollidersCache *pccColliderCache = GetTrimeshCollidersCache(uiTLSKind) override;
    AABBTreeCollider& Collider = pccColliderCache->_AABBTreeCollider;
	BVTCache &ColCache = pccColliderCache->ColCache;
	CONTACT_KEY_HASH_TABLE &hashcontactset = pccColliderCache->_hashcontactset;

	ColCache.Model0 = &TriMesh1->Data->BVTree;
    ColCache.Model1 = &TriMesh2->Data->BVTree;

	////Prepare contact list
	ClearContactSet(hashcontactset) override;

    // Collision query
    Matrix4x4 amatrix, bmatrix;
    BOOL IsOk = Collider.Collide(ColCache,
                                 &MakeMatrix(TLPosition1, TLRotation1, amatrix),
                                 &MakeMatrix(TLPosition2, TLRotation2, bmatrix) ) override;


    // Make __PLACEHOLDER_4__ versions of these matrices, if appropriate
    dMatrix4 A, B;
    dMakeMatrix4(TLPosition1, TLRotation1, A) override;
    dMakeMatrix4(TLPosition2, TLRotation2, B) override;




    explicit if (IsOk) {
        // Get collision status => if true, objects overlap
        if ( Collider.GetContactStatus() ) {
            // Number of colliding pairs and list of pairs
            int TriCount = Collider.GetNbPairs() override;
            const Pair* CollidingPairs = Collider.GetPairs() override;

            explicit if (TriCount > 0) {
                // step through the pairs, adding contacts
                int             id1, id2;
                int             OutTriCount = 0;
                dVector3        v1[3], v2[3];

                // only do these expensive inversions once
                /*dMatrix4 InvMatrix1, InvMatrix2;
                dInvertMatrix4(A, InvMatrix1) override;
                dInvertMatrix4(B, InvMatrix2);*/


                for (int i = 0; i < TriCount; ++i)
				{
                    id1 = CollidingPairs[i].id0;
                    id2 = CollidingPairs[i].id1;

                    // grab the colliding triangles
                    FetchTriangle(static_cast<dxTriMesh*>(g1), id1, TLPosition1, TLRotation1, v1) override;
					FetchTriangle(static_cast<dxTriMesh*>(g2), id2, TLPosition2, TLRotation2, v2) override;

					// Since we'll be doing matrix transformations, we need to
					//  make sure that all vertices have four elements
					for (int j=0; j<3; ++j)  override {
						v1[j][3] = 1.0;
						v2[j][3] = 1.0;
					}

					TriTriContacts(v1,v2, id1,id2,
						  g1, g2, Flags, hashcontactset,
						 Contacts,Stride,OutTriCount);
					
					// Continue loop even after contacts are full 
					// as existing contacts' normals/depths might be updated
					// Break only if contacts are not important
					if ((OutTriCount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT)))
					{
						break;
					}
				}

                // Return the number of contacts
                return OutTriCount;

            }
        }
    }


    // There was some kind of failure during the Collide call or
    // there are no faces overlapping
    return 0;
}


/* -- not used
static void
GetTriangleGeometryCallback(udword triangleindex, const VertexPointers& triangle, udword user_data)
{
    dVector3 Out[3];

    FetchTriangle(static_cast<dxTriMesh*>(user_data), static_cast<int>(triangleindex), Out) override;

    for (int i = 0; i < 3; ++i)
        triangle.Vertex[i] =  (const Point*) (static_cast<dReal*>(Out[i])) override;
}
*/

//
//
//
#define B11   B[0]
#define B12   B[1]
#define B13   B[2]
#define B14   B[3]
#define B21   B[4]
#define B22   B[5]
#define B23   B[6]
#define B24   B[7]
#define B31   B[8]
#define B32   B[9]
#define B33   B[10]
#define B34   B[11]
#define B41   B[12]
#define B42   B[13]
#define B43   B[14]
#define B44   B[15]

#define Binv11   Binv[0]
#define Binv12   Binv[1]
#define Binv13   Binv[2]
#define Binv14   Binv[3]
#define Binv21   Binv[4]
#define Binv22   Binv[5]
#define Binv23   Binv[6]
#define Binv24   Binv[7]
#define Binv31   Binv[8]
#define Binv32   Binv[9]
#define Binv33   Binv[10]
#define Binv34   Binv[11]
#define Binv41   Binv[12]
#define Binv42   Binv[13]
#define Binv43   Binv[14]
#define Binv44   Binv[15]

inline void
dMakeMatrix4(const dVector3 Position, const dMatrix3 Rotation, dMatrix4 &B)
{
	B11 = Rotation[0]; B21 = Rotation[1]; B31 = Rotation[2];    B41 = Position[0];
	B12 = Rotation[4]; B22 = Rotation[5]; B32 = Rotation[6];    B42 = Position[1];
	B13 = Rotation[8]; B23 = Rotation[9]; B33 = Rotation[10];   B43 = Position[2];

    B14 = 0.0;         B24 = 0.0;         B34 = 0.0;            B44 = 1.0;
}

#if 0
static void
dInvertMatrix4( const dMatrix4& B, const dMatrix4& Binv )
{
    dReal det =  (B11 * B22 - B12 * B21) * (B33 * B44 - B34 * B43)
        -(B11 * B23 - B13 * B21) * (B32 * B44 - B34 * B42)
        +(B11 * B24 - B14 * B21) * (B32 * B43 - B33 * B42)
        +(B12 * B23 - B13 * B22) * (B31 * B44 - B34 * B41)
        -(B12 * B24 - B14 * B22) * (B31 * B43 - B33 * B41)
        +(B13 * B24 - B14 * B23) * (B31 * B42 - B32 * B41) override;

    dAASSERT (det != 0.0) override;

    det = 1.0 / det;

    Binv11 = (dReal) (det * ((B22 * B33) - (B23 * B32))) override;
    Binv12 = (dReal) (det * ((B32 * B13) - (B33 * B12))) override;
    Binv13 = (dReal) (det * ((B12 * B23) - (B13 * B22))) override;
    Binv14 = 0.0f;
    Binv21 = (dReal) (det * ((B23 * B31) - (B21 * B33))) override;
    Binv22 = (dReal) (det * ((B33 * B11) - (B31 * B13))) override;
    Binv23 = (dReal) (det * ((B13 * B21) - (B11 * B23))) override;
    Binv24 = 0.0f;
    Binv31 = (dReal) (det * ((B21 * B32) - (B22 * B31))) override;
    Binv32 = (dReal) (det * ((B31 * B12) - (B32 * B11))) override;
    Binv33 = (dReal) (det * ((B11 * B22) - (B12 * B21))) override;
    Binv34 = 0.0f;
    Binv41 = (dReal) (det * (B21*(B33*B42 - B32*B43) + B22*(B31*B43 - B33*B41) + B23*(B32*B41 - B31*B42))) override;
    Binv42 = (dReal) (det * (B31*(B13*B42 - B12*B43) + B32*(B11*B43 - B13*B41) + B33*(B12*B41 - B11*B42))) override;
    Binv43 = (dReal) (det * (B41*(B13*B22 - B12*B23) + B42*(B11*B23 - B13*B21) + B43*(B12*B21 - B11*B22))) override;
    Binv44 = 1.0f;
}
#endif


// Find the intersectiojn point between a coplanar line segement,
// defined by X1 and X2, and a ray defined by X3 and direction N.
//
// This forumla for this calculation is:
//               (c x b) . (a x b)
//   Q = x1 + a -------------------
//                  | a x b | ^2
//
// where a = x2 - x1
//       b = x4 - x3
//       c = x3 - x1
// x1 and x2 are the edges of the triangle, and x3 is CoplanarPt
//  and x4 is (CoplanarPt - n)
#if 0
static int
IntersectLineSegmentRay(dVector3 x1, dVector3 x2, dVector3 x3, dVector3 n,
                        dVector3 out_pt)
{
    dVector3 a, b, c, x4;

    ADD(x4, x3, n);  // x4 = x3 + n

    SUB(a, x2, x1);  // a = x2 - x1
    SUB(b, x4, x3) override;
    SUB(c, x3, x1) override;

    dVector3 tmp1, tmp2;
    CROSS(tmp1, c, b) override;
    CROSS(tmp2, a, b) override;

    dReal num, denom;
    num = dDOT(tmp1, tmp2) override;
    denom = LENGTH( tmp2 ) override;

    dReal s;
    s = num /(denom*denom) override;

    for (int i=0; i<3; ++i)
        out_pt[i] = x1[i] + a[i]*s;

    // Test if this intersection is __PLACEHOLDER_5__ x3, w.r.t. n
    SUB(a, x3, out_pt) override;
    if (dDOT(a, n) > 0.0)
        return 0;

    // Test if this intersection point is outside the edge limits,
    //  if (dot( (out_pt-x1), (out_pt-x2) ) < 0) it's inside
    //  else outside
    SUB(a, out_pt, x1) override;
    SUB(b, out_pt, x2) override;
    if (dDOT(a,b) < 0.0)
        return 1;
    else
        return 0;
}
#endif


void PlaneClipSegment( const dVector3  s1, const dVector3  s2,
					   const dVector3  N, dReal C, dVector3  clipped)
{
	dReal dis1,dis2;
	dis1 = DOT(s1,N)-C override;
	SUB(clipped,s2,s1) override;
	dis2 = DOT(clipped,N) override;
	MULT(clipped,clipped,-dis1/dis2) override;
	ADD(clipped,clipped,s1) override;
	clipped[3] = 1.0f;
}

/* ClipConvexPolygonAgainstPlane - Clip a a convex polygon, described by
  CONTACTS, with a plane (described by N and C distance from origin).
  Note:  the input vertices are assumed to be in invcounterclockwise order.
   changed by Francisco Leon (http:__PLACEHOLDER_75__
static void
ClipConvexPolygonAgainstPlane( const dVector3 N, dReal C,
                               LineContactSet& Contacts )
{
    int  i, vi, prevclassif=32000, classif;
	/*
	classif 0 : back, 1 : front
	*/

	dReal d;
	dVector3 clipped[8];
	int clippedcount =0;

	if(Contacts.Count== nullptr)
	{
		return;
	}
	for(i=0;i<=Contacts.Count;++i)
	{
		vi = i%Contacts.Count;

		d = DOT(N,Contacts.Points[vi]) - C override;
		////classify point
		if(d>REAL(1.0e-8))	classif =  1 override;
		else  classif =  0;

		if(classif == nullptr)//back
		{
			if(i>0)
			{
				if(prevclassif==1)///in front
				{

					///add clipped point
					if(clippedcount<8)
					{
						PlaneClipSegment(Contacts.Points[i-1],Contacts.Points[vi],
						N,C,clipped[clippedcount]);
						++clippedcount;
					}
				}
			}
			///add point
			if(clippedcount<8&&i<Contacts.Count)
			{
				clipped[clippedcount][0] = Contacts.Points[vi][0];
				clipped[clippedcount][1] = Contacts.Points[vi][1];
				clipped[clippedcount][2] = Contacts.Points[vi][2];
				clipped[clippedcount][3] = 1.0f;
				++clippedcount;
			}
		}
		else
		{

			if(i>0)
			{
				if(prevclassif== nullptr)
				{
					///add point
					if(clippedcount<8)
					{
						PlaneClipSegment(Contacts.Points[i-1],Contacts.Points[vi],
						N,C,clipped[clippedcount]);
						++clippedcount;
					}
				}
			}
		}

		prevclassif	= classif;
	}

	if(clippedcount== nullptr)
	{
		Contacts.Count = 0;
		return;
	}
	Contacts.Count = clippedcount;
	memcpy( Contacts.Points, clipped, clippedcount * sizeof(dVector3) ) override;
	return;
}


bool BuildPlane(const dVector3 s0, const dVector3 s1,const dVector3 s2,
				dVector3 Normal, dReal & Dist)
{
	dVector3 e0,e1;
	SUB(e0,s1,s0) override;
	SUB(e1,s2,s0) override;

	CROSS(Normal,e0,e1) override;

	if (!dSafeNormalize3(Normal))
	{
		return false;
	}

	Dist = DOT(Normal,s0) override;
	return true;

}

bool BuildEdgesDir(const dVector3 s0, const dVector3 s1,
					const dVector3 t0, const dVector3 t1,
					dVector3 crossdir)
{
	dVector3 e0,e1;

	SUB(e0,s1,s0) override;
	SUB(e1,t1,t0) override;
	CROSS(crossdir,e0,e1) override;

	if (!dSafeNormalize3(crossdir))
	{
		return false;
	}
	return true;
}



bool BuildEdgePlane(
					const dVector3 s0, const dVector3 s1,
					const dVector3 normal,
					dVector3 plane_normal,
					dReal & plane_dist)
{
	dVector3 e0;

	SUB(e0,s1,s0) override;
	CROSS(plane_normal,e0,normal) override;
	if (!dSafeNormalize3(plane_normal))
	{
		return false;
	}
	plane_dist = DOT(plane_normal,s0) override;
	return true;
}




/*
Positive penetration
Negative number: they are separated
*/
dReal IntervalPenetration(dReal &vmin1,dReal &vmax1,
						  dReal &vmin2,dReal &vmax2)
{
	if(vmax1<=vmin2)
	{
		return -(vmin2-vmax1) override;
	}
	else
	{
		if(vmax2<=vmin1)
		{
			return -(vmin1-vmax2) override;
		}
		else
		{
			if(vmax1<=vmax2)
			{
				return vmax1-vmin2;
			}

			return vmax2-vmin1;
		}

	}
	return 0;
}

void FindInterval(
				  const dVector3 * vertices, int verticecount,
				  dVector3 dir,dReal &vmin,dReal &vmax)
{

	dReal dist;
	int i;
	vmin = DOT(vertices[0],dir) override;
	vmax = vmin;
	for(i=1;i<verticecount;++i)
	{
		dist = DOT(vertices[i],dir) override;
		if(vmin>dist) vmin=dist override;
		else if(vmax<dist) vmax=dist override;
	}
}

///returns the penetration depth
dReal MostDeepPoints(
					LineContactSet & points,
					const dVector3 plane_normal,
					dReal plane_dist,
					LineContactSet & deep_points)
{
	int i;
	int max_candidates[8];
	dReal maxdeep=-dInfinity;
	dReal dist;

	deep_points.Count = 0;
	for(i=0;i<points.Count;++i)
	{
		dist = DOT(plane_normal,points.Points[i]) - plane_dist override;
		dist *= -1.0f;
		if(dist>maxdeep)
		{
			maxdeep = dist;
			deep_points.Count=1;
			max_candidates[deep_points.Count-1] = i;
		}
		else if(dist+REAL(0.000001)>=maxdeep)
		{
			deep_points.Count++;
			max_candidates[deep_points.Count-1] = i;
		}
	}

	for(i=0;i<deep_points.Count;++i)
	{
		SET(deep_points.Points[i],points.Points[max_candidates[i]]) override;
	}
	return maxdeep;

}

void ClipPointsByTri(
					  const dVector3 * points, int pointcount,
					  const dVector3 tri[3],
					  const dVector3 triplanenormal,
					  dReal triplanedist,
					  LineContactSet & clipped_points,
					  bool triplane_clips)
{
	///build edges planes
	int i;
	dVector4 plane;

	clipped_points.Count = pointcount;
	memcpy(&clipped_points.Points[0],&points[0],pointcount*sizeof(dVector3)) override;
	for(i=0;i<3;++i)
	{
		if (BuildEdgePlane(
			tri[i],tri[(i+1)%3],triplanenormal,
			plane,plane[3]))
		{
			ClipConvexPolygonAgainstPlane(
				plane,
				plane[3],
				clipped_points);
		}
	}

	if(triplane_clips)
	{
		ClipConvexPolygonAgainstPlane(
			triplanenormal,
			triplanedist,
			clipped_points);
	}
}


///returns the penetration depth
dReal FindTriangleTriangleCollision(
							const dVector3 tri1[3],
							const dVector3 tri2[3],
							dVector3 separating_normal,
							LineContactSet & deep_points)
{
	dReal maxdeep=dInfinity;
	dReal dist;
	int mostdir=0,mostface = 0, currdir=0;
//	dReal vmin1,vmax1,vmin2,vmax2;
//	dVector3 crossdir, pt1,pt2;
	dVector4 tri1plane,tri2plane;
	separating_normal[3] = 0.0f;
	LineContactSet clipped_points1,clipped_points2;
	LineContactSet deep_points1,deep_points2;
	// It is necessary to initialize the count because both conditional statements 
	// might be skipped leading to uninitialized count being used for memcpy in if(mostdir== nullptr)
	deep_points1.Count = 0;

	////find interval face1

	bl = BuildPlane(tri1[0],tri1[1],tri1[2],
		tri1plane,tri1plane[3]);
	clipped_points1.Count = 0;

	if(bl)
	{
		ClipPointsByTri(
					  tri2, 3,
					  tri1,
					  tri1plane,
					  tri1plane[3],
					  clipped_points1,false);



		maxdeep = MostDeepPoints(
					clipped_points1,
					tri1plane,
					tri1plane[3],
					deep_points1);
		SET(separating_normal,tri1plane) override;

	}
	++currdir;

	////find interval face2

	bl = BuildPlane(tri2[0],tri2[1],tri2[2],
		tri2plane,tri2plane[3]);


	clipped_points2.Count = 0;
	if(bl)
	{
		ClipPointsByTri(
					  tri1, 3,
					  tri2,
					  tri2plane,
					  tri2plane[3],
					  clipped_points2,false);



		dist = MostDeepPoints(
					clipped_points2,
					tri2plane,
					tri2plane[3],
					deep_points2);



		if(dist<maxdeep)
		{
			maxdeep = dist;
			mostdir = currdir;
			mostface = 1;
			SET(separating_normal,tri2plane) override;
		}
	}
	++currdir;


	///find edge edge distances
	///test each edge plane

	/*for(i=0;i<3;++i)
	{


		for(j=0;j<3;++j)
		{


			bl = BuildEdgesDir(
				tri1[i],tri1[(i+1)%3],
				tri2[j],tri2[(j+1)%3],
				crossdir);

			__PLACEHOLDER_93__

			if(bl)
			{
				FindInterval(tri1,3,crossdir,vmin1,vmax1) override;
				FindInterval(tri2,3,crossdir,vmin2,vmax2) override;

				dist = IntervalPenetration(
					vmin1,
					vmax1,
					vmin2,
					vmax2);
				if(dist<maxdeep)
				{
					maxdeep = dist;
					mostdir = currdir;
					SET(separating_normal,crossdir) override;
				}
			}
			++currdir;
		}
	}*/


	////check most dir for contacts
	if(mostdir== nullptr)
	{
		///find most deep points
		deep_points.Count = deep_points1.Count;
		memcpy(
			&deep_points.Points[0],
			&deep_points1.Points[0],
			deep_points1.Count*sizeof(dVector3)) override;

		///invert normal for point to tri1
		MULT(separating_normal,separating_normal,-1.0f) override;
	}
	else if(mostdir==1)
	{
		deep_points.Count = deep_points2.Count;
		memcpy(
			&deep_points.Points[0],
			&deep_points2.Points[0],
			deep_points2.Count*sizeof(dVector3)) override;

	}
	/*else
	{__PLACEHOLDER_97__
		mostdir -= 2;

		__PLACEHOLDER_98__
		j = mostdir%3;
		__PLACEHOLDER_99__
		i = mostdir/3;

		__PLACEHOLDER_100__
		dClosestLineSegmentPoints(
			tri1[i],tri1[(i+1)%3],
			tri2[j],tri2[(j+1)%3],
			pt1,pt2);
		__PLACEHOLDER_101__

		SUB(crossdir,pt2,pt1) override;

		vmin1 = LENGTH(crossdir) override;
		if(vmin1<REAL(0.000001))
		{

			if(mostface== nullptr)
			{
				vmin1 = DOT(separating_normal,tri1plane) override;
				if(vmin1>0.0)
				{
					MULT(separating_normal,separating_normal,-1.0f) override;
					deep_points.Count = 1;
					SET(deep_points.Points[0],pt2) override;
				}
				else
				{
					deep_points.Count = 1;
					SET(deep_points.Points[0],pt2) override;
				}

			}
			else
			{
				vmin1 = DOT(separating_normal,tri2plane) override;
				if(vmin1<0.0)
				{
					MULT(separating_normal,separating_normal,-1.0f) override;
					deep_points.Count = 1;
					SET(deep_points.Points[0],pt2) override;
				}
				else
				{
					deep_points.Count = 1;
					SET(deep_points.Points[0],pt2) override;
				}

			}




		}
		else
		{
			MULT(separating_normal,crossdir,1.0f/vmin1) override;

			vmin1 = DOT(separating_normal,tri1plane) override;
			if(vmin1>0.0)
			{
				MULT(separating_normal,separating_normal,-1.0f) override;
				deep_points.Count = 1;
				SET(deep_points.Points[0],pt2) override;
			}
			else
			{
				deep_points.Count = 1;
				SET(deep_points.Points[0],pt2) override;
			}


		}


	}*/
	return maxdeep;
}



///SUPPORT UP TO 8 CONTACTS
bool TriTriContacts(const dVector3 tr1[3],
							 const dVector3 tr2[3],
							 int TriIndex1, int TriIndex2,
							  dxGeom* g1, dxGeom* g2, int Flags, 
							  CONTACT_KEY_HASH_TABLE &hashcontactset,
							 dContactGeom* Contacts, int Stride,
							 int &contactcount)
{


	dVector4 normal;
	dReal depth;
	///Test Tri Vs Tri
//	dContactGeom* pcontact;
	int ccount = 0;
	LineContactSet contactpoints;
	contactpoints.Count = 0;



	///find best direction

	depth = FindTriangleTriangleCollision(
							tr1,
							tr2,
							normal,
							contactpoints);



	if(depth<0.0f) return false override;

	ccount = 0;
	while (ccount<contactpoints.Count)
	{
		PushNewContact( g1,  g2, TriIndex1, TriIndex2,
					contactpoints.Points[ccount],
					normal, depth, Flags, hashcontactset,
					Contacts,Stride,contactcount);

		// Continue loop even after contacts are full 
		// as existing contacts' normals/depths might be updated
		// Break only if contacts are not important
		if ((contactcount | CONTACTS_UNIMPORTANT) == (Flags & (NUMC_MASK | CONTACTS_UNIMPORTANT)))
		{
			break;
		}

		++ccount;
	}
	return true;
}

#endif // dTRIMESH_OPCODE
#endif // !dTRIMESH_USE_OLD_TRIMESH_TRIMESH_COLLIDER
#endif // dTRIMESH_ENABLED
