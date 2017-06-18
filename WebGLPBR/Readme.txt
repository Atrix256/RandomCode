
* check todos

* then move to specular IBL!

* after IBL, make it so PBR grid can use roughness of 0, not the minimum value it has.

* Make a PBR renderer in WebGL:
 * https://learnopengl.com/#!PBR/Theory

* add a model to the shape drop down list

* profile code to see where time is going so far

* TODO's!

* make shader compiler do non blocking thing with webgl2 render fences

* a starting image that says "click to begin. mouse look, WASD controls"
* an image that says "loading..." for image loading (and later, shader compilations)

* ambient occlusion of meshes, if using more complex meshes.
 * a program that does it, with a blog post about it!

! upload what you have so far to demofox.org?

? add an emissive texture? or get rid of emissive display maybe?

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

* blog post on diffuse IBL at this point, with screenshots?
 * maybe save it for later when you understand the whole picture better.

? is it normal for colors to show up on metal when it shouldn't? ask SE. like red metal showing a green light.

* the PBR images you have are gigantic, at 9MB a pop.  Can you find some smaller ones? Or do they need to be this huge? Could be affecting perf.
 * maybe shrink em down to whatever you can get away with

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
* parallax bump mapping
* and so on!

? blend normals / tangent / bitangent across faces to make smoother normals?
 * make a flag to allow this or not.  The sphere would like it, but box and tetrahedron for example wouldn't.
 * probably need to move to index buffers if doing this, to find all faces to average across. a good idea anyways likely

* when surfaces get wet: https://seblagarde.wordpress.com/2013/04/14/water-drop-3b-physically-based-wet-surfaces/

===== LINKS =====

* Free PBR Materials:
 http://freepbr.com/materials/rusted-iron-pbr-metal-material-alt/

https://webgl2fundamentals.org/


http://blog.selfshadow.com/publications/s2014-shading-course/frostbite/s2014_pbs_frostbite_slides.pdf

https://learnopengl.com/#!PBR/Theory

http://renderwonk.com/publications/s2010-shading-course/hoffman/s2010_physically_based_shading_hoffman_b_notes.pdf

https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf

http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf

===== NOTES =====

* Normal Mapping:
 * We could transform light positions etc into tangent space, in the vertex shader and pass as interpolants.
 * Instead of doing the TBN matrix multiply to transform the texture normal into world space.
 * That would save a matrix multiply in the pixel shader.
 * The goal is to go deferred at some point though, which would need world space normals, so leaving it like it is.

==================== LANDFILL ====================
