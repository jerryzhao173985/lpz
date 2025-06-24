///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Planes-AABB overlap test.
 *	- original code by Ville Miettinen, from Umbra/dPVS (released on the GD-Algorithms mailing list)
 *	- almost used __PLACEHOLDER_0__, I even left the comments (hence the frustum-related notes)
 *
 *	\param		center			[in] box center
 *	\param		extents			[in] box extents
 *	\param		out_clip_mask	[out] bitmask for active planes
 *	\param		in_clip_mask	[in] bitmask for active planes
 *	\return		TRUE if boxes overlap planes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ BOOL PlanesCollider::PlanesAABBOverlap(const Point& center, const Point& extents, udword& out_clip_mask, udword in_clip_mask)
{
	// Stats
	++mNbVolumeBVTests;

	const Plane* p = mPlanes;

	// Evaluate through all active frustum planes. We determine the relation 
	// between the AABB and a plane by using the concept of __PLACEHOLDER_1__ and __PLACEHOLDER_2__
