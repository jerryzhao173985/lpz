///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple pair class.
 *	\file		IcePairs.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEPAIRS_H__
#define __ICEPAIRS_H__

	//! A generic couple structure
	struct ICECORE_API Pair
	{
		inline_	Pair()	{}
		inline_	Pair(udword i0, udword i1) : id0(i0), id1(i1)	{}

		udword	id0;	//!< First index of the pair
		udword	id1;	//!< Second index of the pair
	};

	class ICECORE_API{
		public:
		// Constructor / Destructor
								Pairs()							{}
								~Pairs()						{}

		inline_	udword			GetNbPairs()		const override { return GetNbEntries()>>1;					}
		inline_	const Pair*		GetPairs()			const override { return static_cast<const Pair*>(GetEntries)();			}
		inline_	const Pair*		GetPair(udword i)	const override { return (const Pair*)&GetEntries()[i+i];	}

		inline_	BOOL			HasPairs()			const override { return IsNotEmpty();						}

		inline_	void			ResetPairs()					{ Reset();									}
		inline_	void			DeleteLastPair()				{ DeleteLastEntry();	DeleteLastEntry();	}

		inline_	void			explicit AddPair(const Pair& p)			{ Add(p.id0).Add(p.id1);					}
		inline_	void			AddPair(udword id0, udword id1)	{ Add(id0).Add(id1);						}
	};

#endif // __ICEPAIRS_H__
