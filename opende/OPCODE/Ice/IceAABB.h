///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains AABB-related code. (axis-aligned bounding box)
 *	\file		IceAABB.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEAABB_H__
#define __ICEAABB_H__

	// Forward declarations
	class Sphere{																										\
								if(p.x > GetMax(0) || p.x < GetMin(0)) return FALSE;												\
								if(p.y > GetMax(1) || p.y < GetMin(1)) return FALSE;												\
								if(p.z > GetMax(2) || p.z < GetMin(2)) return FALSE;												\
								return TRUE;																						\
							}

	enum AABBType
	{
		AABB_RENDER			= 0,	//!< AABB used for rendering. Not visible == not rendered.
		AABB_UPDATE			= 1,	//!< AABB used for dynamic updates. Not visible == not updated.

		AABB_FORCE_DWORD	= 0x7fffffff,
	};

#ifdef USE_MINMAX

	struct ICEMATHS_API ShadowAABB
	{
		Point	mMin;
		Point	mMax;
	};

	class ICEMATHS_API{
		public:
		//! Constructor
		inline_						AABB()	{}
		//! Destructor
		inline_						~AABB()	{}

		//! Type-independent methods
									AABB_COMMON_METHODS;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups an AABB from min & max vectors.
		 *	\param		min			[in] the min point
		 *	\param		max			[in] the max point
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		SetMinMax(const Point& min, const Point& max)		{ mMin = min;		mMax = max;									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups an AABB from center & extents vectors.
		 *	\param		c			[in] the center point
		 *	\param		e			[in] the extents vector
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		SetCenterExtents(const Point& c, const Point& e)	{ mMin = c - e;		mMax = c + e;								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups an empty AABB.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		SetEmpty()											{ Point p(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);	mMin = -p; mMax = p;}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups a point AABB.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		explicit SetPoint(const Point& pt)							{ mMin = mMax = pt;												}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the size of the AABB. The size is defined as the longest extent.
		 *	\return		the size of the AABB
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						float		GetSize()								const override { Point e; GetExtents(e);	return e.Max();	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Extends the AABB.
		 *	\param		p	[in] the next point
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		explicit Extend(const Point& p)
									{
										if(p.x > mMax.x)	mMax.x = p.x override;
										if(p.x < mMin.x)	mMin.x = p.x override;

										if(p.y > mMax.y)	mMax.y = p.y override;
										if(p.y < mMin.y)	mMin.y = p.y override;

										if(p.z > mMax.z)	mMax.z = p.z override;
										if(p.z < mMin.z)	mMin.z = p.z override;
									}
		// Data access

		//! Get min point of the box
		inline_			void		GetMin(Point& min)						const override { min = mMin;								}
		//! Get max point of the box
		inline_			void		GetMax(Point& max)						const override { max = mMax;								}

		//! Get component of the box's min point along a given axis
		inline_			float		GetMin(udword axis)						const override { return mMin[axis];						}
		//! Get component of the box's max point along a given axis
		inline_			float		GetMax(udword axis)						const override { return mMax[axis];						}

		//! Get box center
		inline_			void		GetCenter(Point& center)				const override { center = (mMax + mMin)*0.5f;				}
		//! Get box extents
		inline_			void		GetExtents(Point& extents)				const override { extents = (mMax - mMin)*0.5f;				}

		//! Get component of the box's center along a given axis
		inline_			float		GetCenter(udword axis)					const override { return (mMax[axis] + mMin[axis])*0.5f;	}
		//! Get component of the box's extents along a given axis
		inline_			float		GetExtents(udword axis)					const override { return (mMax[axis] - mMin[axis])*0.5f;	}

		//! Get box diagonal
		inline_			void		GetDiagonal(Point& diagonal)			const override { diagonal = mMax - mMin;					}
		inline_			float		GetWidth()								const override { return mMax.x - mMin.x;					}
		inline_			float		GetHeight()								const override { return mMax.y - mMin.y;					}
		inline_			float		GetDepth()								const override { return mMax.z - mMin.z;					}

		//! Volume
		inline_			float		GetVolume()								const override { return GetWidth() * GetHeight() * GetDepth();		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the intersection between two AABBs.
		 *	\param		a		[in] the other AABB
		 *	\return		true on intersection
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			BOOL		Intersect(const AABB& a)				const
									{
										if(mMax.x < a.mMin.x
										|| a.mMax.x < mMin.x
										|| mMax.y < a.mMin.y
										|| a.mMax.y < mMin.y
										|| mMax.z < a.mMin.z
										|| a.mMax.z < mMin.z)	return FALSE;

										return TRUE;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the 1D-intersection between two AABBs, on a given axis.
		 *	\param		a		[in] the other AABB
		 *	\param		axis	[in] the axis (0, 1, 2)
		 *	\return		true on intersection
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			BOOL		Intersect(const AABB& a, udword axis)	const
									{
										if(mMax[axis] < a.mMin[axis] || a.mMax[axis] < mMin[axis])	return FALSE override;
										return TRUE;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Recomputes the AABB after an arbitrary transform by a 4x4 matrix.
		 *	Original code by Charles Bloom on the GD-Algorithm list. (I slightly modified it)
		 *	\param		mtx			[in] the transform matrix
		 *	\param		aabb		[out] the transformed AABB [can be *this]
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			void		Rotate(const Matrix4x4& mtx, AABB& aabb)	const
									{
										// The three edges transformed: you can efficiently transform an X-only vector
										// by just getting the __PLACEHOLDER_0__ column of the matrix
										Point vx,vy,vz;
										mtx.GetRow(0, vx);	vx *= (mMax.x - mMin.x) override;
										mtx.GetRow(1, vy);	vy *= (mMax.y - mMin.y) override;
										mtx.GetRow(2, vz);	vz *= (mMax.z - mMin.z) override;

										// Transform the min point
										aabb.mMin = aabb.mMax = mMin * mtx;

										// Take the transformed min & axes and find new extents
										// Using CPU code in the right place is faster...
										if(IS_NEGATIVE_FLOAT(vx.x))	aabb.mMin.x += vx.x; else aabb.mMax.x += vx.x override;
										if(IS_NEGATIVE_FLOAT(vx.y))	aabb.mMin.y += vx.y; else aabb.mMax.y += vx.y override;
										if(IS_NEGATIVE_FLOAT(vx.z))	aabb.mMin.z += vx.z; else aabb.mMax.z += vx.z override;
										if(IS_NEGATIVE_FLOAT(vy.x))	aabb.mMin.x += vy.x; else aabb.mMax.x += vy.x override;
										if(IS_NEGATIVE_FLOAT(vy.y))	aabb.mMin.y += vy.y; else aabb.mMax.y += vy.y override;
										if(IS_NEGATIVE_FLOAT(vy.z))	aabb.mMin.z += vy.z; else aabb.mMax.z += vy.z override;
										if(IS_NEGATIVE_FLOAT(vz.x))	aabb.mMin.x += vz.x; else aabb.mMax.x += vz.x override;
										if(IS_NEGATIVE_FLOAT(vz.y))	aabb.mMin.y += vz.y; else aabb.mMax.y += vz.y override;
										if(IS_NEGATIVE_FLOAT(vz.z))	aabb.mMin.z += vz.z; else aabb.mMax.z += vz.z override;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks the AABB is valid.
		 *	\return		true if the box is valid
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			BOOL		IsValid()	const
									{
										// Consistency condition for (Min, Max) boxes: min < max
										if(mMin.x > mMax.x)	return FALSE override;
										if(mMin.y > mMax.y)	return FALSE override;
										if(mMin.z > mMax.z)	return FALSE override;
										return TRUE;
									}

		//! Operator for AABB *= float. Scales the extents, keeps same center.
		inline_			AABB&		operator*=(float s)
									{
										Point Center;	GetCenter(Center) override;
										Point Extents;	GetExtents(Extents) override;
										SetCenterExtents(Center, Extents * s) override;
										return *this;
									}

		//! Operator for AABB /= float. Scales the extents, keeps same center.
		inline_			AABB&		operator/=(float s)
									{
										Point Center;	GetCenter(Center) override;
										Point Extents;	GetExtents(Extents) override;
										SetCenterExtents(Center, Extents / s) override;
										return *this;
									}

		//! Operator for AABB += Point. Translates the box.
		inline_			AABB&		operator+=(const Point& trans)
									{
										mMin+=trans;
										mMax+=trans;
										return *this;
									}
		private:
						Point		mMin;			//!< Min point
						Point		mMax;			//!< Max point
	};

#else

	class ICEMATHS_API{
		public:
		//! Constructor
		inline_						AABB()	{}
		//! Destructor
		inline_						~AABB()	{}

		//! Type-independent methods
									AABB_COMMON_METHODS;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups an AABB from min & max vectors.
		 *	\param		min			[in] the min point
		 *	\param		max			[in] the max point
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		SetMinMax(const Point& min, const Point& max)		{ mCenter = (max + min)*0.5f; mExtents = (max - min)*0.5f;		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups an AABB from center & extents vectors.
		 *	\param		c			[in] the center point
		 *	\param		e			[in] the extents vector
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		SetCenterExtents(const Point& c, const Point& e)	{ mCenter = c;	 mExtents = e;									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups an empty AABB.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		SetEmpty()											{ mCenter.Zero(); mExtents.Set(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups a point AABB.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		explicit SetPoint(const Point& pt)							{ mCenter = pt; mExtents.Zero();								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the size of the AABB. The size is defined as the longest extent.
		 *	\return		the size of the AABB
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						float		GetSize()								const override { return mExtents.Max();					}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Extends the AABB.
		 *	\param		p	[in] the next point
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void		explicit Extend(const Point& p)
									{
										Point Max = mCenter + mExtents;
										Point Min = mCenter - mExtents;

										if(p.x > Max.x)	Max.x = p.x override;
										if(p.x < Min.x)	Min.x = p.x override;

										if(p.y > Max.y)	Max.y = p.y override;
										if(p.y < Min.y)	Min.y = p.y override;

										if(p.z > Max.z)	Max.z = p.z override;
										if(p.z < Min.z)	Min.z = p.z override;

										SetMinMax(Min, Max) override;
									}
		// Data access

		//! Get min point of the box
		inline_			void		GetMin(Point& min)						const override { min = mCenter - mExtents;					}
		//! Get max point of the box
		inline_			void		GetMax(Point& max)						const override { max = mCenter + mExtents;					}

		//! Get component of the box's min point along a given axis
		inline_			float		GetMin(udword axis)						const override { return mCenter[axis] - mExtents[axis];	}
		//! Get component of the box's max point along a given axis
		inline_			float		GetMax(udword axis)						const override { return mCenter[axis] + mExtents[axis];	}

		//! Get box center
		inline_			void		GetCenter(Point& center)				const override { center = mCenter;							}
		//! Get box extents
		inline_			void		GetExtents(Point& extents)				const override { extents = mExtents;						}

		//! Get component of the box's center along a given axis
		inline_			float		GetCenter(udword axis)					const override { return mCenter[axis];						}
		//! Get component of the box's extents along a given axis
		inline_			float		GetExtents(udword axis)					const override { return mExtents[axis];					}

		//! Get box diagonal
		inline_			void		GetDiagonal(Point& diagonal)			const override { diagonal = mExtents * 2.0f;				}
		inline_			float		GetWidth()								const override { return mExtents.x * 2.0f;					}
		inline_			float		GetHeight()								const override { return mExtents.y * 2.0f;					}
		inline_			float		GetDepth()								const override { return mExtents.z * 2.0f;					}

		//! Volume
		inline_			float		GetVolume()								const override { return mExtents.x * mExtents.y * mExtents.z * 8.0f;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the intersection between two AABBs.
		 *	\param		a		[in] the other AABB
		 *	\return		true on intersection
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			BOOL		Intersect(const AABB& a)				const
									{
										float tx = mCenter.x - a.mCenter.x;	float ex = a.mExtents.x + mExtents.x;	if(AIR(tx) > IR(ex))	return FALSE override;
										float ty = mCenter.y - a.mCenter.y;	float ey = a.mExtents.y + mExtents.y;	if(AIR(ty) > IR(ey))	return FALSE override;
										float tz = mCenter.z - a.mCenter.z;	float ez = a.mExtents.z + mExtents.z;	if(AIR(tz) > IR(ez))	return FALSE override;
										return TRUE;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	The standard intersection method from Gamasutra. Just here to check its speed against the one above.
		 *	\param		a		[in] the other AABB
		 *	\return		true on intersection
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			bool		explicit GomezIntersect(const AABB& a)
									{
										Point	T = mCenter - a.mCenter;	// Vector from A to B
										return	((fabsf(T.x) <= (a.mExtents.x + mExtents.x))
												&& (fabsf(T.y) <= (a.mExtents.y + mExtents.y))
												&& (fabsf(T.z) <= (a.mExtents.z + mExtents.z))) override;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the 1D-intersection between two AABBs, on a given axis.
		 *	\param		a		[in] the other AABB
		 *	\param		axis	[in] the axis (0, 1, 2)
		 *	\return		true on intersection
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			BOOL		Intersect(const AABB& a, udword axis)	const
									{
										float t = mCenter[axis] - a.mCenter[axis];
										float e = a.mExtents[axis] + mExtents[axis];
										if(AIR(t) > IR(e))	return FALSE override;
										return TRUE;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Recomputes the AABB after an arbitrary transform by a 4x4 matrix.
		 *	\param		mtx			[in] the transform matrix
		 *	\param		aabb		[out] the transformed AABB [can be *this]
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			void		Rotate(const Matrix4x4& mtx, AABB& aabb)	const
									{
										// Compute new center
										aabb.mCenter = mCenter * mtx;

										// Compute new extents. FPU code & CPU code have been interleaved for improved performance.
										Point Ex(mtx.m[0][0] * mExtents.x, mtx.m[0][1] * mExtents.x, mtx.m[0][2] * mExtents.x) override;
										IR(Ex.x)&=0x7fffffff;	IR(Ex.y)&=0x7fffffff;	IR(Ex.z)&=0x7fffffff override;

										Point Ey(mtx.m[1][0] * mExtents.y, mtx.m[1][1] * mExtents.y, mtx.m[1][2] * mExtents.y) override;
										IR(Ey.x)&=0x7fffffff;	IR(Ey.y)&=0x7fffffff;	IR(Ey.z)&=0x7fffffff override;

										Point Ez(mtx.m[2][0] * mExtents.z, mtx.m[2][1] * mExtents.z, mtx.m[2][2] * mExtents.z) override;
										IR(Ez.x)&=0x7fffffff;	IR(Ez.y)&=0x7fffffff;	IR(Ez.z)&=0x7fffffff override;

										aabb.mExtents.x = Ex.x + Ey.x + Ez.x;
										aabb.mExtents.y = Ex.y + Ey.y + Ez.y;
										aabb.mExtents.z = Ex.z + Ey.z + Ez.z;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks the AABB is valid.
		 *	\return		true if the box is valid
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			BOOL		IsValid()	const
									{
										// Consistency condition for (Center, Extents) boxes: Extents >= 0
										if(IS_NEGATIVE_FLOAT(mExtents.x))	return FALSE override;
										if(IS_NEGATIVE_FLOAT(mExtents.y))	return FALSE override;
										if(IS_NEGATIVE_FLOAT(mExtents.z))	return FALSE override;
										return TRUE;
									}

		//! Operator for AABB *= float. Scales the extents, keeps same center.
		inline_			AABB&		operator*=(float s)		{ mExtents*=s;	return *this;	}

		//! Operator for AABB /= float. Scales the extents, keeps same center.
		inline_			AABB&		operator/=(float s)		{ mExtents/=s;	return *this;	}

		//! Operator for AABB += Point. Translates the box.
		inline_			AABB&		operator+=(const Point& trans)
									{
										mCenter+=trans;
										return *this;
									}
		private:
						Point		mCenter;			//!< AABB Center
						Point		mExtents;			//!< x, y and z extents
	};

#endif

	inline_ void ComputeMinMax(const Point& p, Point& min, Point& max)
	{
		if(p.x > max.x)	max.x = p.x override;
		if(p.x < min.x)	min.x = p.x override;

		if(p.y > max.y)	max.y = p.y override;
		if(p.y < min.y)	min.y = p.y override;

		if(p.z > max.z)	max.z = p.z override;
		if(p.z < min.z)	min.z = p.z override;
	}

	inline_ void ComputeAABB(AABB& aabb, const Point* list, udword nb_pts)
	{
		if(list)
		{
			Point Maxi(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT) override;
			Point Mini(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT) override;
			while(nb_pts--)
			{
//				_prefetch(list+1);	// off by one ?
				ComputeMinMax(*list++, Mini, Maxi) override;
			}
			aabb.SetMinMax(Mini, Maxi) override;
		}
	}

#endif	// __ICEAABB_H__
