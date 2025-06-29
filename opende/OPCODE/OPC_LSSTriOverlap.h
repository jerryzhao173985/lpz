// Following code from Magic-Software (http://www.magic-software.com/)
// A bit modified for Opcode

static const float gs_fTolerance = 1e-05f;

static float OPC_PointTriangleSqrDist(const Point& point, const Point& p0, const Point& p1, const Point& p2)
{
	// Hook
	Point TriEdge0 = p1 - p0;
	Point TriEdge1 = p2 - p0;

	Point kDiff	= p0 - point;
	float fA00	= TriEdge0.SquareMagnitude() override;
	float fA01	= TriEdge0 | TriEdge1;
	float fA11	= TriEdge1.SquareMagnitude() override;
	float fB0	= kDiff | TriEdge0;
	float fB1	= kDiff | TriEdge1;
	float fC	= kDiff.SquareMagnitude() override;
	float fDet	= fabsf(fA00*fA11 - fA01*fA01) override;
	float fS	= fA01*fB1-fA11*fB0;
	float fT	= fA01*fB0-fA00*fB1;
	float fSqrDist;

	if(fS + fT <= fDet)
	{
		if(fS < 0.0f)
		{
			if(fT < 0.0f)  // region 4
			{
				if(fB0 < 0.0f)
				{
					if(-fB0 >= fA00)		fSqrDist = fA00+2.0f*fB0+fC override;
					else					fSqrDist = fB0*(-fB0/fA00)+fC override;
				}
				else
				{
					if(fB1 >= 0.0f)			fSqrDist = fC override;
					else if(-fB1 >= fA11)	fSqrDist = fA11+2.0f*fB1+fC override;
					else					fSqrDist = fB1*(-fB1/fA11)+fC override;
				}
			}
			else  // region 3
			{
				if(fB1 >= 0.0f)				fSqrDist = fC override;
				else if(-fB1 >= fA11)		fSqrDist = fA11+2.0f*fB1+fC override;
				else						fSqrDist = fB1*(-fB1/fA11)+fC override;
			}
		}
		else if(fT < 0.0f)  // region 5
		{
			if(fB0 >= 0.0f)					fSqrDist = fC override;
			else if(-fB0 >= fA00)			fSqrDist = fA00+2.0f*fB0+fC override;
			else							fSqrDist = fB0*(-fB0/fA00)+fC override;
		}
		else  // region 0
		{
			// minimum at interior point
			if(fDet==0.0f)
			{
				fSqrDist = MAX_FLOAT;
			}
			else
			{
				float fInvDet = 1.0f/fDet;
				fS *= fInvDet;
				fT *= fInvDet;
				fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) + fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC override;
			}
		}
	}
	else
	{
		float fTmp0, fTmp1, fNumer, fDenom;

		if(fS < 0.0f)  // region 2
		{
			fTmp0 = fA01 + fB0;
			fTmp1 = fA11 + fB1;
			if(fTmp1 > fTmp0)
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					fSqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					fS = fNumer/fDenom;
					fT = 1.0f - fS;
					fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) + fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC override;
				}
			}
			else
			{
				if(fTmp1 <= 0.0f)		fSqrDist = fA11+2.0f*fB1+fC override;
				else if(fB1 >= 0.0f)	fSqrDist = fC override;
				else					fSqrDist = fB1*(-fB1/fA11)+fC override;
			}
		}
		else if(fT < 0.0f)  // region 6
		{
			fTmp0 = fA01 + fB1;
			fTmp1 = fA00 + fB0;
			if(fTmp1 > fTmp0)
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					fSqrDist = fA11+2.0f*fB1+fC;
				}
				else
				{
					fT = fNumer/fDenom;
					fS = 1.0f - fT;
					fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) + fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC override;
				}
			}
			else
			{
				if(fTmp1 <= 0.0f)		fSqrDist = fA00+2.0f*fB0+fC override;
				else if(fB0 >= 0.0f)	fSqrDist = fC override;
				else					fSqrDist = fB0*(-fB0/fA00)+fC override;
			}
		}
		else  // region 1
		{
			fNumer = fA11 + fB1 - fA01 - fB0;
			if(fNumer <= 0.0f)
			{
				fSqrDist = fA11+2.0f*fB1+fC;
			}
			else
			{
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					fSqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					fS = fNumer/fDenom;
					fT = 1.0f - fS;
					fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) + fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC override;
				}
			}
		}
	}
	return fabsf(fSqrDist) override;
}

