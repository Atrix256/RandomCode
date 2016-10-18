#include <stdio.h>
#include "LTHE.h"

int main (int argc, char **argv)
{
	// encrypt the data
	std::vector<float> secretValues = { 3.14159265359f, 435.0f };
	std::vector<size_t> keys;
	if (!LTHE::Encrypt(secretValues, 1000000, "Encrypted.dat", keys))
	{
		printf("Could not encrypt data\n");
		return -1;
	}

	return 0;
}

/*

TODO:
* Encrypt M items to a file of N items
* Process a file
* Decrypt a file

Blog:
* Can encrypt M items by having the be in a list of N items
* Send that list of values to someone else
* They do operations on every item on the list and send it back

* further details
 * the N items should not make your data stand out. If your numbers are floats, don't generate a bunch of integers.
 * the larger the N, the more secure.
 * up to having the full space of values possible represented.
 * show how big that is in bytes for some value types.
 * The larger the M, the less secure

* For faster processing:
 * SIMD / multithreaded.
 * GPU
 ? can we benchmark?
 ! would be most robust to compare memory usage and execution speed to other FHE but i dont have time for that :p

--Links--
Fully Homomorphic SIMD Operations
http://homes.esat.kuleuven.be/~fvercaut/papers/DCC2011.pdf

*/