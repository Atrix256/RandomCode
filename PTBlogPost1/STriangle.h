#pragma once

#include "TVector3.h"
#include "SMaterial.h"
#include "SRayHitInfo.h"

struct STriangle
{
    STriangle(const TVector3& a, const TVector3& b, const TVector3& c, const SMaterial& material)
        : m_material(material)
        , m_a(a)
        , m_b(b)
        , m_c(c)
    {
        TVector3 e1 = m_b - m_a;
        TVector3 e2 = m_c - m_a;
        m_normal = Normalize(Cross(e1, e2));
    }

    TVector3    m_a, m_b, m_c;
    SMaterial   m_material;

    // calculated!
    TVector3    m_normal;
};

//=================================================================================
inline bool RayIntersects (const TVector3& rayPos, const TVector3& rayDir, const STriangle& triangle, SRayHitInfo& info)
{
    // This function adapted from GraphicsCodex.com

    /* If ray P + tw hits triangle V[0], V[1], V[2], then the function returns true,
    stores the barycentric coordinates in b[], and stores the distance to the intersection
    in t. Otherwise returns false and the other output parameters are undefined.*/

    // Edge vectors
    TVector3 e_1 = triangle.m_b - triangle.m_a;
    TVector3 e_2 = triangle.m_c - triangle.m_a;

    const TVector3& q = Cross(rayDir, e_2);
    const float a = Dot(e_1, q);

    if (abs(a) == 0.0f)
        return false;

    const TVector3& s = (rayPos - triangle.m_a) / a;
    const TVector3& r = Cross(s, e_1);
    TVector3 b; // b is barycentric coordinates
    b[0] = Dot(s,q);
    b[1] = Dot(r, rayDir);
    b[2] = 1.0f - b[0] - b[1];
    // Intersected outside triangle?
    if ((b[0] < 0.0f) || (b[1] < 0.0f) || (b[2] < 0.0f)) return false;
    float t = Dot(e_2,r);
    if (t < 0.0f)
        return false;

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && t > info.m_collisionTime)
        return false;

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    TVector3 normal = triangle.m_normal;
    if (Dot(triangle.m_normal, rayDir) > 0.0f)
        normal *= -1.0f;

    info.m_collisionTime = t;
    info.m_intersectionPoint = rayPos + rayDir * t;
    info.m_material = &triangle.m_material;
    info.m_surfaceNormal = normal;
    return true;
}