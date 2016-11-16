#include <stdio.h>
#include <complex>
#include <vector>

#include <fcntl.h>
#include <io.h>
 
// set to 1 to have it show you the steps performed.  Set to 0 to hide the work.
// useful if checking work calculated by hand.
#define SHOW_WORK 1
 
#if SHOW_WORK
    #define PRINT_WORK(...) wprintf(__VA_ARGS__)
#else
    #define PRINT_WORK(...)
#endif

// Use UTF-16 encoding for Greek letters
static const wchar_t kPi = 0x03C0;
static const wchar_t kSigma = 0x03A3;
 
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
        PRINT_WORK(L"    n = %i : (%f, %f)\n", n, calc.real(), calc.imag());
        ret += calc;
    }
    ret /= TRealType(N);
    PRINT_WORK(L"    Sum the above and divide by %i\n", N);
    return ret;
}
 
//=================================================================================
std::vector<TComplexType> DFTSamples (const std::vector<TRealType>& samples)
{
    PRINT_WORK(L"DFT:  X_k = 1/N %cn[0,N) x_k * e^(-2%cikn/N)\n", kSigma, kPi);
 
    size_t N = samples.size();
    std::vector<TComplexType> ret;
    ret.resize(N);
    for (size_t k = 0; k < N; ++k)
    {
        PRINT_WORK(L"  k = %i\n", k);
        ret[k] = DFTSample(samples, k);
        PRINT_WORK(L"  X_%i = (%f, %f)\n", k, ret[k].real(), ret[k].imag());
    }
    PRINT_WORK(L"\n");
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
        PRINT_WORK(L"    n = %i : (%f, %f)\n", n, calc.real(), calc.imag());
        ret += calc;
    }
    PRINT_WORK(L"    Sum the above and take the real component\n");
    return ret.real();
}
 
//=================================================================================
std::vector<TRealType> IDFTSamples (const std::vector<TComplexType>& samples)
{
    PRINT_WORK(L"IDFT:  x_k = %cn[0,N) X_k * e^(2%cikn/N)\n", kSigma, kPi);
 
    size_t N = samples.size();
    std::vector<TRealType> ret;
    ret.resize(N);
    for (size_t k = 0; k < N; ++k)
    {
        PRINT_WORK(L"  k = %i\n", k);
        ret[k] = IDFTSample(samples, k);
        PRINT_WORK(L"  x_%i = %f\n", k, ret[k]);
    }
    PRINT_WORK(L"\n");
    return ret;
}
 
//=================================================================================
template<typename LAMBDA>
std::vector<TRealType> GenerateSamples (int numSamples, LAMBDA lambda)
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
	// Enable Unicode UTF-16 output to console
	_setmode(_fileno(stdout), _O_U16TEXT);

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
    wprintf(L"\nSource = [ ");
    for (TRealType v : sourceData)
        wprintf(L"%f ",v);
    wprintf(L"]\n\n");
 
    // Do a dft and show the results
    std::vector<TComplexType> dft = DFTSamples(sourceData);
    wprintf(L"dft = [ ");
    for (TComplexType v : dft)
        wprintf(L"(%f, %f) ", v.real(), v.imag());
    wprintf(L"]\n\n");
 
    // Do an inverse dft of the dft data, and show the results
    std::vector<TRealType> idft = IDFTSamples(dft);
    wprintf(L"idft = [ ");
    for (TRealType v : idft)
        wprintf(L"%f ", v);
    wprintf(L"]\n");
     
    return 0;
}