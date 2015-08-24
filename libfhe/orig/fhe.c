/* fhe.c -- Fully homorphic encryption after Gentry (2008) §3.2 */

/*
Copyright © 2010 Jan Minář <rdancer@rdancer.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 (two),
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "fhe.h"

/*
* From <http://en.wikipedia.org/w/index.php?title=Adder_(electronics)&oldid=381607326#Full_adder>
* retrieved on 2010-09-01
*/
#define FULL_ADDER(bit1, bit2, carryIn) \
	sum = fhe_xor_bits(fhe_xor_bits(bit1, bit2), carryIn); \
	carryOut = fhe_xor_bits( \
	    fhe_and_bits(bit1, bit2), \
	    fhe_and_bits(carryIn, fhe_xor_bits(bit1, bit2)) \
	);


static inline void *checkMalloc(size_t size) {
    void *ptr = NULL;

    if ((ptr = malloc(size
        + /* valgrind complaints about invalid reads/writes */ 8))
        == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/**
* Create a string representation of a given multi-precision integer
*
* @param s destination pointer
* @param mp the number to represent
* @param base Output the string representation in this base
*/
void static fhe_mp_to_string(char **s, mp_int *mp, int base) {
    int size;	// is int, not size_t; this is how libtommath wants it

    // libtommath documentation says that the maximum base is 64; base < 2 does
    // not make sense.  (Actually, base 1 makes sense, but nobody seems to
    // agree with me on that one.)
    assert(base > 1 && base <= 64);

    /* Allocate space and generate ASCII representation of private key */
    (void)mp_radix_size(mp, 0x10, &size);
    *s = (char *)checkMalloc(size);
    (void)mp_toradix(mp, *s, 0x10);
}

mp_int *privateKey;
unsigned long int securityParameter;
static FILE *randomFile;


/**
* Multiply two signed integers together.
*
* TODO: Determine the bit-length dynamically
*
* @param integer1 Multiplier
* @param integer2 Multiplicand
* @return The product of the two integers
*/
mp_int **fhe_multiply_integers(mp_int **integer1, mp_int **integer2) {
    mp_int **result, **intermediate_product, **factor1, **factor2;
    mp_int **tmp;

    factor1 = (mp_int **)checkMalloc(sizeof(void *[FHE_INTEGER_BIT_WIDTH]));
    factor2 = (mp_int **)checkMalloc(sizeof(void *[FHE_INTEGER_BIT_WIDTH]));
    result = (mp_int **)checkMalloc(sizeof(void *[FHE_INTEGER_BIT_WIDTH]));
    intermediate_product = (mp_int **)checkMalloc(sizeof(void *[FHE_INTEGER_BIT_WIDTH]));

    /* Initialize the objects that we need to be zero */
    for (int i = 0; i < FHE_INTEGER_BIT_WIDTH; i++) {
        factor1[i] = (mp_int *)checkMalloc(sizeof(void *));
        factor2[i] = (mp_int *)checkMalloc(sizeof(void *));
        result[i] = (mp_int *)checkMalloc(sizeof(void *));
        mp_init(result[i]);
        intermediate_product[i] = (mp_int *)checkMalloc(sizeof(void *));
        mp_init(intermediate_product[i]);

        /* Create a local deep copy of the parameters */
        mp_init_copy(factor1[i], integer1[i]);
        mp_init_copy(factor2[i], integer2[i]);
    }

    /*
    * Signed integers multiplier adapted from:
    * <http://en.wikipedia.org/w/index.php?title=Binary_multiplier&oldid=382215754#Engineering_approach:_signed_integers>
    * (retrieved on 2010-09-01)
    *
    1  -p0[7]  p0[6]  p0[5]  p0[4]  p0[3]  p0[2]  p0[1]  p0[0]
    -p1[7] +p1[6] +p1[5] +p1[4] +p1[3] +p1[2] +p1[1] +p1[0]   0
    -p2[7] +p2[6] +p2[5] +p2[4] +p2[3] +p2[2] +p2[1] +p2[0]   0      0
    -p3[7] +p3[6] +p3[5] +p3[4] +p3[3] +p3[2] +p3[1] +p3[0]   0      0      0
    -p4[7] +p4[6] +p4[5] +p4[4] +p4[3] +p4[2] +p4[1] +p4[0]   0      0      0      0
    -p5[7] +p5[6] +p5[5] +p5[4] +p5[3] +p5[2] +p5[1] +p5[0]   0      0      0      0      0
    -p6[7] +p6[6] +p6[5] +p6[4] +p6[3] +p6[2] +p6[1] +p6[0]   0      0      0      0      0      0
    1  +p7[7] -p7[6] -p7[5] -p7[4] -p7[3] -p7[2] -p7[1] -p7[0]   0      0      0      0      0      0      0
    ------------------------------------------------------------------------------------------------------------
    P[15]  P[14]  P[13]  P[12]  P[11]  P[10]   P[9]   P[8]   P[7]   P[6]   P[5]   P[4]   P[3]   P[2]   P[1]  P[0]
    *
    */


    /* Add to the result, then shift left and repeat */
    for (int i = 0; i < FHE_INTEGER_BIT_WIDTH_MULTIPLY; i++) {
        //(void)printf("i: % 2d\n", i);

        // Unfortunately we cannot know whether a given bit is zero, therefore
        // we have to perform all the multiplications, blindly

        /* Compute the product of the multiplicand × i-th bit of the multiplier
        */
        /* The zeroes on the right */
        for (int j = 0; j < FHE_INTEGER_BIT_WIDTH_MULTIPLY; j++) {
            intermediate_product[j] = fhe_encrypt_one_bit((bool)0);
            //(void)printf("i: % 2d j: % 2d\n", i, j);
        }
        /* Now the product on this line itself */
        for (int j = 0; j < FHE_INTEGER_BIT_WIDTH_MULTIPLY - i; j++) {
            intermediate_product[j + i] = fhe_and_bits(factor2[i], factor1[j]);
        }

        // This is important for signed integers -- or something like this...
        //
        //	//if (i == FHE_INTEGER_BIT_WIDTH_MULTIPLY - 1) {
        //	//    /* The last line -- invert all the bits */
        //	//} else {
        //	//    /* Invert the most-significant bit */
        //	//    intermediate_product[FHE_INTEGER_BIT_WIDTH_MULTIPLY] = fhe_xor_bits(
        //	//	    intermediate_product[FHE_INTEGER_BIT_WIDTH_MULTIPLY],
        //	//	    intermediate_product[FHE_INTEGER_BIT_WIDTH_MULTIPLY]);
        //	//}

        /* Add the this product (this line) to the overall result (the grand
        * total) */
        tmp = fhe_add_integers(result, intermediate_product);
        DESTROY_ENCRYPTED_INTEGER(result);
        result = tmp;
    }

    /*
    * Clean-up and return the result
    */

    DESTROY_ENCRYPTED_INTEGER(factor1);
    DESTROY_ENCRYPTED_INTEGER(factor2);
    DESTROY_ENCRYPTED_INTEGER(intermediate_product);

    return result;
}

/**
* Add two signed integers together
*
* TODO: Determine the bit-length dynamically
*
* @param integer1 First addend
* @param integer1 Second addend
* @return The sum of the two addends
*/
mp_int **fhe_add_integers(mp_int **integer1, mp_int **integer2) {
    mp_int **sum, *tmp1, *tmp2, *tmp3;
    INIT_MP_INT(carry);

    sum = (mp_int **)checkMalloc(sizeof(void *[FHE_INTEGER_BIT_WIDTH]));

    for (int i = 0; i < FHE_INTEGER_BIT_WIDTH; i++) {
        assert(integer1[i] != NULL && integer2[i] != NULL);

        /*
        * Full adder adapted from (retrieved on 2010-09-01):
        * <http://en.wikipedia.org/w/index.php?title=Adder_(electronics)&oldid=381607326#Full_adder>
        */

        /* S = A ⊕ B ⊕ C_in */
        tmp1 = fhe_xor_bits(integer1[i], integer2[i]);
        sum[i] = fhe_xor_bits(tmp1, carry);

        /* C_out = (A × B) ⊕ (C_in × (A ⊕ B)) */
        tmp2 = fhe_and_bits(integer1[i], integer2[i]);
        tmp3 = fhe_and_bits(carry, tmp1);

        DESTROY_MP_INT(carry);
        carry = fhe_xor_bits(tmp2, tmp3);

        DESTROY_MP_INT(tmp1);
        DESTROY_MP_INT(tmp2);
        DESTROY_MP_INT(tmp3);
    }

    DESTROY_MP_INT(carry);


    return sum;
}

/**
* Encrypt an arbitrary integer under the scheme.
*
* @param integer An integer
* @return Array of encrypted bits
*/
mp_int **fhe_encrypt_integer(fhe_integer integer) {
    mp_int **encryptedInteger;

    encryptedInteger = (mp_int **)checkMalloc(sizeof(void *[FHE_INTEGER_BIT_WIDTH]));
    for (int i = 0; i < FHE_INTEGER_BIT_WIDTH; i++) {
        encryptedInteger[i] = fhe_encrypt_one_bit((integer >> i) & 0x1);
    }


    return encryptedInteger;
}

/**
* Decrypt an integer.
*
* @param encryptedInteger Array of encrypted bits
* @return An integer
*/
fhe_integer fhe_decrypt_integer(mp_int **encryptedInteger) {
    fhe_integer integer = 0;	// must initialize to 0

    for (int i = 0; i < FHE_INTEGER_BIT_WIDTH; i++) {
        integer += fhe_decrypt_one_bit(encryptedInteger[i]) << i;
    }

    return integer;
}

/**
* Generate random number from range: (–2^numberOfBits, 2^numberOfBits>.
*
* @numberOfBits Number of bits this random integer will have
* @return Random integer
*/
long int fhe_random(unsigned long long int numberOfBits) {
    mp_int *randomInteger;
    INIT_MP_INT(mask);

    mp_set(mask, 1);
    for (unsigned long long int i = 0; i < numberOfBits; i++) {
        mp_mul_2(mask, mask);
    }


    randomInteger = fhe_new_random_integer(numberOfBits + 1);
    mp_xor(randomInteger, mask, randomInteger);

    DESTROY_MP_INT(mask);

    // Note: need to cast from unsigned to signed
    return (long int)mp_get_int(randomInteger);
}

/**
* Generate a numberOfBits-bit long random integer.  Note: The most-significant
* bit will not be random: it will always be 1.  The number of random bits
* therefore is numberOfBits - 1.
*
* @return random number
*/
mp_int *fhe_new_random_integer(unsigned long long int numberOfBits) {
#if defined(__linux__)
    int c;
    unsigned int bitmask;
    unsigned long int i;
    INIT_MP_INT(randomInteger);
    INIT_MP_INT(tmp1);


    /* The whole bytes */
    for (bitmask = 0xff, i = numberOfBits; i > 0;) {
        if ((c = fgetc(randomFile)) == EOF) {
            perror("Error reading " RANDOM_FILE);
            exit(EXIT_FAILURE);
        }

        /* Ensure the most-significant bit of the random integer is always 1 */
        if (i == numberOfBits) {
            assert(i > 0);
            c |= 0x1 << ((i < 8 ? i : 8) - 1);
        }

        if (i < 8) {
            // The last few bits will have a bitmask
            bitmask = 0xff >> (8 - i);
        }
        for (unsigned long int j = 0; j < i; j++) {
            mp_mul_2(randomInteger, randomInteger);
        }
        mp_set_int(tmp1, c & bitmask);
        mp_add(randomInteger, tmp1, randomInteger);

        /* Decrement; check for underflow */
        if (i < 8) {
            break;
        }
        else {
            i -= 8;
        }
    }

    DESTROY_MP_INT(tmp1);
    return randomInteger;
#else // __linux__
# error This function is only implemented for Linux
#endif // __linux__
}

/**
* This function implements the special modulo operation found in §3.2:
* “(c mod p) is the integer c' in (-p/2, p/2> such that p divides c − c')”
*/
mp_int *modulo(mp_int *divident, mp_int *divisor) {
    INIT_MP_INT(remainder);

    // XXX re-enable
    //assert(mpz_cmp_ui(*divisor, 0) != 0);
    mp_mod(divident, divisor, remainder);
    mp_abs(remainder, remainder);

    /* Adjust (divident/2, divident) -> (-divident/2, 0) */
    INIT_MP_INT(divident_half);
    mp_div_2(divident, divident_half);
    mp_abs(divident_half, divident_half);
    if (mp_cmp(remainder, divident_half) == MP_GT) {
        mp_sub(divisor, remainder, remainder);
    }
    DESTROY_MP_INT(divident_half);

    return remainder;
}
/**
* Decrypt cryptotext of a single bit
*
* @param encryptedBit Single bit, encrypted
* @return Decrypted bit
*/
bool fhe_decrypt_one_bit(mp_int *encryptedBit) {
    bool result;
    INIT_MP_INT(modulus);

    // XXX re-enable
    //assert(mpz_cmp_ui(*privateKey, 0) != 0);
    mp_mod(encryptedBit, privateKey, modulus);
    result = (bool)(mp_get_int(modulus) & 0x1);
    DESTROY_MP_INT(modulus);
    return result;
}

/**
* Take a one-bit number, and encrypt it under this scheme
*
* @param plainTextBit One-bit number to be encrypted
* @return Cryptotext under this scheme
*/
mp_int *fhe_encrypt_one_bit(bool plainTextBit) {
    INIT_MP_INT(encryptedBit);
    INIT_MP_INT(tmp1);

    /* noise: 2r */
    assert(bitsN > 0);
    encryptedBit = fhe_new_random_integer(bitsN - 1);
    mp_mul_2(encryptedBit, encryptedBit);

    /* add parity */
    mp_set(tmp1, plainTextBit);
    mp_add(encryptedBit, tmp1, encryptedBit);

    /* add pq */
    INIT_MP_INT(pq);
    tmp1 = fhe_new_random_integer(bitsQ);
    mp_mul(privateKey, tmp1, pq);
    mp_add(encryptedBit, pq, encryptedBit);
    DESTROY_MP_INT(tmp1);
    DESTROY_MP_INT(pq);

    return encryptedBit;
}

/**
* Initialize the security scheme.
*
* @param mySecurityParameter lambda, from this the bit-widths of the various
* parts of the security scheme derive.
*/
void fhe_initialize(unsigned long int mySecurityParameter) {
    securityParameter = mySecurityParameter;

    // TODO Possibly close the file when we're finished, like so:
    //
    //if (fclose(randomFile) == EOF) {
    //    perror("Error closing " RANDOM_FILE);
    //    // Does not have adverse effect on program run
    //}

    if ((randomFile = fopen(RANDOM_FILE, "r")) == NULL) {
        perror("Error opening " RANDOM_FILE);
        exit(EXIT_FAILURE);
    }


    /* Private key is a bitsP-bit wide even integer */
    privateKey = fhe_new_random_integer(bitsP - 1);
    mp_mul_2(privateKey, privateKey);
}


/****************************************************************************
**                        Arithmetical Operations                         **
****************************************************************************/

/**
* Exclusive or — boolean exclusive disjunction.  This function is called Add()
* in Gentry (2008).  Note: The Sub() function in Gentry (2008) gives the
* exactly same results for single bits.
*
* @param bit1 Single encrypted bit
* @param bit2 Single encrypted bit
* @return _Exclusive_or_ of the parameters
*/
mp_int *fhe_xor_bits(mp_int *bit1, mp_int *bit2) {
    INIT_MP_INT(result);

    mp_add(bit1, bit2, result);

    return result;
}

/**
* And — boolean multiplication. This function is called Mult() in Gentry
* (2008) §3.2.
*
* @param bit1 Single encrypted bit
* @param bit2 Single encrypted bit
* @return Logical _and_ of the parameters
*/
mp_int *fhe_and_bits(mp_int *bit1, mp_int *bit2) {
    INIT_MP_INT(result);

    mp_mul(bit1, bit2, result);

    return result;
}


/**
* Program entry point -- used to test the library
*/
int main(int argc, char **argv) {

    int retval = 0, ok;
    mp_int **tmp1;

    // Get rid of compiler warning about unused parameters
    argc = argc;
    argv = argv;

    fhe_initialize(argc > 1 ? atoi(argv[1]) : 2);

    INIT_MP_INT(bitValue0);
    INIT_MP_INT(bitValue1);

    bitValue1 = fhe_encrypt_one_bit(1);
    bitValue0 = fhe_encrypt_one_bit(0);
    mp_int *bitValues[2] = {
        bitValue0,
        bitValue1
    };

    do {
        bool result;
        char *buf;

        /* Allocate space and generate ASCII representation of private key */
        fhe_mp_to_string(&buf, privateKey, 0x10);
        (void)printf("Private key: 0x%s\n", buf);
        free(buf), buf = NULL;

        for (int i = 0; i < 2; i++) {
            fhe_mp_to_string(&buf, bitValues[i], 64);
            (void)printf("Encrypted bit (%d): 0x%s\n", i, buf);
            free(buf), buf = NULL;

            result = fhe_decrypt_one_bit(bitValues[i]);
            ok = (result == i);
            (void)printf("Decrypted bit (%d): %d %s\n",
                i,
                (int)result,
                ok ? "OK" : "FAIL");
            assert(ok);
        }

    } while (0);

    /*
    * Boolean arithmetics
    */

    /* XOR */

    do {
        bool result;

        (void)printf("\n");
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                result = fhe_decrypt_one_bit(/* leak */fhe_xor_bits(bitValues[i],
                    bitValues[j]));
                retval |= !(ok = (result == (i ^ j)));
                (void)printf("%d ⊕ %d = %d %s\n",
                    i,
                    j,
                    result,
                    ok ? "OK" : "FAIL");
                assert(ok);
            }
        }

        /* Sub() */

        (void)printf("\n");
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                result = fhe_decrypt_one_bit(/* leak */fhe_and_bits(bitValues[i],
                    bitValues[j]));
                retval |= !(ok = (result == (i & j)));
                (void)printf("%d × %d = %d %s\n",
                    i,
                    j,
                    result,
                    ok ? "OK" : "FAIL");
                assert(ok);
            }
        }
        break;

    } while (0);

    DESTROY_MP_INT(bitValue0);
    DESTROY_MP_INT(bitValue1);

    /* Encrypt and decrypt an integer */

    do {
        fhe_integer integer = 0x123456789abcdef0LL >> (64 - FHE_INTEGER_BIT_WIDTH);
        fhe_integer result = fhe_decrypt_integer(fhe_encrypt_integer(integer));

        retval |= !(ok = (result == integer));

        (void)printf("\nEncrypt-decrypt integer: 0x%x = 0x%x %s\n",
            integer,
            result,
            ok ? "OK" : "FAIL");
        assert(ok);
    } while (0);


    /*
    * Integral arithmetics
    */

    /* Addition: hard-coded numbers */

    (void)printf("\n");
    for (int i = 0; i < 16; i++) {
        fhe_integer result, addend = FHE_MASK(0x1111111111111111LL * i);


        result = fhe_decrypt_integer(
            tmp1 = fhe_add_integers(
            fhe_encrypt_integer(addend),
            fhe_encrypt_integer(addend)
            )
            );
        DESTROY_ENCRYPTED_INTEGER(tmp1);
        retval |= !(ok = (result == (fhe_integer)FHE_MASK(addend + addend)));

        (void)printf("0x%1$0*5$x + 0x%2$0*5$x = 0x%3$0*5$x %4$s\n",
            addend,
            addend,
            result,
            ok ? "OK" : "FAIL",
            FHE_INTEGER_BIT_WIDTH / 4
            + (FHE_INTEGER_BIT_WIDTH % 4 ? 1 : 0));
        assert(ok);
    }

    /* Addition: random numbers  */

    (void)printf("\n");
    for (int i = 0; i < 16; i++) {
        fhe_integer result, addend1, addend2;
        addend1 = fhe_random(FHE_INTEGER_BIT_WIDTH);
        addend2 = fhe_random(FHE_INTEGER_BIT_WIDTH);


        result = fhe_decrypt_integer(
            tmp1 = fhe_add_integers(
            fhe_encrypt_integer(addend1),
            fhe_encrypt_integer(addend2)
            )
            );
        DESTROY_ENCRYPTED_INTEGER(tmp1);
        retval |= !(ok = (result == (fhe_integer)FHE_MASK(addend1 + addend2)));

        (void)printf("0x%1$0*5$x + 0x%2$0*5$x = 0x%3$0*5$x %4$s\n",
            addend1,
            addend2,
            result,
            ok ? "OK" : "FAIL",
            FHE_INTEGER_BIT_WIDTH / 4
            + (FHE_INTEGER_BIT_WIDTH % 4 ? 1 : 0));
        assert(ok);
    }


    /* Multiplication: hard-coded numbers */


    (void)printf("\nUsing %d-bit precision for multiplication\n\n",
        FHE_INTEGER_BIT_WIDTH_MULTIPLY);

    unsigned long int mask = (0x1 << FHE_INTEGER_BIT_WIDTH_MULTIPLY) - 1;

    for (int i = 0; i < 16; i++) {
        fhe_integer result, factor = mask / 0xf * i;

        result = fhe_decrypt_integer(
            tmp1 = fhe_multiply_integers(
            fhe_encrypt_integer(factor),
            fhe_encrypt_integer(factor)
            )
            );
        DESTROY_ENCRYPTED_INTEGER(tmp1);
        retval |= !(ok = (result == (fhe_integer)((factor * factor) & mask)));

        (void)printf("0x%1$0*5$x × 0x%2$0*5$x = 0x%3$0*5$x %4$s\n",
            factor,
            factor,
            result,
            ok ? "OK" : "FAIL",
            FHE_INTEGER_BIT_WIDTH_MULTIPLY / 4
            + (FHE_INTEGER_BIT_WIDTH_MULTIPLY % 4 ? 1 : 0));
        assert(ok);
    }

    /* Multiplication: random numbers  */

    (void)printf("\n");
    for (int i = 0; i < 16; i++) {
        fhe_integer result, factor1, factor2;
        factor1 = fhe_random(FHE_INTEGER_BIT_WIDTH_MULTIPLY / 2);
        factor2 = fhe_random(FHE_INTEGER_BIT_WIDTH_MULTIPLY / 2);


        result = fhe_decrypt_integer(
            tmp1 = fhe_multiply_integers(
            fhe_encrypt_integer(factor1),
            fhe_encrypt_integer(factor2)
            )
            );
        DESTROY_ENCRYPTED_INTEGER(tmp1);
        retval |= !(ok = (result == (fhe_integer)((factor1 * factor2) &
            mask)));

        (void)printf("%1$ *6$d × %2$ *6$d = %3$ *6$d"
            "    0x%1$0*5$x × 0x%2$0*5$x = 0x%3$0*5$x %4$s\n",
            factor1,
            factor2,
            result,
            ok ? "OK" : "FAIL",
            FHE_INTEGER_BIT_WIDTH_MULTIPLY / 4
            + (FHE_INTEGER_BIT_WIDTH_MULTIPLY % 4 ? 1 : 0),
            FHE_INTEGER_BIT_WIDTH_MULTIPLY / 2
            + (FHE_INTEGER_BIT_WIDTH_MULTIPLY % 2 ? 1 : 0));
        assert(ok);
    }


    // Will only ever return 0 because of the assert()s above
    return retval;
}