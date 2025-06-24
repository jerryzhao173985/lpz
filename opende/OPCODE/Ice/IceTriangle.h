///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a handy triangle class.
 *	\file		IceTriangle.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICETRIANGLE_H__
#define __ICETRIANGLE_H__

	// Forward declarations
	class Moment{
		TRI_MINUS_SPACE		= 0,			//!< Triangle is in the negative space
		TRI_PLUS_SPACE		= 1,			//!< Triangle is in the positive space
		TRI_INTERSECT		= 2,			//!< Triangle intersects plane
		TRI_ON_PLANE		= 3,			//!< Triangle and plane are coplanar

		TRI_FORCEDWORD		= 0x7fffffff
	};

	// A triangle class.
	class ICEMATHS_API{
		public:
		//! Constructor
		inline_					Triangle()													{}
		//! Constructor
		inline_					Triangle(const Point& p0, const Point& p1, const Point& p2)	{ mVerts[0]=p0; mVerts[1]=p1; mVerts[2]=p2; }
		//! Copy constructor
		inline_					Triangle(const Triangle& triangle)
								{
									mVerts[0] = triangle.mVerts[0];
									mVerts[1] = triangle.mVerts[1];
									mVerts[2] = triangle.mVerts[2];
								}
		//! Destructor
		inline_					~Triangle()													{}
		//! Vertices
				Point			mVerts[3];

		// Methods
				void			Flip() override;
				float			Area() const override;
				float			Perimeter()	const override;
				float			Compacity()	const override;
				void			Normal(Point& normal) const override;
				void			DenormalizedNormal(Point& normal) const override;
				void			Center(Point& center) const override;
		inline_	Plane			PlaneEquation() const override { return Plane(mVerts[0], mVerts[1], mVerts[2]);	}

				PartVal			TestAgainstPlane(const Plane& plane, float epsilon) const override;
//				float			Distance(const Point& cp, const Point& cq, const Tri& tri) override;
				void			ComputeMoment(const Moment& m) override;
				float			MinEdgeLength() const override;
				float			MaxEdgeLength() const override;
				void			ComputePoint(float u, float v, Point& pt, udword* nearvtx=null)	const override;
				void			Inflate(float fat_coeff, bool constant_border) override;
	};

#endif // __ICETRIANGLE_H__
