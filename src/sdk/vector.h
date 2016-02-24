#pragma once

// Credits: LuaStoned

#define M_PI 3.1415925f

inline unsigned long& FloatBits(float& f)
{
	return *reinterpret_cast<unsigned long*>(&f);
}

inline unsigned long const& FloatBits(float const& f)
{
	return *reinterpret_cast<unsigned long const*>(&f);
}

inline float BitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}

inline bool IsFinite(float f)
{
	return ((FloatBits(f) & 0x7F800000) != 0x7F800000);
}

inline float FloatNegate(float f)
{
	return BitsToFloat(FloatBits(f) ^ 0x80000000);
}

class Vector
{
public:
	Vector();
	Vector(float _x, float _y, float _z);

	Vector&		operator = (const Vector&);
	Vector&		operator +=(const Vector&);
	Vector&		operator -=(const Vector&);
	Vector&		operator *=(const Vector&);
	Vector&		operator *=(const float);
	Vector&		operator /=(const Vector&);
	Vector&		operator /=(const float);
	Vector		operator + (const Vector&);
	Vector		operator - (const Vector&);
	Vector		operator / (const Vector&);
	Vector		operator / (const float&);
	Vector		operator * (const Vector&);
	Vector		operator * (const float&);
	float&		operator [](int index);
	bool		operator ==(const Vector&);
	bool		operator !=(const Vector&);

	void		Zero();
	bool		IsZero(float tolerance = 0.01f);
	float		Length();
	float		Length2D();
	float		LengthSqr();
	float		Length2DSqr();
	float		DistTo(const Vector&);
	float		DistToSqr(const Vector&);
	float		Dot(const Vector&);
	float		DotProduct(const Vector&);
	Vector		Cross(const Vector&);
	Vector		CrossProduct(const Vector&);
	Vector		Angle();
	float		Normalize();
	float		NormalizeInPlace();

public:
	float x;
	float y;
	float z;
};

typedef Vector QAngle;

struct matrix3x4
{
	matrix3x4() {}
	matrix3x4(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	matrix3x4(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector &vecOrigin)
	{
		m_flMatVal[0][0] = xAxis.x; m_flMatVal[0][1] = yAxis.x; m_flMatVal[0][2] = zAxis.x; m_flMatVal[0][3] = vecOrigin.x;
		m_flMatVal[1][0] = xAxis.y; m_flMatVal[1][1] = yAxis.y; m_flMatVal[1][2] = zAxis.y; m_flMatVal[1][3] = vecOrigin.y;
		m_flMatVal[2][0] = xAxis.z; m_flMatVal[2][1] = yAxis.z; m_flMatVal[2][2] = zAxis.z; m_flMatVal[2][3] = vecOrigin.z;
	}

	float* operator[](int i) { return m_flMatVal[i]; }
	const float* operator[](int i) const { return m_flMatVal[i]; }
	float* Base() { return &m_flMatVal[0][0]; }
	const float* Base() const { return &m_flMatVal[0][0]; }
	float m_flMatVal[3][4];
};

struct matrix4x4
{
	matrix4x4()
	{
		memset(this, 0, sizeof(matrix4x4));
	}

	float* operator[](int i)
	{
		return m[i];
	}

	const float* operator[](int i) const
	{
		return m[i];
	}

