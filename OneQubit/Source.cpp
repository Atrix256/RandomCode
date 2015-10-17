#include <stdio.h>
#include <array>
#include <complex>

typedef std::array<std::complex<float>, 2> TQubit;
typedef std::array<std::complex<float>, 4> TComplexMatrix;

//=================================================================================
static const TQubit c_qubit0 = { 1.0f, 0.0f };  // a qubit storing false aka |0>
static const TQubit c_qubit1 = { 0.0f, 1.0f };  // a qubit storing true aka |1>

// A not gate AKA Pauli-X gate
// Flips false and true probabilities (amplitudes)
// Maps |0> to |1>, and |1> to |0>
// Rotates PI radians around the x axis of the Bloch Sphere
static const TComplexMatrix c_notGate =
{
    {
        0.0f, 1.0f,
        1.0f, 0.0f
    }
};
static const TComplexMatrix c_pauliXGate = c_notGate;

// Pauli-Y gate
// Maps |0> to i|1>, and |1> to -i|0>
// Rotates PI radians around the y axis of the Bloch Sphere
static const TComplexMatrix c_pauliYGate =
{
    {
        { 0.0f, 0.0f }, { 0.0f, -1.0f },
        { 0.0f, 1.0f }, { 0.0f, 0.0f }
    }
};

// Pauli-Z gate
// Negates the phase of the |1> state
// Rotates PI radians around the z axis of the Bloch Sphere
static const TComplexMatrix c_pauliZGate =
{
    {
        1.0f, 0.0f,
        0.0f, -1.0f
    }
};

// Hadamard gate
// Takes a pure |0> or |1> state and makes a 50/50 superposition between |0> and |1>.
// Put a 50/50 superposition through and get the pure |0> or |1> back.
// Encodes the origional value in the phase information as either matching or
// mismatching phase.
static const TComplexMatrix c_hadamardGate =
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
TQubit ApplyGate (const TQubit& qubit, const TComplexMatrix& gate)
{
    // multiply qubit amplitude vector by unitary gate matrix
    return 
    {
        qubit[0] * gate[0] + qubit[1] * gate[1],
        qubit[0] * gate[2] + qubit[1] * gate[3]
    };
}

//=================================================================================
float ProbabilityOfBeingTrue (const TQubit& qubit)
{
    return (qubit[1] * qubit[1]).real();
}

//=================================================================================
TComplexMatrix MakePhaseAdjustmentGate (float radians)
{
    // This makes a gate like this:
    //
    // [ 1  0             ]
    // [ 0  e^(i*radians) ]
    //
    // The gate will adjust the phase of the |1> state by the specified amount.
    // A more general version of the pauli-z gate

    return
    {
        {
            1.0f, 0.0f,
            0.0f, std::exp(std::complex<float>(0.0f,1.0f) * radians)
        }
    };
}

//=================================================================================
int main (int argc, char **argv)
{
    TQubit b = c_qubit1;

    float prob1 = ProbabilityOfBeingTrue(b);

    b = ApplyGate(b, c_hadamardGate);

    float prob2 = ProbabilityOfBeingTrue(b);

    TComplexMatrix m = MakePhaseAdjustmentGate(3.14159265359f);

    return 0;
}

/*

TODO:
* do all the gates
* show the input and outputs
* do a multi step gate -> like H->Not->H maybe

*/