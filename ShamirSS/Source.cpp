#include <stdio.h>
#include <array>
#include <vector>
#include <math.h>

// Basically ported from https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing

typedef std::array<int, 2> TShare;
typedef std::vector<TShare> TShares;

// Split number into the shares
TShares split(int number, int available, int needed, int prime) {
    std::vector<int> coef = { number, 166, 94 };
    int x = 0;
    int exp = 0;
    int c = 0;
    int accum = 0;
    TShares shares;
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
    shares.resize(available);
    for (x = 1; x <= available; x++) {
        // coef = [1234, 166, 94] which is 1234x^0 + 166x^1 + 94x^2
        for (exp = 1, accum = coef[0]; exp < needed; exp++)
            accum = (accum + (coef[exp] * (int(pow(x, exp)) % prime) % prime)) % prime;
        // Store values as (1, 132), (2, 66), (3, 188), (4, 241), (5, 225) (6, 140)
        shares[x - 1] = { x, accum };
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

// Join the shares into a number 
int join(const TShares& shares, int prime) {

    int accum, count, formula, startposition, nextposition, value, numerator, denominator;
    for(formula = accum = 0; formula < int(shares.size()); formula++) {
        /* Multiply the numerator across the top and denominators across the bottom to do Lagrange's interpolation
         * Result is x0(2), x1(4), x2(5) -> -4*-5 and (2-4=-2)(2-5=-3), etc for l0, l1, l2...
         */
        for(count = 0, numerator = denominator = 1; count < int(shares.size()); count++) {
            if(formula == count) continue; // If not the same value
            startposition = shares[formula][0];
            nextposition = shares[count][0];
            numerator = (numerator * -nextposition) % prime;
            denominator = (denominator * (startposition - nextposition)) % prime;
        }
        value = shares[formula][1];
        accum = (prime + accum + (value * numerator * modInverse(denominator, prime))) % prime;
    }
    return accum;
}


void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

int main (int argc, char **argv)
{
    const int c_prime = 257;

    TShares sh = split(129, 6, 3, c_prime); // split the secret value 129 into 6 components - at least 3 of which will be needed to figure out the secret value 
    TShares newshares = { sh[1], sh[3], sh[4] }; // pick any selection of 3 shared keys from sh 

    int answer = join(newshares, c_prime);
    printf("%i", answer);

    return 0;
}

/*

TODO:
* finish porting this
* make the code good and flexible
* use a typedef instead of int, so it can be changed out easily?
* make constants for sizes of things, and make them be template params or something, for optimal speed?
* make it pick random numbers using good c++ random number generation
* should the prime be chosen randomly?

https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing
*/