	float m[4][4];
};

inline void SinCos(float flRadians, float *flSine, float *flCosine)
{
	__asm
	{
		fld dword ptr[flRadians]
		fsincos

			mov edx, dword ptr[flCosine]
			mov eax, dword ptr[flSine]

			fstp dword ptr[edx]
			fstp dword ptr[eax]
	}
}

inline static float Rad2Deg(const float &flRadians)
{
	return flRadians * (float)(180.f / M_PI);
}

inline static float Deg2Rad(const float &flDegrees)
{
	return flDegrees * (float)(M_PI / 180.f);
}

inline void NormalizeAngle( float &angle )
{
	while (angle > 180.f)
		angle -= 360.f;

	while (angle < -180.f)
		angle += 360.f;
}

inline void NormalizeAngles(Vector &vecAngles)
{
	for (int iAxis = 0; iAxis < 3; iAxis++)
	{
		while (vecAngles[iAxis] > 180.f)
			vecAngles[iAxis] -= 360.f;

		while (vecAngles[iAxis] < -180.f)
			vecAngles[iAxis] += 360.f;
	}
}

inline Vector NormalizeVector(Vector vecInput)
{
	float flLength = vecInput.Length();
	float flLengthNormal = 1.f / (FLT_EPSILON + flLength);
	return vecInput * flLengthNormal;
}

inline static void CrossProduct(const Vector &vecOne, const Vector &vecTwo, Vector &vecCross)
{
	vecCross.x = ((vecOne.y * vecTwo.z) - (vecOne.z * vecTwo.y));
	vecCross.y = ((vecOne.z * vecTwo.x) - (vecOne.x * vecTwo.z));
	vecCross.z = ((vecOne.x * vecTwo.y) - (vecOne.y * vecTwo.x));
}

inline static void AngleVectors(const Vector &vecAngles, Vector *vecForward)
{
	float flSinPitch, flSinYaw, flCosPitch, flCosYaw;

	SinCos(Deg2Rad(vecAngles.x), &flSinPitch, &flCosPitch);
	SinCos(Deg2Rad(vecAngles.y), &flSinYaw, &flCosYaw);

	vecForward->x = flCosPitch * flCosYaw;
	vecForward->y = flCosPitch * flSinYaw;
	vecForward->z = -flSinPitch;
	vecForward->Normalize();
}

inline static void AngleVectors(const Vector &vecAngles, Vector *vecForward, Vector *vecRight, Vector *vecUp)
{
	float flSinPitch, flSinYaw, flSinRoll, flCosPitch, flCosYaw, flCosRoll;

	SinCos(Deg2Rad(vecAngles.x), &flSinPitch, &flCosPitch);
	SinCos(Deg2Rad(vecAngles.y), &flSinYaw, &flCosYaw);
	SinCos(Deg2Rad(vecAngles.z), &flSinRoll, &flCosRoll);

	if (vecForward)
	{
		vecForward->x = flCosPitch * flCosYaw;
		vecForward->y = flCosPitch * flSinYaw;
		vecForward->z = -flSinPitch;
		vecForward->Normalize();
	}

	if (vecRight)
	{
		vecRight->x = -1.f * flSinRoll * flSinPitch * flCosYaw + -1.f * flCosRoll * -flSinYaw;
		vecRight->y = -1.f * flSinRoll * flSinPitch * flSinYaw + -1.f * flCosRoll * flCosYaw;
		vecRight->z = -1.f * flSinRoll * flCosPitch;
		vecRight->Normalize();
	}

	if (vecUp)
	{
		vecUp->x = flCosRoll * flSinPitch * flCosYaw + -flSinRoll * -flSinYaw;
		vecUp->y = flCosRoll * flSinPitch * flSinYaw + -flSinRoll * flCosYaw;
		vecUp->z = flCosRoll * flCosPitch;
		vecUp->Normalize();
	}
}

inline static void VectorAngles(const Vector &vecDirection, Vector &vecAngles)
{
	if (vecDirection.x == 0 && vecDirection.y == 0)
	{
		if (vecDirection.z > 0.f)
			vecAngles.x = 270.f;
		else
			vecAngles.x = 90.f;

		vecAngles.y = 0.f;
	}
	else
	{
		vecAngles.x = Rad2Deg(atan2(-vecDirection.z, sqrtf(vecDirection.x * vecDirection.x + vecDirection.y * vecDirection.y)));
		vecAngles.y = Rad2Deg(atan2(vecDirection.y, vecDirection.x));

		if (vecAngles.x < 0.f)
			vecAngles.x += 360.f;

		if (vecAngles.y < 0.f)
			vecAngles.y += 360.f;
	}

	vecAngles.z = 0.f;
	NormalizeAngles(vecAngles);
}

inline static void VectorAngles(const Vector &vecDirection, const Vector &vecUp, Vector &vecAngles)
{
	Vector vecLeft;
	CrossProduct(vecUp, vecDirection, vecLeft);
	vecLeft.Normalize();

	float flLength2D = sqrtf((vecDirection.x * vecDirection.x) + (vecDirection.y * vecDirection.y));
	vecAngles.x = Rad2Deg(atan2(-vecDirection.z, flLength2D));

	if (flLength2D > .001f)
	{
		vecAngles.y = Rad2Deg(atan2(vecDirection.y, vecDirection.x));

		// todo: if > 84 || < -84, sqrt / dunno if this is needed
		float flUp = ((vecLeft.y * vecDirection.x) - (vecLeft.x * vecDirection.y));
		vecAngles.z = Rad2Deg(atan2(vecLeft.z, flUp));
	}
	else
	{
		vecAngles.y = Rad2Deg(atan2(-vecLeft.x, vecLeft.y));
		vecAngles.z = 0.f;
	}

	NormalizeAngles(vecAngles);
}

inline static void VectorTransform(const Vector &vecIn, const matrix3x4 &matrixIn, Vector &vecOut)
{
	Vector vecInNew = vecIn;
	vecOut.x = vecInNew.Dot(Vector(matrixIn[0][0], matrixIn[0][1], matrixIn[0][2])) + matrixIn[0][3];
	vecOut.y = vecInNew.Dot(Vector(matrixIn[1][0], matrixIn[1][1], matrixIn[1][2])) + matrixIn[1][3];
	vecOut.z = vecInNew.Dot(Vector(matrixIn[2][0], matrixIn[2][1], matrixIn[2][2])) + matrixIn[2][3];
}

inline static void MatrixGetColumn(const matrix3x4 &matrixIn, int iColumn, Vector &vecOut)
{
	vecOut.x = matrixIn[0][iColumn];
	vecOut.y = matrixIn[1][iColumn];
	vecOut.z = matrixIn[2][iColumn];
}

inline void MatrixPosition(const matrix3x4 &matrixIn, Vector &vecPosition)
{
	MatrixGetColumn(matrixIn, 3, vecPosition);
}

inline static void MatrixAngles(const matrix3x4 &matrixIn, Vector &vecAngles)
{
	float forward[3];
	float left[3];
	float up[3];

	//
	// Extract the basis vectors from the matrix. Since we only need the Z
	// component of the up vector, we don't get X and Y.
	//
	forward[0] = matrixIn[0][0];
	forward[1] = matrixIn[1][0];
	forward[2] = matrixIn[2][0];
	left[0] = matrixIn[0][1];
	left[1] = matrixIn[1][1];
	left[2] = matrixIn[2][1];
	up[2] = matrixIn[2][2];

	float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

	// enough here to get angles?
	if (xyDist > 0.001f)
	{
		// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
		vecAngles[1] = Rad2Deg(atan2f(forward[1], forward[0]));

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		vecAngles[0] = Rad2Deg(atan2f(-forward[2], xyDist));

		// (roll)	z = ATAN( left.z, up.z );
		vecAngles[2] = Rad2Deg(atan2f(left[2], up[2]));
	}
	else	// forward is mostly Z, gimbal lock-
	{
		// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
		vecAngles[1] = Rad2Deg(atan2f(-left[0], left[1]));

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		vecAngles[0] = Rad2Deg(atan2f(-forward[2], xyDist));

		// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
		vecAngles[2] = 0;
	}
}

inline void MatrixAngles(const matrix3x4 &matrixIn, Vector &vecAngles, Vector &vecPosition)
{
	MatrixAngles(matrixIn, vecAngles);
	MatrixPosition(matrixIn, vecPosition);
}

class VectorAligned : public Vector
{
public:
	VectorAligned()
	{
		x = y = z = 0;
	}

