#include <stdio.h>
#include <stdlib.h>
#include <array>

// A rational number, to handle fractional numbers without typical floating point issues
struct CRationalNumber
{
	CRationalNumber (size_t numerator = 0, size_t denominator = 1)
		: m_numerator(numerator)
		, m_denominator(denominator)
	{ }

	size_t m_numerator;
	size_t m_denominator;

	CRationalNumber Reciprocal () const
	{
		return CRationalNumber(m_denominator, m_numerator);
	}
};

// Define a vector as an array of rational numbers
template<size_t N>
using TVector = std::array<CRationalNumber, N>;

// Define a matrix as an array of vectors
template<size_t M, size_t N>
using TMatrix = std::array<TVector<N>, M>;

//===================================================================================================================================
//                                              RATIONAL NUMBER MATH
//===================================================================================================================================

bool operator == (const CRationalNumber& a, const CRationalNumber& b)
{
	// TODO: make common denominator and compare numerator?
	return false;
}

//===================================================================================================================================
//                                              GAUSS-JORDAN ELIMINATION CODE
//===================================================================================================================================

// From my blog post: http://blog.demofox.org/2017/04/10/solving-n-equations-and-n-unknowns-the-fine-print-gauss-jordan-elimination/

// Make a specific row have a 1 in the colIndex, and make all other rows have 0 there
template <size_t M, size_t N>
bool MakeRowClaimVariable (TMatrix<M, N>& matrix, size_t rowIndex, size_t colIndex)
{
    // Find a row that has a non zero value in this column and swap it with this row
    {
        // Find a row that has a non zero value
        size_t nonZeroRowIndex = rowIndex;
        while (nonZeroRowIndex < M && matrix[nonZeroRowIndex][colIndex] == CRationalNumber(0))
            ++nonZeroRowIndex;
 
        // If there isn't one, nothing to do
        if (nonZeroRowIndex == M)
            return false;
 
        // Otherwise, swap the row
        if (rowIndex != nonZeroRowIndex)
            std::swap(matrix[rowIndex], matrix[nonZeroRowIndex]);
    }
 
    // Scale this row so that it has a leading one
    CRationalNumber scale = matrix[rowIndex][colIndex].Reciprocal();
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

//===================================================================================================================================
//                                                       2D Textures / Quadratic Curves
//===================================================================================================================================
//
// Find the limitations of this pattern and show equivalence to Bernstein Polynomials (Bezier Curve Equations). Pattern details below.
//
//  --- For first curve, do:
//
//  P00 P01
//  P10 P11
//
//  P00 = C0
//  P01 + P10 = 2 * C1
//  P11 = C2
//
//  --- For each additional curve, add two points to the end like this:
//
//  P00 P01
//  P10 P11
//  P20 P21
//
//  P00 = C0
//  P01 + P10 = 2 * C1
//  P11 = C2
//
//  P10 = C3
//  P11 + P20 = 2 * C4
//  P21 = C5
//
//  --- Other details:
//  
//  * 3 control points per curve.
//  * image width it 2
//  * image height is 1 + NumCurves.  NumCurves > 0.
//  * there are 3 equations per curve, so 3 rows in the augmented matrix per curve.
//  * augmented matrix columns = num pixels (left columns) + num control points (right columns)
//

template <size_t NUMCURVES>
void Test2DQuadratic ()
{
	const size_t c_imageWidth = 2;
	const size_t c_imageHeight = NUMCURVES + 1;
	const size_t c_numPixels = c_imageWidth * c_imageHeight;
	const size_t c_numControlPoints = NUMCURVES * 3;
	const size_t c_numEquations = NUMCURVES * 3;

	// report values for this test
	printf("  %zu curves.  %zu control points.  2x%zu texture = %zu pixels.\n", NUMCURVES, c_numControlPoints, c_imageHeight, c_numPixels);

	// create the equations
	TMatrix<c_numEquations, c_numPixels + c_numControlPoints> augmentedMatrix;
	for (size_t curveIndex = 0; curveIndex < NUMCURVES; ++curveIndex)
	{
		TVector<c_numPixels + c_numControlPoints>* rows = &augmentedMatrix[curveIndex * 3 + 0];

		const size_t c_leftRowOffset = curveIndex * 2;
		const size_t c_rightRowOffset = curveIndex * 3 + c_numPixels;

		rows[0][c_leftRowOffset + 0] = CRationalNumber(1);
		rows[0][c_rightRowOffset + 0] = CRationalNumber(1);

		rows[1][c_leftRowOffset + 1] = CRationalNumber(1);
		rows[1][c_leftRowOffset + 2] = CRationalNumber(1);
		rows[1][c_rightRowOffset + 1] = CRationalNumber(2);

		rows[2][c_leftRowOffset + 3] = CRationalNumber(1);
		rows[2][c_rightRowOffset + 2] = CRationalNumber(1);
	}

	// TODO: put into rref?  http://blog.demofox.org/2017/04/10/solving-n-equations-and-n-unknowns-the-fine-print-gauss-jordan-elimination/

	GaussJordanElimination(augmentedMatrix);

	int ijkl = 0;
}

void Test2DQuadratics ()
{
	printf("Testing 2D Textures / Quadratic Curves\n");

	Test2DQuadratic<1>();
	Test2DQuadratic<2>();
	Test2DQuadratic<3>();
}

//===================================================================================================================================
//                                                                 main
//===================================================================================================================================

int main (int agrc, char **argv)
{
	Test2DQuadratics();

	printf("\n");
	system("pause");

	return 0;
}