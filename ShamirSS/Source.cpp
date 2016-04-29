#include <stdio.h>
#include <array>
#include <vector>
#include <math.h>
#include <random>
#include <assert.h>

// This code is basically ported from https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing

typedef std::array<int, 2> TShare;
typedef std::vector<TShare> TShares;

class CShamirSecretSharing
{
public:
    CShamirSecretSharing (int secretNumber, size_t sharesNeeded, int prime)
        : c_sharesNeeded(sharesNeeded), c_prime(prime)
    {
        // There needs to be at least 1 share needed
        assert(sharesNeeded > 0);

        // store the secret number as the first coefficient
        m_coefficients.resize(c_sharesNeeded);
        m_coefficients[0] = secretNumber;

        // randomize the rest of the coefficients with numbers from 1 to 100
        std::array<int, std::mt19937::state_size> seed_data;
        std::random_device r;
        std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 gen(seq);
        std::uniform_int_distribution<int> dis(1, c_prime);  // TODO: mauybe should be between 1 and prime? or prime-1? see which it should be
        for (size_t i = 1; i < c_sharesNeeded; ++i)
            m_coefficients[i] = dis(gen);
    }

    // Generate N shares
    TShares GenerateShares (size_t numShares) const
    {
        TShares shares;
        shares.resize(numShares);
        for (size_t i = 0; i < numShares; ++i)
            shares[i] = GenerateShare(i+1);
        return shares;
    }

    int JoinShares (const TShares& shares)
    {
        // make sure there is at elast the minimum number of shares
        assert(shares.size() >= c_sharesNeeded);

        int accum, count, formula, startposition, nextposition, value, numerator, denominator;
        for (formula = accum = 0; formula < int(c_sharesNeeded); formula++) {
            /* Multiply the numerator across the top and denominators across the bottom to do Lagrange's interpolation
            * Result is x0(2), x1(4), x2(5) -> -4*-5 and (2-4=-2)(2-5=-3), etc for l0, l1, l2...
            */
            for (count = 0, numerator = denominator = 1; count < int(c_sharesNeeded); count++) {
                if (formula == count) continue; // If not the same value
                startposition = shares[formula][0];
                nextposition = shares[count][0];
                numerator = (numerator * -nextposition) % c_prime;
                denominator = (denominator * (startposition - nextposition)) % c_prime;
            }
            value = shares[formula][1];
            accum = (c_prime + accum + (value * numerator * modInverse(denominator, c_prime))) % c_prime;
        }
        
        return accum;
    }

private:

    // Generate a single share in the form of (x, f(x))
    TShare GenerateShare (int x) const
    {
        int xpow = x;
        int y = m_coefficients[0];
        for (size_t i = 1; i < c_sharesNeeded; ++i) {
            y += m_coefficients[i] * xpow;
            xpow *= xpow;
        }
        return{ x, y % c_prime };
    }

    // Gives the decomposition of the gcd of a and b.  Returns [x,y,z] such that x = gcd(a,b) and y*a + z*b = x
    const std::array<int, 3> gcdD (int a, int b) {
        if (b == 0)
            return{ a, 1, 0 };

        const size_t n = a / b;
        const size_t c = a % b;
        const std::array<int, 3> r = gcdD(b, c);

        return{ r[0], r[2], r[1] - r[2] * n };
    }

    // Gives the multiplicative inverse of k mod prime.  In other words (k * modInverse(k)) % prime = 1 for all prime > k >= 1 
    size_t modInverse (int k, int prime) {
        k = k % prime;
        int r = (k < 0) ? -gcdD(prime, -k)[2] : gcdD(prime, k)[2];
        return (prime + r) % prime;
    }

private:

    const int           c_prime;
    const size_t        c_sharesNeeded;

    std::vector<int>    m_coefficients;
};

void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

int main (int argc, char **argv)
{

    CShamirSecretSharing secretSharer(129, 3, 257);
    TShares shares = secretSharer.GenerateShares(6);
    TShares newShares = { shares[1], shares[3], shares[4] }; // pick any selection of 3 shared keys from sh 
    int an = secretSharer.JoinShares(newShares);
    printf("%i\r\n\r\n", an);

    // TODO: print public info? shares needed and prime
    // TODO: maybe have constructor make shares so that they aren't stored anywhere? probably still useful to have a class i guess though.
    // TODO: shuffle shares and join them
    // TODO: why doesnt the above work with 4 shares required?  I think the join function is wrong
    // TOOD: doesn't work with linear (2 shares / points) either!
    // TODO: test a lot

    WaitForEnter();
    return 0;
}

/*

TODO:
* make std::vector<TShare> into tshares?
* finish porting this
* make the code good and flexible
* use a typedef instead of int, so it can be changed out easily?
* make constants for sizes of things, and make them be template params or something, for optimal speed?
* make it pick random numbers using good c++ random number generation
* should the prime be chosen randomly?
? maybe make a templated class to interface with this stuff?
* should we put horizontal separations to make code easier to read?
* do a couple diff sized runs on the blog

https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing
*/