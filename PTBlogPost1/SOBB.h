#pragma once

#include "SAABB.h"

struct SOBB
{
    SOBB (const SAABB& aabb, TVector3 axis, float rotation)
        : m_aabb(aabb)
    {
        // make sure the axis we get is normalized
        axis = Normalize(axis);

        // calculate the X,Y,Z axis of the OBB
        const float cosTheta = cos(rotation);
        const float sinTheta = sin(rotation);
        m_XAxis =
        {
            cosTheta + axis[0] * axis[0] * (1.0f - cosTheta),
            axis[0] * axis[1] * (1.0f - cosTheta) - axis[2] * sinTheta,
            axis[0] * axis[2] * (1.0f - cosTheta) + axis[1] * sinTheta
        };

        m_YAxis =
        {
            axis[1] * axis[0] * (1.0f - cosTheta) + axis[2] * sinTheta,
            cosTheta + axis[1] * axis[1] * (1.0f - cosTheta),
            axis[1] * axis[2] * (1.0f - cosTheta) - axis[0] * sinTheta
        };

        m_ZAxis =
        {
            axis[2] * axis[0] * (1.0f - cosTheta) - axis[1] * sinTheta,
            axis[2] * axis[1] * (1.0f - cosTheta) + axis[0] * sinTheta,
            cosTheta + axis[2] * axis[2] * (1.0f - cosTheta)
        };
    }

    SAABB           m_aabb;

    // calculated!
    TVector3        m_XAxis;
    TVector3        m_YAxis;
    TVector3        m_ZAxis;
};

//=================================================================================
inline bool RayIntersects(const TVector3& rayPos, const TVector3& rayDir, const SOBB& obb, SRayHitInfo& info)
{
    // put the ray into local space of the obb
    TVector3 newRayPos = ChangeBasis(rayPos - obb.m_aabb.m_position, obb.m_XAxis, obb.m_YAxis, obb.m_ZAxis) + obb.m_aabb.m_position;
    TVector3 newRayDir = ChangeBasis(rayDir, obb.m_XAxis, obb.m_YAxis, obb.m_ZAxis);

    // do ray vs abb intersection
    if (!RayIntersects(newRayPos, newRayDir, obb.m_aabb, info))
        return false;

    // convert surface normal back to global space and calculate global space intersection point
    TVector3 newSurfaceNormal = UndoChangeBasis(info.m_surfaceNormal, obb.m_XAxis, obb.m_YAxis, obb.m_ZAxis);
    info.m_intersectionPoint = rayPos + rayDir * info.m_collisionTime;
    info.m_surfaceNormal = newSurfaceNormal;
    return true;
}