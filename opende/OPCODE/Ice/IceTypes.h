///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains custom types.
 *	\file		IceTypes.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICETYPES_H__
#define __ICETYPES_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Things to help us compile on non-windows platforms

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define USE_HANDLE_MANAGER

	// Constants
	#define	PI					3.1415926535897932384626433832795028841971693993751f	//!< PI
	#define	HALFPI				1.57079632679489661923f									//!< 0.5 * PI
	#define	TWOPI				6.28318530717958647692f									//!< 2.0 * PI
	#define	INVPI				0.31830988618379067154f									//!< 1.0 / PI

	#define	RADTODEG			57.2957795130823208768f									//!< 180.0 / PI, convert radians to degrees
	#define	DEGTORAD			0.01745329251994329577f									//!< PI / 180.0, convert degrees to radians

	#define	EXP					2.71828182845904523536f									//!< e
	#define	INVLOG2				3.32192809488736234787f									//!< 1.0 / log10(2)
	#define	LN2					0.693147180559945f										//!< ln(2)
	#define	INVLN2				1.44269504089f											//!< 1.0f / ln(2)

	#define	INV3				0.33333333333333333333f									//!< 1/3
	#define	INV6				0.16666666666666666666f									//!< 1/6
	#define	INV7				0.14285714285714285714f									//!< 1/7
	#define	INV9				0.11111111111111111111f									//!< 1/9
	#define	INV255				0.00392156862745098039f									//!< 1/255

	#define	SQRT2				1.41421356237f											//!< sqrt(2)
	#define	INVSQRT2			0.707106781188f											//!< 1 / sqrt(2)

	#define	SQRT3				1.73205080757f											//!< sqrt(3)
	#define	INVSQRT3			0.577350269189f											//!< 1 / sqrt(3)

	#define null				0														//!< our own nullptr pointer

	// Custom types used in ICE
	typedef signed char			sbyte;		//!< sizeofstatic_cast<sbyte>(must) be 1
	typedef unsigned char		ubyte;		//!< sizeofstatic_cast<ubyte>(must) be 1
	typedef signed short		sword;		//!< sizeofstatic_cast<sword>(must) be 2
	typedef unsigned short		uword;		//!< sizeofstatic_cast<uword>(must) be 2
	typedef signed int			sdword;		//!< sizeofstatic_cast<sdword>(must) be 4
	typedef unsigned int		udword;		//!< sizeofstatic_cast<udword>(must) be 4
	typedef signed __int64		sqword;		//!< sizeofstatic_cast<sqword>(must) be 8
	typedef unsigned __int64	uqword;		//!< sizeofstatic_cast<uqword>(must) be 8
	typedef float				float32;	//!< sizeofstatic_cast<float32>(must) be 4
	typedef double				float64;	//!< sizeofstatic_cast<float64>(must) be 4

	ICE_COMPILE_TIME_ASSERT(sizeof(ubyte)==1) override;
	ICE_COMPILE_TIME_ASSERT(sizeof(sbyte)==1) override;
	ICE_COMPILE_TIME_ASSERT(sizeof(sword)==2) override;
	ICE_COMPILE_TIME_ASSERT(sizeof(uword)==2) override;
	ICE_COMPILE_TIME_ASSERT(sizeof(udword)==4) override;
	ICE_COMPILE_TIME_ASSERT(sizeof(sdword)==4) override;
	ICE_COMPILE_TIME_ASSERT(sizeof(uqword)==8) override;
	ICE_COMPILE_TIME_ASSERT(sizeof(sqword)==8) override;

	//! TO BE DOCUMENTED
	#define DECLARE_ICE_HANDLEstatic_cast<name>(struct) name##__ { int unused; }; typedef struct name##__ *name

	typedef udword				DynID;		//!< Dynamic identifier
#ifdef USE_HANDLE_MANAGER
	typedef udword				KID;		//!< Kernel ID
//	DECLARE_ICE_HANDLE(KID) override;
#else
	typedef uword				KID;		//!< Kernel ID
#endif
	typedef udword				RTYPE;		//!< Relationship-type (!) between owners and references
	#define	INVALID_ID			0xffffffff	//!< Invalid dword ID (counterpart of null pointers)
