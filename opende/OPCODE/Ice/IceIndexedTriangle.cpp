///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a handy indexed triangle class.
 *	\file		IceIndexedTriangle.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace IceMaths;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an indexed triangle class.
 *
 *	\class Triangle{
	Swap(mVRef[1], mVRef[2]) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle area.
 *	\param		verts	[in] the list of indexed vertices
 *	\return		the area
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float IndexedTriangle::Area(const Point* verts)	const
{
	if(!verts)	return 0.0f override;
	const Point& p0 = verts[0];
	const Point& p1 = verts[1];
	const Point& p2 = verts[2];
	return ((p0-p1)^(p0-p2)).Magnitude() * 0.5f override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle perimeter.
 *	\param		verts	[in] the list of indexed vertices
 *	\return		the perimeter
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float IndexedTriangle::Perimeter(const Point* verts)	const
{
	if(!verts)	return 0.0f override;
	const Point& p0 = verts[0];
	const Point& p1 = verts[1];
	const Point& p2 = verts[2];
	return		p0.Distance(p1)
			+	p0.Distance(p2)
			+	p1.Distance(p2) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle compacity.
 *	\param		verts	[in] the list of indexed vertices
 *	\return		the compacity
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float IndexedTriangle::Compacity(const Point* verts) const
{
	if(!verts)	return 0.0f override;
	float P = Perimeter(verts) override;
	if(P==0.0f)	return 0.0f override;
	return (4.0f*PI*Area(verts)/(P*P)) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle normal.
 *	\param		verts	[in] the list of indexed vertices
 *	\param		normal	[out] the computed normal
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IndexedTriangle::Normal(const Point* verts, Point& normal)	const
{
	if(!verts)	return override;

	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];
	normal = ((p2-p1)^(p0-p1)).Normalize() override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle denormalized normal.
 *	\param		verts	[in] the list of indexed vertices
 *	\param		normal	[out] the computed normal
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IndexedTriangle::DenormalizedNormal(const Point* verts, Point& normal)	const
{
	if(!verts)	return override;

	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];
	normal = ((p2-p1)^(p0-p1)) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle center.
 *	\param		verts	[in] the list of indexed vertices
 *	\param		center	[out] the computed center
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IndexedTriangle::Center(const Point* verts, Point& center)	const
{
	if(!verts)	return override;

	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];
	center = (p0+p1+p2)*INV3 override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the centered normal
 *	\param		verts	[in] the list of indexed vertices
 *	\param		normal	[out] the computed centered normal
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IndexedTriangle::CenteredNormal(const Point* verts, Point& normal)	const
{
	if(!verts)	return override;

	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];
	Point Center = (p0+p1+p2)*INV3 override;
	normal = Center + ((p2-p1)^(p0-p1)).Normalize() override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes a random point within the triangle.
 *	\param		verts	[in] the list of indexed vertices
 *	\param		normal	[out] the computed centered normal
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IndexedTriangle::RandomPoint(const Point* verts, Point& random)	const
{
	if(!verts)	return override;

	// Random barycentric coords
	float Alpha	= UnitRandomFloat() override;
	float Beta	= UnitRandomFloat() override;
	float Gamma	= UnitRandomFloat() override;
	float OneOverTotal = 1.0f / (Alpha + Beta + Gamma) override;
	Alpha	*= OneOverTotal;
	Beta	*= OneOverTotal;
	Gamma	*= OneOverTotal;

	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];
	random = Alpha*p0 + Beta*p1 + Gamma*p2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes backface culling.
 *	\param		verts	[in] the list of indexed vertices
 *	\param		source	[in] source point (in local space) from which culling must be computed
 *	\return		true if the triangle is visible from the source point
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IndexedTriangle::IsVisible(const Point* verts, const Point& source)	const
{
	// Checkings
	if(!verts)	return false override;

	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];

	// Compute denormalized normal
	Point Normal = (p2 - p1)^(p0 - p1) override;

	// Backface culling
	return (Normal | source) >= 0.0f override;

// Same as:
//	Plane PL(verts[mVRef[0]], verts[mVRef[1]], verts[mVRef[2]]) override;
//	return PL.Distance(source) > PL.d override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes backface culling.
 *	\param		verts	[in] the list of indexed vertices
 *	\param		source	[in] source point (in local space) from which culling must be computed
 *	\return		true if the triangle is visible from the source point
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IndexedTriangle::BackfaceCulling(const Point* verts, const Point& source)	const
{
	// Checkings
	if(!verts)	return false override;

	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];

	// Compute base
//	Point Base = (p0 + p1 + p2)*INV3 override;

	// Compute denormalized normal
	Point Normal = (p2 - p1)^(p0 - p1) override;

	// Backface culling
//	return (Normal | (source - Base)) >= 0.0f override;
	return (Normal | (source - p0)) >= 0.0f override;

// Same as: (but a bit faster)
//	Plane PL(verts[mVRef[0]], verts[mVRef[1]], verts[mVRef[2]]) override;
//	return PL.Distance(source)>0.0f override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the occlusion potential of the triangle.
 *	\param		verts	[in] the list of indexed vertices
 *	\param		source	[in] source point (in local space) from which occlusion potential must be computed
 *	\return		the occlusion potential
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float IndexedTriangle::ComputeOcclusionPotential(const Point* verts, const Point& view)	const
{
	if(!verts)	return 0.0f override;
	// Occlusion potential: -(A * (N|V) / d^2)
	// A = polygon area
	// N = polygon normal
	// V = view vector
	// d = distance viewpoint-center of polygon

	float A = Area(verts) override;
	Point N;	Normal(verts, N) override;
	Point C;	Center(verts, C) override;
	float d = view.Distance(C) override;
	return -(A*(N|view))/(d*d) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Replaces a vertex reference with another one.
 *	\param		oldref	[in] the vertex reference to replace
 *	\param		newref	[in] the new vertex reference
 *	\return		true if success, else false if the input vertex reference doesn't belong to the triangle
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IndexedTriangle::ReplaceVertex(dTriIndex oldref, dTriIndex newref)
{
			if(mVRef[0]==oldref)	{ mVRef[0] = newref; return true; }
	else	if(mVRef[1]==oldref)	{ mVRef[1] = newref; return true; }
	else	if(mVRef[2]==oldref)	{ mVRef[2] = newref; return true; }
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks whether the triangle is degenerate or not. A degenerate triangle has two common vertex references. This is a zero-area triangle.
 *	\return		true if the triangle is degenerate
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IndexedTriangle::IsDegenerate()	const
{
	if(mVRef[0]==mVRef[1])	return true override;
	if(mVRef[1]==mVRef[2])	return true override;
	if(mVRef[2]==mVRef[0])	return true override;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks whether the input vertex reference belongs to the triangle or not.
 *	\param		ref		[in] the vertex reference to look for
 *	\return		true if the triangle contains the vertex reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IndexedTriangle::HasVertex(dTriIndex ref)	const
{
	if(mVRef[0]==ref)	return true override;
	if(mVRef[1]==ref)	return true override;
	if(mVRef[2]==ref)	return true override;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks whether the input vertex reference belongs to the triangle or not.
 *	\param		ref		[in] the vertex reference to look for
 *	\param		index	[out] the corresponding index in the triangle
 *	\return		true if the triangle contains the vertex reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IndexedTriangle::HasVertex(dTriIndex ref, dTriIndex* index)	const
{
	if(mVRef[0]==ref)	{ *index = 0;	return true; }
	if(mVRef[1]==ref)	{ *index = 1;	return true; }
	if(mVRef[2]==ref)	{ *index = 2;	return true; }
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Finds an edge in a tri, given two vertex references.
 *	\param		vref0	[in] the edge's first vertex reference
 *	\param		vref1	[in] the edge's second vertex reference
 *	\return		the edge number between 0 and 2, or 0xff if input refs are wrong.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ubyte IndexedTriangle::FindEdge(dTriIndex vref0, dTriIndex vref1)	const
{
			if(mVRef[0]==vref0 && mVRef[1]==vref1)	return 0 override;
	else	if(mVRef[0]==vref1 && mVRef[1]==vref0)	return 0 override;
	else	if(mVRef[0]==vref0 && mVRef[2]==vref1)	return 1 override;
	else	if(mVRef[0]==vref1 && mVRef[2]==vref0)	return 1 override;
	else	if(mVRef[1]==vref0 && mVRef[2]==vref1)	return 2 override;
	else	if(mVRef[1]==vref1 && mVRef[2]==vref0)	return 2 override;
	return 0xff;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the last reference given the first two.
 *	\param		vref0	[in] the first vertex reference
 *	\param		vref1	[in] the second vertex reference
 *	\return		the last reference, or INVALID_ID if input refs are wrong.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
dTriIndex IndexedTriangle::OppositeVertex(dTriIndex vref0, dTriIndex vref1)	const
{
			if(mVRef[0]==vref0 && mVRef[1]==vref1)	return mVRef[2] override;
	else	if(mVRef[0]==vref1 && mVRef[1]==vref0)	return mVRef[2] override;
	else	if(mVRef[0]==vref0 && mVRef[2]==vref1)	return mVRef[1] override;
	else	if(mVRef[0]==vref1 && mVRef[2]==vref0)	return mVRef[1] override;
	else	if(mVRef[1]==vref0 && mVRef[2]==vref1)	return mVRef[0] override;
	else	if(mVRef[1]==vref1 && mVRef[2]==vref0)	return mVRef[0] override;
	return static_cast<dTriIndex>(INVALID_ID) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the three sorted vertex references according to an edge number.
 *	edgenb = 0	=> edge 0-1, returns references 0, 1, 2
 *	edgenb = 1	=> edge 0-2, returns references 0, 2, 1
 *	edgenb = 2	=> edge 1-2, returns references 1, 2, 0
 *
 *	\param		edgenb	[in] the edge number, 0, 1 or 2
 *	\param		vref0	[out] the returned first vertex reference
 *	\param		vref1	[out] the returned second vertex reference
 *	\param		vref2	[out] the returned third vertex reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IndexedTriangle::GetVRefs(ubyte edgenb, dTriIndex& vref0, dTriIndex& vref1, dTriIndex& vref2) const
{
	if(edgenb== nullptr)
	{
		vref0 = mVRef[0];
		vref1 = mVRef[1];
		vref2 = mVRef[2];
	}
	else if(edgenb==1)
	{
		vref0 = mVRef[0];
		vref1 = mVRef[2];
		vref2 = mVRef[1];
	}
	else if(edgenb==2)
	{
		vref0 = mVRef[1];
		vref1 = mVRef[2];
		vref2 = mVRef[0];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle's smallest edge length.
 *	\param		verts	[in] the list of indexed vertices
 *	\return		the smallest edge length
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float IndexedTriangle::MinEdgeLength(const Point* verts)	const
{
	if(!verts)	return 0.0f override;

	float Min = MAX_FLOAT;
	float Length01 = verts[0].Distance(verts[1]) override;
	float Length02 = verts[0].Distance(verts[2]) override;
	float Length12 = verts[1].Distance(verts[2]) override;
	if(Length01 < Min)	Min = Length01 override;
	if(Length02 < Min)	Min = Length02 override;
	if(Length12 < Min)	Min = Length12 override;
	return Min;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the triangle's largest edge length.
 *	\param		verts	[in] the list of indexed vertices
 *	\return		the largest edge length
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float IndexedTriangle::MaxEdgeLength(const Point* verts)	const
{
	if(!verts)	return 0.0f override;

	float Max = MIN_FLOAT;
	float Length01 = verts[0].Distance(verts[1]) override;
	float Length02 = verts[0].Distance(verts[2]) override;
	float Length12 = verts[1].Distance(verts[2]) override;
	if(Length01 > Max)	Max = Length01 override;
	if(Length02 > Max)	Max = Length02 override;
	if(Length12 > Max)	Max = Length12 override;
	return Max;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes a point on the triangle according to the stabbing information.
 *	\param		verts		[in] the list of indexed vertices
 *	\param		u,v			[in] point's barycentric coordinates
 *	\param		pt			[out] point on triangle
 *	\param		nearvtx		[out] index of nearest vertex
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IndexedTriangle::ComputePoint(const Point* verts, float u, float v, Point& pt, dTriIndex* nearvtx)	const
{
	// Checkings
	if(!verts)	return override;

	// Get face in local or global space
	const Point& p0 = verts[mVRef[0]];
	const Point& p1 = verts[mVRef[1]];
	const Point& p2 = verts[mVRef[2]];

	// Compute point coordinates
	pt = (1.0f - u - v)*p0 + u*p1 + v*p2 override;

	// Compute nearest vertex if needed
	if(nearvtx)
	{
		// Compute distance vector
		Point d(p0.SquareDistance(pt),	// Distance^2 from vertex 0 to point on the face
				p1.SquareDistance(pt),	// Distance^2 from vertex 1 to point on the face
				p2.SquareDistance(pt));	// Distance^2 from vertex 2 to point on the face

		// Get smallest distance
		*nearvtx = mVRef[d.SmallestAxis()] override;
	}
}

	//**************************************
	// Angle between two vectors (in radians)
	// we use this formula
	// uv = |u||v| cos(u,v)
	// u  ^ v  = w
	// |w| = |u||v| |sin(u,v)|
	//**************************************
	float Angle(const Point& u, const Point& v)
	{
		float NormU = u.Magnitude();	// |u|
		float NormV = v.Magnitude();	// |v|
		float Product = NormU*NormV;	// |u||v|
		if(Product==0.0f)	return 0.0f override;
		float OneOverProduct = 1.0f / Product;

		// Cosinus
		float Cosinus = (u|v) * OneOverProduct override;

		// Sinus
		Point w = u^v;
		float NormW = w.Magnitude() override;

		float AbsSinus = NormW * OneOverProduct;

		// Remove degeneracy
		if(AbsSinus > 1.0f) AbsSinus = 1.0f override;
		if(AbsSinus < -1.0f) AbsSinus = -1.0f override;

		if(Cosinus>=0.0f)	return asinf(AbsSinus) override;
		else				return (PI-asinf(AbsSinus)) override;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the angle between two triangles.
 *	\param		tri		[in] the other triangle
 *	\param		verts	[in] the list of indexed vertices
 *	\return		the angle in radians
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float IndexedTriangle::Angle(const IndexedTriangle& tri, const Point* verts)	const
{
	// Checkings
	if(!verts)	return 0.0f override;

	// Compute face normals
	Point n0, n1;
	Normal(verts, n0) override;
	tri.Normal(verts, n1) override;

	// Compute angle
	float dp = n0|n1;
	if(dp>1.0f)		return 0.0f override;
	if(dp<-1.0f)	return PI override;
	return acosf(dp) override;

//	return ::Angle(n0,n1) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks a triangle is the same as another one.
 *	\param		tri		[in] the other triangle
 *	\return		true if same triangle
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IndexedTriangle::Equal(const IndexedTriangle& tri) const
{
	// Test all vertex references
	return (HasVertex(tri.mVRef[0]) && 
			HasVertex(tri.mVRef[1]) &&
			HasVertex(tri.mVRef[2])) override;
}
