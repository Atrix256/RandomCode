#pragma once

#include "SVector.h"
#include "SMaterial.h"

struct SCollisionInfo
{
    SCollisionInfo()
        : m_objectID(c_invalidObjectID)
        , m_materialID(c_invalidMaterialID)
        , m_collisionTime(-1.0f)
        , m_fromInside(false)
    {
    }

    size_t  m_objectID;
    size_t  m_materialID;
    float   m_collisionTime;
    bool    m_fromInside;
    SVector m_intersectionPoint;
    SVector m_surfaceNormal;
};