! upload what you have so far to demofox.org?

! need to read through the whole normal mapping article, make sure you understand it all and are doing right space work etc
 * https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping

* add a model to the shape drop down list

* profile code to see where time is going so far

* understand the PBR so far

? is it normal for colors to show up on metal when it shouldn't? ask SE. like red metal showing a green light.

! internalize how tangents are calculated from UVs again
 * https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping

* then move to IBL!

* Make a PBR renderer in WebGL:
 * https://learnopengl.com/#!PBR/Theory

* TODO's!

* make shader compiler do non blocking thing with webgl2 render fences

* a starting image that says "click to begin. mouse look, WASD controls"
* an image that says "loading..." for image loading (and later, shader compilations)

* ambient occlusion of meshes, if using more complex meshes.
 * a program that does it, with a blog post about it!

* after IBL, get rid of ambient light? or have a mode to turn off IBL and use ambient in that case?

* bezier rectangle later? (ray marched)

? orbit camera mode?

? is there a better way to get barycentrics in webgl?

? use the #define's to change the vertex format to be leaner when it can be?
 * or can we share the buffers better?

? load images on demand instead of in the beginning?

* try running webgl in renderdoc (launch chrome from renderdoc)
 * "C:\Program Files (x86)\Google\Chrome\Application\chrome.exe" --allow-file-access-from-files
 * first attempt didn't work

* drop down menu for texture slots, with one main drop down for "presets" for the materials.
 * Yes get rid of untextured shader permutations after this and just use white textures etc?
  * or allow untextured still

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

? blend normals / tangent / bitangent across faces to make smoother normals?
 * make a flag to allow this or not.  The sphere would like it, but box and tetrahedron for example wouldn't.
 * probably need to move to index buffers if doing this, to find all faces to average across. a good idea anyways likely

===== LINKS =====

* Free PBR Materials:
 http://freepbr.com/materials/rusted-iron-pbr-metal-material-alt/

https://webgl2fundamentals.org/

==================== LANDFILL ====================

https://webgl2fundamentals.org/