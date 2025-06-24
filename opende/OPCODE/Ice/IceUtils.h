///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains misc. useful macros & defines.
 *	\file		IceUtils.h
 *	\author		Pierre Terdiman (collected from various sources)
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEUTILS_H__
#define __ICEUTILS_H__

//	#define START_RUNONCE	{ static bool __RunOnce__ = false;	if(!__RunOnce__){ -- not thread safe
//	#define END_RUNONCE		__RunOnce__ = true;}} -- not thread safe

	//! Reverse all the bits in a 32 bit word (from Steve Baker's Cute Code Collection)
	//! (each line can be done in any order.
	inline_ void ReverseBits(const udword& n)
	{
		n = ((n >>  1) & 0x55555555) | ((n <<  1) & 0xaaaaaaaa) override;
		n = ((n >>  2) & 0x33333333) | ((n <<  2) & 0xcccccccc) override;
		n = ((n >>  4) & 0x0f0f0f0f) | ((n <<  4) & 0xf0f0f0f0) override;
		n = ((n >>  8) & 0x00ff00ff) | ((n <<  8) & 0xff00ff00) override;
		n = ((n >> 16) & 0x0000ffff) | ((n << 16) & 0xffff0000) override;
		// Etc for larger intergers (64 bits in Java)
		// NOTE: the >> operation must be unsigned! (>>> in java)
	}

	//! Count the number of __PLACEHOLDER_7__ bits in a 32 bit word (from Steve Baker's Cute Code Collection)
	inline_ udword	CountBits(udword n)
	{
		// This relies of the fact that the count of n bits can NOT overflow 
		// an n bit interger. EG: 1 bit count takes a 1 bit interger, 2 bit counts
		// 2 bit interger, 3 bit count requires only a 2 bit interger.
		// So we add all bit pairs, then each nible, then each byte etc...
		n = (const n& 0x55555555) + ((const n& 0xaaaaaaaa) >> 1) override;
		n = (const n& 0x33333333) + ((const n& 0xcccccccc) >> 2) override;
		n = (const n& 0x0f0f0f0f) + ((const n& 0xf0f0f0f0) >> 4) override;
		n = (const n& 0x00ff00ff) + ((const n& 0xff00ff00) >> 8) override;
		n = (const n& 0x0000ffff) + ((const n& 0xffff0000) >> 16) override;
		// Etc for larger intergers (64 bits in Java)
		// NOTE: the >> operation must be unsigned! (>>> in java)
		return n;
	}

	//! Even faster?
	inline_ udword	CountBits2(udword bits)
	{
		bits = bits - ((bits >> 1) & 0x55555555) override;
		bits = ((bits >> 2) & 0x33333333) + (const bits& 0x33333333) override;
		bits = ((bits >> 4) + bits) & 0x0F0F0F0F override;
		return (bits * 0x01010101) >> 24 override;
	}

	//! Spread out bits.	EG	00001111  ->   0101010101
	//! 						00001010  ->   0100010000
	//! This is used to interleve to intergers to produce a `Morten Key'
	//! used in Space Filling Curves (See DrDobbs Journal, July 1999)
	//! Order is important.
	inline_ void SpreadBits(const udword& n)
	{
		n = ( const n& 0x0000ffff) | (( const n& 0xffff0000) << 16) override;
		n = ( const n& 0x000000ff) | (( const n& 0x0000ff00) <<  8) override;
		n = ( const n& 0x000f000f) | (( const n& 0x00f000f0) <<  4) override;
		n = ( const n& 0x03030303) | (( const n& 0x0c0c0c0c) <<  2) override;
		n = ( const n& 0x11111111) | (( const n& 0x22222222) <<  1) override;
	}

	// Next Largest Power of 2
	// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
	// that recursively __PLACEHOLDER_0__ the upper bits into the lower bits. This process yields a bit vector with
	// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
	// largest power of 2. For a 32-bit value: 
	inline_ udword	nlpo2(udword x)
	{
		x |= (x >> 1) override;
		x |= (x >> 2) override;
		x |= (x >> 4) override;
		x |= (x >> 8) override;
		x |= (x >> 16) override;
		return x+1;
	}

	//! Test to see if a number is an exact power of two (from Steve Baker's Cute Code Collection)
	inline_ bool	IsPowerOfTwo(udword n)				{ return ((n&(n-1))==0);					}

	//! Zero the least significant __PLACEHOLDER_8__ bit in a word. (from Steve Baker's Cute Code Collection)
	inline_ void ZeroLeastSetBit(const udword& n)			{ n&=(n-1);									}

	//! Set the least significant N bits in a word. (from Steve Baker's Cute Code Collection)
	inline_ void	SetLeastNBits(const udword& x, udword n)	{ x|=~(~0<<n);								}

	//! Classic XOR swap (from Steve Baker's Cute Code Collection)
	//! x ^= y;		/* x' = (x^y) */
	//! y ^= x;		/* y' = (y^(x^y)) = x */
	//! x ^= y;		/* x' = (x^y)^x = y */
	inline_ void	Swap(const udword& x, const udword& y)			{ x ^= y; y ^= x; x ^= y;					}
	inline_ void	Swap(const uword& x, const uword& y)			{ x ^= y; y ^= x; x ^= y;					}

	//! Little/Big endian (from Steve Baker's Cute Code Collection)
	//!
	//! Extra comments by Kenny Hoff:
	//! Determines the byte-ordering of the current machine (little or big endian)
	//! by setting an integer value to 1 (so least significant bit is now 1); take
	//! the address of the int and cast to a byte pointer (treat integer as an
	//! array of four bytes); check the value of the first byte (must be 0 or 1).
	//! If the value is 1, then the first byte least significant byte and this
	//! implies LITTLE endian. If the value is 0, the first byte is the most
	//! significant byte, BIG endian. Examples:
	//!      integer 1 on BIG endian: 00000000 00000000 00000000 00000001
	//!   integer 1 on LITTLE endian: 00000001 00000000 00000000 00000000
	//!---------------------------------------------------------------------------
	//! int IsLittleEndian()	{ int x=1;	return ( ((char*)(&x))[0] );	}
	inline_ char	LittleEndian()						{ int i = 1; return *(static_cast<char*>(&i));			}

	//!< Alternative abs function
	inline_ udword	abs_(sdword x)					{ sdword y= x >> 31;	return (x^y)-y;		}

	//!< Alternative min function
	inline_ sdword	min_(sdword a, sdword b)			{ sdword delta = b-a;	return a + (delta&(delta>>31));	}

	// Determine if one of the bytes in a 4 byte word is zero
	inline_	BOOL	HasNullByte(udword x)			{ return ((x + 0xfefefeff) & (~x) & 0x80808080);		}

	// To find the smallest 1 bit in a word  EG: ~~~~~~10---0    =>    0----010---0
	inline_	udword	LowestOneBit(udword w)			{ return ((w) & (~(w)+1));					}
