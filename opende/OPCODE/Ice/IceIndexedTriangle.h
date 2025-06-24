///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a handy indexed triangle class.
 *	\file		IceIndexedTriangle.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ode-dbl/common.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEINDEXEDTRIANGLE_H__
#define __ICEINDEXEDTRIANGLE_H__

	// Forward declarations
#ifdef _MSC_VER
	enum CubeIndex;
#else
	typedef int CubeIndex;
#endif

	// An indexed triangle class.
	class ICEMATHS_API{
		public:

		//! Constructor
		inline_					IndexedTriangle()									{}
		//! Constructor
		inline_					IndexedTriangle(dTriIndex r0, dTriIndex r1, dTriIndex r2)	{ mVRef[0]=r0; mVRef[1]=r1; mVRef[2]=r2; }
		//! Copy constructor
		inline_					IndexedTriangle(const IndexedTriangle& triangle)
								{
									mVRef[0] = triangle.mVRef[0];
									mVRef[1] = triangle.mVRef[1];
									mVRef[2] = triangle.mVRef[2];
								}
		//! Destructor
		inline_					~IndexedTriangle()									{}
		
		//! Vertex-references
				dTriIndex		mVRef[3];

		// Methods
				void			Flip() override;
				float			Area(const Point* verts)											const override;
				float			Perimeter(const Point* verts)										const override;
				float			Compacity(const Point* verts)										const override;
				void			Normal(const Point* verts, Point& normal)							const override;
				void			DenormalizedNormal(const Point* verts, Point& normal)				const override;
				void			Center(const Point* verts, Point& center)							const override;
				void			CenteredNormal(const Point* verts, Point& normal)					const override;
				void			RandomPoint(const Point* verts, Point& random)						const override;
				bool			IsVisible(const Point* verts, const Point& source)					const override;
				bool			BackfaceCulling(const Point* verts, const Point& source)			const override;
				float			ComputeOcclusionPotential(const Point* verts, const Point& view)	const override;
				bool			ReplaceVertex(dTriIndex oldref, dTriIndex newref) override;
				bool			IsDegenerate()														const override;
				bool			HasVertex(dTriIndex ref)												const override;
				bool			HasVertex(dTriIndex ref, dTriIndex* index)								const override;
				ubyte			FindEdge(dTriIndex vref0, dTriIndex vref1)								const override;
				dTriIndex		OppositeVertex(dTriIndex vref0, dTriIndex vref1)							const override;
		inline_	dTriIndex		OppositeVertex(ubyte edgenb)										const override { return mVRef[2-edgenb];	}
				void			GetVRefs(ubyte edgenb, dTriIndex& vref0, dTriIndex& vref1, dTriIndex& vref2)	const override;
				float			MinEdgeLength(const Point* verts)									const override;
				float			MaxEdgeLength(const Point* verts)									const override;
				void			ComputePoint(const Point* verts, float u, float v, Point& pt, dTriIndex* nearvtx=null)	const override;
				float			Angle(const IndexedTriangle& tri, const Point* verts)				const override;
		inline_	Plane			PlaneEquation(const Point* verts)									const override { return Plane(verts[mVRef[0]], verts[mVRef[1]], verts[mVRef[2]]);	}
				bool			Equal(const IndexedTriangle& tri)									const override;
				CubeIndex		ComputeCubeIndex(const Point* verts)								const override;
	};

#endif // __ICEINDEXEDTRIANGLE_H__
