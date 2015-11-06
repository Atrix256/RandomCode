#include <stdio.h>
#include <vector>
#include <complex>
#include <assert.h>

typedef std::complex<float> TComplex;

//=================================================================================
struct SComplexVector
{
public:
    TComplex& Get (size_t i) { return v[i]; }

    const TComplex& Get (size_t i) const { return v[i]; }

    size_t Size() const
    {
        return v.size();
    }

    void Resize (size_t s)
    {
        v.resize(s);
    }

    void Print () const
    {
        printf("[");
        for (size_t i = 0, c = v.size(); i < c; ++i)
        {
            if (i > 0)
                printf(", ");
            const TComplex& val = Get(i);
            if (val.imag() == 0.0f)
            {
                if (val.real() == 0.0f)
                    printf("0");
                else if (val.real() == 1.0f)
                    printf("1");
                else
                    printf("%0.2f", val.real());
            }
            else
                printf("%0.2f + %0.2fi", val.real(), val.imag());
        }
        printf("]\n");
    }

    std::vector<TComplex> v;
};

//=================================================================================
struct SComplexMatrix
{
public:
    TComplex& Get (size_t x, size_t y)
    {
        return v[y*Size() + x];
    }

    const TComplex& Get (size_t x, size_t y) const
    {
        return v[y*Size() + x];
    }

    // For an MxM matrix, this returns M
    size_t Size () const
    {
        size_t ret = (size_t)sqrt(v.size());
        assert(ret*ret == v.size());
        return ret;
    }

    // For an MxM matrix, this sets M
    void Resize(size_t s)
    {
        v.resize(s*s);
    }

    void Print() const
    {
        const size_t size = Size();

        for (size_t y = 0; y < size; ++y)
        {
            printf("[");
            for (size_t x = 0; x < size; ++x)
            {
                if (x > 0)
                    printf(", ");
                
                const TComplex& val = Get(x, y);
                if (val.imag() == 0.0f)
                {
                    if (val.real() == 0.0f)
                        printf("0");
                    else if (val.real() == 1.0f)
                        printf("1");
                    else
                        printf("%0.2f", val.real());
                }
                else
                    printf("%0.2f + %0.2fi", val.real(), val.imag());
            }
            printf("]\n");
        }
    }

    std::vector<TComplex> v;
};

//=================================================================================
static const SComplexVector c_qubit0 = { { 1.0f, 0.0f } };  // false aka |0>
static const SComplexVector c_qubit1 = { { 0.0f, 1.0f } };  // true aka |1>

// 2x2 identity matrix
static const SComplexMatrix c_identity2x2 =
{
    {
        1.0f, 0.0f,
        0.0f, 1.0f,
    }
};

// Given the states |00>, |01>, |10>, |11>, swaps the |01> and |10> state
// If swapping the probabilities of two qubits, it won't affect the probabilities
// of them both being on or off since those add together.  It will swap the odds of
// only one of them being on.
static const SComplexMatrix c_swapGate =
{
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }
};

// Controlled not gate
// If the first qubit is true, flips the value of the second qubit
static const SComplexMatrix c_controlledNotGate =
{
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    }
};

// Hadamard gate
// Takes a pure |0> or |1> state and makes a 50/50 superposition between |0> and |1>.
// Put a 50/50 superposition through and get the pure |0> or |1> back.
// Encodes the origional value in the phase information as either matching or
// mismatching phase.
static const SComplexMatrix c_hadamardGate =
{
    {
        1.0f / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f),
        1.0f / std::sqrt(2.0f), 1.0f / -std::sqrt(2.0f)
    }
};

