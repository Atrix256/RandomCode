//=================================================================================
// TMaterialID Enum
//=================================================================================

#define MATERIAL(name, diffuse, emissive, reflective, refractive, refractionIndex, brdf) name,
#define MATERIALEX(name) name,
    
enum class TMaterialID : size_t {
    Error = 0,
    First = 0,
    MATERIALLIST()
    Count
};

#undef MATERIAL
#undef MATERIALEX

//=================================================================================
// GetMaterial Function
//=================================================================================

#define MATERIAL(name, diffuse, emissive, reflective, refractive, refractionIndex, brdf) \
    case TMaterialID::##name: \
    { \
        static const SMaterial material(diffuse, emissive, reflective, refractive, refractionIndex, brdf, TMaterialID::##name); \
        return material; \
    }
#define MATERIALEX(name) case TMaterialID::##name: scratchMaterial.m_materialID = TMaterialID::##name; GetMaterial_##name(info, scratchMaterial, normal);  return scratchMaterial;

const SMaterial& GetMaterial (const SCollisionInfo& info, SMaterial& scratchMaterial, SVector& normal)
{
    normal = info.m_surfaceNormal;
    switch (info.m_materialID)
    {
        MATERIALLIST()
    }
    static const SMaterial errorMaterial(SVector(1.0f, 0.0f, 1.0f), SVector(), SVector(), SVector(), 1.0f, EBRDF::standard, TMaterialID::Error);
    return errorMaterial;
}

#undef MATERIALEX
#undef MATERIAL

//=================================================================================
// Clean Up
//=================================================================================
#undef MATERIALLIST