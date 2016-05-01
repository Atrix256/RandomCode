#include <stdio.h>
#include <array>
#include <vector>
#include <math.h>
#include <random>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

typedef int64_t TINT;
typedef std::array<TINT, 2> TShare;
typedef std::vector<TShare> TShares;

class CShamirSecretSharing
{
public:
    CShamirSecretSharing(size_t sharesNeeded, TINT prime)
        : c_sharesNeeded(sharesNeeded), c_prime(prime)
    {
        // There needs to be at least 1 share needed
        assert(sharesNeeded > 0);
    }

    // Generate N shares for a secretNumber
    TShares GenerateShares(TINT secretNumber, TINT numShares) const
    {
        // calculate our curve coefficients
        std::vector<TINT> coefficients;
        {
            // store the secret number as the first coefficient;
            coefficients.resize((size_t)c_sharesNeeded);
            coefficients[0] = secretNumber;

            // randomize the rest of the coefficients
            std::array<int, std::mt19937::state_size> seed_data;
            std::random_device r;
            std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
            std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
            std::mt19937 gen(seq);
            std::uniform_int_distribution<TINT> dis(1, c_prime - 1);
            for (TINT i = 1; i < c_sharesNeeded; ++i)
                coefficients[(size_t)i] = dis(gen);
        }

        // generate the shares
        TShares shares;
        shares.resize((size_t)numShares);
        for (size_t i = 0; i < numShares; ++i)
            shares[i] = GenerateShare(i + 1, coefficients);
        return shares;
    }

    // use lagrange polynomials to find f(0) of the curve, which is the secret number
    TINT JoinShares(const TShares& shares) const
    {
        // make sure there is at elast the minimum number of shares
        assert(shares.size() >= size_t(c_sharesNeeded));

        // Sigma summation loop
        TINT sum = 0;
        for (TINT j = 0; j < c_sharesNeeded; ++j)
        {
            TINT y_j = shares[(size_t)j][1];

            TINT numerator = 1;
            TINT denominator = 1;

            // Pi product loop
            for (TINT m = 0; m < c_sharesNeeded; ++m)
            {
                if (m == j)
                    continue;

                numerator = (numerator * shares[(size_t)m][0]) % c_prime;
                denominator = (denominator * (shares[(size_t)m][0] - shares[(size_t)j][0])) % c_prime;
            }

            sum = (c_prime + sum + y_j * numerator * modInverse(denominator, c_prime)) % c_prime;
        }
        return sum;
    }

    const TINT GetPrime() const { return c_prime; }
    const TINT GetSharesNeeded() const { return c_sharesNeeded; }

private:

    // Generate a single share in the form of (x, f(x))
    TShare GenerateShare(TINT x, const std::vector<TINT>& coefficients) const
    {
        TINT xpow = x;
        TINT y = coefficients[0];
        for (TINT i = 1; i < c_sharesNeeded; ++i) {
            y += coefficients[(size_t)i] * xpow;
            xpow *= x;
        }
        return{ x, y % c_prime };
    }

    // Gives the decomposition of the gcd of a and b.  Returns [x,y,z] such that x = gcd(a,b) and y*a + z*b = x
    static const std::array<TINT, 3> gcdD(TINT a, TINT b) {
        if (b == 0)
            return{ a, 1, 0 };

        const TINT n = a / b;
        const TINT c = a % b;
        const std::array<TINT, 3> r = gcdD(b, c);

        return{ r[0], r[2], r[1] - r[2] * n };
    }

    // Gives the multiplicative inverse of k mod prime.  In other words (k * modInverse(k)) % prime = 1 for all prime > k >= 1 
    static TINT modInverse(TINT k, TINT prime) {
        k = k % prime;
        TINT r = (k < 0) ? -gcdD(prime, -k)[2] : gcdD(prime, k)[2];
        return (prime + r) % prime;
    }

private:

    // Publically known information
    const TINT          c_prime;
    const TINT          c_sharesNeeded;
};

void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

int main(int argc, char **argv)
{
    // Parameters
    const TINT c_secretNumber = 435;
    const TINT c_sharesNeeded = 7;
    const TINT c_sharesMade = 50;
    const TINT c_prime = 439;   // must be a prime number larger than the other three numbers above

    // set up a secret sharing object with the public information
    CShamirSecretSharing secretSharer(c_sharesNeeded, c_prime);

    // split a secret value into multiple shares
    TShares shares = secretSharer.GenerateShares(c_secretNumber, c_sharesMade);

    // shuffle the shares, so it's random which ones are used to join
    std::array<int, std::mt19937::state_size> seed_data;
    std::random_device r;
    std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 gen(seq);
    std::shuffle(shares.begin(), shares.end(), gen);

    // join the shares
    TINT joinedSecret = secretSharer.JoinShares(shares);

    // show the public information and the secrets being joined
    printf("%" PRId64 " shares needed, %i shares made\n", secretSharer.GetSharesNeeded(), shares.size());
    printf("Prime = %" PRId64 "\n\n", secretSharer.GetPrime());
    for (TINT i = 0, c = secretSharer.GetSharesNeeded(); i < c; ++i)
        printf("Share %" PRId64 " = (%" PRId64 ", %" PRId64 ")\n", i + 1, shares[i][0], shares[i][1]);

    // show the result
    printf("\nJoined Secret = %" PRId64 "\nActual Secret = %" PRId64 "\n\n", joinedSecret, c_secretNumber);
    assert(joinedSecret == c_secretNumber);
    WaitForEnter();
    return 0;
}

/*

TODO:
* do a couple diff sized runs on the blog post
* Note on blog that its breaks down if there is integer overflow, so large integer advised (at what levels does it break down?)
 * using a modulus of 257, 10 shares is about the limit of what i'm able to do with an int64, with 50 shares total generated.

https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing
http://stackoverflow.com/questions/19327651/java-implementation-of-shamirs-secret-sharing
*/