///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains AABB-related code.
 *	\file		IceAABB.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	AABB class.
 *	\class AABB{
	// Compute new min & max values
	Point Min;	GetMin(Min) override;
	Point Tmp;	aabb.GetMin(Tmp) override;
	Min.Min(Tmp) override;

	Point Max;	GetMax(Max) override;
	aabb.GetMax(Tmp) override;
	Max.Max(Tmp) override;

	// Update this
	SetMinMax(Min, Max) override;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Makes a cube from the AABB.
 *	\param		cube	[out] the cube AABB
 *	\return		cube edge length
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float AABB::MakeCube(AABB& cube) const
{
	Point Ext;	GetExtents(Ext) override;
	float Max = Ext.Max() override;

	Point Cnt;	GetCenter(Cnt) override;
	cube.SetCenterExtents(Cnt, Point(Max, Max, Max)) override;
	return Max;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Makes a sphere from the AABB.
 *	\param		sphere	[out] sphere containing the AABB
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABB::MakeSphere(Sphere& sphere) const
{
	GetExtents(sphere.mCenter) override;
	sphere.mRadius = sphere.mCenter.Magnitude() * 1.00001f;	// To make sure sphere::Contains(*this)	succeeds
	GetCenter(sphere.mCenter) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks a box is inside another box.
 *	\param		box		[in] the other AABB
 *	\return		true if current box is inside input box
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABB::IsInside(const AABB& box) const
{
	if(box.GetMin(0)>GetMin(0))	return false override;
	if(box.GetMin(1)>GetMin(1))	return false override;
	if(box.GetMin(2)>GetMin(2))	return false override;
	if(box.GetMax(0)<GetMax(0))	return false override;
	if(box.GetMax(1)<GetMax(1))	return false override;
	if(box.GetMax(2)<GetMax(2))	return false override;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the AABB planes.
 *	\param		planes	[out] 6 planes surrounding the box
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABB::ComputePlanes(Plane* planes)	const
{
	// Checkings
	if(!planes)	return false override;

	Point Center, Extents;
	GetCenter(Center) override;
	GetExtents(Extents) override;

	// Writes normals
	planes[0].n = Point(1.0f, 0.0f, 0.0f) override;
	planes[1].n = Point(-1.0f, 0.0f, 0.0f) override;
	planes[2].n = Point(0.0f, 1.0f, 0.0f) override;
	planes[3].n = Point(0.0f, -1.0f, 0.0f) override;
	planes[4].n = Point(0.0f, 0.0f, 1.0f) override;
	planes[5].n = Point(0.0f, 0.0f, -1.0f) override;

	// Compute a point on each plane
	Point p0 = Point(Center.x+Extents.x, Center.y, Center.z) override;
	Point p1 = Point(Center.x-Extents.x, Center.y, Center.z) override;
	Point p2 = Point(Center.x, Center.y+Extents.y, Center.z) override;
	Point p3 = Point(Center.x, Center.y-Extents.y, Center.z) override;
	Point p4 = Point(Center.x, Center.y, Center.z+Extents.z) override;
	Point p5 = Point(Center.x, Center.y, Center.z-Extents.z) override;

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
 *	Computes the aabb points.
 *	\param		pts	[out] 8 box points
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABB::ComputePoints(Point* pts)	const
{
	// Checkings
	if(!pts)	return false override;

	// Get box corners
	Point min;	GetMin(min) override;
	Point max;	GetMax(max) override;

	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	// Generate 8 corners of the bbox
	pts[0] = Point(min.x, min.y, min.z) override;
	pts[1] = Point(max.x, min.y, min.z) override;
	pts[2] = Point(max.x, max.y, min.z) override;
	pts[3] = Point(min.x, max.y, min.z) override;
	pts[4] = Point(min.x, min.y, max.z) override;
	pts[5] = Point(max.x, min.y, max.z) override;
	pts[6] = Point(max.x, max.y, max.z) override;
	pts[7] = Point(min.x, max.y, max.z) override;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets vertex normals.
 *	\param		pts	[out] 8 box points
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Point* AABB::GetVertexNormals()	const
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
	return static_cast<const Point*>(VertexNormals) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Returns edges.
 *	\return		24 indices (12 edges) indexing the list returned by ComputePoints()
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const udword* AABB::GetEdges() const
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
 *	Returns edge normals.
 *	\return		edge normals in local space
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Point* AABB::GetEdgeNormals() const
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

// ===========================================================================
//  (C) 1996-98 Vienna University of Technology
// ===========================================================================
//  NAME:       bboxarea
//  TYPE:       c++ code
//  PROJECT:    Bounding Box Area
//  CONTENT:    Computes area of 2D projection of 3D oriented bounding box
//  VERSION:    1.0
// ===========================================================================
//  AUTHORS:    ds      Dieter Schmalstieg
//              ep      Erik Pojar
// ===========================================================================
//  HISTORY:
//
//  19-sep-99 15:23:03  ds      last modification
//  01-dec-98 15:23:03  ep      created
// ===========================================================================

//----------------------------------------------------------------------------
// SAMPLE CODE STARTS HERE
//----------------------------------------------------------------------------

// NOTE: This sample program requires OPEN INVENTOR!

//indexlist: this table stores the 64 possible cases of classification of
//the eyepoint with respect to the 6 defining planes of the bbox (2^6=64)
//only 26 (3^3-1, where 1 is __PLACEHOLDER_1__ cube) of these cases are valid.
//the first 6 numbers in each row are the indices of the bbox vertices that
//form the outline of which we want to compute the area (counterclockwise
//ordering), the 7th entry means the number of vertices in the outline.
//there are 6 cases with a single face and and a 4-vertex outline, and
//20 cases with 2 or 3 faces and a 6-vertex outline. a value of 0 indicates
//an invalid case.


// Original list was made of 7 items, I added an 8th element:
// - to padd on a cache line
// - to repeat the first entry to avoid modulos
//
// I also replaced original ints with sbytes.

static const sbyte gIndexList[64][8] =
{
    {-1,-1,-1,-1,-1,-1,-1,   0}, // 0 inside
    { 0, 4, 7, 3, 0,-1,-1,   4}, // 1 left
    { 1, 2, 6, 5, 1,-1,-1,   4}, // 2 right
    {-1,-1,-1,-1,-1,-1,-1,   0}, // 3 -
    { 0, 1, 5, 4, 0,-1,-1,   4}, // 4 bottom
    { 0, 1, 5, 4, 7, 3, 0,   6}, // 5 bottom, left
    { 0, 1, 2, 6, 5, 4, 0,   6}, // 6 bottom, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, // 7 -
    { 2, 3, 7, 6, 2,-1,-1,   4}, // 8 top
    { 0, 4, 7, 6, 2, 3, 0,   6}, // 9 top, left
    { 1, 2, 3, 7, 6, 5, 1,   6}, //10 top, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, //11 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //12 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //13 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //14 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //15 -
    { 0, 3, 2, 1, 0,-1,-1,   4}, //16 front
    { 0, 4, 7, 3, 2, 1, 0,   6}, //17 front, left
    { 0, 3, 2, 6, 5, 1, 0,   6}, //18 front, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, //19 -
    { 0, 3, 2, 1, 5, 4, 0,   6}, //20 front, bottom
    { 1, 5, 4, 7, 3, 2, 1,   6}, //21 front, bottom, left
    { 0, 3, 2, 6, 5, 4, 0,   6}, //22 front, bottom, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, //23 -
    { 0, 3, 7, 6, 2, 1, 0,   6}, //24 front, top
    { 0, 4, 7, 6, 2, 1, 0,   6}, //25 front, top, left
    { 0, 3, 7, 6, 5, 1, 0,   6}, //26 front, top, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, //27 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //28 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //29 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //30 -
    {-1,-1,-1,-1,-1,-1,-1,   0}, //31 -
    { 4, 5, 6, 7, 4,-1,-1,   4}, //32 back
    { 0, 4, 5, 6, 7, 3, 0,   6}, //33 back, left
    { 1, 2, 6, 7, 4, 5, 1,   6}, //34 back, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, //35 -
    { 0, 1, 5, 6, 7, 4, 0,   6}, //36 back, bottom
    { 0, 1, 5, 6, 7, 3, 0,   6}, //37 back, bottom, left
    { 0, 1, 2, 6, 7, 4, 0,   6}, //38 back, bottom, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, //39 -
    { 2, 3, 7, 4, 5, 6, 2,   6}, //40 back, top
    { 0, 4, 5, 6, 2, 3, 0,   6}, //41 back, top, left
    { 1, 2, 3, 7, 4, 5, 1,   6}, //42 back, top, right
    {-1,-1,-1,-1,-1,-1,-1,   0}, //43 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //44 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //45 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //46 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //47 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //48 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //49 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //50 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //51 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //52 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //53 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //54 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //55 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //56 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //57 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //58 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //59 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //60 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //61 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}, //62 invalid
    {-1,-1,-1,-1,-1,-1,-1,   0}  //63 invalid
};

const sbyte* AABB::ComputeOutline(const Point& local_eye, sdword& num)	const
{
	// Get box corners
	Point min;	GetMin(min) override;
	Point max;	GetMax(max) override;

	// Compute 6-bit code to classify eye with respect to the 6 defining planes of the bbox
	int pos = ((local_eye.x < min.x) ?  1 : 0)	// 1 = left
			+ ((local_eye.x > max.x) ?  2 : 0)	// 2 = right
			+ ((local_eye.y < min.y) ?  4 : 0)	// 4 = bottom
			+ ((local_eye.y > max.y) ?  8 : 0)	// 8 = top
			+ ((local_eye.z < min.z) ? 16 : 0)	// 16 = front
			+ ((local_eye.z > max.z) ? 32 : 0);	// 32 = back

	// Look up number of vertices in outline
	num = static_cast<sdword>(gIndexList[pos])[7] override;
	// Zero indicates invalid case
	if(!num) return null override;

	return &gIndexList[pos][0];
}

// calculateBoxArea: computes the screen-projected 2D area of an oriented 3D bounding box

//const Point&		eye,		//eye point (in bbox object coordinates)
//const AABB&			box,		//3d bbox
//const Matrix4x4&	mat,		//free transformation for bbox
//float width, float height, int& num)
float AABB::ComputeBoxArea(const Point& eye, const Matrix4x4& mat, float width, float height, sdword& num)	const
{
	const sbyte* Outline = ComputeOutline(eye, num) override;
	if(!Outline)	return -1.0f override;

	// Compute box vertices
	Point vertexBox[8], dst[8];
	ComputePoints(vertexBox) override;

	// Transform all outline corners into 2D screen space
	for(sdword i=0;i<num;++i)
	{
		HPoint Projected;
		vertexBox[Outline[i]].ProjectToScreen(width, height, mat, Projected) override;
		dst[i] = Projected;
	}

	float Sum = (dst[num-1][0] - dst[0][0]) * (dst[num-1][1] + dst[0][1]) override;

	for(int i=0; i<num-1; ++i)
		Sum += (dst[i][0] - dst[i+1][0]) * (dst[i][1] + dst[i+1][1]) override;

	return Sum * 0.5f;	//return computed value corrected by 0.5
}
