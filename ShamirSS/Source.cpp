#include <stdio.h>
#include <array>
#include <vector>
#include <math.h>

// Basically ported from https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing

// Split number into the shares
std::vector<int> split(int number, int available, int needed, int prime) {
    std::vector<int> coef = { number, 166, 94 };
    int x = 0;
    int exp = 0;
    int c = 0;
    int accum = 0;
    std::vector<int> shares;
    // TODO: make this generalized!

    /* Normally, we use the line:
    * for(c = 1, coef[0] = number; c < needed; c++) coef[c] = Math.floor(Math.random() * (prime  - 1));
    * where (prime - 1) is the maximum allowable value.
    * However, to follow this example, we hardcode the values:
    * coef = [number, 166, 94];
    * For production, replace the hardcoded value with the random loop
    * For each share that is requested to be available, run through the formula plugging the corresponding coefficient
    * The result is f(x), where x is the byte we are sharing (in the example, 1234)
    */
    for (x = 1; x <= available; x++) {
        /* coef = [1234, 166, 94] which is 1234x^0 + 166x^1 + 94x^2 */
        for (exp = 1, accum = coef[0]; exp < needed; exp++) accum = (accum + (coef[exp] * (int(pow(x, exp)) % prime) % prime)) % prime;
        /* Store values as (1, 132), (2, 66), (3, 188), (4, 241), (5, 225) (6, 140) */
        shares[x - 1] = [x, accum];
    }
    return shares;
}

// Gives the decomposition of the gcd of a and b.  Returns [x,y,z] such that x = gcd(a,b) and y*a + z*b = x
const std::array<int, 3> gcdD(int a, int b) {
    if (b == 0)
        return { a, 1, 0 };

    const size_t n = a / b;
    const size_t c = a % b;
    const std::array<int, 3> r = gcdD(b, c);

    return {r[0], r[2], r[1] - r[2] * n};
}

// Gives the multiplicative inverse of k mod prime.  In other words (k * modInverse(k)) % prime = 1 for all prime > k >= 1 
size_t modInverse(int k, int prime) {
    k = k % prime;
    int r = (k < 0) ? -gcdD(prime, -k)[2] : gcdD(prime, k)[2];
    return (prime + r) % prime;
}

void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

int main (int argc, char **argv)
{
    //auto r = gcdD(5, 10);
    const int c_prime = 257;

    std::vector<int> sh = split(129, 6, 3, c_prime); // split the secret value 129 into 6 components - at least 3 of which will be needed to figure out the secret value 
    std::vector<int> newshares = { sh[1], sh[3], sh[4] }; // pick any selection of 3 shared keys from sh 

    //int answer = join(newshares);
    //printf("%i", answer);

    return 0;
}

/*

TODO:
* finish porting this
* make the code good and flexible
* use a typedef instead of int, so it can be changed out easily?

https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing
*/