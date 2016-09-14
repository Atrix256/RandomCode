#pragma once

#include "TVector3.h"
#include "SMaterial.h"
#include "SRayHitInfo.h"

struct SAABB
{
    SAABB (const TVector3& position, const TVector3& radius, const SMaterial& material)
        : m_position(position)
        , m_radius(radius)
        , m_material(material)
    { }

    TVector3        m_position;
    TVector3        m_radius;
    SMaterial       m_material;
};


//=================================================================================
inline bool RayIntersects (const TVector3& rayPos, const TVector3& rayDir, const SAABB& aabb, SRayHitInfo& info)
{
    float rayMinTime = 0.0;
    float rayMaxTime = FLT_MAX;

    // find the intersection of the intersection times of each axis to see if / where the
    // ray hits.
    for (int axis = 0; axis < 3; ++axis)
    {
        //calculate the min and max of the box on this axis
        float axisMin = aabb.m_position[axis] - aabb.m_radius[axis];
        float axisMax = aabb.m_position[axis] + aabb.m_radius[axis];

        //if the ray is paralel with this axis
        if (abs(rayDir[axis]) < 0.0001f)
        {
            //if the ray isn't in the box, bail out we know there's no intersection
            if (rayPos[axis] < axisMin || rayPos[axis] > axisMax)
                return false;
        }
        else
        {
            //figure out the intersection times of the ray with the 2 values of this axis
            float axisMinTime = (axisMin - rayPos[axis]) / rayDir[axis];
            float axisMaxTime = (axisMax - rayPos[axis]) / rayDir[axis];

            //make sure min < max
            if (axisMinTime > axisMaxTime)
            {
                float temp = axisMinTime;
                axisMinTime = axisMaxTime;
                axisMaxTime = temp;
            }

            //union this time slice with our running total time slice
            if (axisMinTime > rayMinTime)
                rayMinTime = axisMinTime;

            if (axisMaxTime < rayMaxTime)
                rayMaxTime = axisMaxTime;

            //if our time slice shrinks to below zero of a time window, we don't intersect
            if (rayMinTime > rayMaxTime)
                return false;
        }
    }

    //if we got here, we do intersect, return our collision info
    bool fromInside = (rayMinTime == 0.0);
    float collisionTime;
    if (fromInside)
        collisionTime = rayMaxTime;
    else
        collisionTime = rayMinTime;

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && collisionTime > info.m_collisionTime)
        return false;

    TVector3 intersectionPoint = rayPos + rayDir * collisionTime;

    // figure out the surface normal by figuring out which axis we are closest to
    float closestDist = FLT_MAX;
    TVector3 normal;
    float u = 0.0f;
    float v = 0.0f;
    for (int axis = 0; axis < 3; ++axis)
    {
        float distFromPos = abs(aabb.m_position[axis] - intersectionPoint[axis]);
        float distFromEdge = abs(distFromPos - aabb.m_radius[axis]);

        if (distFromEdge < closestDist)
        {
            closestDist = distFromEdge;
            normal = { 0.0f, 0.0f, 0.0f };
            if (intersectionPoint[axis] < aabb.m_position[axis])
                normal[axis] = -1.0;
            else
                normal[axis] = 1.0;
        }
    }

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    if (Dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    info.m_collisionTime = collisionTime;
    info.m_intersectionPoint = rayPos + rayDir * collisionTime;
    info.m_material = &aabb.m_material;
    info.m_surfaceNormal = normal;
    return true;
}