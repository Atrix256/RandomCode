? isn't specular supposed to follow the camera somehow? it doesn't seem to be.

* Make a PBR renderer in WebGL:
 * https://learnopengl.com/#!PBR/Theory

* TODO's!

* put something meaningful in the debug panel that actually works, whenever it makes sense to.

* make shader compiler do non blocking thing with webgl2 render fences
* Debug displays to see all the buffers

* be able to load or define models somehow

* be able to load and use textures

* make wireframe mode better.
* turn on back face culling

* a decent automatic way of making vertex formats and vertex output to pixel input.
 * aka in and out for both vertex and pixel shaders

* a "reload" button that reloads the page, but puts you in the same location with the same debug options?

* a starting image that says "click to begin. mouse look, WASD controls"

* ambient occlusion

? what #version should i be using? im using 300 es, but the tutorial uses 330 core. if i try to use 330 core i get an error?

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
* and so on!

===== LINKS =====

* Free PBR Materials:
 http://freepbr.com/materials/rusted-iron-pbr-metal-material-alt/

https://webgl2fundamentals.org/

===== DONE =====

--5/31/17
* started on this in the morning at home
* lunch
 * got a triangle rendering
 * got a basic collapsible debug panel working that has semi transparent background
* night
 * projection matrix

--6/1/17
* morning: frame delta time, FPS display, right align debug panel text so it doesn't move when you open and close it.
 * key presses control scaling.
* lunch: more automatic uniform passing, basic camera

--6/2/17
* morning: mouse capture and free camera
* lunch: sphere mesh generation, and drawing multiple spheres.

--6/3/7
* morning: normals on spheres and non PBR directional + ambient lighting. Object Color.
 * spheres on a grid
 * streamline uniform passing a bit
* night: basic image loading, uv passing, chrome link to load local images.
 * image loading seemingly done (actually no, can't use 2 images in 1 shader?)

--6/4/17
* morning: fixed image loading.
 * got basic pbr spheres working woo!
 * i think specular may be wrong though, not following camera

==================== LANDFILL ====================

https://webgl2fundamentals.org/