#pragma once

#include "SVector.h"

#pragma once

enum class TMaterialID : size_t { };

//=================================================================================
static const TMaterialID c_invalidMaterialID = (TMaterialID)-1;

//=================================================================================
struct SMaterial
{
    SMaterial(SVector diffuse, SVector emissive, SVector reflection)
        : m_diffuse(diffuse)
        , m_emissive(emissive)
        , m_reflection(reflection)
    {
    }

    SVector m_diffuse;
    SVector m_emissive;
    SVector m_reflection;   // TODO: get rid of, when we have BRDF with impulse!
};