#include <stdio.h>
#include <vector>
#include <complex>
#include <assert.h>

typedef std::complex<float> TComplex;
typedef public std::vector<TComplex> TComplexVector;
typedef public std::vector<TComplex> TComplexMatrix;

const float c_pi = 3.14159265359f;

//=================================================================================
static const TComplexVector c_qubit0 = { 1.0f, 0.0f };  // false aka |0>
static const TComplexVector c_qubit1 = { 0.0f, 1.0f };  // true aka |1>
static const TComplexVector c_qubit01_0deg = { 1.0f / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f) }; // 50% true. 0 degree phase
static const TComplexVector c_qubit01_180deg = { 1.0f / std::sqrt(2.0f), -1.0f / std::sqrt(2.0f) }; // 50% true. 180 degree phase
static const TComplexVector c_qubit01_25p = { std::sqrt(3.0f) / 2.0f, 1.0f / 2.0f }; // 25% true. 0 degree phase

// Given the states |00>, |01>, |10>, |11>, swaps the |01> and |10> state
// If swapping the probabilities of two qubits, it won't affect the probabilities
// of them both being on or off since those add together.  It will swap the odds of
// only one of them being on.
static const TComplexMatrix c_swapGate =
{
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }
};

// does a half way c_swapGate
// TODO: is this averaging or what?
static const TComplexMatrix c_sqrtSwapGate =
{
    {
        1.0f,  0.0f        ,         0.0f , 0.0f,
        0.0f, (0.5f,  0.5f), (0.5f,  0.5f), 0.0f,
        0.0f, (0.5f, -0.5f), (0.5f, -0.5f), 0.0f,
        0.0f,  0.0f        ,         0.0f , 1.0f
    }
};

// Controlled not gate
// If the first qubit is true, flips the value of the second qubit
static const TComplexMatrix c_controlledNotGate =
{
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    }
};

// TODO: Toffoli
// TODO: Fredkin

//=================================================================================
TComplexMatrix MakeControlledGate (const TComplexMatrix& matrix)
{
    assert(matrix.size() == 4);

    TComplexMatrix ret;
    ret.resize(16);
    std::fill(ret.begin(), ret.end(), 0.0f);
    ret[0] = 1.0f;
    ret[5] = 1.0f;
    ret[10] = matrix[0];
    ret[11] = matrix[1];
    ret[14] = matrix[2];
    ret[15] = matrix[3];

    return ret;
}

//=================================================================================
void WaitForEnter ()
{
    printf("\nPress Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
int main (int argc, char **argv) {

    TComplexMatrix b;
    auto a = MakeControlledGate(b);

    WaitForEnter();

    return 0;
}

/*

TODO:
* make a generic controlled gate. 4x4 matrix.  lower right 2x2 is the matrix operation to be part of the contolled circuit.
? what does the sqrtSwap gate do in practice? does it average the odds or what?
* get the common multi qubit gates working
* maybe some circuits involving singular qubits too?
* do outer product (tensor product)
* do kronecker product
? combine matrices into a single matrix for a final result? or do it one by one? maybe show that doing it each way is equivelant?

*/