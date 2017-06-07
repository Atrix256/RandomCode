* drop down menu for texture slots, with one main drop down for "presets" for the materials. Yes get rid of untextured shader permutations.

* add cube, tetrahedron, model to draw object list

* yes make vertex format automating a little better.

* get normals working and transforming in scenes before doing normal mapping'
 (get box, tetrahedron, model?)

* an option to rotate the objects over time (will also help make sure normal transforms work correctly)

* make wireframe take derivative of uv's to see how wide to be?

* see if there's any places where you need to use VectorSubtract etc instead of doing it manually?

* get normal mapping working for texturing on
 * https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping
 * including the Gram-Schmidt process

? should blend the tangent / bitangent vectors for a vertex to be the normalized average of the faces?
 * I'm not sure that we can because each triangle is it's own thing.
 * we aren't using index buffers
 ? maybe we should use index buffers for meshes?

* profile code to see where time is going so far

* understand the PBR so far

? is it normal for colors to show up on metal when it shouldn't? ask SE. like red metal showing a green light

* then move to IBL!

* Make a PBR renderer in WebGL:
 * https://learnopengl.com/#!PBR/Theory

* show ms next to fps

* TODO's!

* make shader compiler do non blocking thing with webgl2 render fences

* be able to load or define models somehow (or switch from spheres to boxes or tetrahedrons?)

* a starting image that says "click to begin. mouse look, WASD controls"
* a starting image that says "loading..." for image loading (and later, shader compilations)

* ambient occlusion of meshes, if using more complex meshes.
 * a program that does it, with a blog post about it!

* after IBL, get rid of ambient light?

* bezier rectangle later? (ray marched)

? orbit camera mode?

? is there a better way to get barycentrics in webgl?

? use the #define's to change the vertex format to be leaner when it can be?

? load images on demand instead of in the beginning?

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

? blend normals across faces to make smoother normals?
 * make a flag to allow this or not.  The sphere would like it, but box and tetrahedron for example wouldn't.

===== LINKS =====

* Free PBR Materials:
 http://freepbr.com/materials/rusted-iron-pbr-metal-material-alt/

https://webgl2fundamentals.org/

==================== LANDFILL ====================

https://webgl2fundamentals.org/