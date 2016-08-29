#pragma once

#include "SVector.h"
#include "SMaterial.h"

struct SCollisionInfo
{
    SCollisionInfo()
        : m_objectID(c_invalidObjectID)
        , m_materialID()
        , m_maxCollisionTime(-1.0f)
        , m_fromInside(false)
        , m_u(0.0f)
        , m_v(0.0f)
    {
    }

    TObjectID   m_objectID;
    TMaterialID m_materialID;
    SVector     m_intersectionPoint;
    SVector     m_surfaceNormal;
    SVector     m_tangent;
    SVector     m_biTangent;
    float       m_u;
    float       m_v;
    bool        m_fromInside;

    // NOTE: you can set this to a positive value to limit a search by distance.
    // If there was a hit, this will be the time that it hit it at.
    float       m_maxCollisionTime;

    void SuccessfulHit(
        TObjectID objectID,
        TMaterialID materialID,
        SVector intersectionPoint,
        SVector surfaceNormal,
        bool fromInside,
        float intersectionTime,
        SVector tangent = SVector(),   // TODO: make these required instead of optional after all shapes work!
        SVector biTangent = SVector(),
        float u = 0.0f,
        float v = 0.0f
    )
    {
        m_objectID = objectID;
        m_materialID = materialID;
        m_intersectionPoint = intersectionPoint;
        m_surfaceNormal = surfaceNormal;
        m_fromInside = fromInside;
        m_maxCollisionTime = intersectionTime;
        m_tangent = tangent;
        m_biTangent = biTangent;
        m_u = u;
        m_v = v;
    }
};