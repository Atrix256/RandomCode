#pragma once

#include "SVector.h"
#include "ObjectID.h"
#include "SCollisionInfo.h"

//=================================================================================
struct SPlane
{
    SPlane(SVector normal, float distance, TMaterialID materialID)
        : m_normal(normal)
        , m_distance(distance)
        , m_objectID(GenerateObjectID())
        , m_materialID(materialID)
    {
        Normalize(m_normal);
    }

    SVector     m_normal;
    float       m_distance;
    TObjectID   m_objectID;
    TMaterialID m_materialID;
};

//=================================================================================
inline bool RayIntersects (const SVector& rayPos, const SVector& rayDir, const SPlane& plane, SCollisionInfo& info, TObjectID ignoreObjectId = c_invalidObjectID)
{
    if (ignoreObjectId == plane.m_objectID)
        return false;

    float collisionTime = (-plane.m_distance - Dot(plane.m_normal, rayPos)) / Dot(plane.m_normal, rayDir);

    // only consider positive hit times
    if (collisionTime < 0.0f)
        return false;

    //enforce a max distance if we should
    if (info.m_maxCollisionTime >= 0.0 && collisionTime > info.m_maxCollisionTime)
        return false;

    // handle hitting from inside or not
    bool fromInside = Dot(plane.m_normal, rayDir) > 0.0f;
    SVector normal = fromInside ? -plane.m_normal : plane.m_normal;

    // successful hit!
    info.SuccessfulHit(
        plane.m_objectID,
        plane.m_materialID,
        rayPos + rayDir * collisionTime,
        normal,
        fromInside,
        collisionTime
    );
    return true;
}