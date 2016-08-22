#define MATERIAL(name, diffuse, emissive, reflective, refractive, refractionIndex, brdf) name,
    
enum class TMaterialID : size_t {
    Error = 0,
    First = 0,
    MATERIALLIST()
    Count
};

#undef MATERIAL

#define MATERIAL(name, diffuse, emissive, reflective, refractive, refractionIndex, brdf) , SMaterial(diffuse, emissive, reflective, refractive, refractionIndex, brdf, TMaterialID::##name)
    
auto c_materials = make_array(
    SMaterial(SVector(1.0f, 0.0f, 1.0f), SVector(), SVector(), SVector(), 1.0f, EBRDF::standard, TMaterialID::Error)  // the default "error" material
    MATERIALLIST()
);

#undef MATERIAL

#undef MATERIALLIST