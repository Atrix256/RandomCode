#pragma once

#include "TVector3.h"
#include "SMaterial.h"
#include "SRayHitInfo.h"

// assumes a,b,c,d are counter clockwise
struct SQuad
{
    SQuad(const TVector3& a, const TVector3& b, const TVector3& c, const TVector3& d, const SMaterial& material)
        : m_material(material)
        , m_a(a)
        , m_b(b)
        , m_c(c)
        , m_d(d)
    {
        TVector3 e1 = m_b - m_a;
        TVector3 e2 = m_c - m_a;
        m_normal = Normalize(Cross(e1, e2));

        // TODO: temp!
        float lenAB = Length(e1);
        float lenAC = Length(e2);

        TVector3 xAxis = Normalize(m_b - m_a);
        TVector3 yAxis = Normalize(m_d - m_a);

        int ijkl = 0;
    }

    TVector3    m_a, m_b, m_c, m_d;
    SMaterial   m_material;

    // calculated!
    TVector3    m_normal;
};

//=================================================================================
inline bool RayIntersects(const TVector3& rayPos, const TVector3& rayDir, const SQuad& quad, SRayHitInfo& info)
{
    // This function adapted from "Real Time Collision Detection" 5.3.5 Intersecting Line Against Quadrilateral
    // IntersectLineQuad()
    TVector3 pa = quad.m_a - rayPos;
    TVector3 pb = quad.m_b - rayPos;
    TVector3 pc = quad.m_c - rayPos;
    // Determine which triangle to test against by testing against diagonal first
    TVector3 m = Cross(pc, rayDir);
    TVector3 r;
    float v = Dot(pa, m); // ScalarTriple(pq, pa, pc);
    if (v >= 0.0f) {
        // Test intersection against triangle abc
        float u = -Dot(pb, m); // ScalarTriple(pq, pc, pb);
        if (u < 0.0f) return false;
        float w = ScalarTriple(rayDir, pb, pa);
        if (w < 0.0f) return false;
        // Compute r, r = u*a + v*b + w*c, from barycentric coordinates (u, v, w)
        float denom = 1.0f / (u + v + w);
        u *= denom;
        v *= denom;
        w *= denom; // w = 1.0f - u - v;
        r = u*quad.m_a + v*quad.m_b + w*quad.m_c;
    }
    else {
        // Test intersection against triangle dac
        TVector3 pd = quad.m_d - rayPos;
        float u = Dot(pd, m); // ScalarTriple(pq, pd, pc);
        if (u < 0.0f) return false;
        float w = ScalarTriple(rayDir, pa, pd);
        if (w < 0.0f) return false;
        v = -v;
        // Compute r, r = u*a + v*d + w*c, from barycentric coordinates (u, v, w)
        float denom = 1.0f / (u + v + w);
        u *= denom;
        v *= denom;
        w *= denom; // w = 1.0f - u - v;
        r = u*quad.m_a + v*quad.m_d + w*quad.m_c;
    }

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    TVector3 normal = quad.m_normal;
    if (Dot(quad.m_normal, rayDir) > 0.0f)
        normal *= -1.0f;

    // figure out the time t that we hit the plane (quad)
    float t;
    if (abs(rayDir[0]) > 0.0f)
        t = (r[0] - rayPos[0]) / rayDir[0];
    else if (abs(rayDir[1]) > 0.0f)
        t = (r[1] - rayPos[1]) / rayDir[1];
    else if (abs(rayDir[2]) > 0.0f)
        t = (r[2] - rayPos[2]) / rayDir[2];

    // only positive time hits allowed!
    if (t < 0.0f)
        return false;

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && t > info.m_collisionTime)
        return false;

    info.m_collisionTime = t;
    info.m_intersectionPoint = r;
    info.m_material = &quad.m_material;
    info.m_surfaceNormal = normal;
    return true;
}