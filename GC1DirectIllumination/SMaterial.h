#pragma once

#include "SVector.h"

#pragma once

enum class TMaterialID : size_t;

enum class EBRDF {
    standard
};

//=================================================================================
struct SMaterial
{
    SMaterial(const SVector& diffuse, const SVector& emissive, const SVector& reflection, const SVector& refraction, float refractionIndex, EBRDF brdf, TMaterialID materialID)
        : m_diffuse(diffuse)
        , m_emissive(emissive)
        , m_reflection(reflection)
        , m_refraction(refraction)
        , m_refractionIndex(refractionIndex)
        , m_brdf(brdf)
        , m_materialID(materialID)
    {
    }

    const SVector     m_diffuse;
    const SVector     m_emissive;
    const SVector     m_reflection;   // TODO: get rid of, when we have BRDF with impulse!
    const SVector     m_refraction;   // TODO: beer's law and stuff may modify how this field is used.
    const float       m_refractionIndex;
    const EBRDF       m_brdf;
    const TMaterialID m_materialID;   // TODO: we only need this when debugging.  Maybe a #define to include this or not? smaller memory = less cache misses!
};