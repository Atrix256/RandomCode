#pragma once

#include "SVector.h"
#include "ObjectID.h"
#include "SCollisionInfo.h"

//=================================================================================
struct SSphere
{
    SSphere(SVector position = SVector(), float radius = 0.0f, size_t materialID = 0)
        : m_position(position)
        , m_radius(radius)
        , m_objectID(++g_lastObjectID)
        , m_materialID(materialID)
    {
    }

    SVector m_position;
    float   m_radius;
    size_t  m_objectID;
    size_t  m_materialID;
};

//=================================================================================
inline bool RayIntersects(const SVector& rayPos, const SVector& rayDir, const SSphere& sphere)
{
    //get the vector from the center of this circle to where the ray begins.
    SVector m = rayPos - sphere.m_position;

    //get the dot product of the above vector and the ray's vector
    float b = Dot(m, rayDir);

    float c = Dot(m, m) - sphere.m_radius * sphere.m_radius;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return false;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr < 0.0)
        return false;

    return true;
}

//=================================================================================
inline bool RayIntersects(const SVector& rayPos, const SVector& rayDir, const SSphere& sphere, SCollisionInfo& info, size_t ignoreObjectId = c_invalidObjectID)
{
    if (ignoreObjectId == sphere.m_objectID)
        return false;

    //get the vector from the center of this circle to where the ray begins.
    SVector m = rayPos - sphere.m_position;

    //get the dot product of the above vector and the ray's vector
    float b = Dot(m, rayDir);

    float c = Dot(m, m) - sphere.m_radius * sphere.m_radius;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return false;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr < 0.0)
        return false;

    //not inside til proven otherwise
    bool fromInside = false;

    //ray now found to intersect sphere, compute smallest t value of intersection
    float collisionTime = -b - sqrt(discr);

    //if t is negative, ray started inside sphere so clamp t to zero and remember that we hit from the inside
    if (collisionTime < 0.0)
    {
        collisionTime = -b + sqrt(discr);
        fromInside = true;
    }

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && collisionTime > info.m_collisionTime)
        return false;

    // set all the info params since we are garaunteed a hit at this point
    info.m_fromInside = fromInside;
    info.m_collisionTime = collisionTime;
    info.m_materialID = sphere.m_materialID;

    //compute the point of intersection
    info.m_intersectionPoint = rayPos + rayDir * info.m_collisionTime;

    // calculate the normal
    info.m_surfaceNormal = info.m_intersectionPoint - sphere.m_position;
    Normalize(info.m_surfaceNormal);

    // we found a hit!
    info.m_objectID = sphere.m_objectID;
    return true;
}