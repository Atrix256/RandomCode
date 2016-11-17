# WaveFunctionCollapse
This program generates bitmaps that are locally similar to the input bitmap.

<p align="center"><img alt="main collage" src="http://i.imgur.com/g1yGvL7.png"></p>
<p align="center"><img alt="main gif" src="http://i.imgur.com/sNuBVSr.gif"></p>

Local similarity means that

* (C1) Each NxN pattern of pixels in the output should occur at least once in the input.
* (Weak C2) Distribution of NxN patterns in the input should be similar to the distribution of NxN patterns over a sufficiently large number of outputs. In other words, probability to meet a particular pattern in the output should be close to the density of such patterns in the input.

In our examples typical value of N is 3.

<p align="center"><img alt="local similarity" src="http://i.imgur.com/KULGX86.png"></p>

WFC initializes output bitmap in a completely unobserved state, where each pixel value is in superposition of colors of the input bitmap (so if the input was black & white then the unobserved states are shown in different shades of grey). The coefficients in these superpositions are real numbers, not complex numbers, so it doesn't do the actual quantum mechanics, but it was inspired by QM. Then the program goes into the observation-propagation cycle:

* On each observation step an NxN region is chosen among the unobserved which has the lowest Shannon entropy. This region's state then collapses into a definite state according to its coefficients and the distribution of NxN patterns in the input.
* On each propagation step new information gained from the collapse on the previous step propagates through the output.

On each step the overall entropy decreases and in the end we have a completely observed state, the wave function has collapsed.

It may happen that during propagation all the coefficients for a certain pixel become zero. That means that the algorithm have run into a contradiction and can not continue. The problem of determining whether a certain bitmap allows other nontrivial bitmaps satisfying condition (C1) is NP-hard, so it's impossible to create a fast solution that always finishes. In practice, however, our algorithm runs into contradictions surprisingly rarely.

