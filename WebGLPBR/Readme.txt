* visualize point light locations
 * need emissive to make them really shine :P

! going far away from the spheres (pressing S for a long while) screws the lighting, i think something is wrong here.

* make it so you can toggle on and off texturing? (maybe under debug panel?)

* get normal mapping working for texturing on

! phong is not working correctly. Might be due to mapping from roughness.  Also there's stuff on the backside? and the spec highlights aren't moving w/ camera view.

* then move to IBL?

* Make a PBR renderer in WebGL:
 * https://learnopengl.com/#!PBR/Theory

* TODO's!

* could animate lights and/or objects over time

* put something meaningful in the debug panel that actually works, whenever it makes sense to.

* make shader compiler do non blocking thing with webgl2 render fences
* Debug displays to see all the buffers

* be able to load or define models somehow

* be able to load and use textures

* make wireframe mode better.
* turn on back face culling

* a decent automatic way of making vertex formats and vertex output to pixel input.
 * aka in and out for both vertex and pixel shaders

* a starting image that says "click to begin. mouse look, WASD controls"
* a starting image that says "loading..." for image loading (and later, shader compilations)

* ambient occlusion

? do we want to allow difference base reflectivity for different materials instead of just 0.04?

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