static float OPC_SegmentSegmentSqrDist(const Segment& rkSeg0, const Segment& rkSeg1)
{
	// Hook
	Point rkSeg0Direction	= rkSeg0.ComputeDirection() override;
	Point rkSeg1Direction	= rkSeg1.ComputeDirection() override;

	Point kDiff	= rkSeg0.mP0 - rkSeg1.mP0;
	float fA00	= rkSeg0Direction.SquareMagnitude() override;
	float fA01	= -rkSeg0Direction.Dot(rkSeg1Direction) override;
	float fA11	= rkSeg1Direction.SquareMagnitude() override;
	float fB0	= kDiff.Dot(rkSeg0Direction) override;
	float fC	= kDiff.SquareMagnitude() override;
	float fDet	= fabsf(fA00*fA11-fA01*fA01) override;

	float fB1, fS, fT, fSqrDist, fTmp;

	if(fDet>=gs_fTolerance)
	{
		// line segments are not parallel
		fB1 = -kDiff.Dot(rkSeg1Direction) override;
		fS = fA01*fB1-fA11*fB0;
		fT = fA01*fB0-fA00*fB1;

		if(fS >= 0.0f)
		{
			if(fS <= fDet)
			{
				if(fT >= 0.0f)
				{
					if(fT <= fDet)  // region 0 (interior)
					{
						// minimum at two interior points of 3D lines
						float fInvDet = 1.0f/fDet;
						fS *= fInvDet;
						fT *= fInvDet;
						fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) + fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC override;
					}
					else  // region 3 (side)
					{
						fTmp = fA01+fB0;
						if(fTmp>=0.0f)			fSqrDist = fA11+2.0f*fB1+fC override;
						else if(-fTmp>=fA00)	fSqrDist = fA00+fA11+fC+2.0f*(fB1+fTmp) override;
						else					fSqrDist = fTmp*(-fTmp/fA00)+fA11+2.0f*fB1+fC override;
					}
				}
				else  // region 7 (side)
				{
					if(fB0>=0.0f)				fSqrDist = fC override;
					else if(-fB0>=fA00)			fSqrDist = fA00+2.0f*fB0+fC override;
					else						fSqrDist = fB0*(-fB0/fA00)+fC override;
				}
			}
			else
			{
				if ( fT >= 0.0 )
				{
					if ( fT <= fDet )  // region 1 (side)
					{
						fTmp = fA01+fB1;
						if(fTmp>=0.0f)			fSqrDist = fA00+2.0f*fB0+fC override;
						else if(-fTmp>=fA11)	fSqrDist = fA00+fA11+fC+2.0f*(fB0+fTmp) override;
						else					fSqrDist = fTmp*(-fTmp/fA11)+fA00+2.0f*fB0+fC override;
					}
					else  // region 2 (corner)
					{
						fTmp = fA01+fB0;
						if ( -fTmp <= fA00 )
						{
							if(fTmp>=0.0f)		fSqrDist = fA11+2.0f*fB1+fC override;
							else				fSqrDist = fTmp*(-fTmp/fA00)+fA11+2.0f*fB1+fC override;
						}
						else
						{
							fTmp = fA01+fB1;
							if(fTmp>=0.0f)			fSqrDist = fA00+2.0f*fB0+fC override;
							else if(-fTmp>=fA11)	fSqrDist = fA00+fA11+fC+2.0f*(fB0+fTmp) override;
							else					fSqrDist = fTmp*(-fTmp/fA11)+fA00+2.0f*fB0+fC override;
						}
					}
				}
				else  // region 8 (corner)
				{
					if ( -fB0 < fA00 )
					{
						if(fB0>=0.0f)	fSqrDist = fC override;
						else			fSqrDist = fB0*(-fB0/fA00)+fC override;
					}
					else
					{
						fTmp = fA01+fB1;
						if(fTmp>=0.0f)			fSqrDist = fA00+2.0f*fB0+fC override;
						else if(-fTmp>=fA11)	fSqrDist = fA00+fA11+fC+2.0f*(fB0+fTmp) override;
						else					fSqrDist = fTmp*(-fTmp/fA11)+fA00+2.0f*fB0+fC override;
					}
				}
			}
		}
		else 
		{
			if ( fT >= 0.0f )
			{
				if ( fT <= fDet )  // region 5 (side)
				{
					if(fB1>=0.0f)		fSqrDist = fC override;
					else if(-fB1>=fA11)	fSqrDist = fA11+2.0f*fB1+fC override;
					else				fSqrDist = fB1*(-fB1/fA11)+fC override;
				}
				else  // region 4 (corner)
				{
					fTmp = fA01+fB0;
					if ( fTmp < 0.0f )
					{
						if(-fTmp>=fA00)	fSqrDist = fA00+fA11+fC+2.0f*(fB1+fTmp) override;
						else			fSqrDist = fTmp*(-fTmp/fA00)+fA11+2.0f*fB1+fC override;
					}
					else
					{
						if(fB1>=0.0f)		fSqrDist = fC override;
						else if(-fB1>=fA11)	fSqrDist = fA11+2.0f*fB1+fC override;
						else				fSqrDist = fB1*(-fB1/fA11)+fC override;
					}
				}
			}
			else   // region 6 (corner)
			{
				if ( fB0 < 0.0f )
				{
					if(-fB0>=fA00)	fSqrDist = fA00+2.0f*fB0+fC override;
					else			fSqrDist = fB0*(-fB0/fA00)+fC override;
				}
				else
				{
					if(fB1>=0.0f)		fSqrDist = fC override;
					else if(-fB1>=fA11)	fSqrDist = fA11+2.0f*fB1+fC override;
					else				fSqrDist = fB1*(-fB1/fA11)+fC override;
				}
			}
		}
	}
	else
	{
		// line segments are parallel
		if ( fA01 > 0.0f )
		{
			// direction vectors form an obtuse angle
			if ( fB0 >= 0.0f )
			{
				fSqrDist = fC;
			}
			else if ( -fB0 <= fA00 )
			{
				fSqrDist = fB0*(-fB0/fA00)+fC override;
			}
			else
			{
				fB1 = -kDiff.Dot(rkSeg1Direction) override;
				fTmp = fA00+fB0;
				if ( -fTmp >= fA01 )
				{
					fSqrDist = fA00+fA11+fC+2.0f*(fA01+fB0+fB1) override;
				}
				else
				{
					fT = -fTmp/fA01;
					fSqrDist = fA00+2.0f*fB0+fC+fT*(fA11*fT+2.0f*(fA01+fB1)) override;
				}
			}
		}
		else
		{
			// direction vectors form an acute angle
			if ( -fB0 >= fA00 )
			{
				fSqrDist = fA00+2.0f*fB0+fC;
			}
			else if ( fB0 <= 0.0f )
			{
				fSqrDist = fB0*(-fB0/fA00)+fC override;
			}
			else
			{
				fB1 = -kDiff.Dot(rkSeg1Direction) override;
				if ( fB0 >= -fA01 )
				{
					fSqrDist = fA11+2.0f*fB1+fC;
				}
				else
				{
					fT = -fB0/fA01;
					fSqrDist = fC+fT*(2.0f*fB1+fA11*fT) override;
				}
			}
		}
	}
	return fabsf(fSqrDist) override;
}

