#pragma once

#pragma once

#include <cmath>

//=================================================================================
struct SVector
{
    SVector()
        : m_x(0.0f)
        , m_y(0.0f)
        , m_z(0.0f)
    {}

    SVector (float x, float y, float z)
        : m_x(x)
        , m_y(y)
        , m_z(z)
    {}

    float m_x;
    float m_y;
    float m_z;
};

//=================================================================================
//  Vector vs Vector
//=================================================================================
inline SVector operator + (const SVector& a, const SVector& b)
{
    return SVector(a.m_x + b.m_x, a.m_y + b.m_y, a.m_z + b.m_z);
}

//=================================================================================
inline SVector operator - (const SVector& a, const SVector& b)
{
    return SVector(a.m_x - b.m_x, a.m_y - b.m_y, a.m_z - b.m_z);
}

//=================================================================================
inline SVector operator * (const SVector& a, const SVector& b)
{
    return SVector(a.m_x * b.m_x, a.m_y * b.m_y, a.m_z * b.m_z);
}

//=================================================================================
inline SVector operator / (const SVector& a, const SVector& b)
{
    return SVector(a.m_x / b.m_x, a.m_y / b.m_y, a.m_z / b.m_z);
}

//=================================================================================
inline SVector operator += (SVector& a, const SVector& b)
{
    a.m_x += b.m_x;
    a.m_y += b.m_y;
    a.m_z += b.m_z;
    return a;
}

//=================================================================================
inline SVector operator -= (SVector& a, const SVector& b)
{
    a.m_x -= b.m_x;
    a.m_y -= b.m_y;
    a.m_z -= b.m_z;
    return a;
}

//=================================================================================
// Vector Unitary Ops
//=================================================================================
inline SVector operator - (const SVector& a)
{
    return SVector(-a.m_x, -a.m_y, -a.m_z);
}

//=================================================================================
//  Vector vs Scalar
//=================================================================================
inline SVector operator * (const SVector& a, float f)
{
    return SVector(a.m_x*f, a.m_y*f, a.m_z*f);
}

//=================================================================================
inline SVector operator * (float f, const SVector& a)
{
    return SVector(a.m_x*f, a.m_y*f, a.m_z*f);
}

//=================================================================================
inline SVector operator / (const SVector& a, float f)
{
    return SVector(a.m_x/f, a.m_y/f, a.m_z/f);
}

//=================================================================================
inline SVector operator *= (SVector& a, float b)
{
    a.m_x *= b;
    a.m_y *= b;
    a.m_z *= b;
    return a;
}

//=================================================================================
inline SVector operator /= (SVector& a, float b)
{
    a.m_x /= b;
    a.m_y /= b;
    a.m_z /= b;
    return a;
}

//=================================================================================
//  Utility Functions
//=================================================================================
inline float LengthSq (const SVector& a)
{
    return (a.m_x * a.m_x) + (a.m_y * a.m_y) + (a.m_z * a.m_z);
}

//=================================================================================
inline float Length (const SVector& a)
{
    return sqrt(LengthSq(a));
}

//=================================================================================
inline void Normalize (SVector& a)
{
    float len = Length(a);
    a.m_x /= len;
    a.m_y /= len;
    a.m_z /= len;
}

//=================================================================================
inline float Dot (const SVector& a, const SVector& b)
{
    return
        a.m_x*b.m_x +
        a.m_y*b.m_y +
        a.m_z*b.m_z;
}

//=================================================================================
inline SVector Cross (const SVector& a, const SVector& b)
{
    return SVector(
        a.m_y*b.m_z - a.m_z*b.m_y,
        a.m_z*b.m_x - a.m_x*b.m_z,
        a.m_x*b.m_y - a.m_y*b.m_x
    );
}

//=================================================================================
inline SVector Reflect (const SVector& incident, const SVector& normal)
{
    return incident - 2.0f * normal * Dot(incident, normal);
}

//=================================================================================
inline bool NotZero (const SVector& a)
{
    return a.m_x != 0.0f && a.m_y != 0.0f && a.m_z != 0.0f;
}