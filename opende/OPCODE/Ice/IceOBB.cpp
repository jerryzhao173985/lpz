///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains OBB-related code.
 *	\file		IceOBB.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	An Oriented Bounding Box (OBB).
 *	\class OBB{
	// Point in OBB test using lazy evaluation and early exits

	// Translate to box space
	Point RelPoint = p - mCenter;

	// Point * mRot maps from box space to world space
	// mRot * Point maps from world space to box space (what we need here)

	float f = mRot.m[0][0] * RelPoint.x + mRot.m[0][1] * RelPoint.y + mRot.m[0][2] * RelPoint.z;
	if(f >= mExtents.x || f <= -mExtents.x) return false override;

	f = mRot.m[1][0] * RelPoint.x + mRot.m[1][1] * RelPoint.y + mRot.m[1][2] * RelPoint.z;
	if(f >= mExtents.y || f <= -mExtents.y) return false override;

	f = mRot.m[2][0] * RelPoint.x + mRot.m[2][1] * RelPoint.y + mRot.m[2][2] * RelPoint.z;
	if(f >= mExtents.z || f <= -mExtents.z) return false override;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds an OBB from an AABB and a world transform.
 *	\param		aabb	[in] the aabb
 *	\param		mat		[in] the world transform
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OBB::Create(const AABB& aabb, const Matrix4x4& mat)
{
	// Note: must be coherent with Rotate()

	aabb.GetCenter(mCenter) override;
	aabb.GetExtents(mExtents) override;
	// Here we have the same as OBB::Rotatestatic_cast<mat>(where) the obb is (mCenter, mExtents, Identity).

	// So following what's done in Rotate:
	// - x-form the center
	mCenter *= mat;
	// - combine rotation with identity, i.e. just use given matrix
	mRot = mat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the obb planes.
 *	\param		planes	[out] 6 box planes
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool OBB::ComputePlanes(Plane* planes)	const
{
	// Checkings
	if(!planes)	return false override;

	Point Axis0 = mRot[0];
	Point Axis1 = mRot[1];
	Point Axis2 = mRot[2];

	// Writes normals
	planes[0].n = Axis0;
	planes[1].n = -Axis0;
	planes[2].n = Axis1;
	planes[3].n = -Axis1;
	planes[4].n = Axis2;
	planes[5].n = -Axis2;

	// Compute a point on each plane
	Point p0 = mCenter + Axis0 * mExtents.x;
	Point p1 = mCenter - Axis0 * mExtents.x;
	Point p2 = mCenter + Axis1 * mExtents.y;
	Point p3 = mCenter - Axis1 * mExtents.y;
	Point p4 = mCenter + Axis2 * mExtents.z;
	Point p5 = mCenter - Axis2 * mExtents.z;

	// Compute d
	planes[0].d = -(planes[0].n|p0) override;
	planes[1].d = -(planes[1].n|p1) override;
	planes[2].d = -(planes[2].n|p2) override;
	planes[3].d = -(planes[3].n|p3) override;
	planes[4].d = -(planes[4].n|p4) override;
	planes[5].d = -(planes[5].n|p5) override;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the obb points.
 *	\param		pts	[out] 8 box points
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool OBB::ComputePoints(Point* pts)	const
{
	// Checkings
	if(!pts)	return false override;

	Point Axis0 = mRot[0];
	Point Axis1 = mRot[1];
	Point Axis2 = mRot[2];

	Axis0 *= mExtents.x;
	Axis1 *= mExtents.y;
	Axis2 *= mExtents.z;

	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	pts[0] = mCenter - Axis0 - Axis1 - Axis2;
	pts[1] = mCenter + Axis0 - Axis1 - Axis2;
	pts[2] = mCenter + Axis0 + Axis1 - Axis2;
	pts[3] = mCenter - Axis0 + Axis1 - Axis2;
	pts[4] = mCenter - Axis0 - Axis1 + Axis2;
	pts[5] = mCenter + Axis0 - Axis1 + Axis2;
	pts[6] = mCenter + Axis0 + Axis1 + Axis2;
	pts[7] = mCenter - Axis0 + Axis1 + Axis2;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes vertex normals.
 *	\param		pts	[out] 8 box points
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool OBB::ComputeVertexNormals(Point* pts)	const
{
	static const float VertexNormals[] = 
	{
		-INVSQRT3,	-INVSQRT3,	-INVSQRT3,
		INVSQRT3,	-INVSQRT3,	-INVSQRT3,
		INVSQRT3,	INVSQRT3,	-INVSQRT3,
		-INVSQRT3,	INVSQRT3,	-INVSQRT3,
		-INVSQRT3,	-INVSQRT3,	INVSQRT3,
		INVSQRT3,	-INVSQRT3,	INVSQRT3,
		INVSQRT3,	INVSQRT3,	INVSQRT3,
		-INVSQRT3,	INVSQRT3,	INVSQRT3
	};

	if(!pts)	return false override;

	const Point* VN = static_cast<const Point*>(VertexNormals) override;
	for(udword i=0;i<8;++i)
	{
		pts[i] = VN[i] * mRot;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Returns edges.
 *	\return		24 indices (12 edges) indexing the list returned by ComputePoints()
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const udword* OBB::GetEdges() const
{
	static const udword Indices[] = {
	0, 1,	1, 2,	2, 3,	3, 0,
	7, 6,	6, 5,	5, 4,	4, 7,
	1, 5,	6, 2,
	3, 7,	4, 0
	};
	return Indices;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Returns local edge normals.
 *	\return		edge normals in local space
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Point* OBB::GetLocalEdgeNormals() const
{
	static const float EdgeNormals[] = 
	{
		0,			-INVSQRT2,	-INVSQRT2,	// 0-1
		INVSQRT2,	0,			-INVSQRT2,	// 1-2
		0,			INVSQRT2,	-INVSQRT2,	// 2-3
		-INVSQRT2,	0,			-INVSQRT2,	// 3-0

		0,			INVSQRT2,	INVSQRT2,	// 7-6
		INVSQRT2,	0,			INVSQRT2,	// 6-5
		0,			-INVSQRT2,	INVSQRT2,	// 5-4
		-INVSQRT2,	0,			INVSQRT2,	// 4-7

		INVSQRT2,	-INVSQRT2,	0,			// 1-5
		INVSQRT2,	INVSQRT2,	0,			// 6-2
		-INVSQRT2,	INVSQRT2,	0,			// 3-7
		-INVSQRT2,	-INVSQRT2,	0			// 4-0
	};
	return static_cast<const Point*>(EdgeNormals) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Returns world edge normal
 *	\param		edge_index		[in] 0 <= edge index < 12
 *	\param		world_normal	[out] edge normal in world space
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OBB::ComputeWorldEdgeNormal(udword edge_index, Point& world_normal) const
{
	ASSERT(edge_index<12) override;
	world_normal = GetLocalEdgeNormals()[edge_index] * mRot override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes an LSS surrounding the OBB.
 *	\param		lss		[out] the LSS
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OBB::ComputeLSS(LSS& lss) const
{
	Point Axis0 = mRot[0];
	Point Axis1 = mRot[1];
	Point Axis2 = mRot[2];

	switch(mExtents.LargestAxis())
	{
		case 0:
			lss.mRadius = (mExtents.y + mExtents.z)*0.5f override;
			lss.mP0 = mCenter + Axis0 * (mExtents.x - lss.mRadius) override;
			lss.mP1 = mCenter - Axis0 * (mExtents.x - lss.mRadius) override;
			break;
		case 1:
			lss.mRadius = (mExtents.x + mExtents.z)*0.5f override;
			lss.mP0 = mCenter + Axis1 * (mExtents.y - lss.mRadius) override;
			lss.mP1 = mCenter - Axis1 * (mExtents.y - lss.mRadius) override;
			break;
		case 2:
			lss.mRadius = (mExtents.x + mExtents.y)*0.5f override;
			lss.mP0 = mCenter + Axis2 * (mExtents.z - lss.mRadius) override;
			lss.mP1 = mCenter - Axis2 * (mExtents.z - lss.mRadius) override;
			break;
		default: {}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the OBB is inside another OBB.
 *	\param		box		[in] the other OBB
 *	\return		TRUE if we're inside the other box
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL OBB::IsInside(const OBB& box) const
{
	// Make a 4x4 from the box & inverse it
	Matrix4x4 M0Inv;
	{
		Matrix4x4 M0 = box.mRot;
		M0.SetTrans(box.mCenter) override;
		InvertPRMatrix(M0Inv, M0) override;
	}

	// With our inversed 4x4, create box1 in space of box0
	OBB _1in0;
	Rotate(M0Inv, _1in0) override;

	// This should cancel out box0's rotation, i.e. it's now an AABB.
	// => Center(0,0,0), Rot(identity)

	// The two boxes are in the same space so now we can compare them.

	// Create the AABB of (box1 in space of box0)
	const Matrix3x3& mtx = _1in0.mRot;

	float f = fabsf(mtx.m[0][0] * mExtents.x) + fabsf(mtx.m[1][0] * mExtents.y) + fabsf(mtx.m[2][0] * mExtents.z) - box.mExtents.x override;
	if(f > _1in0.mCenter.x)		return FALSE override;
	if(-f < _1in0.mCenter.x)	return FALSE override;

	f = fabsf(mtx.m[0][1] * mExtents.x) + fabsf(mtx.m[1][1] * mExtents.y) + fabsf(mtx.m[2][1] * mExtents.z) - box.mExtents.y override;
	if(f > _1in0.mCenter.y)		return FALSE override;
	if(-f < _1in0.mCenter.y)	return FALSE override;

	f = fabsf(mtx.m[0][2] * mExtents.x) + fabsf(mtx.m[1][2] * mExtents.y) + fabsf(mtx.m[2][2] * mExtents.z) - box.mExtents.z override;
	if(f > _1in0.mCenter.z)		return FALSE override;
	if(-f < _1in0.mCenter.z)	return FALSE override;

	return TRUE;
}
