#pragma once

#include "SVector.h"

#pragma once

enum class TMaterialID : size_t { };

//=================================================================================
static const TMaterialID c_invalidMaterialID = (TMaterialID)-1;

//=================================================================================
struct SMaterial
{
    SMaterial(SVector diffuse = SVector(), SVector emissive = SVector())
        : m_diffuse(diffuse)
        , m_emissive(emissive)
    {
    }

    SVector m_diffuse;
    SVector m_emissive;
};