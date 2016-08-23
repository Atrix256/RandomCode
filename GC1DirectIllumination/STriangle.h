#pragma once

#include "SVector.h"
#include "ObjectID.h"
#include "SCollisionInfo.h"

//=================================================================================
struct STriangle
{
    STriangle (SVector A, SVector B, SVector C, TMaterialID materialID)
        : m_A(A)
        , m_B(B)
        , m_C(C)
        , m_objectID(GenerateObjectID())
        , m_materialID(materialID)
    {
        SVector e_1 = m_B - m_A;
        SVector e_2 = m_C - m_A;

        m_normal = Cross(e_1, e_2);
        Normalize(m_normal);
    }

    SVector     m_A;
    SVector     m_B;
    SVector     m_C;
    SVector     m_normal;
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

    // Edge vectors
    SVector e_1 = triangle.m_B - triangle.m_A;
    SVector e_2 = triangle.m_C - triangle.m_A;

    const SVector& q = Cross(rayDir,e_2);
    const float a = Dot(e_1, q);

    if (abs(a) == 0.0f)
        return false;

    const SVector& s = (rayPos - triangle.m_A) / a;
    const SVector& r = Cross(s,e_1);
    SVector b; // NOTE: barycentric coords!
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

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    bool fromInside = false;
    SVector normal = triangle.m_normal;
    if (Dot(triangle.m_normal, rayDir) > 0.0f)
    {
        normal *= -1.0f;
        fromInside = true;
    }

    info.SuccessfulHit(
        triangle.m_objectID,
        triangle.m_materialID,
        rayPos + rayDir * t,
        normal,
        fromInside,
        t
    );

    return true;
}