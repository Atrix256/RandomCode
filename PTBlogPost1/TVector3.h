#pragma once

#include <array>
#include "Utils.h"

typedef std::array<float, 3> TVector3;

//=================================================================================
// Vector operations
inline float LengthSq (const TVector3& v)
{
    return (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
}

inline float Length (const TVector3& v)
{
    return sqrt(LengthSq(v));
}

inline TVector3 Normalize (const TVector3& v)
{
    float len = Length(v);
    return
    {
        v[0] / len,
        v[1] / len,
        v[2] / len
    };
}

inline TVector3 operator+ (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] + b[0],
        a[1] + b[1],
        a[2] + b[2]
    };
}

inline TVector3 operator- (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] - b[0],
        a[1] - b[1],
        a[2] - b[2]
    };
}

inline void operator+= (TVector3& a, const TVector3& b)
{
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
}

inline TVector3 operator* (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] * b[0],
        a[1] * b[1],
        a[2] * b[2]
    };
}

inline TVector3 operator* (float b, const TVector3& a)
{
    return
    {
        a[0] * b,
        a[1] * b,
        a[2] * b
    };
}

inline TVector3 operator* (const TVector3& a, float b)
{
    return
    {
        a[0] * b,
        a[1] * b,
        a[2] * b
    };
}

inline TVector3 operator/ (const TVector3& a, float b)
{
    return
    {
        a[0] / b,
        a[1] / b,
        a[2] / b
    };
}

inline void operator*= (TVector3& a, float b)
{
    a[0] *= b;
    a[1] *= b;
    a[2] *= b;
}

inline TVector3 operator - (const TVector3& a)
{
    return
    {
        -a[0], -a[1], -a[2]
    };
}

inline float Dot (const TVector3& a, const TVector3& b)
{
    return
        a[0] * b[0] +
        a[1] * b[1] +
        a[2] * b[2];
}

inline TVector3 Cross (const TVector3& a, const TVector3& b)
{
    return
    {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

// TODO: remove this after you figure out noise issue
//=================================================================================
inline TVector3 CosineSampleHemisphere (const TVector3& normal)
{
    // from smallpt: http://www.kevinbeason.com/smallpt/

    float r1 = 2.0f * c_pi *RandomFloat();
    float r2 = RandomFloat();
    float r2s = sqrt(r2);

    TVector3 w = normal;
    TVector3 u;
    if (fabs(w[0]) > 0.1f)
        u = Cross({ 0.0f, 1.0f, 0.0f }, w);
    else
        u = Cross({ 1.0f, 0.0f, 0.0f }, w);

    u = Normalize(u);
    TVector3 v = Cross(w, u);
    TVector3 d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2));
    d = Normalize(d);

    return d;
}

//=================================================================================
inline TVector3 UniformSampleHemisphere (const TVector3& N)
{
    // Uniform point on sphere
    // from http://mathworld.wolfram.com/SpherePointPicking.html
    float u = RandomFloat();
    float v = RandomFloat();

    float theta = 2.0f * c_pi * u;
    float phi = acos(2.0f * v - 1.0f);

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    TVector3 dir;
    dir[0] = cosTheta * sinPhi;
    dir[1] = sinTheta * sinPhi;
    dir[2] = cosPhi;

    // if our vector is facing the wrong way vs the normal, flip it!
    if (Dot(dir, N) <= 0.0f)
        dir *= -1.0f;

    return dir;
}