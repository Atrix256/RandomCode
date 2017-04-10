#include <stdio.h>
#include <array>
#include <vector>
#include <assert.h>

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
	assert(matrixData.size() == M);
    for (const std::initializer_list<float>& rowData : matrixData)
    {
		assert(rowData.size() == N);
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
	for (size_t colIndex = 0; colIndex < N; ++colIndex)
	{
		if (MakeRowClaimVariable(matrix, rowIndex, colIndex))
		{
			++rowIndex;
			if (rowIndex == M)
				return;
		}
	}
}

int main (int argc, char **argv)
{
	auto matrix = MakeMatrix<3, 4>(
	{
		{ 2.0f, 0.0f, 0.0f, 2.0f },
		{ 0.0f, 2.0f, 0.0f, 4.0f },
        { 0.0f, 0.0f, 2.0f, 8.0f },
	});

    GaussJordanElimination(matrix);

    return 0;
}