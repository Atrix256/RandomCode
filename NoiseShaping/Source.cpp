

int main (int argc, char ** argv)
{
    return 0;
}

/*

TODO:

* blue noise: 
 * generate white noise
 * blur it, subtract blur from white noise, make it be full range again.
 * to rehape it, make a struct of: value, x, y.  shuffle it. sort by value. replace value by percentage in list.

* do similar for red noise.

* animated gifs showing it evolve?

* maybe also animate blue noise w/ golden ratio, and mention that in this post? vs a flip book of blue noise?

* note: timothy lottes says to use sort that's more efficient for fixed sizes keys (radix sort), and fits it in 64 bits instead of a struct.

* note: you can't make blue noise by making white noise, doing DFT, modifying stuff, then doing IDFT. that is filtering it and is equivelant to what you are doing here.

? how to demo the quality of this noise?

* links
 * https://bartwronski.com/2016/10/30/dithering-part-two-golden-ratio-sequence-blue-noise-and-highpass-and-remap/
 * https://gpuopen.com/vdr-follow-up-fine-art-of-film-grain/

*/