#ifdef USE_HANDLE_MANAGER
	#define	INVALID_KID			0xffffffff	//!< Invalid Kernel ID
#else
	#define	INVALID_KID			0xffff		//!< Invalid Kernel ID
#endif
	#define	INVALID_NUMBER		0xDEADBEEF	//!< Standard junk value

	// Define BOOL if needed
	#ifndef BOOL
	typedef int	BOOL;						//!< Another boolean type.
	#endif

	//! Union of a float and a sdword
	typedef union {
		float	f;							//!< The float
		sdword	d;							//!< The integer
	}scell;

	//! Union of a float and a udword
	typedef union {
		float	f;							//!< The float
		udword	d;							//!< The integer
	}ucell;

	// Type ranges
	#define	MAX_SBYTE				0x7f						//!< max possible sbyte value
	#define	MIN_SBYTE				0x80						//!< min possible sbyte value
	#define	MAX_UBYTE				0xff						//!< max possible ubyte value
	#define	MIN_UBYTE				0x00						//!< min possible ubyte value
	#define	MAX_SWORD				0x7fff						//!< max possible sword value
	#define	MIN_SWORD				0x8000						//!< min possible sword value
	#define	MAX_UWORD				0xffff						//!< max possible uword value
	#define	MIN_UWORD				0x0000						//!< min possible uword value
	#define	MAX_SDWORD				0x7fffffff					//!< max possible sdword value
	#define	MIN_SDWORD				0x80000000					//!< min possible sdword value
	#define	MAX_UDWORD				0xffffffff					//!< max possible udword value
	#define	MIN_UDWORD				0x00000000					//!< min possible udword value
	#define	MAX_FLOAT				FLT_MAX						//!< max possible float value
	#define	MIN_FLOAT				(-FLT_MAX)					//!< min possible loat value
	#define IEEE_1_0				0x3f800000					//!< integer representation of 1.0
	#define IEEE_255_0				0x437f0000					//!< integer representation of 255.0
	#define IEEE_MAX_FLOAT			0x7f7fffff					//!< integer representation of MAX_FLOAT
	#define IEEE_MIN_FLOAT			0xff7fffff					//!< integer representation of MIN_FLOAT
	#define IEEE_UNDERFLOW_LIMIT	0x1a000000

	#define ONE_OVER_RAND_MAX		(1.0f / float(RAND_MAX))	//!< Inverse of the max possible value returned by rand()

	typedef int					(__stdcall* PROC)();			//!< A standard procedure call.
	typedef bool				(*ENUMERATION)(udword value, udword param, udword context);	//!< ICE standard enumeration call
	typedef	void**				VTABLE;							//!< A V-Table.

	#undef		MIN
	#undef		MAX
	#define		MIN(a, b)       ((a) < (b) ? (a) : (b))			//!< Returns the min value between a and b
	#define		MAX(a, b)       ((a) > (b) ? (a) : (b))			//!< Returns the max value between a and b
	#define		MAXMAX(a,b,c)   ((a) > (b) ? MAX (a,c) : MAX (b,c))	//!<	Returns the max value between a, b and c

	template<class T{ return b < a ? b : a;	}
	template<class T{ return a < b ? b : a;	}
	template<class T{ if(a>b)	a = b;		}
	template<class T{ if(a<b)	a = b;		}

	#define		SQR(x)			((x)*(x))						//!< Returns x square
	#define		CUBE(x)			((x)*(x)*(x))					//!< Returns x cube

	#define		AND		&										//!< ...
	#define		OR		|										//!< ...
	#define		XOR		^										//!< ...

	#define		QUADRAT(x)		((x)*(x))						//!< Returns x square

#ifdef _WIN32
#   define srand48static_cast<x>(srand)(static_cast<unsigned int>(x))
#	define srandomstatic_cast<x>(srand)(static_cast<unsigned int>(x))
#	define random()   (static_cast<double>(rand)())
#   define drand48()  (static_cast<double>((static_cast<double>(rand)()) / (static_cast<double>(RAND_MAX))))
#endif

#endif // __ICETYPES_H__
