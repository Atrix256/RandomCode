#pragma once

#include "SVector.h"
#include "ObjectID.h"
#include "SCollisionInfo.h"

//=================================================================================
struct STriangle
{
    STriangle (SVector A, SVector B, SVector C, size_t materialID)
        : m_A(A)
        , m_B(B)
        , m_C(C)
        , m_objectID(GenerateObjectID())
        , m_materialID((TMaterialID)materialID)
    {
    }

    SVector     m_A;
    SVector     m_B;
    SVector     m_C;
    TObjectID   m_objectID;
    TMaterialID m_materialID;
};

//=================================================================================
inline bool RayIntersects (const SVector& rayPos, const SVector& rayDir, const STriangle& triangle, SCollisionInfo& info, TObjectID ignoreObjectId = c_invalidObjectID)
{
    // This function adapted from GraphicsCodex.com

    if (ignoreObjectId == triangle.m_objectID)
        return false;

    /* If ray P + tw hits triangle V[0], V[1], V[2], then the function returns true,
    stores the barycentric coordinates in b[], and stores the distance to the intersection
    in t. Otherwise returns false and the other output parameters are undefined.*/

    // TODO: epsilon needed?
    const float eps = 0.001f;

    // TODO: these are barrycentric coordinates, may need!
    SVector b;

    // TODO: clean up this code!

    // Edge vectors
    SVector e_1 = triangle.m_B - triangle.m_A;
    SVector e_2 = triangle.m_C - triangle.m_A;

    // Face normal
    SVector n = Cross(e_1,e_2);
    Normalize(n);
    const SVector& q = Cross(rayDir,e_2);
    const float a = Dot(e_1, q);
    // Backfacing / nearly parallel, or close to the limit of precision?
    //if ((Dot(n, rayDir) >= 0) || (abs(a) <= eps)) return false;
    const SVector& s = (rayPos - triangle.m_A) / a;
    const SVector& r = Cross(s,e_1);
    b.m_x = Dot(s,q);
    b.m_y = Dot(r, rayDir);
    b.m_z = 1.0f - b.m_x - b.m_y;
    // Intersected outside triangle?
    if ((b.m_x < 0.0f) || (b.m_y < 0.0f) || (b.m_z < 0.0f)) return false;
    float t = Dot(e_2,r);
    if (t < 0.0f)
        return false;

    //enforce a max distance if we should
    if (info.m_maxCollisionTime >= 0.0 && t > info.m_maxCollisionTime)
        return false;

    info.m_maxCollisionTime = t;

    info.m_objectID = triangle.m_objectID;
    info.m_materialID = triangle.m_materialID;
    info.m_fromInside = false; // TODO: need to figure this out for real! 
    info.m_intersectionPoint = rayPos + rayDir * info.m_maxCollisionTime;
    info.m_surfaceNormal = n;  // TODO: flip this around if we hit from the inside!  Maybe flip it around above?

    return true;
}