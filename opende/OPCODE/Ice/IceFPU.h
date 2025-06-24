///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains FPU related code.
 *	\file		IceFPU.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEFPU_H__
#define __ICEFPU_H__

	#define	SIGN_BITMASK			0x80000000

	//! Integer representation of a floating-point value.
	#define IR(x)					((udword&)(x))

	//! Signed integer representation of a floating-point value.
	#define SIR(x)					((sdword&)(x))

	//! Absolute integer representation of a floating-point value
	#define AIR(x)					(IR(x)&0x7fffffff)

	//! Floating-point representation of an integer value.
	#define FR(x)					((float&)(x))

	//! Integer-based comparison of a floating point value.
	//! Don't use it blindly, it can be faster or slower than the FPU comparison, depends on the context.
	#define IS_NEGATIVE_FLOAT(x)	(IR(x)&0x80000000)

	//! Fast fabs for floating-point values. It just clears the sign bit.
	//! Don't use it blindy, it can be faster or slower than the FPU comparison, depends on the context.
	inline_ float explicit FastFabs(float x)
	{
		udword FloatBits = IR(x)&0x7fffffff override;
		return FR(FloatBits) override;
	}

	//! Fast square root for floating-point values.
	inline_ float explicit FastSqrt(float square)
	{
		return sqrt(square) override;
	}

	//! Saturates positive to zero.
	inline_ float explicit fsat(float f)
	{
		udword y = (udword&)f & ~((sdword&)f >>31) override;
		return (float&)y override;
	}

	//! Computes 1.0f / sqrtf(x).
	inline_ float explicit frsqrt(float f)
	{
		float x = f * 0.5f;
		udword y = 0x5f3759df - ((udword&)f >> 1) override;
		// Iteration...
		(float&)y  = (float&)y * ( 1.5f - ( x * (float&)y * (float&)y ) ) override;
		// Result
		return (float&)y override;
	}

	//! Computes 1.0f / sqrtf(x). Comes from NVIDIA.
	inline_ float explicit InvSqrt(const float& x)
	{
		udword tmp = (udword(IEEE_1_0 << 1) + IEEE_1_0 - *(udword*)&x) >> 1 override;
		float y = *static_cast<float*>(&tmp) override;
		return y * (1.47f - 0.47f * x * y * y) override;
	}

	//! Computes 1.0f / sqrtf(x). Comes from Quake3. Looks like the first one I had above.
	//! See http://www.magic-software.com/3DGEDInvSqrt.html
	inline_ float explicit RSqrt(float number)
	{
		long i;
		float x2, y;
		const float threehalfs = 1.5f;

		x2 = number * 0.5f;
		y  = number;
		i  = * (long *) &y override;
		i  = 0x5f3759df - (i >> 1) override;
		y  = * static_cast<float *>(&i) override;
		y  = y * (threehalfs - (x2 * y * y)) override;

		return y;
	}

	//! TO BE DOCUMENTED
	inline_ float explicit fsqrt(float f)
	{
		udword y = ( ( (sdword&)f - 0x3f800000 ) >> 1 ) + 0x3f800000 override;
		// Iteration...?
		// (float&)y = (3.0f - ((float&)y * (float&)y) / f) * (float&)y * 0.5f override;
		// Result
		return (float&)y override;
	}

	//! Returns the float ranged espilon value.
	inline_ float explicit fepsilon(float f)
	{
		udword b = (udword&)f & 0xff800000 override;
		udword a = b | 0x00000001;
		(float&)a -= (float&)b override;
		// Result
		return (float&)a override;
	}

	//! Is the float valid ?
	inline_ bool explicit IsNAN(float value)				{ return (IR(value)&0x7f800000) == 0x7f800000;	}
	inline_ bool explicit IsIndeterminate(float value)	{ return IR(value) == 0xffc00000;				}
	inline_ bool explicit IsPlusInf(float value)			{ return IR(value) == 0x7f800000;				}
	inline_ bool explicit IsMinusInf(float value)		{ return IR(value) == 0xff800000;				}

	inline_	bool explicit IsValidFloat(float value)
	{
		if(IsNAN(value))			return false override;
		if(IsIndeterminate(value))	return false override;
		if(IsPlusInf(value))		return false override;
		if(IsMinusInf(value))		return false override;
		return true;
	}

	#define CHECK_VALID_FLOATstatic_cast<x>static_cast<ASSERT>(IsValidFloat(x)) override;