//=================================================================================
void WaitForEnter ()
{
    printf("\nPress Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
SComplexVector KroneckerProduct (const SComplexVector& a, const SComplexVector& b)
{
    const size_t aSize = a.Size();
    const size_t bSize = b.Size();

    SComplexVector ret;
    ret.Resize(aSize*bSize);

    for (size_t i = 0, ic = aSize; i < ic; ++i)
    {
        for (size_t j = 0, jc = bSize; j < jc; ++j)
        {
            size_t n = i * bSize + j;
            ret.Get(n) = a.Get(i)*b.Get(j);
        }
    }
    return ret;
}

//=================================================================================
SComplexMatrix KroneckerProduct (const SComplexMatrix& a, const SComplexMatrix& b)
{
    const size_t aSize = a.Size();
    const size_t bSize = b.Size();

    SComplexMatrix ret;
    ret.Resize(aSize*bSize);

    for (size_t ax = 0; ax < aSize; ++ax)
    {
        for (size_t ay = 0; ay < aSize; ++ay)
        {
            const TComplex& aValue = a.Get(ax, ay);

            for (size_t bx = 0; bx < bSize; ++bx)
            {
                for (size_t by = 0; by < bSize; ++by)
                {
                    const TComplex& bValue = b.Get(bx, by);

                    size_t nx = ax*bSize + bx;
                    size_t ny = ay*bSize + by;

                    ret.Get(nx,ny) = aValue * bValue;
                }
            }
        }
    }

    return ret;
}

//=================================================================================
SComplexMatrix operator* (const SComplexMatrix& a, const SComplexMatrix& b)
{
    assert(a.Size() == b.Size());
    const size_t size = a.Size();

    SComplexMatrix ret;
    ret.Resize(size);

    for (size_t nx = 0; nx < size; ++nx)
    {
        for (size_t ny = 0; ny < size; ++ny)
        {
            TComplex& val = ret.Get(nx, ny);
            val = 0.0f;
            for (size_t i = 0; i < size; ++i)
                val += a.Get(i, ny) * b.Get(nx, i);
        }
    }

    return ret;
}

//=================================================================================
SComplexVector operator* (const SComplexVector& a, const SComplexMatrix& b)
{
    assert(a.Size() == b.Size());
    const size_t size = a.Size();

    SComplexVector ret;
    ret.Resize(size);

    for (size_t i = 0; i < size; ++i)
    {
        TComplex& val = ret.Get(i);
        val = 0;
        for (size_t j = 0; j < size; ++j)
            val += a.Get(j) * b.Get(i, j);
    }

    return ret;
}

//=================================================================================
int main (int argc, char **argv) {

    // 2 qubit entanglement circuit demo
    {
        // make the circuit
        const SComplexMatrix H1 = KroneckerProduct(c_hadamardGate, c_identity2x2);
        const SComplexMatrix circuit = H1 * c_controlledNotGate;

        // display the circuit
        printf("Entanglement circuit:\n");
        circuit.Print();

        // permute the inputs and see what comes out when we pass them through the circuit!
        SComplexVector input = KroneckerProduct(c_qubit0, c_qubit0);
        SComplexVector output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(c_qubit0, c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(c_qubit1, c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(c_qubit1, c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();
    }

    // 4 qubit demo: flip the second qubit if the fourth qubit is true
    {
        // make the circuit
        const SComplexMatrix cnot4qubit = KroneckerProduct(KroneckerProduct(c_controlledNotGate, c_identity2x2), c_identity2x2);
        const SComplexMatrix swap12 = KroneckerProduct(KroneckerProduct(c_swapGate, c_identity2x2), c_identity2x2);
        const SComplexMatrix swap23 = KroneckerProduct(KroneckerProduct(c_identity2x2, c_swapGate), c_identity2x2);
        const SComplexMatrix swap34 = KroneckerProduct(KroneckerProduct(c_identity2x2, c_identity2x2), c_swapGate);
        const SComplexMatrix circuit = 
            swap34 *
            swap23 *
            swap12 *
            swap23 *
            cnot4qubit *
            swap23 *
            swap12 *
            swap23 *
            swap34;

        // display the circuit
        printf("\nFlip 2nd qubit if 4th qubit true circuit:\n");
        circuit.Print();

        // permute the inputs and see what comes out when we pass them through the circuit!
        SComplexVector input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit0), c_qubit0), c_qubit0);
        SComplexVector output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit0), c_qubit0), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit0), c_qubit1), c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit0), c_qubit1), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit1), c_qubit0), c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit1), c_qubit0), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit1), c_qubit1), c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit0, c_qubit1), c_qubit1), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit0), c_qubit0), c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit0), c_qubit0), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit0), c_qubit1), c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit0), c_qubit1), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit1), c_qubit0), c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit1), c_qubit0), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit1), c_qubit1), c_qubit0);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();

        input = KroneckerProduct(KroneckerProduct(KroneckerProduct(c_qubit1, c_qubit1), c_qubit1), c_qubit1);
        output = input * circuit;
        printf("\ninput:");
        input.Print();
        printf("output:");
        output.Print();
    }


    WaitForEnter();

    return 0;
}