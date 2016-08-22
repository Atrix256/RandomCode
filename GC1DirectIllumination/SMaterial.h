#pragma once

#include "SVector.h"

#pragma once

enum class TMaterialID : size_t;

enum class EBRDF {
    diffuse,
    reflect,
    refract
};

//=================================================================================
struct SMaterial
{
    SMaterial(SVector diffuse, SVector emissive, SVector reflection, EBRDF brdf)
        : m_diffuse(diffuse)
        , m_emissive(emissive)
        , m_reflection(reflection)
        , m_brdf(brdf)
    {
    }

    SVector m_diffuse;
    SVector m_emissive;
    SVector m_reflection;   // TODO: get rid of, when we have BRDF with impulse!
    EBRDF   m_brdf;
};