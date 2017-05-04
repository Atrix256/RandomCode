#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <random>
#include <vector>

#define EQUALITY_TEST_SAMPLES 1000

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
		if (m_numerator != 0 && m_denominator != 0)
		{
			TINT div = CalculateGCD(m_numerator, m_denominator);
			m_numerator /= div;
			m_denominator /= div;
		}

		if (m_denominator < 0)
		{
			m_numerator *= -1;
			m_denominator *= -1;
		}
	}

	bool IsZero () const
	{
		return m_numerator == 0 && m_denominator != 0;
	}

	bool IsOne () const
	{
		return m_numerator == m_denominator;
	}

	bool IsWholeNumber () const
	{
		return m_denominator == 1;
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
//                                                           Shared Testing Code
//===================================================================================================================================

template <size_t M, size_t N>
bool SolveMatrixAndPrintEquations (TMatrix<M, N>& augmentedMatrix, size_t numPixels, std::unordered_set<size_t>& freeVariables)
{
	// put augmented matrix into rref
	GaussJordanElimination(augmentedMatrix);

	// reduce all fractions
	for (TVector<N>& row : augmentedMatrix)
		for (CRationalNumber& value : row)
			value.Reduce();

	// print out the equations
	bool constraintFound = false;
	for (const TVector<N>& row : augmentedMatrix)
	{
		printf("    ");
		bool leftHasATerm = false;
		for (size_t i = 0; i < numPixels; ++i)
		{
			if (!row[i].IsZero())
			{
				if (leftHasATerm)
				{
					printf(" + ");
					freeVariables.insert(i);
				}
				if (row[i].IsOne())
					printf("%c", 'A' + (int)i);
				else if (row[i].IsWholeNumber() == 1)
					printf("%c * %i", 'A' + (int)i, row[i].m_numerator);
				else
					printf("%c * %i/%i", 'A' + (int)i, row[i].m_numerator, row[i].m_denominator);
				leftHasATerm = true;
			}
		}
		if (!leftHasATerm)
			printf("0 = ");
		else
			printf(" = ");

		bool rightHasATerm = false;
		for (size_t i = numPixels; i < N; ++i)
		{
			if (!row[i].IsZero())
			{
				if (rightHasATerm)
					printf(" + ");
				if (row[i].IsOne())
					printf("C%zu", i - numPixels);
				else if (row[i].IsWholeNumber())
					printf("C%zu * %i", i - numPixels, row[i].m_numerator);
				else
					printf("C%zu * %i/%i", i - numPixels, row[i].m_numerator, row[i].m_denominator);
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
		return false;
	}

	printf("    %zu free variables.\n", freeVariables.size());
	return true;
}

float lerp (float t, float a, float b)
{
	return a * (1.0f - t) + b * t;
}

template <size_t NUMPIXELS, size_t NUMCONTROLPOINTS, size_t NUMEQUATIONS>
void FillInPixelsAndControlPoints (
	std::array<float, NUMPIXELS>& pixels,
	std::array<float, NUMCONTROLPOINTS>& controlPoints,
	const TMatrix<NUMEQUATIONS, NUMPIXELS+ NUMCONTROLPOINTS>& augmentedMatrix,
	const std::unordered_set<size_t>& freeVariables)
{
	// come up with random values for the control points and free variable pixels
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
	for (float& cp : controlPoints)
		cp = dist(mt);
	for (size_t var : freeVariables)
		pixels[var] = dist(mt);

	// fill in the non free variable pixels per the equations
	for (const TVector<NUMPIXELS + NUMCONTROLPOINTS>& row : augmentedMatrix)
	{
		// the first non zero value is the non free pixel we need to set.
		// all other non zero values are free variables that we previously calculated values for
		bool foundPixel = false;
		size_t pixelIndex = 0;
		for (size_t i = 0; i < NUMPIXELS; ++i)
		{
			if (!row[i].IsZero())
			{
				// we are setting the first pixel we find
				if (!foundPixel)
				{
					pixelIndex = i;
					foundPixel = true;
				}
				// subtract out all free variables which is the same as moving them to the right side of the equation
				else
				{
					pixels[pixelIndex] -= pixels[i] * float(row[i].m_numerator) / float(row[i].m_denominator);
				}
			}
		}

		// if there is no pixel value to set on the left side of the equation, ignore this row
		if (!foundPixel)
			continue;

		// add in the values from the right side of the equation
		for (size_t i = NUMPIXELS; i < NUMPIXELS + NUMCONTROLPOINTS; ++i)
		{
			if (!row[i].IsZero())
				pixels[pixelIndex] += controlPoints[i - NUMPIXELS] * float(row[i].m_numerator) / float(row[i].m_denominator);
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

template <size_t N>
float EvaluateBernsteinPolynomial2DQuadratic (float t, const std::array<float, N>& coefficients)
{
	const size_t c_numCurves = N / 3;
	t *= float(c_numCurves);
	size_t iOffset = std::min(size_t(t), c_numCurves - 1) * 3;
	t = std::fmodf(t, 1.0f);

	float s = 1.0f - t;
	return
		coefficients[iOffset + 0] * s * s +
		coefficients[iOffset + 1] * s * t * 2 +
		coefficients[iOffset + 2] * t * t;
}

template <size_t N>
float EvaluateLinearInterpolation2DQuadratic (float t, const std::array<float, N>& pixels)
{
	const size_t c_numCurves = (N / 2) - 1;
	t *= float(c_numCurves);
	size_t startRow = std::min(size_t(t), c_numCurves - 1);
	t = std::fmodf(t, 1.0f);

	float row0 = lerp(t, pixels[startRow * 2], pixels[startRow * 2 + 1]);
	float row1 = lerp(t, pixels[(startRow + 1) * 2], pixels[(startRow + 1) * 2 + 1]);
	return lerp(t, row0, row1);
}

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

	// solve the matrix if possible and print out the equations
	std::unordered_set<size_t> freeVariables;
	if (!SolveMatrixAndPrintEquations(augmentedMatrix, c_numPixels, freeVariables))
		return;

	// Next we need to show equality between the N-linear interpolation of our pixels and bernstein polynomials with our control points as coefficients

	// Fill in random values for our control points and free variable pixels, and fill in the other pixels as the equations dictate 
	std::array<float, c_numPixels> pixels = { 0 };
	std::array<float, c_numControlPoints> controlPoints = { 0 };
	FillInPixelsAndControlPoints<c_numPixels, c_numControlPoints, c_numEquations>(pixels, controlPoints, augmentedMatrix, freeVariables);

	// do a number of samples of each method at the same time values, and report the largest difference (error)
	float largestDifference = 0.0f;
	for (size_t i = 0; i < EQUALITY_TEST_SAMPLES; ++i)
	{
		float t = float(i) / float(EQUALITY_TEST_SAMPLES - 1);

		float value1 = EvaluateBernsteinPolynomial2DQuadratic(t, controlPoints);
		float value2 = EvaluateLinearInterpolation2DQuadratic(t, pixels);

		largestDifference = std::max(largestDifference, std::abs(value1 - value2));
	}
	printf("    %i Samples, Largest Error = %f\n", EQUALITY_TEST_SAMPLES, largestDifference);
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

template <size_t N>
float EvaluateBernsteinPolynomial2DQuadraticC0 (float t, const std::array<float, N>& coefficients)
{
	const size_t c_numCurves = (N - 1) / 2;
	t *= float(c_numCurves);
	size_t iOffset = std::min(size_t(t), c_numCurves - 1) * 2;
	t = std::fmodf(t, 1.0f);

	float s = 1.0f - t;
	return
		coefficients[iOffset + 0] * s * s +
		coefficients[iOffset + 1] * s * t * 2 +
		coefficients[iOffset + 2] * t * t;
}

template <size_t N>
float EvaluateLinearInterpolation2DQuadraticC0 (float t, const std::array<float, N>& pixels)
{
	const size_t c_numCurves = (N / 2) - 1;
	t *= float(c_numCurves);
	size_t startRow = std::min(size_t(t), c_numCurves - 1);
	t = std::fmodf(t, 1.0f);

	// Note we flip x axis direction every odd row to get the zig zag
	float horizT = (startRow % 2) == 0 ? t : 1.0f - t;

	float row0 = lerp(horizT, pixels[startRow * 2], pixels[startRow * 2 + 1]);
	++startRow;
	float row1 = lerp(horizT, pixels[startRow * 2], pixels[startRow * 2 + 1]);
	return lerp(t, row0, row1);
}

template <size_t NUMCURVES>
void Test2DQuadraticC0 ()
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
			size_t base = (i / 2) * 2;
			if (((i / 2) % 2) == 0)
				++base;
			row[base] = CRationalNumber(1);

			base = ((i + 1) / 2) * 2;
			if (((i / 2) % 2) == 1)
				++base;
			row[base] = CRationalNumber(1);
		}
	}
	
	// solve the matrix if possible and print out the equations
	std::unordered_set<size_t> freeVariables;
	if (!SolveMatrixAndPrintEquations(augmentedMatrix, c_numPixels, freeVariables))
		return;

	// Next we need to show equality between the N-linear interpolation of our pixels and bernstein polynomials with our control points as coefficients

	// Fill in random values for our control points and free variable pixels, and fill in the other pixels as the equations dictate 
	std::array<float, c_numPixels> pixels = { 0 };
	std::array<float, c_numControlPoints> controlPoints = { 0 };
	FillInPixelsAndControlPoints<c_numPixels, c_numControlPoints, c_numEquations>(pixels, controlPoints, augmentedMatrix, freeVariables);

	// do a number of samples of each method at the same time values, and report the largest difference (error)
	float largestDifference = 0.0f;
	for (size_t i = 0; i < EQUALITY_TEST_SAMPLES; ++i)
	{
		float t = float(i) / float(EQUALITY_TEST_SAMPLES - 1);

		float value1 = EvaluateBernsteinPolynomial2DQuadraticC0(t, controlPoints);
		float value2 = EvaluateLinearInterpolation2DQuadraticC0(t, pixels);

		largestDifference = std::max(largestDifference, std::abs(value1 - value2));
	}
	printf("    %i Samples, Largest Error = %f\n", EQUALITY_TEST_SAMPLES, largestDifference);
}

void Test2DQuadraticsC0 ()
{
	printf("\nTesting 2D Textures / Quadratic Curves with C0 continuity\n");

	Test2DQuadraticC0<1>();
	Test2DQuadraticC0<2>();
	Test2DQuadraticC0<3>();
	Test2DQuadraticC0<4>();

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
 * a #define to show pixels as (coordinate) numbers instead of letters?
 * as part of reduce, if bottom is negative, make it pos, and flip sign of top number
 * probably don't need pascal's triangle code
*/