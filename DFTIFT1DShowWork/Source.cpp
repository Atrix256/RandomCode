#include <stdio.h>
#include <complex>
#include <vector>

// set to 1 to have it show you the steps performed.  Set to 0 to hide the work.
// useful if checking work calculated by hand.
#define SHOW_WORK 1

#if SHOW_WORK
    #define PRINT_WORK(...) printf(__VA_ARGS__)
#else
    #define PRINT_WORK(...)
#endif

#define CHAR_SIGMA 228
#define CHAR_PI 227

typedef float TRealType;
typedef std::complex<TRealType> TComplexType;

const TRealType c_pi = (TRealType)3.14159265359;
const TRealType c_twoPi = (TRealType)2.0 * c_pi;

//=================================================================================
TComplexType DFTSample (const std::vector<TRealType>& samples, int k)
{
    size_t N = samples.size();
    TComplexType ret;
    for (size_t n = 0; n < N; ++n)
    {
        TComplexType calc = TComplexType(samples[n], 0.0f) * std::polar<TRealType>(1.0f, -c_twoPi * TRealType(k) * TRealType(n) / TRealType(N));
        PRINT_WORK("    n = %i : (%f, %f)\n", n, calc.real(), calc.imag());
        ret += calc;
    }
    ret /= TRealType(N);
    PRINT_WORK("    Sum the above and divide by %i\n", N);
    return ret;
}

//=================================================================================
std::vector<TComplexType> DFTSamples (const std::vector<TRealType>& samples)
{
    PRINT_WORK("DFT:  X_k = 1/N %cn[0,N) x_k * e^(-2%cikn/N)\n", CHAR_SIGMA, CHAR_PI);

    size_t N = samples.size();
    std::vector<TComplexType> ret;
    ret.resize(N);
    for (size_t k = 0; k < N; ++k)
    {
        PRINT_WORK("  k = %i\n", k);
        ret[k] = DFTSample(samples, k);
        PRINT_WORK("  X_%i = (%f, %f)\n", k, ret[k].real(), ret[k].imag());
    }
    PRINT_WORK("\n");
    return ret;
}

//=================================================================================
TRealType IDFTSample (const std::vector<TComplexType>& samples, int k)
{
    size_t N = samples.size();
    TComplexType ret;
    for (size_t n = 0; n < N; ++n)
    {
        TComplexType calc = samples[n] * std::polar<TRealType>(1.0f, c_twoPi * TRealType(k) * TRealType(n) / TRealType(N));
        PRINT_WORK("    n = %i : (%f, %f)\n", n, calc.real(), calc.imag());
        ret += calc;
    }
    PRINT_WORK("    Sum the above and take the real component\n");
    return ret.real();
}

//=================================================================================
std::vector<TRealType> IDFTSamples (const std::vector<TComplexType>& samples)
{
    PRINT_WORK("IDFT:  x_k = %cn[0,N) X_k * e^(2%cikn/N)\n", CHAR_SIGMA, CHAR_PI);

    size_t N = samples.size();
    std::vector<TRealType> ret;
    ret.resize(N);
    for (size_t k = 0; k < N; ++k)
    {
        PRINT_WORK("  k = %i\n", k);
        ret[k] = IDFTSample(samples, k);
        PRINT_WORK("  x_%i = %f\n", k, ret[k]);
    }
    PRINT_WORK("\n");
    return ret;
}

//=================================================================================
template<typename LAMBDA>
std::vector<TRealType> GenerateSamples (int numSamples, const LAMBDA& lambda)
{
    std::vector<TRealType> ret;
    ret.resize(numSamples);
    for (int i = 0; i < numSamples; ++i)
    {
        TRealType percent = TRealType(i) / TRealType(numSamples);
        ret[i] = lambda(percent);
    }
    return ret;
}

//=================================================================================
int main (int argc, char **argv)
{
    // You can test specific data samples like this:
    //std::vector<TRealType> sourceData = { 1, 0, 1, 0 }; 
    //std::vector<TRealType> sourceData = { 1, -1, 1, -1 };

    // Or you can generate data samples from a function like this
    std::vector<TRealType> sourceData = GenerateSamples(
        4,
        [] (TRealType percent)
        {
            const TRealType c_frequency = TRealType(1.0);
            return cos(percent * c_twoPi * c_frequency);
        }
    );

    // Show the source data
    printf("\nSource = [ ");
    for (TRealType v : sourceData)
        printf("%f ",v);
    printf("]\n\n");

    // Do a dft and show the results
    std::vector<TComplexType> dft = DFTSamples(sourceData);
    printf("dft = [ ");
    for (TComplexType v : dft)
        printf("(%f, %f) ", v.real(), v.imag());
    printf("]\n\n");

    // Do an inverse dft of the dft data, and show the results
    std::vector<TRealType> idft = IDFTSamples(dft);
    printf("idft = [ ");
    for (TRealType v : idft)
        printf("%f ", v);
    printf("]\n");
    
    return 0;
}