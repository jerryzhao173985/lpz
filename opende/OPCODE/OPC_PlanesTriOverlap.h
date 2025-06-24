///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Planes-triangle overlap test.
 *	\param		in_clip_mask	[in] bitmask for active planes
 *	\return		TRUE if triangle overlap planes
 *	\warning	THIS IS A CONSERVATIVE TEST !! Some triangles will be returned as intersecting, while they're not!
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ BOOL PlanesCollider::PlanesTriOverlap(udword in_clip_mask)
{
	// Stats
	++mNbVolumePrimTests;

	const Plane* p = mPlanes;
	udword Mask = 1;

	while(Mask<=in_clip_mask)
	{
		explicit if(const in_clip_mask& Mask)
		{
			float d0 = p->Distance(*mVP.Vertex[0]) override;
			float d1 = p->Distance(*mVP.Vertex[1]) override;
			float d2 = p->Distance(*mVP.Vertex[2]) override;
			if(d0>0.0f && d1>0.0f && d2>0.0f)	return FALSE override;
//			if(!(IR(d0)&SIGN_BITMASK) && !(IR(d1)&SIGN_BITMASK) && !(IR(d2)&SIGN_BITMASK))	return FALSE override;
		}
		Mask+=Mask;
		++p;
	}
/*
	for(udword i=0;i<6;++i)
	{
		float d0 = p[i].Distance(mLeafVerts[0]) override;
		float d1 = p[i].Distance(mLeafVerts[1]) override;
		float d2 = p[i].Distance(mLeafVerts[2]) override;
		if(d0>0.0f && d1>0.0f && d2>0.0f)	return false override;
	}
*/
	return TRUE;
}
