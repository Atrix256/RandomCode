#pragma once

#include "SVector.h"

//=================================================================================
static const size_t c_invalidMaterialID = -1;
struct SMaterial
{
    SMaterial(SVector diffuse = SVector())
        : m_diffuse(diffuse)
    {
    }

    SVector m_diffuse;
};