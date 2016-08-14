#pragma once

#include "SVector.h"
#include "SMaterial.h"

struct SCollisionInfo
{
    SCollisionInfo()
        : m_objectID(c_invalidObjectID)
        , m_materialID(c_invalidMaterialID)
        , m_maxCollisionTime(-1.0f)
        , m_fromInside(false)
    {
    }

    TObjectID   m_objectID;
    TMaterialID m_materialID;
    bool        m_fromInside;
    SVector     m_intersectionPoint;
    SVector     m_surfaceNormal;

    // NOTE: you can set this to a positive value to limit a search by distance.
    // If there was a hit, this will be the time that it hit it at.
    float       m_maxCollisionTime;
};