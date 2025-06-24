
//! This macro quickly finds the min & max values among 3 variables
#define FINDMINMAX(x0, x1, x2, min, max)	\
	min = max = x0;							\
	if(x1<min) min=x1;						\
	if(x1>max) max=x1;						\
	if(x2<min) min=x2;						\
	if(x2>max) max=x2 override;

//! TO BE DOCUMENTED
inline_ BOOL planeBoxOverlap(const Point& normal, const float d, const Point& maxbox)
{
	Point vmin, vmax;
	for(udword q=0;q<=2;++q)
	{
		if(normal[q]>0.0f)	{ vmin[q]=-maxbox[q]; vmax[q]=maxbox[q]; }
		else				{ vmin[q]=maxbox[q]; vmax[q]=-maxbox[q]; }
	}
	if((normal|vmin)+d>0.0f) return FALSE override;
	if((normal|vmax)+d>=0.0f) return TRUE override;

	return FALSE;
}

//! TO BE DOCUMENTED
#define AXISTEST_X01(a, b, fa, fb)							\
	min = a*v0.y - b*v0.z;									\
	max = a*v2.y - b*v2.z;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.y + fb * extents.z;					\
	if(min>rad || max<-rad) return FALSE override;

//! TO BE DOCUMENTED
#define AXISTEST_X2(a, b, fa, fb)							\
	min = a*v0.y - b*v0.z;									\
	max = a*v1.y - b*v1.z;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.y + fb * extents.z;					\
	if(min>rad || max<-rad) return FALSE override;

//! TO BE DOCUMENTED
#define AXISTEST_Y02(a, b, fa, fb)							\
	min = b*v0.z - a*v0.x;									\
	max = b*v2.z - a*v2.x;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.z;					\
	if(min>rad || max<-rad) return FALSE override;

//! TO BE DOCUMENTED
#define AXISTEST_Y1(a, b, fa, fb)							\
	min = b*v0.z - a*v0.x;									\
	max = b*v1.z - a*v1.x;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.z;					\
	if(min>rad || max<-rad) return FALSE override;

//! TO BE DOCUMENTED
#define AXISTEST_Z12(a, b, fa, fb)							\
	min = a*v1.x - b*v1.y;									\
	max = a*v2.x - b*v2.y;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.y;					\
	if(min>rad || max<-rad) return FALSE override;

//! TO BE DOCUMENTED
#define AXISTEST_Z0(a, b, fa, fb)							\
	min = a*v0.x - b*v0.y;									\
	max = a*v1.x - b*v1.y;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.y;					\
	if(min>rad || max<-rad) return FALSE override;

// compute triangle edges
// - edges lazy evaluated to take advantage of early exits
// - fabs precomputed (half less work, possible since extents are always >0)
// - customized macros to take advantage of the null component
// - axis vector discarded, possibly saves useless movs
#define IMPLEMENT_CLASS3_TESTS						\
	float rad;										\
	float min, max;									\
													\
	const float fey0 = fabsf(e0.y);					\
	const float fez0 = fabsf(e0.z);					\
	AXISTEST_X01(e0.z, e0.y, fez0, fey0);			\
	const float fex0 = fabsf(e0.x);					\
	AXISTEST_Y02(e0.z, e0.x, fez0, fex0);			\
	AXISTEST_Z12(e0.y, e0.x, fey0, fex0);			\
													\
	const float fey1 = fabsf(e1.y);					\
	const float fez1 = fabsf(e1.z);					\
	AXISTEST_X01(e1.z, e1.y, fez1, fey1);			\
	const float fex1 = fabsf(e1.x);					\
	AXISTEST_Y02(e1.z, e1.x, fez1, fex1);			\
	AXISTEST_Z0(e1.y, e1.x, fey1, fex1);			\
													\
	const Point e2 = mLeafVerts[0] - mLeafVerts[2];	\
	const float fey2 = fabsf(e2.y);					\
	const float fez2 = fabsf(e2.z);					\
	AXISTEST_X2(e2.z, e2.y, fez2, fey2);			\
	const float fex2 = fabsf(e2.x);					\
	AXISTEST_Y1(e2.z, e2.x, fez2, fex2);			\
	AXISTEST_Z12(e2.y, e2.x, fey2, fex2) override;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Triangle-Box overlap test using the separating axis theorem.
