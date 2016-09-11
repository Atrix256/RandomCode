#pragma once

#include "TVector3.h"

struct SMaterial;

struct SRayHitInfo
{
    SRayHitInfo()
        : m_material(nullptr)
        , m_collisionTime(-1.0f)
    { }

    const SMaterial*    m_material;
    TVector3            m_intersectionPoint;
    TVector3            m_surfaceNormal;
    float               m_collisionTime;
};