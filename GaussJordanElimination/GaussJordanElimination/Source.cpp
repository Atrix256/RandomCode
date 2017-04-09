#include <stdio.h>
#include <array>
#include <vector>

// Define a vector as an array of floats
template<size_t N>
using TVector = std::array<float, N>;

// Define a matrix as an array of vectors
template<size_t M, size_t N>
using TMatrix = std::array<TVector<N>, M>;

// Helper function to fill out a matrix
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

// Make a specific row have a 1 in the colIndex, and make all other rows have 0 there
template <size_t M, size_t N>
bool MakeRowClaimVariable (TMatrix<M, N>& matrix, size_t rowIndex, size_t colIndex)
{
	// Find a row that has a non zero value in this column and swap it with this row
	{
		// Find a row that has a non zero value
		size_t nonZeroRowIndex = rowIndex;
		while (nonZeroRowIndex < M && matrix[nonZeroRowIndex][colIndex] == 0.0f)
			++nonZeroRowIndex;

		// If there isn't one, nothing to do
		if (nonZeroRowIndex == M)
			return false;

		// Otherwise, swap the row
		if (rowIndex != nonZeroRowIndex)
			std::swap(matrix[rowIndex], matrix[nonZeroRowIndex]);
	}

	// Scale this row so that it has a leading one
	float scale = 1.0f / matrix[rowIndex][colIndex];
	for (size_t normalizeColIndex = colIndex; normalizeColIndex < N; ++normalizeColIndex)
		matrix[rowIndex][normalizeColIndex] *= scale;

	// Make sure all rows except this one have a zero in this column.
	// Do this by subtracting this row from other rows, multiplied by a multiple that makes the column disappear.
	for (size_t eliminateRowIndex = 0; eliminateRowIndex < M; ++eliminateRowIndex)
	{
		if (eliminateRowIndex == rowIndex)
			continue;

		float scale = matrix[eliminateRowIndex][colIndex];
		for (size_t eliminateColIndex = 0; eliminateColIndex < N; ++eliminateColIndex)
			matrix[eliminateRowIndex][eliminateColIndex] -= matrix[rowIndex][eliminateColIndex] * scale;
	}

	return true;
}

// make matrix into reduced row echelon form
template <size_t M, size_t N>
void GaussJordanElimination (TMatrix<M, N>& matrix)
{
	size_t rowIndex = 0;
	for (size_t colIndex = 0; colIndex < std::min(M, N); ++colIndex)
	{
		if(MakeRowClaimVariable(matrix, rowIndex, colIndex))
			++rowIndex;
	}
}

int main (int argc, char **argv)
{
    // the input matrix
	/*
    auto inputMatrix = MakeMatrix<3, 4>(
    {
        { 0.0f, 0.5f, 0.5f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    });
	*/

	/*
	// 3x + y = 7
	// 2x + 4y = 3
	auto inputMatrix = MakeMatrix<2, 3>(
	{
		{ 3.0f, 1.0f, 7.0f },
		{ 2.0f, 4.0f, 3.0f },
	});
	*/

	/*
	// x+y=3
	// x-y=5
	auto inputMatrix = MakeMatrix<2, 3>(
	{
		{ 1.0f, 1.0f, 3.0f },
		{ 1.0f, -1.0f, 5.0f },
	});
	*/

	// TODO: this case below isn't well handled.  we end up with two values for z.  should collapse into one i think.
	// x+z=3
	// x-z=5
	auto matrix = MakeMatrix<3, 4>(
	{
		{ 1.0f, 0.0f, 1.0f, 3.0f },
		{ 1.0f, 0.0f, -1.0f, 5.0f },
		{ 1.0f, 0.0f, 2.0f, 5.0f },
	});

    GaussJordanElimination(matrix);

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
* review and update all comments
? should we also calculate the null space?
? should we say if the matrix is inconsistent? that assumes an augmented matrix though, so maybe say "if an augmented matrix, it's inconsistent"
? should we do augmented matrix vs not?
? what if not enough items are given for the MakeMatrix function, what's it do?

Blog:
Note, not most efficient code, but made to be readable.

Link:
more compact algorithm, but less explicit (so less understandable imo): https://en.wikipedia.org/wiki/Gaussian_elimination#Pseudocode
 * note that it does row reduction, but doesn't go to rref
 * also assumes that there isn't a full column of zeros!
decent explanation of gauss jordan (could be better though, it doesn't take the matrix to the proper final place!)
 * https://www.youtube.com/watch?v=Xzqa7ztokao

*/