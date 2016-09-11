#pragma once

#include "TVector3.h"
#include "SMaterial.h"
#include "SRayHitInfo.h"

struct SSphere
{
    TVector3        m_position;
    float           m_radius;
    SMaterial       m_material;
};

//=================================================================================
inline bool RayIntersects (const TVector3& rayPos, const TVector3& rayDir, const SSphere& sphere, SRayHitInfo& info)
{
    //get the vector from the center of this circle to where the ray begins.
    TVector3 m = rayPos - sphere.m_position;

    //get the dot product of the above vector and the ray's vector
    float b = Dot(m, rayDir);

    float c = Dot(m, m) - sphere.m_radius * sphere.m_radius;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return false;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr <= 0.0)
        return false;

    //ray now found to intersect sphere, compute smallest t value of intersection
    float collisionTime = -b - sqrt(discr);

    //if t is negative, ray started inside sphere so clamp t to zero and remember that we hit from the inside
    if (collisionTime < 0.0)
        collisionTime = -b + sqrt(discr);

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && collisionTime > info.m_collisionTime)
        return false;

    TVector3 normal = Normalize((rayPos + rayDir * collisionTime) - sphere.m_position);

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    if (Dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    info.m_collisionTime = collisionTime;
    info.m_intersectionPoint = rayPos + rayDir * collisionTime;
    info.m_material = &sphere.m_material;
    info.m_surfaceNormal = normal;
    return true;
}