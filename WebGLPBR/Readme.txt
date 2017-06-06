* add some more materials to the draw material list
* add cube, tetrahedron, model to draw object list

* yes make vertex format automating a little better.

* get normals working and transforming in scenes before doing normal mapping'
 (get box, tetrahedron, model?)

* later, blend normals between faces to make smoother normals?

* get normal mapping working for texturing on
 * https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping
 * including the Gram-Schmidt process

? should blend the tangent / bitangent vectors for a vertex to be the normalized average of the faces?
 * I'm not sure that we can because each triangle is it's own thing.
 * we aren't using index buffers
 ? maybe we should use index buffers for meshes?

* then move to IBL!

* Make a PBR renderer in WebGL:
 * https://learnopengl.com/#!PBR/Theory

* TODO's!

* could animate lights and/or objects over time? (maybe a debug option to animate the lights)

* make shader compiler do non blocking thing with webgl2 render fences

* be able to load or define models somehow (or switch from spheres to boxes or tetrahedrons?)

* a starting image that says "click to begin. mouse look, WASD controls"
* a starting image that says "loading..." for image loading (and later, shader compilations)

* ambient occlusion of meshes, if using more complex meshes.
 * a program that does it, with a blog post about it!

* bezier rectangle later? (ray marched)

===== LATER =====

* blog post with links to resources, basic shell webgl pbr program, any programs you made or used.
* SSR
* shadow maps (temporal soft shadow maps from gpu pro 2?)
* Deferred rendering w/ gbuffer and forward rendering pass
* temporal AA / temporal super sampling / temporal filtering
* red/ blue 3d glasses mode
* spherical harmonics
* HDR tonemapping and bloom
* different depth formats (reversed, non linear, etc)
* subsurface scattering
* reflection / refraction / absorption / etc
* second specular lobe like in disney brdf
* and so on!

===== LINKS =====

* Free PBR Materials:
 http://freepbr.com/materials/rusted-iron-pbr-metal-material-alt/

https://webgl2fundamentals.org/

==================== LANDFILL ====================

https://webgl2fundamentals.org/