	VectorAligned(const Vector &in)
	{
		x = in.x;
		y = in.y;
		z = in.z;
	}

	float w;
};

inline Vector::Vector()
{
	x = y = z = 0.f;
}

inline Vector::Vector(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

inline Vector Vector::Angle()
{
	Vector out;
	VectorAngles(*this, out);

	return out;
}

inline Vector& Vector::operator = (const Vector &in)
{
	x = in.x;
	y = in.y;
	z = in.z;

	return *this;
}

inline Vector& Vector::operator += (const Vector &in)
{
	x += in.x;
	y += in.y;
	z += in.z;

	return *this;
}

inline Vector& Vector::operator -= (const Vector &in)
{
	x -= in.x;
	y -= in.y;
	z -= in.z;

	return *this;
}

inline Vector& Vector::operator *= (const Vector &in)
{
	x *= in.x;
	y *= in.y;
	z *= in.z;

	return *this;
}

inline Vector& Vector::operator *= (const float in)
{
	x *= in;
	y *= in;
	z *= in;

	return *this;
}

inline Vector& Vector::operator /= (const Vector &in)
{
	x /= in.x;
	y /= in.y;
	z /= in.z;

	return *this;
}

inline Vector& Vector::operator /= (const float in)
{
	x /= in;
	y /= in;
	z /= in;

	return *this;
}

inline Vector Vector::operator + (const Vector &in)
{
	Vector temp;

	temp.x = x + in.x;
	temp.y = y + in.y;
	temp.z = z + in.z;

	return temp;
}

inline Vector Vector::operator - (const Vector &in)
{
	Vector temp;

	temp.x = x - in.x;
	temp.y = y - in.y;
	temp.z = z - in.z;

	return temp;
}

inline Vector Vector::operator / (const Vector &in)
{
	Vector temp;

	// Use epsilon to avoid division by zero
	temp.x = x / ((in.x == 0.f) ? FLT_EPSILON : in.x);
	temp.y = y / ((in.y == 0.f) ? FLT_EPSILON : in.y);
	temp.z = z / ((in.x == 0.f) ? FLT_EPSILON : in.z);

	return temp;
}

inline Vector Vector::operator / (const float &in)
{
	Vector temp;

	float flDiv = in == 0.f ? FLT_EPSILON : in;

	// Use epsilon to avoid division by zero
	temp.x = x / flDiv;
	temp.y = y / flDiv;
	temp.z = z / flDiv;

	return temp;
}

inline Vector Vector::operator * (const Vector &in)
{
	Vector temp;

	temp.x = x * in.x;
	temp.y = y * in.y;
	temp.z = z * in.z;

	return temp;
}

inline Vector Vector::operator * (const float &in)
{
	Vector temp;

	temp.x = x * in;
	temp.y = y * in;
	temp.z = z * in;

	return temp;
}

inline bool Vector::operator == (const Vector &in)
{
	if (fabs(in.x - x) < FLT_EPSILON &&
		fabs(in.y - y) < FLT_EPSILON &&
		fabs(in.z - z) < FLT_EPSILON)
	{
		return true;
	}

	return false;
}

inline bool Vector::operator != (const Vector &in)
{
	if (fabs(in.x - x) > FLT_EPSILON &&
		fabs(in.y - y) > FLT_EPSILON &&
		fabs(in.z - z) > FLT_EPSILON)
	{
		return true;
	}

	return false;
}

inline float& Vector::operator [] (int index)
{
	return ((&x)[index]);
}

inline void Vector::Zero()
{
	x = y = z = 0.f;
}

inline bool Vector::IsZero(float tolerance)
{
	return (x > -tolerance && x < tolerance && y > -tolerance && y < tolerance && z > -tolerance && z < tolerance);
}

inline float Vector::Length()
{
	float root = 0.f;
	float sqsr = x*x + y*y + z*z;

	__asm
	{
		sqrtss xmm0, sqsr
		movss root, xmm0
	}

	return root;
	// return sqrt((x*x) + (y*y) + (z*z));
}

inline float Vector::Length2D()
{
	float root = 0.f;
	float sqst = x*x + y*y;

	__asm
	{
		sqrtss xmm0, sqst
		movss root, xmm0
	}

	return root;
	// return sqrt((x*x) + (y*y));
}

inline float Vector::LengthSqr()
{
	return x*x + y*y + z*z;
}

inline float Vector::Length2DSqr()
{
	return x*x + y*y;
}

inline float Vector::DistTo(const Vector &in)
{
	Vector vecDelta = *this - in;
	return vecDelta.Length();
}

inline float Vector::DistToSqr(const Vector &in)
{
	Vector vecDelta = *this - in;
	return vecDelta.LengthSqr();
}

inline float Vector::Normalize()
{
	float flLength = Length();
	float flLengthNormal = 1.f / (FLT_EPSILON + flLength);

	x = x * flLengthNormal;
	y = y * flLengthNormal;
	z = z * flLengthNormal;

	return flLength;
}

inline float Vector::NormalizeInPlace()
{
	return Normalize();
}

inline float Vector::Dot(const Vector &in)
{
	return (x * in.x + y * in.y + z * in.z);
}

inline float Vector::DotProduct(const Vector &in)
{
	return Dot(in);
}

inline Vector Vector::Cross(const Vector &in)
{
	return Vector(y * in.z - z * in.y, z * in.x - x * in.z, x * in.y - y * in.x);
}

inline Vector Vector::CrossProduct(const Vector &in)
{
	return Cross(in);
}



inline static void VectorCopy(const Vector& src, Vector& dst)
{
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
}


inline static void VectorAdd(const Vector& a, const Vector& b, Vector& c)
{
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;

}

inline static void VectorSubtract(const Vector& a, const Vector& b, Vector& c)
{
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
}

inline static void VectorMultiply(const Vector& a, const Vector& b, Vector& c)
{
	c.x = a.x * b.x;
	c.y = a.y * b.y;
	c.z = a.z * b.z;
}

// for backwards compatability
inline static void VectorScale(const Vector& in, Vector scale, Vector& result)
{
	VectorMultiply(in, scale, result);
}