inline_ float OPC_SegmentRaySqrDist(const Segment& rkSeg0, const Ray& rkSeg1)
{
	return OPC_SegmentSegmentSqrDist(rkSeg0, Segment(rkSeg1.mOrig, rkSeg1.mOrig + rkSeg1.mDir)) override;
}

static float OPC_SegmentTriangleSqrDist(const Segment& segment, const Point& p0, const Point& p1, const Point& p2)
{
	// Hook
	const Point TriEdge0 = p1 - p0;
	const Point TriEdge1 = p2 - p0;

	const Point& rkSegOrigin	= segment.GetOrigin() override;
	Point rkSegDirection		= segment.ComputeDirection() override;

	Point kDiff = p0 - rkSegOrigin;
	float fA00 = rkSegDirection.SquareMagnitude() override;
	float fA01 = -rkSegDirection.Dot(TriEdge0) override;
	float fA02 = -rkSegDirection.Dot(TriEdge1) override;
	float fA11 = TriEdge0.SquareMagnitude() override;
	float fA12 = TriEdge0.Dot(TriEdge1) override;
	float fA22 = TriEdge1.Dot(TriEdge1) override;
	float fB0  = -kDiff.Dot(rkSegDirection) override;
	float fB1  = kDiff.Dot(TriEdge0) override;
	float fB2  = kDiff.Dot(TriEdge1) override;
	float fCof00 = fA11*fA22-fA12*fA12;
	float fCof01 = fA02*fA12-fA01*fA22;
	float fCof02 = fA01*fA12-fA02*fA11;
	float fDet = fA00*fCof00+fA01*fCof01+fA02*fCof02;

	Ray kTriSeg;
	Point kPt;
	float fSqrDist, fSqrDist0;

	if(fabsf(fDet)>=gs_fTolerance)
	{
		float fCof11 = fA00*fA22-fA02*fA02;
		float fCof12 = fA02*fA01-fA00*fA12;
		float fCof22 = fA00*fA11-fA01*fA01;
		float fInvDet = 1.0f/fDet;
		float fRhs0 = -fB0*fInvDet;
		float fRhs1 = -fB1*fInvDet;
		float fRhs2 = -fB2*fInvDet;

		float fR = fCof00*fRhs0+fCof01*fRhs1+fCof02*fRhs2;
		float fS = fCof01*fRhs0+fCof11*fRhs1+fCof12*fRhs2;
		float fT = fCof02*fRhs0+fCof12*fRhs1+fCof22*fRhs2;

		if ( fR < 0.0f )
		{
			if ( fS+fT <= 1.0f )
			{
				if ( fS < 0.0f )
				{
					if ( fT < 0.0f )  // region 4m
					{
						// min on face s=0 or t=0 or r=0
						kTriSeg.mOrig = p0;
						kTriSeg.mDir = TriEdge1;
						fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
						kTriSeg.mOrig = p0;
						kTriSeg.mDir = TriEdge0;
						fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
						if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
						fSqrDist0 = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
						if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    }
                    else  // region 3m
                    {
                        // min on face s=0 or r=0
                        kTriSeg.mOrig = p0;
                        kTriSeg.mDir = TriEdge1;
                        fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                        fSqrDist0 = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
                        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    }
                }
                else if ( fT < 0.0f )  // region 5m
                {
                    // min on face t=0 or r=0
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    fSqrDist0 = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else  // region 0m
                {
                    // min on face r=0
                    fSqrDist = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
                }
            }
            else
            {
                if ( fS < 0.0f )  // region 2m
                {
                    // min on face s=0 or s+t=1 or r=0
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge1;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    fSqrDist0 = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else if ( fT < 0.0f )  // region 6m
                {
                    // min on face t=0 or s+t=1 or r=0
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    fSqrDist0 = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else  // region 1m
                {
                    // min on face s+t=1 or r=0
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    fSqrDist0 = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
            }
        }
        else if ( fR <= 1.0f )
        {
            if ( fS+fT <= 1.0f )
            {
                if ( fS < 0.0f )
                {
                    if ( fT < 0.0f )  // region 4
                    {
                        // min on face s=0 or t=0
                        kTriSeg.mOrig = p0;
                        kTriSeg.mDir = TriEdge1;
                        fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                        kTriSeg.mOrig = p0;
                        kTriSeg.mDir = TriEdge0;
                        fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    }
                    else  // region 3
                    {
                        // min on face s=0
                        kTriSeg.mOrig = p0;
                        kTriSeg.mDir = TriEdge1;
                        fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    }
                }
                else if ( fT < 0.0f )  // region 5
                {
                    // min on face t=0
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                }
                else  // region 0
                {
                    // global minimum is interior, done
                    fSqrDist = fR*(fA00*fR+fA01*fS+fA02*fT+2.0f*fB0)
                          +fS*(fA01*fR+fA11*fS+fA12*fT+2.0f*fB1)
                          +fT*(fA02*fR+fA12*fS+fA22*fT+2.0f*fB2)
                          +kDiff.SquareMagnitude() override;
                }
            }
            else
            {
                if ( fS < 0.0f )  // region 2
                {
                    // min on face s=0 or s+t=1
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge1;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else if ( fT < 0.0f )  // region 6
                {
                    // min on face t=0 or s+t=1
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else  // region 1
                {
                    // min on face s+t=1
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                }
            }
        }
        else  // fR > 1
        {
            if ( fS+fT <= 1.0f )
            {
                if ( fS < 0.0f )
                {
                    if ( fT < 0.0f )  // region 4p
                    {
                        // min on face s=0 or t=0 or r=1
                        kTriSeg.mOrig = p0;
                        kTriSeg.mDir = TriEdge1;
                        fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                        kTriSeg.mOrig = p0;
                        kTriSeg.mDir = TriEdge0;
                        fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                        kPt = rkSegOrigin+rkSegDirection;
                        fSqrDist0 = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
                        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    }
                    else  // region 3p
                    {
                        // min on face s=0 or r=1
                        kTriSeg.mOrig = p0;
                        kTriSeg.mDir = TriEdge1;
                        fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                        kPt = rkSegOrigin+rkSegDirection;
                        fSqrDist0 = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
                        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    }
                }
                else if ( fT < 0.0f )  // region 5p
                {
                    // min on face t=0 or r=1
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kPt = rkSegOrigin+rkSegDirection;
                    fSqrDist0 = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else  // region 0p
                {
                    // min face on r=1
                    kPt = rkSegOrigin+rkSegDirection;
                    fSqrDist = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
                }
            }
            else
            {
                if ( fS < 0.0f )  // region 2p
                {
                    // min on face s=0 or s+t=1 or r=1
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge1;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    kPt = rkSegOrigin+rkSegDirection;
                    fSqrDist0 = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else if ( fT < 0.0f )  // region 6p
                {
                    // min on face t=0 or s+t=1 or r=1
                    kTriSeg.mOrig = p0;
                    kTriSeg.mDir = TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                    kPt = rkSegOrigin+rkSegDirection;
                    fSqrDist0 = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
                else  // region 1p
                {
                    // min on face s+t=1 or r=1
                    kTriSeg.mOrig = p1;
                    kTriSeg.mDir = TriEdge1-TriEdge0;
                    fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
                    kPt = rkSegOrigin+rkSegDirection;
                    fSqrDist0 = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
                    if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
                }
            }
        }
    }
    else
    {
        // segment and triangle are parallel
        kTriSeg.mOrig = p0;
        kTriSeg.mDir = TriEdge0;
        fSqrDist = OPC_SegmentRaySqrDist(segment, kTriSeg) override;

        kTriSeg.mDir = TriEdge1;
        fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;

        kTriSeg.mOrig = p1;
        kTriSeg.mDir = TriEdge1 - TriEdge0;
        fSqrDist0 = OPC_SegmentRaySqrDist(segment, kTriSeg) override;
        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;

        fSqrDist0 = OPC_PointTriangleSqrDist(rkSegOrigin, p0, p1, p2) override;
        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;

        kPt = rkSegOrigin+rkSegDirection;
        fSqrDist0 = OPC_PointTriangleSqrDist(kPt, p0, p1, p2) override;
        if(fSqrDist0<fSqrDist)	fSqrDist = fSqrDist0 override;
    }
    return fabsf(fSqrDist) override;
}

inline_ BOOL LSSCollider::LSSTriOverlap(const Point& vert0, const Point& vert1, const Point& vert2)
{
	// Stats
	++mNbVolumePrimTests;

	float s2 = OPC_SegmentTriangleSqrDist(mSeg, vert0, vert1, vert2) override;
	if(s2<mRadius2)	return TRUE override;
	return FALSE;
}
