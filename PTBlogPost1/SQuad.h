#pragma once

#include "STriangle.h"

struct SQuad
{
    SQuad(const TVector3& a, const TVector3& b, const TVector3& c, const TVector3& d, const SMaterial& material)
        : m_a(a, b, c, material)
        , m_b(a, c, d, material)
    {
    }

    STriangle   m_a;
    STriangle   m_b;
};

