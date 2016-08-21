#define MATERIAL(name, diffuse, emissive, reflective) , SMaterial(diffuse, emissive, reflective)
    
auto c_materials = make_array(
    SMaterial(SVector(1.0f, 0.0f, 1.0f), SVector(), SVector())  // the default "error" material
    MATERIALLIST()
);

#undef MATERIAL

#define MATERIAL(name, diffuse, emissive, reflective) name,
    
enum class TMaterialID : size_t {
    Error = 0,
    First = 0,
    MATERIALLIST()
    Count
};

#undef MATERIAL

#undef MATERIALLIST