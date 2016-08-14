#pragma once

#pragma once

#include <cmath>

//=================================================================================
struct SVector
{
    SVector (float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : m_x(x)
        , m_y(y)
        , m_z(z)
    {}

    float m_x;
    float m_y;
    float m_z;
};

//=================================================================================
inline SVector operator * (const SVector& a, float f)
{
    return SVector(a.m_x*f, a.m_y*f, a.m_z*f);
}

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
inline SVector operator += (SVector& a, const SVector& b)
{
    a.m_x += b.m_x;
    a.m_y += b.m_y;
    a.m_z += b.m_z;
    return a;
}

//=================================================================================
inline void Normalize(SVector& a)
{
    float len = sqrt((a.m_x * a.m_x) + (a.m_y * a.m_y) + (a.m_z * a.m_z));
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