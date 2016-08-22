#pragma once

#pragma once

#include <cmath>
#include "Utils.h"
#include "Random.h"

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
inline SVector Refract (const SVector& incident, const SVector& normal, float ratio)
{
    // from https://www.opengl.org/sdk/docs/man/html/refract.xhtml
    /*
        k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
        if (k < 0.0)
            R = genType(0.0);       // or genDType(0.0)
        else
            R = eta * I - (eta * dot(N, I) + sqrt(k)) * N;
    */
    float k = 1.0f - ratio * ratio * (1.0f - Dot(normal, incident) * Dot(normal, incident));
    if (k < 0.0f) // TODO: assert or something? it's weird that it returns a 0 vector!
        return SVector();
    else
        return ratio * incident - (ratio * Dot(normal, incident) + sqrt(k)) * normal;
}

//=================================================================================
inline bool NotZero (const SVector& a)
{
    return a.m_x != 0.0f && a.m_y != 0.0f && a.m_z != 0.0f;
}

//=================================================================================
inline SVector RandomUnitVectorInHemisphere (const SVector& v)
{
    /*
    // from http://math.stackexchange.com/questions/1163260/random-directions-on-hemisphere-oriented-by-an-arbitrary-vector
    SVector ret;

    do {
        float d;
        do
        {
            ret.m_x = RandomFloat() * 2.0f - 1.0f;
            ret.m_y = RandomFloat() * 2.0f - 1.0f;
            ret.m_z = RandomFloat() * 2.0f - 1.0f;
            d = Length(ret);
        } while (d > 1);

        ret /= d;
    }
    while (Dot(ret, v) <= 0.0f);

    return ret;
    */

    float theta0 = RandomFloat() * 2.0f * c_pi;
    float theta1 = acosf(1.0f - 2.0f * RandomFloat());

    SVector ret;
    ret.m_x = sin(theta0) * sin(theta1);
    ret.m_y = sin(theta0) * cos(theta1);
    ret.m_z = sin(theta1);

    if (Dot(ret, v) < 0.0f)
        ret *= -1;

    return ret;
}

//=================================================================================
inline float MaxComponentValue (const SVector& s)
{
    return (s.m_x > s.m_y && s.m_x > s.m_z)
            ? s.m_x
            : s.m_y > s.m_z
                ? s.m_y
                : s.m_z;
}