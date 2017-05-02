#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <algorithm>
#include <unordered_set>

#define EQUALITY_TEST_SAMPLES 100

typedef int32_t TINT;

TINT CalculateGCD (TINT smaller, TINT larger);
TINT CalculateLCM (TINT smaller, TINT larger);

// A rational number, to handle fractional numbers without typical floating point issues
struct CRationalNumber
{
	CRationalNumber (TINT numerator = 0, TINT denominator = 1)
		: m_numerator(numerator)
		, m_denominator(denominator)
	{ }

	TINT m_numerator;
	TINT m_denominator;

	CRationalNumber Reciprocal () const
	{
		return CRationalNumber(m_denominator, m_numerator);
	}

	void Reduce ()
	{
		TINT div = CalculateGCD(m_numerator, m_denominator);
		m_numerator /= div;
		m_denominator /= div;
	}
};

// Define a vector as an array of rational numbers
template<size_t N>
using TVector = std::array<CRationalNumber, N>;

// Define a matrix as an array of vectors
template<size_t M, size_t N>
using TMatrix = std::array<TVector<N>, M>;

//===================================================================================================================================
//                                              GCD / LCM
//===================================================================================================================================

// from my blog post: http://blog.demofox.org/2015/01/24/programmatically-calculating-gcd-and-lcm/

TINT CalculateGCD (TINT smaller, TINT larger)
{
	// make sure A <= B before starting
	if (larger < smaller)
		std::swap(smaller, larger);

	// loop
	while (1)
	{
		// if the remainder of larger / smaller is 0, they are the same
		// so return smaller as the GCD
		TINT remainder = larger % smaller;
		if (remainder == 0)
			return smaller;

		// otherwise, the new larger number is the old smaller number, and
		// the new smaller number is the remainder
		larger = smaller;
		smaller = remainder;
	}
}

TINT CalculateLCM (TINT A, TINT B)
{
	// LCM(A,B) = (A/GCD(A,B))*B
	return (A / CalculateGCD(A, B))*B;
}

//===================================================================================================================================
//                                              RATIONAL NUMBER MATH
//===================================================================================================================================

void CommonDenominators (CRationalNumber& a, CRationalNumber& b)
{
	TINT gcm = CalculateGCD(a.m_denominator, b.m_denominator);

	a.m_numerator *= gcm / a.m_denominator;
	b.m_numerator *= gcm / b.m_denominator;

	a.m_denominator = gcm;
	b.m_denominator = gcm;
}

bool operator == (const CRationalNumber& a, const CRationalNumber& b)
{
	CRationalNumber _a(a), _b(b);
	CommonDenominators(_a, _b);
	return _a.m_numerator == _b.m_numerator;
}

void operator *= (CRationalNumber& a, const CRationalNumber& b)
{
	a.m_numerator *= b.m_numerator;
	a.m_denominator *= b.m_denominator;
}

CRationalNumber operator * (const CRationalNumber& a, const CRationalNumber& b)
{
	return CRationalNumber(a.m_numerator * b.m_numerator, a.m_denominator * b.m_denominator);
}