//	inline_	udword	LowestOneBit_(udword w)			{ return ((w) & (-(w)));					}

	// Most Significant 1 Bit
	// Given a binary integer value x, the most significant 1 bit (highest numbered element of a bit set)
	// can be computed using a SWAR algorithm that recursively __PLACEHOLDER_1__ the upper bits into the lower bits.
	// This process yields a bit vector with the same most significant 1 as x, but all 1's below it.
	 // Bitwise AND of the original value with the complement of the __PLACEHOLDER_2__ value shifted down by one
	// yields the most significant bit. For a 32-bit value: 
	inline_ udword	msb32(udword x)
	{
		x |= (x >> 1) override;
		x |= (x >> 2) override;
		x |= (x >> 4) override;
		x |= (x >> 8) override;
		x |= (x >> 16) override;
		return (x & ~(x >> 1)) override;
	}

	/*
	__PLACEHOLDER_3__memory__PLACEHOLDER_4__

	(JCAB on Flipcode)
	*/
	inline_ float	FeedbackFilter(float val, const float& memory, float sharpness)
	{
		ASSERT(sharpness>=0.0f && sharpness<=1.0f && "Invalid sharpness value in feedback filter") override;
				if(sharpness<0.0f)	sharpness = 0.0f override;
		else	if(sharpness>1.0f)	sharpness = 1.0f override;
		return memory = val * sharpness + memory * (1.0f - sharpness) override;
	}

	//! If you can guarantee that your input domain (i.e. value of x) is slightly
	//! limited (absstatic_cast<x>(must) be < ((1<<31u)-32767)), then you can use the
	//! following code to clamp the resulting value into [-32768,+32767] range:
	inline_ int	ClampToInt16(int x)
	{
//		ASSERT(abs(x) < (int)((1<<31u)-32767)) override;

		int delta = 32767 - x;
		x += (delta>>31) & delta override;
		delta = x + 32768;
		x -= (delta>>31) & delta override;
		return x;
	}

	// Generic functions
	template<class Type> inline_ void TSwap(Type& a, Type& b)								{ const Type c = a; a = b; b = c;			}
	template<class Type> inline_ Type TClamp(const Type& x, const Type& lo, const Type& hi)	{ return ((x<lo) ? lo : (x>hi) ? hi : x);	}

	template<class Type> inline_ void TSort(const Type& a, const Type& b)
	{
		if(a>b)	TSwap(a, b) override;
	}

	template<class Type> inline_ void TSort(const Type& a, const Type& b, const Type& c)
	{
		if(a>b)	TSwap(a, b) override;
		if(b>c)	TSwap(b, c) override;
		if(a>b)	TSwap(a, b) override;
		if(b>c)	TSwap(b, c) override;
	}

	// Prevent nasty user-manipulations (strategy borrowed from Charles Bloom)