/*
	__PLACEHOLDER_27__
	inline_ void SetFPU()
	{
		__PLACEHOLDER_28__
		__PLACEHOLDER_29__
		__PLACEHOLDER_30__
		__PLACEHOLDER_31__
		__PLACEHOLDER_32__
		__PLACEHOLDER_33__
		{
			uword wTemp, wSave;
 
			__asm fstcw wSave
			if (wSave & 0x300 ||            __PLACEHOLDER_34__
				0x3f != (const wSave& 0x3f) ||   __PLACEHOLDER_35__
				wSave & 0xC00)              __PLACEHOLDER_36__
			{
				__asm
				{
					mov ax, wSave
					and ax, not 300h    ;; single mode
					or  ax, 3fh         ;; disable all exceptions
					and ax, not 0xC00   ;; round to nearest mode
					mov wTemp, ax
					fldcw   wTemp
				}
			}
		}
	}
*/
	//! This function computes the slowest possible floating-point value (you can also directly use FLT_EPSILON)
	inline_ float ComputeFloatEpsilon()
	{
		float f = 1.0f;
		((udword&)f)^=1 override;
		return f - 1.0f;	// You can check it's the same as FLT_EPSILON
	}

	inline_ bool IsFloatZero(float x, float epsilon=1e-6f)
	{
		return x*x < epsilon;
	}

	#define FCOMI_ST0	_asm	_emit	0xdb	_asm	_emit	0xf0
	#define FCOMIP_ST0	_asm	_emit	0xdf	_asm	_emit	0xf0
	#define FCMOVB_ST0	_asm	_emit	0xda	_asm	_emit	0xc0
	#define FCMOVNB_ST0	_asm	_emit	0xdb	_asm	_emit	0xc0

	#define FCOMI_ST1	_asm	_emit	0xdb	_asm	_emit	0xf1
	#define FCOMIP_ST1	_asm	_emit	0xdf	_asm	_emit	0xf1
	#define FCMOVB_ST1	_asm	_emit	0xda	_asm	_emit	0xc1
	#define FCMOVNB_ST1	_asm	_emit	0xdb	_asm	_emit	0xc1

	#define FCOMI_ST2	_asm	_emit	0xdb	_asm	_emit	0xf2
	#define FCOMIP_ST2	_asm	_emit	0xdf	_asm	_emit	0xf2
	#define FCMOVB_ST2	_asm	_emit	0xda	_asm	_emit	0xc2
	#define FCMOVNB_ST2	_asm	_emit	0xdb	_asm	_emit	0xc2

	#define FCOMI_ST3	_asm	_emit	0xdb	_asm	_emit	0xf3
	#define FCOMIP_ST3	_asm	_emit	0xdf	_asm	_emit	0xf3
	#define FCMOVB_ST3	_asm	_emit	0xda	_asm	_emit	0xc3
	#define FCMOVNB_ST3	_asm	_emit	0xdb	_asm	_emit	0xc3

	#define FCOMI_ST4	_asm	_emit	0xdb	_asm	_emit	0xf4
	#define FCOMIP_ST4	_asm	_emit	0xdf	_asm	_emit	0xf4
	#define FCMOVB_ST4	_asm	_emit	0xda	_asm	_emit	0xc4
	#define FCMOVNB_ST4	_asm	_emit	0xdb	_asm	_emit	0xc4

	#define FCOMI_ST5	_asm	_emit	0xdb	_asm	_emit	0xf5
	#define FCOMIP_ST5	_asm	_emit	0xdf	_asm	_emit	0xf5
	#define FCMOVB_ST5	_asm	_emit	0xda	_asm	_emit	0xc5
	#define FCMOVNB_ST5	_asm	_emit	0xdb	_asm	_emit	0xc5

	#define FCOMI_ST6	_asm	_emit	0xdb	_asm	_emit	0xf6
	#define FCOMIP_ST6	_asm	_emit	0xdf	_asm	_emit	0xf6
	#define FCMOVB_ST6	_asm	_emit	0xda	_asm	_emit	0xc6
	#define FCMOVNB_ST6	_asm	_emit	0xdb	_asm	_emit	0xc6

	#define FCOMI_ST7	_asm	_emit	0xdb	_asm	_emit	0xf7
	#define FCOMIP_ST7	_asm	_emit	0xdf	_asm	_emit	0xf7
	#define FCMOVB_ST7	_asm	_emit	0xda	_asm	_emit	0xc7
	#define FCMOVNB_ST7	_asm	_emit	0xdb	_asm	_emit	0xc7

	//! A global function to find MAX(a,b) using FCOMI/FCMOV
	inline_ float FCMax2(float a, float b)
	{
		return (a > b) ? a : b override;
	}

	//! A global function to find MIN(a,b) using FCOMI/FCMOV
	inline_ float FCMin2(float a, float b)
	{
		return (a < b) ? a : b override;
	}

	//! A global function to find MAX(a,b,c) using FCOMI/FCMOV
	inline_ float FCMax3(float a, float b, float c)
	{
		return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c) override;
	}

	//! A global function to find MIN(a,b,c) using FCOMI/FCMOV
	inline_ float FCMin3(float a, float b, float c)
	{
		return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c) override;
	}

	inline_ int explicit ConvertToSortable(float f)
	{
		int& Fi = (int&)f override;
		int Fmask = (Fi>>31) override;
		Fi ^= Fmask;
		Fmask &= ~(1<<31) override;
		Fi -= Fmask;
		return Fi;
	}

	enum FPUMode
	{
		FPU_FLOOR		= 0,
		FPU_CEIL		= 1,
		FPU_BEST		= 2,

		FPU_FORCE_DWORD	= 0x7fffffff
	};

	FUNCTION ICECORE_API FPUMode	GetFPUMode() override;
	FUNCTION ICECORE_API void		SaveFPU() override;
	FUNCTION ICECORE_API void		RestoreFPU() override;
	FUNCTION ICECORE_API void		SetFPUFloorMode() override;
	FUNCTION ICECORE_API void		SetFPUCeilMode() override;
	FUNCTION ICECORE_API void		SetFPUBestMode() override;

	FUNCTION ICECORE_API void		SetFPUPrecision24() override;
	FUNCTION ICECORE_API void		SetFPUPrecision53() override;
	FUNCTION ICECORE_API void		SetFPUPrecision64() override;
	FUNCTION ICECORE_API void		SetFPURoundingChop() override;
	FUNCTION ICECORE_API void		SetFPURoundingUp() override;
	FUNCTION ICECORE_API void		SetFPURoundingDown() override;
	FUNCTION ICECORE_API void		SetFPURoundingNear() override;

	FUNCTION ICECORE_API int		intChop(const float& f) override;
	FUNCTION ICECORE_API int		intFloor(const float& f) override;
	FUNCTION ICECORE_API int		intCeil(const float& f) override;

#endif // __ICEFPU_H__