void operator -= (CRationalNumber& a, const CRationalNumber& b)
{
	CRationalNumber _b(b);
	CommonDenominators(a, _b);
	a.m_numerator -= _b.m_numerator;
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
 
        CRationalNumber scale = matrix[eliminateRowIndex][colIndex];
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
		TVector<c_numPixels + c_numControlPoints>* rows = &augmentedMatrix[curveIndex * 3];

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

	// put augmented matrix into rref
	GaussJordanElimination(augmentedMatrix);

	// print out the equations
	std::unordered_set<size_t> freeVariables;
	bool constraintFound = false;
	for (const TVector<c_numPixels + c_numControlPoints>& row : augmentedMatrix)
	{
		printf("    ");
		bool leftHasATerm = false;
		for (size_t i = 0; i < c_numPixels; ++i)
		{
			if (row[i].m_numerator != 0)
			{
				if (leftHasATerm)
				{
					printf(" + ");
					freeVariables.insert(i);
				}
				if (row[i].m_numerator == 1 && row[i].m_denominator == 1)
					printf("%c", 'A' + i);
				else if (row[i].m_denominator == 1)
					printf("%c * %i", 'A' + i, row[i].m_numerator);
				else
					printf("%c * %i/%i", 'A' + i, row[i].m_numerator, row[i].m_denominator);
				leftHasATerm = true;
			}
		}
		if (!leftHasATerm)
			printf("0 = ");
		else
			printf(" = ");

		bool rightHasATerm = false;
		for (size_t i = c_numPixels; i < c_numPixels + c_numControlPoints; ++i)
		{
			if (row[i].m_numerator != 0)
			{
				if (rightHasATerm)
					printf(" + ");
				if (row[i].m_numerator == 1 && row[i].m_denominator == 1)
					printf("C%zu", i - c_numPixels);
				else if (row[i].m_denominator == 1)
					printf("C%zu * %i", i - c_numPixels, row[i].m_numerator);
				else
					printf("C%zu * %i/%i", i - c_numPixels, row[i].m_numerator, row[i].m_denominator);
				rightHasATerm = true;
			}
		}

		printf("\n");

		if (!leftHasATerm && rightHasATerm)
			constraintFound = true;
	}

	if (constraintFound)
	{
		printf("    Constraint Found.  This configuration doesn't work!\n");
		return;
	}

	printf("    %zu free variables.\n", freeVariables.size());

	// TODO: test equality next of N-linear interpolation and the bezier formulas. do N samples, where N is a define.  report largest difference value found.
	int ijkl = 0;
}

void Test2DQuadratics ()
{
	printf("Testing 2D Textures / Quadratic Curves\n");

	Test2DQuadratic<1>();
	Test2DQuadratic<2>();
	Test2DQuadratic<3>();

	printf("\n");
	system("pause");
}

//===================================================================================================================================
//                                    2D Textures / Quadratic Curves With C0 Continuity
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
//  --- For second curve, do:
//
//  P00 P01
//  P10 P11
//  P20 P21
//
//  P00 = C0
//  P01 + P10 = 2 * C1
//  P11 = C2
//
//  P10 + P21 = 2 * C3
//  P20 = C4
//
//
//  --- For third curve, do:
//
//  P00 P01
//  P10 P11
//  P20 P21
//  P30 P31
//
//  P00 = C0
//  P01 + P10 = 2 * C1
//  P11 = C2
//
//  P10 + P21 = 2 * C3
//  P20 = C4
//
//  P21 + P30 = 2 * C5
//  P31 = C6
//
//  --- Other details:
//  
//  * control points: 1 + NumCurves*2.  NumCurves > 0.
//  * image width it 2
//  * image height is 1 + NumCurves.  NumCurves > 0.
//  * equations: 1 + NumCurves*2.  NumCurves > 0.  This many rows in the augmented matrix.
//  * augmented matrix columns = num pixels (left columns) + num control points (right columns)
//

