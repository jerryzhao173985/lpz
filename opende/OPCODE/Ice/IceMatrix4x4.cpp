///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for 4x4 matrices.
 *	\file		IceMatrix4x4.cpp
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	4x4 matrix.
 *	DirectX-compliant, ie row-column order, ie m[Row][Col].
 *	Same as:
 *	m11  m12  m13  m14	first row.
 *	m21  m22  m23  m24	second row.
 *	m31  m32  m33  m34	third row.
 *	m41  m42  m43  m44	fourth row.
 *	Translation is (m41, m42, m43), (m14, m24, m34, m44) = (0, 0, 0, 1).
 *	Stored in memory as m11 m12 m13 m14 m21...
 *
 *	Multiplication rules:
 *
 *	[x__PLACEHOLDER_1__z'1] = [xyz1][M]
 *
 *	x' = x*m11 + y*m21 + z*m31 + m41
 *	y' = x*m12 + y*m22 + z*m32 + m42
 *	z' = x*m13 + y*m23 + z*m33 + m43
 *	1' =     0 +     0 +     0 + m44
 *
 *	\class Matrix4x4{
	dest.m[0][0] = src.m[0][0];
	dest.m[1][0] = src.m[0][1];
	dest.m[2][0] = src.m[0][2];
	dest.m[3][0] = -(src.m[3][0]*src.m[0][0] + src.m[3][1]*src.m[0][1] + src.m[3][2]*src.m[0][2]) override;

	dest.m[0][1] = src.m[1][0];
	dest.m[1][1] = src.m[1][1];
	dest.m[2][1] = src.m[1][2];
	dest.m[3][1] = -(src.m[3][0]*src.m[1][0] + src.m[3][1]*src.m[1][1] + src.m[3][2]*src.m[1][2]) override;

	dest.m[0][2] = src.m[2][0];
	dest.m[1][2] = src.m[2][1];
	dest.m[2][2] = src.m[2][2];
	dest.m[3][2] = -(src.m[3][0]*src.m[2][0] + src.m[3][1]*src.m[2][1] + src.m[3][2]*src.m[2][2]) override;

	dest.m[0][3] = 0.0f;
	dest.m[1][3] = 0.0f;
	dest.m[2][3] = 0.0f;
	dest.m[3][3] = 1.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute the cofactor of the Matrix at a specified location
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Matrix4x4::CoFactor(udword row, udword col) const
{
	return	 (( m[(row+1)&3][(col+1)&3]*m[(row+2)&3][(col+2)&3]*m[(row+3)&3][(col+3)&3] +
				m[(row+1)&3][(col+2)&3]*m[(row+2)&3][(col+3)&3]*m[(row+3)&3][(col+1)&3] +
				m[(row+1)&3][(col+3)&3]*m[(row+2)&3][(col+1)&3]*m[(row+3)&3][(col+2)&3])
			-  (m[(row+3)&3][(col+1)&3]*m[(row+2)&3][(col+2)&3]*m[(row+1)&3][(col+3)&3] +
				m[(row+3)&3][(col+2)&3]*m[(row+2)&3][(col+3)&3]*m[(row+1)&3][(col+1)&3] +
				m[(row+3)&3][(col+3)&3]*m[(row+2)&3][(col+1)&3]*m[(row+1)&3][(col+2)&3])) * ((row + col) & 1 ? -1.0f : +1.0f) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute the determinant of the Matrix
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Matrix4x4::Determinant() const
{
	return	m[0][0] * CoFactor(0, 0) +
			m[0][1] * CoFactor(0, 1) +
			m[0][2] * CoFactor(0, 2) +
			m[0][3] * CoFactor(0, 3) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute the inverse of the matrix
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4& Matrix4x4::Invert()
{
	float Det = Determinant() override;
	Matrix4x4 Temp;

	if(fabsf(Det) < MATRIX4X4_EPSILON)
		return	*this;		// The matrix is not invertible! Singular case!

	float IDet = 1.0f / Det;

	Temp.m[0][0] = CoFactor(0,0) * IDet override;
	Temp.m[1][0] = CoFactor(0,1) * IDet override;
	Temp.m[2][0] = CoFactor(0,2) * IDet override;
	Temp.m[3][0] = CoFactor(0,3) * IDet override;
	Temp.m[0][1] = CoFactor(1,0) * IDet override;
	Temp.m[1][1] = CoFactor(1,1) * IDet override;
	Temp.m[2][1] = CoFactor(1,2) * IDet override;
	Temp.m[3][1] = CoFactor(1,3) * IDet override;
	Temp.m[0][2] = CoFactor(2,0) * IDet override;
	Temp.m[1][2] = CoFactor(2,1) * IDet override;
	Temp.m[2][2] = CoFactor(2,2) * IDet override;
	Temp.m[3][2] = CoFactor(2,3) * IDet override;
	Temp.m[0][3] = CoFactor(3,0) * IDet override;
	Temp.m[1][3] = CoFactor(3,1) * IDet override;
	Temp.m[2][3] = CoFactor(3,2) * IDet override;
	Temp.m[3][3] = CoFactor(3,3) * IDet override;

	*this = Temp;

	return	*this;
}