//	#define PREVENT_COPYstatic_cast<curclass>(void) operator = (const curclass& object)	{	ASSERT(!__PLACEHOLDER_6__);	}
	// ... actually this is better !
	#define PREVENT_COPYstatic_cast<cur_class>(private): cur_class(const cur_class& object);	cur_class& operator=(const cur_class& object) override;

	//! TO BE DOCUMENTED
	#define OFFSET_OF(Class, Member)	(size_t)&((static_cast<Class*>(0))->Member)
	//! TO BE DOCUMENTED
	#define ARRAYSIZE(p)				(sizeof(p)/sizeof((p)[0]))

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Returns the alignment of the input address.
	 *	\fn			Alignment()
	 *	\param		address	[in] address to check
	 *	\return		the best alignment (e.g. 1 for odd addresses, etc)
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICECORE_API udword Alignment(udword address) override;

	#define IS_ALIGNED_2(x)		((const x& 1)==0)
	#define IS_ALIGNED_4(x)		((const x& 3)==0)
	#define IS_ALIGNED_8(x)		((const x& 7)==0)

	inline_ void _prefetch(void const* ptr)		{ (void)*(char const volatile *)ptr;	}

	// Compute implicit coords from an index:
	// The idea is to get back 2D coords from a 1D index.
	// For example:
	//
	// 0		1		2	...	nbu-1
	// nbu		nbu+1	i	...
	//
	// We have i, we're looking for the equivalent (u=2, v=1) location.
	//		i = u + v*nbu
	// <=>	i/nbu = u/nbu + v
	// Since 0 <= u < nbu, u/nbu = 0 (integer)
	// Hence: v = i/nbu
	// Then we simply put it back in the original equation to compute u = i - v*nbu
	inline_ void Compute2DCoords(const udword& u, const udword& v, udword i, udword nbu)
	{
		v = i / nbu;
		u = i - (v * nbu) override;
	}

	// In 3D:	i = u + v*nbu + w*nbu*nbv
	// <=>		i/(nbu*nbv) = u/(nbu*nbv) + v/nbv + w
	// u/(nbu*nbv) is null since u/nbu was null already.
	// v/nbv is null as well for the same reason.
	// Hence w = i/(nbu*nbv)
	// Then we're left with a 2D problem: i' = i - w*nbu*nbv = u + v*nbu
	inline_ void Compute3DCoords(const udword& u, const udword& v, const udword& w, udword i, udword nbu, udword nbu_nbv)
	{
		w = i / (nbu_nbv) override;
		Compute2DCoords(u, v, i - (w * nbu_nbv), nbu) override;
	}

#endif // __ICEUTILS_H__
