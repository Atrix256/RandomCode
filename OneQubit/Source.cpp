#include <stdio.h>
#include <array>
#include <complex>

typedef std::array<std::complex<float>, 2> TQubit;
typedef std::array<std::complex<float>, 4> TComplexMatrix;
const float c_pi = 3.14159265359f;

//=================================================================================
static const TQubit c_qubit0 = { 1.0f, 0.0f };  // false aka |0>
static const TQubit c_qubit1 = { 0.0f, 1.0f };  // true aka |1>
static const TQubit c_qubit01_0deg = { 1.0f / std::sqrt(2.0f), 1.0f / std::sqrt(2.0f) }; // 50% true. 0 degree phase
static const TQubit c_qubit01_180deg = { 1.0f / std::sqrt(2.0f), -1.0f / std::sqrt(2.0f) }; // 50% true. 180 degree phase

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
int ProbabilityOfBeingTrue (const TQubit& qubit)
{
    float prob = std::round((qubit[1] * qubit[1]).real() * 100.0f);
    return int(prob);
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
void Print (const TQubit& qubit)
{
    printf("[(%0.2f, %0.2fi), (%0.2f, %0.2fi)] %i%% true",
        qubit[0].real(), qubit[0].imag(),
        qubit[1].real(), qubit[1].imag(),
        ProbabilityOfBeingTrue(qubit));
}

//=================================================================================
int main (int argc, char **argv)
{
    // Not Gate
    {
        printf("Not gate:\n  ");

        // Qubit: false
        TQubit v = c_qubit0;
        Print(v);
        printf("\n  ! = ");
        v = ApplyGate(v, c_notGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: true
        v = c_qubit1;
        Print(v);
        printf("\n  ! = ");
        v = ApplyGate(v, c_notGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: 50% chance, reverse phase
        v = c_qubit01_180deg;
        Print(v);
        printf("\n  ! = ");
        v = ApplyGate(v, c_notGate);
        Print(v);
        printf("\n\n");
    }

    // Pauli-y gate
    {
        printf("Pauli-y gate:\n  ");

        // Qubit: false
        TQubit v = c_qubit0;
        Print(v);
        printf("\n  Y = ");
        v = ApplyGate(v, c_pauliYGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: true
        v = c_qubit1;
        Print(v);
        printf("\n  Y = ");
        v = ApplyGate(v, c_pauliYGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: 50% chance, reverse phase
        v = c_qubit01_180deg;
        Print(v);
        printf("\n  Y = ");
        v = ApplyGate(v, c_pauliYGate);
        Print(v);
        printf("\n\n");
    }

    // Pauli-z gate
    {
        printf("Pauli-z gate:\n  ");

        // Qubit: false
        TQubit v = c_qubit0;
        Print(v);
        printf("\n  Z = ");
        v = ApplyGate(v, c_pauliZGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: true
        v = c_qubit1;
        Print(v);
        printf("\n  Z = ");
        v = ApplyGate(v, c_pauliZGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: 50% chance, reverse phase
        v = c_qubit01_180deg;
        Print(v);
        printf("\n  Z = ");
        v = ApplyGate(v, c_pauliZGate);
        Print(v);
        printf("\n\n");
    }

    // 45 degree phase adjustment gate
    {
        printf("45 degree phase gate:\n  ");
        TComplexMatrix gate = MakePhaseAdjustmentGate(c_pi / 4.0f);

        // Qubit: false
        TQubit v = c_qubit0;
        Print(v);
        printf("\n  M = ");
        v = ApplyGate(v, gate);
        Print(v);
        printf("\n\n  ");

        // Qubit: true
        v = c_qubit1;
        Print(v);
        printf("\n  M = ");
        v = ApplyGate(v, gate);
        Print(v);
        printf("\n\n  ");

        // Qubit: 50% chance, reverse phase
        v = c_qubit01_180deg;
        Print(v);
        printf("\n  M = ");
        v = ApplyGate(v, gate);
        Print(v);
        printf("\n\n");
    }

    // Hadamard gate
    {
        printf("Hadamard gate round trip:\n  ");

        // Qubit: false
        TQubit v = c_qubit0;
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: true
        v = c_qubit1;
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: 50% chance, reverse phase
        v = c_qubit01_180deg;
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n\n");
    }

    // 1 bit circuit
    // Hadamard -> Pauli-Z -> Hadamard
    {
        printf("Circuit Hadamard->Pauli-Z->Hadamard:\n  ");

        // Qubit: false
        TQubit v = c_qubit0;
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n  Z = ");
        v = ApplyGate(v, c_pauliZGate);
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: true
        v = c_qubit1;
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n  Z = ");
        v = ApplyGate(v, c_pauliZGate);
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n\n  ");

        // Qubit: 50% chance, reverse phase
        v = c_qubit01_180deg;
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n  Z = ");
        v = ApplyGate(v, c_pauliZGate);
        Print(v);
        printf("\n  H = ");
        v = ApplyGate(v, c_hadamardGate);
        Print(v);
        printf("\n");
    }

    WaitForEnter();

    return 0;
}

/*

TODO:
* what is the correct way to get probability? squaring (0,i) comes up with -100% so isn't correct
* also, the phase adjustment gate seems to be adjusting probabilities, maybe related!
 * maybe it isn't normalized anymore after the gate or something?

*/