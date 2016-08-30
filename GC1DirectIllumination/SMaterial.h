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
    SMaterial()
        : m_refractionIndex(1.0f)
        , m_brdf()
        , m_materialID()
        , m_roughness(0.0f)
    {
    }

    SMaterial(const SVector& diffuse, const SVector& emissive, const SVector& reflection, const SVector& refraction, float refractionIndex, float roughness, EBRDF brdf, TMaterialID materialID)
        : m_diffuse(diffuse)
        , m_emissive(emissive)
        , m_reflection(reflection)
        , m_refraction(refraction)
        , m_refractionIndex(refractionIndex)
        , m_roughness(roughness)
        , m_brdf(brdf)
        , m_materialID(materialID)
    {
    }

    SVector     m_diffuse;
    SVector     m_emissive;
    SVector     m_reflection;   // TODO: get rid of, when we have BRDF with impulse!
    SVector     m_refraction;   // TODO: beer's law and stuff may modify how this field is used.
    float       m_refractionIndex;
    float       m_roughness;
    EBRDF       m_brdf;
    TMaterialID m_materialID;   // TODO: we only need this when debugging.  Maybe a #define to include this or not? smaller memory = less cache misses!
};