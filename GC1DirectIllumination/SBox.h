#pragma once

#include "SVector.h"
#include "ObjectID.h"
#include "SCollisionInfo.h"
#include <array>

//=================================================================================
struct SBox
{
    SBox(SVector position, SVector scale, const std::array<TMaterialID,6>& materialIDs)
        : m_position(position)
        , m_scale(scale)
        , m_objectID(GenerateObjectID())
        , m_materialIDs(materialIDs)
    {
    }

    SBox(SVector position, SVector scale, TMaterialID materialID)
        : m_position(position)
        , m_scale(scale)
        , m_objectID(GenerateObjectID())
    {
        for(TMaterialID& m : m_materialIDs)
            m = materialID;
    }

    SVector     m_position;
    SVector     m_scale;
    TObjectID   m_objectID;
    std::array<TMaterialID,6> m_materialIDs;
};

//=================================================================================
inline bool RayIntersects(const SVector& rayPos, const SVector& rayDir, const SBox& box, SCollisionInfo& info, TObjectID ignoreObjectId = c_invalidObjectID)
{
    if (ignoreObjectId == box.m_objectID)
        return false;

    float rayMinTime = 0.0;
    float rayMaxTime = FLT_MAX;

    // find the intersection of the intersection times of each axis to see if / where the
    // ray hits.
    for (int axis = 0; axis < 3; ++axis)
    {
        //calculate the min and max of the box on this axis
        float axisMin = box.m_position[axis] - box.m_scale[axis] * 0.5f;
        float axisMax = axisMin + box.m_scale[axis];

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
    if (info.m_maxCollisionTime >= 0.0 && collisionTime > info.m_maxCollisionTime)
        return false;

    SVector intersectionPoint = rayPos + rayDir * collisionTime;

    // figure out the surface normal by figuring out which axis we are closest to
    TMaterialID materialId;
    float closestDist = FLT_MAX;
    SVector normal, tangent, biTangent;
    float u = 0.0f;
    float v = 0.0f;
    for (int axis = 0; axis < 3; ++axis)
    {
        float distFromPos = abs(box.m_position[axis] - intersectionPoint[axis]);
        float distFromEdge = abs(distFromPos - (box.m_scale[axis] * 0.5f));

        if (distFromEdge < closestDist)
        {
            closestDist = distFromEdge;
            normal = SVector(0.0f, 0.0f, 0.0f);
            if (intersectionPoint[axis] < box.m_position[axis])
            {
                normal[axis] = -1.0;
                materialId = box.m_materialIDs[axis * 2];
            }
            else
            {
                normal[axis] = 1.0;
                materialId = box.m_materialIDs[axis * 2 + 1];
            }
        }
    }

    if (abs(normal.m_x) > 0.1f)
    {
        tangent = SVector(0.0f, 0.0f, 1.0f);
        biTangent = SVector(0.0f, 1.0f, 0.0f);
        if (normal.m_x < 0.0f)
        {
            tangent *= -1.0f;
            biTangent *= -1.0f;
        }
    }
    else if (abs(normal.m_y) > 0.1f)
    {
        tangent = SVector(1.0f, 0.0f, 0.0f);
        biTangent = SVector(0.0f, 0.0f, 1.0f);
        if (normal.m_y < 0.0f)
        {
            tangent *= -1.0f;
            biTangent *= -1.0f;
        }
    }
    else if (abs(normal.m_z) > 0.1f)
    {
        tangent = SVector(1.0f, 0.0f, 0.0f);
        biTangent = SVector(0.0f, 1.0f, 0.0f);
        if (normal.m_z < 0.0f)
        {
            tangent *= -1.0f;
            biTangent *= -1.0f;
        }
    }

    u = Dot((intersectionPoint - box.m_position), tangent);
    v = Dot((intersectionPoint - box.m_position), biTangent);

    if (fromInside)
    {
        normal *= -1.0f;
        tangent *= -1.0f;
        biTangent *= -1.0f;
    }

    info.SuccessfulHit(
        box.m_objectID,
        materialId,
        intersectionPoint,
        normal,
        fromInside,
        collisionTime,
        tangent,
        biTangent,
        u,
        v
    );

    return true;

    // TODO: clean up and optimize the above!
}