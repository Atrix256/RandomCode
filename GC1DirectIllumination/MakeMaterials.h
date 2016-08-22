#define MATERIAL(name, diffuse, emissive, reflective, brdf) , SMaterial(diffuse, emissive, reflective, brdf)
    
auto c_materials = make_array(
    SMaterial(SVector(1.0f, 0.0f, 1.0f), SVector(), SVector(), EBRDF::diffuse)  // the default "error" material
    MATERIALLIST()
);

#undef MATERIAL

#define MATERIAL(name, diffuse, emissive, reflective, brdf) name,
    
enum class TMaterialID : size_t {
    Error = 0,
    First = 0,
    MATERIALLIST()
    Count
};

#undef MATERIAL

#undef MATERIALLIST