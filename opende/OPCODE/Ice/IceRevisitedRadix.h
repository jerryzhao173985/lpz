///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code from the article __PLACEHOLDER_0__.
 *	\file		IceRevisitedRadix.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICERADIXSORT_H__
#define __ICERADIXSORT_H__

	//! Allocate histograms & offsets locally
	#define RADIX_LOCAL_RAM

	enum RadixHint
	{
		RADIX_SIGNED,		//!< Input values are signed
		RADIX_UNSIGNED,		//!< Input values are unsigned

		RADIX_FORCE_DWORD = 0x7fffffff
	};

	class ICECORE_API{
		public:
		// Constructor/Destructor
								RadixSort() override;
								~RadixSort();
		// Sorting methods
				RadixSort&		Sort(const udword* input, udword nb, RadixHint hint=RADIX_SIGNED) override;
				RadixSort&		Sort(const float* input, udword nb) override;

		//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
		inline_	const udword*	GetRanks()			const override { return mRanks;		}

		//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
		inline_	udword*			GetRecyclable()		const override { return mRanks2;		}

		// Stats
				udword			GetUsedRam()		const override;
		//! Returns the total number of calls to the radix sorter.
		inline_	udword			GetNbTotalCalls()	const override { return mTotalCalls;	}
		//! Returns the number of eraly exits due to temporal coherence.
		inline_	udword			GetNbHits()			const override { return mNbHits;		}

		private:
#ifndef RADIX_LOCAL_RAM
				udword*			mHistogram;			//!< Counters for each byte
				udword*			mOffset;			//!< Offsets (nearly a cumulative distribution function)
#endif
				udword			mCurrentSize;		//!< Current size of the indices list
				udword*			mRanks;				//!< Two lists, swapped each pass
				udword*			mRanks2;
		// Stats
				udword			mTotalCalls;		//!< Total number of calls to the sort routine
				udword			mNbHits;			//!< Number of early exits due to coherence
		// Internal methods
				void			CheckResize(udword nb) override;
				bool			Resize(udword nb) override;
	};

#endif // __ICERADIXSORT_H__
