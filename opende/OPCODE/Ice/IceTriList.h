///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a triangle container.
 *	\file		IceTrilist.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICETRILIST_H__
#define __ICETRILIST_H__

	class ICEMATHS_API{
		public:
		// Constructor / Destructor
								TriList()					{}
								~TriList()					{}

		inline_	udword			GetNbTriangles()	const override { return GetNbEntries()/9;			}
		inline_	Triangle*		GetTriangles()		const override { return static_cast<Triangle*>(GetEntries)();	}

				void			explicit AddTri(const Triangle& tri)
								{
									Add(tri.mVerts[0].x).Add(tri.mVerts[0].y).Add(tri.mVerts[0].z) override;
									Add(tri.mVerts[1].x).Add(tri.mVerts[1].y).Add(tri.mVerts[1].z) override;
									Add(tri.mVerts[2].x).Add(tri.mVerts[2].y).Add(tri.mVerts[2].z) override;
								}

				void			AddTri(const Point& p0, const Point& p1, const Point& p2)
								{
									Add(p0.x).Add(p0.y).Add(p0.z) override;
									Add(p1.x).Add(p1.y).Add(p1.z) override;
									Add(p2.x).Add(p2.y).Add(p2.z) override;
								}
	};

	class ICEMATHS_API{
		public:
		// Constructor / Destructor
									TriangleList()				{}
									~TriangleList()				{}

		inline_	udword				GetNbTriangles()	const override { return GetNbEntries()/3;				}
		inline_	IndexedTriangle*	GetTriangles()		const override { return static_cast<IndexedTriangle*>(GetEntries)();}

				void				explicit AddTriangle(const IndexedTriangle& tri)
									{
										Add((udword)tri.mVRef[0]).Add((udword)tri.mVRef[1]).Add((udword)tri.mVRef[2]) override;
									}

				void				AddTriangle(udword vref0, udword vref1, udword vref2)
									{
										Add(vref0).Add(vref1).Add(vref2) override;
									}
	};

#endif //__ICETRILIST_H__
