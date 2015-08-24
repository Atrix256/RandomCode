/* Epsilon.h -- Fully homorphic encryption Gentry (2008) §3.2 */

/*
* Copyright © 2010 Jan Minář <rdancer@rdancer.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 (two),
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/* Only include this file once */
#ifndef FHE_H
#define FHE_H

#ifdef __native_client__
/* tommath.h results in some errors that go away when LTC_NO_PROTOTYPES is
* defined */
# define LTC_NO_PROTOTYPES
#endif /* __native_client__ */
#include <tommath.h>
//#include <tomcrypt.h>
#include <stdbool.h>
#include <stdint.h>

#define RANDOM_FILE "/dev/urandom" // XXX unsecure -- not random, but fast


/* TODO Do not hard-code bit width */
/*
* Multiplication consumes too much memory and processing power in 32 bits.
* Therefore it may be advantageous to set it separately.
*
* Note: Multiplication in 16 bits performs quickly with libgmp.  However, with
* libtommath, it is very slow.  15 bits are the minimum for production,
* otherwise we couldn't even compute single arbitrary 2-digit number
* multiplication: log_2(99 × 99) = 14; plus 1 bit for sign.
*/
typedef int32_t fhe_integer;
#ifndef FHE_INTEGER_BIT_WIDTH
# define FHE_INTEGER_BIT_WIDTH 11
#endif /* FHE_INTEGER_BIT_WIDTH */
#define FHE_INTEGER_BIT_WIDTH_MULTIPLY FHE_INTEGER_BIT_WIDTH
#define FHE_MASK(x) ((x) & (0xffffffff >> (32 - FHE_INTEGER_BIT_WIDTH)))


/*
* Function macros and constants
*/

//#define assert(x) do {} while(0)

#define bitsN /* λ  */ securityParameter
#define bitsP /* λ² */ (securityParameter * securityParameter)
#define bitsQ /* λ⁵ */ (securityParameter \
	* securityParameter \
	* securityParameter \
	* securityParameter \
	* securityParameter)


#define INIT_MP_INT(x) \
    mp_int *x; \
    do { \
	x = (mp_int *)checkMalloc(sizeof(void *)); \
	mp_init(x); \
        } while (0)

#define DESTROY_MP_INT(x) \
    do { \
	if (x != NULL) { \
	   mp_clear(x); \
    	} \
        } while (0)

#define DESTROY_ENCRYPTED_INTEGER(x) \
    do { \
	assert(x != NULL); \
	for (int i = 0; i < FHE_INTEGER_BIT_WIDTH; i++) { \
	    assert (x[i] != NULL); \
	    DESTROY_MP_INT(x[i]); \
    	} \
	free(x); \
        } while (0)


mp_int *fhe_new_random_integer(unsigned long long int numberOfBits);
mp_int *fhe_encrypt_one_bit(bool plainTextBit);
bool fhe_decrypt_one_bit(mp_int *encryptedBit);
void fhe_initialize(unsigned long int mySecurityParameter);
mp_int *fhe_xor_bits(mp_int *bit1, mp_int *bit2);
mp_int *fhe_and_bits(mp_int *bit1, mp_int *bit2);
mp_int **fhe_encrypt_integer(fhe_integer integer);
fhe_integer fhe_decrypt_integer(mp_int **encryptedInteger);
mp_int **fhe_add_integers(mp_int **integer1, mp_int **integer2);
mp_int **fhe_multiply_integers(mp_int **integer1, mp_int **integer2);
long int fhe_random(unsigned long long int numberOfBits);

#endif // FHE_H