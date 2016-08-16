#pragma once

#include "SVector.h"
#include "Utils.h"

struct SPointLight
{
    SPointLight(SVector position, SVector color)
        : m_position(position)
        , m_color(color)
    {
    }

    SVector m_position;
    SVector m_color;
};

//=================================================================================
inline SVector Biradiance (const SPointLight& light, const SVector& position)
{
    const float distanceSq = LengthSq(light.m_position - position);
    return light.m_color / (4.0f * c_pi * distanceSq);
}