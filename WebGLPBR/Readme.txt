* there is a strange white spot in the center of the spheres. Need to check that out.  May be that something is being allowed to go over 1.0 somehow?
* the cube maps are speckly at lower mips. i think you could sample the source data at lower mips to help that. OR use more samples.
* the cube maps aren't seamless. probably bad source data. need to look into it. maybe we are improperly bicubically interpolating across face edges, or we aren't adding half a pixel or something
* get rid of emissive display, since it isn't used.
* make sure ambient term goes away when IBL is on.

? maybe have debug panel open by default?

? what of the below is actually important?
 * cull as much as possible
 * blog post with demo, links to IBL specular / diffuse, learnopengl (https://learnopengl.com/#!PBR/Theory)
 * upload new version to demofox.org

* check todos in C++ and webgl

? is it normal for colors to show up on metal when it shouldn't? ask SE. like red metal showing a green light.

* the PBR images you have are gigantic, at 9MB a pop.  Can you find some smaller ones? Or do they need to be this huge? Could be affecting perf.
 * maybe shrink em down to whatever you can get away with

===== LATER =====

* model loading
* ray march bezier rectangle

* ambient occlusion of meshes, if using more complex meshes.
 * a program that does it, with a blog post about it!

* make shader compiler do non blocking thing with webgl2 render fences
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