Watch a video demonstration of WFC algorithm on YouTube: [https://youtu.be/DOQTr2Xmlz0](https://youtu.be/DOQTr2Xmlz0)

## Algorithm
1. Read the input bitmap and count NxN patterns.
  1. (optional) Augment pattern data with rotations and reflections.
2. Create an array with the dimensions of the output (called "wave" in the source). Each element of this array represents a state of an NxN region in the output. A state of an NxN region is a superpostion of NxN patterns of the input with boolean coefficients (so a state of a pixel in the output is a superposition of input colors with real coefficients). False coefficient means that the corresponding pattern is forbidden, true coefficient means that the corresponding pattern is not yet forbidden.
3. Initialize the wave in the completely unobserved state, i.e. with all the boolean coefficients being true.
4. Repeat the following steps:
  1. Observation:
    1. Find a wave element with the minimal nonzero entropy. If there is no such elements (if all elements have zero or undefined entropy) then break the cycle (4) and go to step (5).
    2. Collapse this element into a definite state according to its coefficients and the distribution of NxN patterns in the input.
  2. Propagation: propagate information gained on the previous observation step.
5. By now all the wave elements are either in a completely observed state (all the coefficients except one being zero) or in the contradictive state (all the coefficients being zero). In the first case return the output. In the second case finish the work without returning anything.

## Tilemap generation
The simplest nontrivial case of our algorithm is when NxN=1x2 (well, NxM). If we simplify it even further by storing not the probabilities of pairs of colors but the probabilities of colors themselves, we get what we call a "simple tiled model". The propagation phase in this model is just adjacency constraint propagation. It's convenient to initialize the simple tiled model with a list of tiles and their adjacency data (adjacency data can be viewed as a large set of very small samples) rather than a sample bitmap.

<p align="center">
  <a href="http://i.imgur.com/jIctSoT.gif">GIF</a> |
  <a href="http://i.imgur.com/jIctSoT.gifv">GIFV</a>
</p>

Lists of all the possible pairs of adjacent tiles in practical tilesets can be quite long, so we implemented a symmetry system for tiles to shorten the enumeration. In that system each tile should be assigned with its symmetry type.

<p align="center"><img alt="symmetries" src="http://i.imgur.com/9H0frmK.png"></p>

Note that the tiles have the same symmetry type as their assigned letters (or, in other words, actions of the 
dihedral group D4 are isomorphic for tiles and their corresponding letters). With this system it's enough to enumerate pairs of adjacent tiles only up to symmetry, which makes lists of adjacencies for tilesets with many symmetrical tiles (even the summer tileset, despite drawings not being symmetrical the system considers such tiles to be symmetrical) several times shorter.

<p align="center">
	<img alt="knots" src="http://i.imgur.com/EnBkcVN.png">
	<img alt="tiled rooms" src="http://i.imgur.com/BruxOx9.png">
	<img alt="circuit 1" src="http://i.imgur.com/BYt7AR6.png">
	<img alt="circuit 2" src="http://i.imgur.com/yYHbMx8.png">
	<img alt="circles" src="http://i.imgur.com/Hrs0Ir8.png">
	<img alt="castle" src="http://i.imgur.com/Nd2mQOC.png">
	<img alt="summer 1" src="http://i.imgur.com/re8WBud.png">
	<img alt="summer 2" src="http://i.imgur.com/OmUHk1t.png">
</p>

Note that the unrestrained knot tileset (with all 5 tiles being allowed) is not interesting for WFC, because you can't run into a situation where you can't place a tile. We call tilesets with this property "easy". For example, Wang tilesets are easy. Without special heuristics easy tilesets don't produce interesting global arrangements, because correlations of tiles in easy tilesets quickly fall off with a distance.

Btw, a lot of cool Wang tilesets can be found on [cr31's site](http://s358455341.websitehome.co.uk/stagecast/wang/tiles_e.html). Consider the "Dual" 2-edge tileset there. How can it generate knots (without t-junctions, not easy) while being easy? The answer is, it can only generate a narrow class of knots, it can't produce an arbitrary knot.

## Higher dimensions
WFC algorithm in higher dimensions works completely the same way as in dimension 2, though performance becomes a big issue. These voxel models were generated with N=2 overlapping tiled model using 5x5x5 and 5x5x2 blocks and additional heuristics (height, density, curvature, ...).

<p align="center"><img alt="voxels" src="http://i.imgur.com/hsqPdQl.png"></p>

Higher resolution screenshots: [1](http://i.imgur.com/0bsjlBY.png), [2](http://i.imgur.com/GduN0Vr.png), [3](http://i.imgur.com/IEOsbIy.png).

Voxel models generated with WFC and other algorithms will be in a separate repo.

## Constrained synthesis
WFC algorithm supports constraints. Therefore, it can be easely combined with other generative algorithms or with manual creation.

Here is WFC autocompleting a level started by a human:

<p align="center">
  <a href="http://i.imgur.com/X3aNDUv.gif">GIF</a> |
  <a href="http://i.imgur.com/X3aNDUv.gifv">GIFV</a>
</p>

[ConvChain](https://github.com/mxgmn/ConvChain) algorithm satisfies the strong version of the condition (C2): the limit distribution of NxN patterns in the outputs it is producing is exactly the same as the distributions of patterns in the input. However, ConvChain doesn't satisfy (C1): it often produces noticeable artefacts. It makes sense to run ConvChain first to get a well-sampled configuration and then run WFC to correct local artefacts. This is similar to a common strategy in optimization: first run a Monte-Carlo method to find a point close to a global optimum and then run a gradient descent from that point for greater accuracy.

P. F. Harrison's [texture synthesis](https://github.com/mxgmn/SynTex) algorithm is significantly faster than WFC, but it has trouble with long correlations (for example, it's difficult for this algorithm to synthesize brick wall textures with correctly aligned bricks). But this is exactly where WFC shines, and Harrison's algorithm supports constraints. It makes sense first to generate a perfect brick wall blueprint with WFC and then run a constrained texture synthesis algorithm on that blueprint.

## Comments
Why the minimal entropy heuristic? I noticed that when humans draw something they often follow the minimal entropy heuristic themselves. That's why the algorithm is so enjoyable to watch.

The overlapping model relates to the simple tiled model the same way higher order Markov chains relate to order one Markov chains.

Note that the entropy of any node can't increase during the propagation phase, i.e. possibilities are not arising, but can be canceled. When propagation step can not decrease entropy further, we activate observation step. If the observation step can not decrease entropy, that means that the algorithm has finished working.

WFC's propagation phase is very similar to the loopy belief propagation algorithm. In fact, I first programmed belief propagation, but then switched to constraint propagation with a saved stationary distribution, because BP is significantly slower without a massive parallelization (on a CPU) and didn't produce significantly better results in my problems.

Note that the "Simple Knot" and "Trick Knot" samples have 3 colors, not 2.

One of the dimensions can be time. In particular, d-dimensional WFC captures the behaviour of any (d-1)-dimensional cellular automata.

## References
This project builds upon Paul Merrell's work on model synthesis, in particular discrete model synthesis chapter of [his dissertation](http://graphics.stanford.edu/~pmerrell/thesis.pdf). Paul propagates adjacency constraints in what we call a simple tiled model with a heuristic that tries to complete propagation in a small moving region.

It was also heavily influenced by declarative texture synthesis chapter of [Paul F. Harrison's dissertation](http://logarithmic.net/pfh-files/thesis/dissertation.pdf). Paul defines adjacency data of tiles by labeling their borders and uses backtracking search to fill the tilemap.

## Ports, notable forks and other projects based on this work

* Emil Ernerfeldt made a [C++ port](https://github.com/emilk/wfc).
* [Max Aller](https://github.com/nanodeath) is making a Kotlin (JVM) library, [Kollapse](https://gitlab.com/nanodeath/kollapse).
* [Kevin Chapelier](https://github.com/kchapelier) made a [JavaScript port](http://www.kchapelier.com/wfc-example/overlapping-model.html).
* Oskar Stalberg programmed a 3d tiled model, a 2d tiled model for irregular grids on a sphere and is building beautiful 3d tilesets for them: [1](https://twitter.com/OskSta/status/787319655648100352), [2](https://twitter.com/OskSta/status/784847588893814785), [3](https://twitter.com/OskSta/status/784847933686575104), [4](https://twitter.com/OskSta/status/784848286272327680), [5](https://twitter.com/OskSta/status/793545297376972801), [6](https://twitter.com/OskSta/status/793806535898136576).
* [Joseph Parker](https://github.com/selfsame) adapted [WFC to Unity](https://selfsame.itch.io/unitywfc).
* [Martin O'Leary](https://github.com/mewo2) applied a WFC-like algorithm to poetry generation: [1](https://twitter.com/mewo2/status/789167437518217216), [2](https://twitter.com/mewo2/status/789177702620114945), [3](https://twitter.com/mewo2/status/789187174683987968), [4](https://twitter.com/mewo2/status/789897712372183041).
* [Nick Nenov](https://github.com/NNNenov) made a [3d voxel tileset](https://twitter.com/NNNenov/status/789903180226301953) based on my Castle tileset. Nick uses text output option in the tiled model to reconstruct 3d models in Cinema 4D.
* Sean Leffler implemented the [overlapping model in Rust](https://github.com/sdleffler/collapse).
* rid5x is making an [OCaml version of WFC](https://twitter.com/rid5x/status/782442620459114496).
* I published a very basic [3d tiled model](https://bitbucket.org/mxgmn/basic3dwfc/overview) so people could make their own 3d tilesets without waiting for the full 3d repository.

## How to build
WFC is a console application that depends only on the standard library. Build instructions from the community for various platforms can be found in the [relevant issue](https://github.com/mxgmn/WaveFunctionCollapse/issues/3). Casey Marshall made a [pull request](https://github.com/mxgmn/WaveFunctionCollapse/pull/18) that makes using the program with the command line more convenient and includes snap packaging.

## Credits
Some samples are taken from the games Ultima IV and [Dungeon Crawl](https://github.com/crawl/crawl). Circles tileset is taken from [Mario Klingemann](https://twitter.com/quasimondo/status/778196128957403136). Idea of generating integrated circuits was suggested to me by [Moonasaur](https://twitter.com/Moonasaur/status/759890746350731264) and their style was taken from Zachtronics' [Ruckingenur II](http://www.zachtronics.com/ruckingenur-ii/). Summer tileset was made by Hermann Hillmann. Voxel models were rendered in [MagicaVoxel](http://ephtracy.github.io/).

<p align="center"><img alt="second collage" src="http://i.imgur.com/CZsvnc7.png"></p>
<p align="center"><img alt="voxel perspective" src="http://i.imgur.com/RywXCHn.png"></p>
