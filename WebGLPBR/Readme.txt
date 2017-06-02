* Make a PBR renderer in WebGL:
 * https://learnopengl.com/#!PBR/Theory

* put something meaningful in the debug panel that actually works, whenever it makes sense to.

* make shader compiler do non blocking thing with webgl2 render fences
* Debug displays to see all the buffers

* be able to load or define models somehow

* be able to load and use textures

* have object matrices?

* a decent automatic way of making vertex formats and vertex output to pixel input.
 * aka in and out for both vertex and pixel shaders

===== LATER =====

* SSR
* shadow maps (temporal soft shadow maps from gpu pro 2?)
* Deferred rendering w/ gbuffer and forward rendering pass
* temporal AA / temporal super sampling / temporal filtering
* red/ blue 3d glasses mode
* spherical harmonics
* HDR tonemapping and bloom
* different depth formats (reversed, non linear, etc)
* and so on!

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

==================== LANDFILL ====================

https://webgl2fundamentals.org/