template <size_t NUMCURVES>
void Test2DQuadraticsC0 ()
{
	const size_t c_imageWidth = 2;
	const size_t c_imageHeight = NUMCURVES + 1;
	const size_t c_numPixels = c_imageWidth * c_imageHeight;
	const size_t c_numControlPoints = 1 + NUMCURVES * 2;
	const size_t c_numEquations = 1 + NUMCURVES * 2;

	// report values for this test
	printf("  %zu curves.  %zu control points.  2x%zu texture = %zu pixels.\n", NUMCURVES, c_numControlPoints, c_imageHeight, c_numPixels);

	// create the equations
	TMatrix<c_numEquations, c_numPixels + c_numControlPoints> augmentedMatrix;

	for (size_t i = 0; i < c_numEquations; ++i)
	{
		TVector<c_numPixels + c_numControlPoints>& row = augmentedMatrix[i];

		// right side of the equation is always the same pattern.  Along the diagonal odd rows get a 1, even rows get a 2.
		row[c_numPixels + i] = (i % 2 == 0) ? CRationalNumber(1) : CRationalNumber(2);

		// even rows get a single value on the left side of the equation
		if (i % 2 == 0)
		{
			size_t offset = (i / 2) % 2;
			row[i + offset] = CRationalNumber(1);
		}
		// odd rows get two values on left side of the equation
		else
		{
			// it goes in this pattern. maybe have one calculation to get last and use it also for current?
			// 01
			// 10
			// 21
			// 30
			// 41

			// TODO: this is wrong somehow!
			size_t base = (i / 2) * 2;
			size_t offset = ((1 / 2) % 2) == 1 ? 0 : 1;
			row[base + offset] = CRationalNumber(1);

			base = base + 2;
			offset = offset ? 0 : 1;
			row[base + offset] = CRationalNumber(1);
		}
	}
	
	// put augmented matrix into rref
	GaussJordanElimination(augmentedMatrix);

	// print out the equations
	std::unordered_set<size_t> freeVariables;
	bool constraintFound = false;
	for (const TVector<c_numPixels + c_numControlPoints>& row : augmentedMatrix)
	{
		printf("    ");
		bool leftHasATerm = false;
		for (size_t i = 0; i < c_numPixels; ++i)
		{
			if (row[i].m_numerator != 0)
			{
				if (leftHasATerm)
				{
					printf(" + ");
					freeVariables.insert(i);
				}
				if (row[i].m_numerator == 1 && row[i].m_denominator == 1)
					printf("%c", 'A' + i);
				else if (row[i].m_denominator == 1)
					printf("%c * %i", 'A' + i, row[i].m_numerator);
				else
					printf("%c * %i/%i", 'A' + i, row[i].m_numerator, row[i].m_denominator);
				leftHasATerm = true;
			}
		}
		if (!leftHasATerm)
			printf("0 = ");
		else
			printf(" = ");

		bool rightHasATerm = false;
		for (size_t i = c_numPixels; i < c_numPixels + c_numControlPoints; ++i)
		{
			if (row[i].m_numerator != 0)
			{
				if (rightHasATerm)
					printf(" + ");
				if (row[i].m_numerator == 1 && row[i].m_denominator == 1)
					printf("C%zu", i - c_numPixels);
				else if (row[i].m_denominator == 1)
					printf("C%zu * %i", i - c_numPixels, row[i].m_numerator);
				else
					printf("C%zu * %i/%i", i - c_numPixels, row[i].m_numerator, row[i].m_denominator);
				rightHasATerm = true;
			}
		}

		printf("\n");

		if (!leftHasATerm && rightHasATerm)
			constraintFound = true;
	}

	if (constraintFound)
	{
		printf("    Constraint Found.  This configuration doesn't work!\n");
		return;
	}

	printf("    %zu free variables.\n", freeVariables.size());

	// TODO: test equality next of N-linear interpolation and the bezier formulas. do N samples, where N is a define.  report largest difference value found.
	int ijkl = 0;

	// TODO: factor out some of the common stuff, like displaying formulas
}

void Test2DQuadraticsC0 ()
{
	printf("\nTesting 2D Textures / Quadratic Curves with C0 continuity\n");

	Test2DQuadraticsC0<1>();
	Test2DQuadraticsC0<2>();
	Test2DQuadraticsC0<3>();
	Test2DQuadraticsC0<4>();

	// TODO: how many should we show?

	printf("\n");
	system("pause");
}

//===================================================================================================================================
//                                                                 main
//===================================================================================================================================

int main (int agrc, char **argv)
{
	Test2DQuadratics();
	Test2DQuadraticsC0();

	// TODO: then 3d, 3d multiple curves, 3d zig zag, and so on? maybe general case or 4d or who knows.

	return 0;
}

/*
TODO:
 * should we display pixels per control point and pixels per curve information?
*/