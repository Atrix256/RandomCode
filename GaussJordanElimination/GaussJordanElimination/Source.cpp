#include <stdio.h>
#include <array>
#include <vector>

// define a vector as an array of floats
template<size_t N>
using TVector = std::array<float, N>;

// define a matrix as an array of vectors
template<size_t M, size_t N>
using TMatrix = std::array<TVector<N>, M>;

// helper function to fill out a matrix
template <size_t M, size_t N>
TMatrix<M, N> MakeMatrix (std::initializer_list<std::initializer_list<float>> matrixData)
{
    TMatrix<M, N> matrix;

    size_t m = 0;
    for (const std::initializer_list<float>& rowData : matrixData)
    {
        size_t n = 0;
        for (float value : rowData)
        {
            matrix[m][n] = value;
            ++n;
        }
        ++m;
    }
   
    return matrix;
}

// swap two rows in the matrix
template <size_t M, size_t N>
void SwapRows (TMatrix<M, N>& matrix, size_t a, size_t b)
{
    if (a == b)
        return;

    std::swap(matrix[a], matrix[b]);
}

// reduce the matrix for a single variable
template <size_t M, size_t N>
void ReduceVariable (TMatrix<M, N>& matrix, size_t varIndex)
{
    // find the row that has the largest value at position [varIndex] and swap it with row [varIndex]
    {
        size_t maxRow = varIndex;
        for (size_t rowIndex = varIndex + 1; rowIndex < M; ++rowIndex)
        {
            if (matrix[rowIndex][varIndex] > matrix[maxRow][varIndex])
                maxRow = rowIndex;
        }

        SwapRows(matrix, varIndex, maxRow);
    }

    // TODO: what if matrix[varIndex][varIndex] is 0? divide by 0!
    // TODO: make sure variables are named well.

    // for all rows before this row, we want to remove their reference to this variable
    {
        for (size_t rowIndex = varIndex + 1; rowIndex < M; ++rowIndex)
        {
            // TODO: explain this operation better!
            // TODO: name f better
            float f = matrix[rowIndex][varIndex] / matrix[varIndex][varIndex];

            // TODO: explain this better too!
            // for all remaining elements in the row
            for (size_t colIndex = varIndex + 1; colIndex < N; ++colIndex)
                matrix[rowIndex][colIndex] -= matrix[varIndex][colIndex] * f;

            // TODO: explain this as well
            matrix[rowIndex][varIndex] = 0.0f;
        }
    }
}

// make matrix into reduced row echelon form
template <size_t M, size_t N>
void GaussJordanElimination (TMatrix<M, N>& matrix)
{
    // do reduction for each variable in the set of equations
    for (size_t varIndex = 0; varIndex < std::min(M,N); ++varIndex)
        ReduceVariable(matrix, varIndex);

    // TODO: now do the forward pass thing.  THe jordan elimination, to make it be in rref form!
}

int main (int argc, char **argv)
{
    // the input matrix
    auto inputMatrix = MakeMatrix<3, 4>(
    {
        { 0.0f, 0.5f, 0.5f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    });

    GaussJordanElimination(inputMatrix);

    return 0;
}


/*

TODO:
* take matrix as input
* do the work as much as can be done
* spit out csv of results, so it can be viewed in eg excel?
 * maybe spit out a text file as well, or instead?
* handle todo's
* test with matrices that have missing variables etc

Link:
more compact algorithm, but less explicit (so less understandable imo): https://en.wikipedia.org/wiki/Gaussian_elimination#Pseudocode
 * note that it does row reduction, but doesn't go to rref

*/