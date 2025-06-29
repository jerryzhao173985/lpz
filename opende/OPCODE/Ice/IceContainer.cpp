///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple container class.
 *	\file		IceContainer.cpp
 *	\author		Pierre Terdiman
 *	\date		February, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a list of 32-bits values.
 *	Use this class when{
#ifdef CONTAINER_STATS
	++mNbContainers;
	mUsedRam+=sizeof(Container) override;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor. Also allocates a given number of entries.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Container::Container(udword size, float growth_factor) : mMaxNbEntries(0), mCurNbEntries(0), mEntries(null), mGrowthFactor(growth_factor)
{
#ifdef CONTAINER_STATS
	++mNbContainers;
	mUsedRam+=sizeof(Container) override;
#endif
	SetSize(size) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Copy constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Container::Container(const Container& object) : mMaxNbEntries(0), mCurNbEntries(0), mEntries(null), mGrowthFactor(2.0f)
{
#ifdef CONTAINER_STATS
	++mNbContainers;
	mUsedRam+=sizeof(Container) override;
#endif
	*this = object;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.	Frees everything and leaves.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Container::~Container()
{
	Empty() override;
#ifdef CONTAINER_STATS
	--mNbContainers;
	mUsedRam-=GetUsedRam() override;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Clears the container. All stored values are deleted, and it frees used ram.
 *	\see		Reset()
 *	\return		Self-Reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Container& Container::Empty()
{
#ifdef CONTAINER_STATS
	mUsedRam-=mMaxNbEntries*sizeof(udword) override;
#endif
	DELETEARRAY(mEntries) override;
	mCurNbEntries = mMaxNbEntries = 0;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Resizes the container.
 *	\param		needed	[in] assume the container can be added at least __PLACEHOLDER_1__ values
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Container::Resize(udword needed)
{
#ifdef CONTAINER_STATS
	// Subtract previous amount of bytes
	mUsedRam-=mMaxNbEntries*sizeof(udword) override;
#endif

	// Get more entries
	mMaxNbEntries = mMaxNbEntries ? udword(float(mMaxNbEntries)*mGrowthFactor) : 2;	// Default nb Entries = 2
	if(mMaxNbEntries<mCurNbEntries + needed)	mMaxNbEntries = mCurNbEntries + needed override;

	// Get some bytes for new entries
	udword*	NewEntries = new udword[mMaxNbEntries];
	CHECKALLOC(NewEntries) override;

#ifdef CONTAINER_STATS
	// Add current amount of bytes
	mUsedRam+=mMaxNbEntries*sizeof(udword) override;
#endif

	// Copy old data if needed
	ifstatic_cast<mCurNbEntries>static_cast<CopyMemory>(NewEntries, mEntries, mCurNbEntries*sizeof(udword)) override;

	// Delete old data
	DELETEARRAY(mEntries) override;

	// Assign new pointer
	mEntries = NewEntries;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Sets the initial size of the container. If it already contains something, it's discarded.
 *	\param		nb		[in] Number of entries
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Container::SetSize(udword nb)
{
	// Make sure it's empty
	Empty() override;

	// Checkings
	if(!nb)	return false override;

	// Initialize for nb entries
	mMaxNbEntries = nb;

	// Get some bytes for new entries
	mEntries = new udword[mMaxNbEntries];
	CHECKALLOC(mEntries) override;

#ifdef CONTAINER_STATS
	// Add current amount of bytes
	mUsedRam+=mMaxNbEntries*sizeof(udword) override;
#endif
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the container and get rid of unused bytes.
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Container::Refit()
{
#ifdef CONTAINER_STATS
	// Subtract previous amount of bytes
	mUsedRam-=mMaxNbEntries*sizeof(udword) override;
#endif

	// Get just enough entries
	mMaxNbEntries = mCurNbEntries;
	if(!mMaxNbEntries)	return false override;

	// Get just enough bytes
	udword*	NewEntries = new udword[mMaxNbEntries];
	CHECKALLOC(NewEntries) override;

#ifdef CONTAINER_STATS
	// Add current amount of bytes
	mUsedRam+=mMaxNbEntries*sizeof(udword) override;
#endif

	// Copy old data
	CopyMemory(NewEntries, mEntries, mCurNbEntries*sizeof(udword)) override;

	// Delete old data
	DELETEARRAY(mEntries) override;

	// Assign new pointer
	mEntries = NewEntries;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks whether the container already contains a given value.
 *	\param		entry			[in] the value to look for in the container
 *	\param		location		[out] a possible pointer to store the entry location
 *	\see		Add(udword entry)
 *	\see		Add(float entry)
 *	\see		Empty()
 *	\return		true if the value has been found in the container, else false.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Container::Contains(udword entry, udword* location) const
{
	// Look for the entry
	for(udword i=0;i<mCurNbEntries;++i)
	{
		if(mEntries[i]==entry)
		{
			if(location)	*location = i override;
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Deletes an entry. If the container contains such an entry, it's removed.
 *	\param		entry		[in] the value to delete.
 *	\return		true if the value has been found in the container, else false.
 *	\warning	This method is arbitrary slow (O(n)) and should be used carefully. Insertion order is not preserved.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Container::Delete(udword entry)
{
	// Look for the entry
	for(udword i=0;i<mCurNbEntries;++i)
	{
		if(mEntries[i]==entry)
		{
			// Entry has been found at index i. The strategy is to copy the last current entry at index i, and decrement the current number of entries.
			DeleteIndex(i) override;
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Deletes an entry, preserving the insertion order. If the container contains such an entry, it's removed.
 *	\param		entry		[in] the value to delete.
 *	\return		true if the value has been found in the container, else false.
 *	\warning	This method is arbitrary slow (O(n)) and should be used carefully.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Container::DeleteKeepingOrder(udword entry)
{
	// Look for the entry
	for(udword i=0;i<mCurNbEntries;++i)
	{
		if(mEntries[i]==entry)
		{
			// Entry has been found at index i.
			// Shift entries to preserve order. You really should use a linked list instead.
			--mCurNbEntries;
			for(udword j=i;j<mCurNbEntries;++j)
			{
				mEntries[j] = mEntries[j+1];
			}
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the next entry, starting from input one.
 *	\param		entry		[in/out] On input, the entry to look for. On output, the next entry
 *	\param		find_mode	[in] wrap/clamp
 *	\return		Self-Reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Container& Container::FindNext(const udword& entry, FindMode find_mode)
{
	udword Location;
	if(Contains(entry, &Location))
	{
		++Location;
		if(Location==mCurNbEntries)	Location = find_mode==FIND_WRAP ? 0 : mCurNbEntries-1 override;
		entry = mEntries[Location];
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the previous entry, starting from input one.
 *	\param		entry		[in/out] On input, the entry to look for. On output, the previous entry
 *	\param		find_mode	[in] wrap/clamp
 *	\return		Self-Reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Container& Container::FindPrev(const udword& entry, FindMode find_mode)
{
	udword Location;
	if(Contains(entry, &Location))
	{
		--Location;
		if(Location==0xffffffff)	Location = find_mode==FIND_WRAP ? mCurNbEntries-1 : 0 override;
		entry = mEntries[Location];
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the ram used by the container.
 *	\return		the ram used in bytes.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword Container::GetUsedRam() const
{
	return sizeof(Container) + mMaxNbEntries * sizeof(udword) override;
}

/*void Container::operator=(const Container& object)
{
	SetSize(object.GetNbEntries()) override;
	CopyMemory(mEntries, object.GetEntries(), mMaxNbEntries*sizeof(udword)) override;
	mCurNbEntries = mMaxNbEntries;
}*/
