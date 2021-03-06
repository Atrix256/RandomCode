#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <random>
#include <vector>

#define SHOW_MATHJAX_MATRIX() 0
#define SHOW_MATHJAX_EQUATIONS() 0
#define SHOW_EQUATIONS_BEFORE_SOLVE() 0
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
		
		if (m_numerator == 0)
			m_denominator = 1;
	}

	bool IsZero () const
	{
		return m_numerator == 0 && m_denominator != 0;
	}

	// NOTE: the functions below assume Reduce() has happened
	bool IsOne () const
	{
		return m_numerator == 1 && m_denominator == 1;
	}

	bool IsMinusOne () const
	{
		return m_numerator == -1 && m_denominator == 1;
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
	TINT lcm = CalculateLCM(a.m_denominator, b.m_denominator);

	a.m_numerator *= lcm / a.m_denominator;
	b.m_numerator *= lcm / b.m_denominator;

	a.m_denominator = lcm;
	b.m_denominator = lcm;
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
        while (nonZeroRowIndex < M && matrix[nonZeroRowIndex][colIndex].IsZero())
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
	{
		matrix[rowIndex][normalizeColIndex] *= scale;
		matrix[rowIndex][normalizeColIndex].Reduce();
	}
 
    // Make sure all rows except this one have a zero in this column.
    // Do this by subtracting this row from other rows, multiplied by a multiple that makes the column disappear.
    for (size_t eliminateRowIndex = 0; eliminateRowIndex < M; ++eliminateRowIndex)
    {
        if (eliminateRowIndex == rowIndex)
            continue;
 
        CRationalNumber scale = matrix[eliminateRowIndex][colIndex];
		for (size_t eliminateColIndex = 0; eliminateColIndex < N; ++eliminateColIndex)
		{
			matrix[eliminateRowIndex][eliminateColIndex] -= matrix[rowIndex][eliminateColIndex] * scale;
			matrix[eliminateRowIndex][eliminateColIndex].Reduce();
		}
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

template <size_t M, size_t N, typename LAMBDA>
void PrintEquations (
	TMatrix<M, N>& augmentedMatrix,
	size_t numPixels,
	LAMBDA& pixelIndexToCoordinates
)
{
	char pixelCoords[10];

#if SHOW_MATHJAX_MATRIX()
	// print the matrix opening stuff
	printf("\\left[\\begin{array}{");
	for (size_t i = 0; i < N; ++i)
	{
		if (i == numPixels)
			printf("|");
		printf("r");
	}
	printf("}\n");
	// print the header row
	for (size_t i = 0; i < numPixels; ++i)
	{
		pixelIndexToCoordinates(i, pixelCoords);
		if (i == 0)
			printf("P_{%s}", pixelCoords);
		else
			printf(" & P_{%s}", pixelCoords);
	}
	for (size_t i = numPixels; i < N; ++i)
	{
		printf(" & C_{%zu}", i-numPixels);
	}
	printf(" \\\\\n");

	// Print the matrix
	for (const TVector<N>& row : augmentedMatrix)
	{
		bool first = true;
		for (const CRationalNumber& n : row)
		{
			if (first)
				first = false;
			else
				printf(" & ");

			if (n.IsWholeNumber())
				printf("%i", n.m_numerator);
			else
				printf("\\frac{%i}{%i}", n.m_numerator, n.m_denominator);
		}
		printf(" \\\\\n");
	}

	// print the matrix closing stuff
	printf("\\end{array}\\right]\n");
#endif

	// print equations
	for (const TVector<N>& row : augmentedMatrix)
	{
		// indent
		#if SHOW_MATHJAX_EQUATIONS() == 0
			printf("    ");
		#endif

		// left side of the equation
		bool leftHasATerm = false;
		for (size_t i = 0; i < numPixels; ++i)
		{
			if (!row[i].IsZero())
			{
				if (leftHasATerm)
					printf(" + ");
				pixelIndexToCoordinates(i, pixelCoords);

				#if SHOW_MATHJAX_EQUATIONS()
					if (row[i].IsOne())
						printf("P_{%s}", pixelCoords);
					else if (row[i].IsMinusOne())
						printf("-P_{%s}", pixelCoords);
					else if (row[i].IsWholeNumber())
						printf("%iP_{%s}", row[i].m_numerator, pixelCoords);
					else if (row[i].m_numerator == 1)
						printf("P_{%s}/%i", pixelCoords, row[i].m_denominator);
					else
						printf("P_{%s} * %i/%i", pixelCoords, row[i].m_numerator, row[i].m_denominator);
				#else
					if (row[i].IsOne())
						printf("P%s", pixelCoords);
					else if (row[i].IsMinusOne())
						printf("-P%s", pixelCoords);
					else if (row[i].IsWholeNumber())
						printf("%iP%s", row[i].m_numerator, pixelCoords);
					else if (row[i].m_numerator == 1)
						printf("P%s/%i", pixelCoords, row[i].m_denominator);
					else
						printf("P%s * %i/%i", pixelCoords, row[i].m_numerator, row[i].m_denominator);
				#endif
				leftHasATerm = true;
			}
		}
		if (!leftHasATerm)
			printf("0 = ");
		else
			printf(" = ");

		// right side of the equation
		bool rightHasATerm = false;
		for (size_t i = numPixels; i < N; ++i)
		{
			if (!row[i].IsZero())
			{
				if (rightHasATerm)
					printf(" + ");

				#if SHOW_MATHJAX_EQUATIONS()
					if (row[i].IsOne())
						printf("C_{%zu}", i - numPixels);
					else if (row[i].IsMinusOne())
						printf("-C_{%zu}", i - numPixels);
					else if (row[i].IsWholeNumber())
						printf("%iC_{%zu}", row[i].m_numerator, i - numPixels);
					else if (row[i].m_numerator == 1)
						printf("C_{%zu}/%i", i - numPixels, row[i].m_denominator);
					else
						printf("C_{%zu} * %i/%i", i - numPixels, row[i].m_numerator, row[i].m_denominator);
				#else
					if (row[i].IsOne())
						printf("C%zu", i - numPixels);
					else if (row[i].IsMinusOne())
						printf("-C%zu", i - numPixels);
					else if (row[i].IsWholeNumber())
						printf("%iC%zu", row[i].m_numerator, i - numPixels);
					else if (row[i].m_numerator == 1)
						printf("C%zu/%i", i - numPixels, row[i].m_denominator);
					else
						printf("C%zu * %i/%i", i - numPixels, row[i].m_numerator, row[i].m_denominator);
				#endif
				rightHasATerm = true;
			}
		}

		#if SHOW_MATHJAX_EQUATIONS()
			printf("\\\\\n");
		#else
			printf("\n");
		#endif
	}
}

template <size_t M, size_t N, typename LAMBDA>
bool SolveMatrixAndPrintEquations (
	TMatrix<M, N>& augmentedMatrix,
	size_t numPixels,
	std::unordered_set<size_t>& freeVariables,
	LAMBDA& pixelIndexToCoordinates
)
{
	#if SHOW_EQUATIONS_BEFORE_SOLVE()
	printf("   Initial Equations:\n");
	PrintEquations(augmentedMatrix, numPixels, pixelIndexToCoordinates);
	printf("   Solved Equations:\n");
	#endif

	// put augmented matrix into rref
	GaussJordanElimination(augmentedMatrix);

	// Print equations
	PrintEquations(augmentedMatrix, numPixels, pixelIndexToCoordinates);

	// Get free variables and check for control point constraint
	bool constraintFound = false;
	for (const TVector<N>& row : augmentedMatrix)
	{
		bool leftHasATerm = false;
		for (size_t i = 0; i < numPixels; ++i)
		{
			if (!row[i].IsZero())
			{
				if (leftHasATerm)
					freeVariables.insert(i);
				else
					leftHasATerm = true;
			}
		}

		bool rightHasATerm = false;
		for (size_t i = numPixels; i < N; ++i)
		{
			if (!row[i].IsZero())
				rightHasATerm = true;
		}

		if (!leftHasATerm && rightHasATerm)
			constraintFound = true;
	}

	printf("  %zu free variables.\n", freeVariables.size());

	if (constraintFound)
	{
		printf("  Constraint Found.  This configuration doesn't work for the general case!\n\n");
		return false;
	}

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

size_t TextureCoordinateToPixelIndex2d (size_t width, size_t height, size_t y, size_t x)
{
	return y * width + x;
};

void PixelIndexToTextureCoordinate2d (size_t width, size_t height, size_t pixelIndex, size_t& y, size_t& x)
{
	x = pixelIndex % width;
	y = pixelIndex / width;
}

size_t TextureCoordinateToPixelIndex3d (size_t width, size_t height, size_t depth, size_t z, size_t y, size_t x)
{
	return 
		z * width * height + 
		y * width +
		x;
};

void PixelIndexToTextureCoordinate3d (size_t width, size_t height, size_t depth, size_t pixelIndex, size_t& z, size_t& y, size_t& x)
{
	x = pixelIndex % width;

	pixelIndex = pixelIndex / width;

	y = pixelIndex % height;

	pixelIndex = pixelIndex / height;

	z = pixelIndex;
}

void PiecewiseCurveTime (float time, size_t numCurves, size_t& outCurveIndex, float& outTime)
{
	time *= float(numCurves);
	outCurveIndex = size_t(time);

	if (outCurveIndex == numCurves)
	{
		outCurveIndex = numCurves - 1;
		outTime = 1.0f;
	}
	else
	{
		outTime = std::fmodf(time, 1.0f);
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
//  P00 = C0                        0
//  P01 + P10 = 2 * C1              1 2
//  P11 = C2                        3
//
//  --- For each additional curve, add two points to the end like this:
//
//  P00 P01
//  P10 P11
//  P20 P21
//
//  P00 = C0                        0
//  P01 + P10 = 2 * C1              1 2
//  P11 = C2                        3
//
//  P10 = C3                        1
//  P11 + P20 = 2 * C4              3 4
//  P21 = C5                        5
//
//  and so on...
//  each equation is then multiplied by a value so the right side is identity and left side coefficients add up to 1.
//
//  --- Other details:
//  
//  * 3 control points per curve.
//  * image width it 2
//  * image height is 1 + NumCurves.
//  * there are 3 equations per curve, so 3 rows in the augmented matrix per curve.
//  * augmented matrix columns = num pixels (left columns) + num control points (right columns)
//

template <size_t N>
float EvaluateBernsteinPolynomial2DQuadratic (float totalTime, const std::array<float, N>& coefficients)
{
	const size_t c_numCurves = N / 3;

	float t;
	size_t startCurve;
	PiecewiseCurveTime(totalTime, c_numCurves, startCurve, t);

	size_t offset = startCurve * 3;

	float s = 1.0f - t;
	return
		coefficients[offset + 0] * s * s +
		coefficients[offset + 1] * s * t * 2.0f +
		coefficients[offset + 2] * t * t;
}

template <size_t N>
float EvaluateLinearInterpolation2DQuadratic (float totalTime, const std::array<float, N>& pixels)
{
	const size_t c_numCurves = (N / 2) - 1;

	float t;
	size_t startRow;
	PiecewiseCurveTime(totalTime, c_numCurves, startRow, t);

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
	printf("  %f pixels per curve.  %f pixels per control point.\n", float(c_numPixels) / float(NUMCURVES), float(c_numPixels) / float(c_numControlPoints));

	// lambdas to convert between pixel index and texture coordinates
	auto TextureCoordinateToPixelIndex = [&](size_t y, size_t x) -> size_t
	{
		return TextureCoordinateToPixelIndex2d(c_imageWidth, c_imageHeight, y, x);
	};
	auto pixelIndexToCoordinates = [&](size_t pixelIndex, char pixelCoords[10])
	{
		size_t y, x;
		PixelIndexToTextureCoordinate2d(c_imageWidth, c_imageHeight, pixelIndex, y, x);
		sprintf(pixelCoords, "%zu%zu", y, x);
	};

	// create the equations
	TMatrix<c_numEquations, c_numPixels + c_numControlPoints> augmentedMatrix;
	for (size_t i = 0; i < c_numEquations; ++i)
	{
		TVector<c_numPixels + c_numControlPoints>& row = augmentedMatrix[i];

		// left side of the equation goes in this yx coordinate pattern:
		//   00 
		//   01 10
		//   11
		// But, curve index is added to the y index.
		// Also, left side coefficients must add up to 1.
		size_t curveIndex = i / 3;
		switch (i % 3)
		{
			case 0:
			{
				row[TextureCoordinateToPixelIndex(curveIndex + 0, 0)] = CRationalNumber(1, 1);
				break;
			}
			case 1:
			{
				row[TextureCoordinateToPixelIndex(curveIndex + 0, 1)] = CRationalNumber(1, 2);
				row[TextureCoordinateToPixelIndex(curveIndex + 1, 0)] = CRationalNumber(1, 2);
				break;
			}
			case 2:
			{
				row[TextureCoordinateToPixelIndex(curveIndex + 1, 1)] = CRationalNumber(1, 1);
				break;
			}
		}

		// right side of the equation is identity
		row[c_numPixels + i] = CRationalNumber(1);
	}

	// solve the matrix if possible and print out the equations
	std::unordered_set<size_t> freeVariables;
	if (!SolveMatrixAndPrintEquations(augmentedMatrix, c_numPixels, freeVariables, pixelIndexToCoordinates))
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
	printf("  %i Samples, Largest Error = %f\n\n", EQUALITY_TEST_SAMPLES, largestDifference);
}

void Test2DQuadratics ()
{
	printf("Testing 2D Textures / Quadratic Curves\n\n");

	Test2DQuadratic<1>();
	Test2DQuadratic<2>();
	Test2DQuadratic<3>();

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
//  P00 = C0                        0
//  P01 + P10 = 2 * C1              1 2
//  P11 = C2                        3
//
//  --- For second curve, do:
//
//  P00 P01
//  P10 P11
//  P20 P21
//
//  P00 = C0                        0
//  P01 + P10 = 2 * C1              1 2
//  P11 = C2                        3
//
//  P10 + P21 = 2 * C3              2 5
//  P20 = C4                        4
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
//  and so on...
//  each equation is then multiplied by a value so the right side is identity and left side coefficients add up to 1.
//
//  --- Other details:
//  
//  * control points: 1 + NumCurves*2.
//  * image width it 2
//  * image height is 1 + NumCurves.
//  * equations: 1 + NumCurves*2.  This many rows in the augmented matrix.
//  * augmented matrix columns = num pixels (left columns) + num control points (right columns)
//

template <size_t N>
float EvaluateBernsteinPolynomial2DQuadraticC0 (float totalTime, const std::array<float, N>& coefficients)
{
	const size_t c_numCurves = (N - 1) / 2;

	float t;
	size_t startCurve;
	PiecewiseCurveTime(totalTime, c_numCurves, startCurve, t);

	size_t offset = startCurve * 2;

	float s = 1.0f - t;
	return
		coefficients[offset + 0] * s * s +
		coefficients[offset + 1] * s * t * 2.0f +
		coefficients[offset + 2] * t * t;
}

template <size_t N>
float EvaluateLinearInterpolation2DQuadraticC0 (float totalTime, const std::array<float, N>& pixels)
{
	const size_t c_numCurves = (N / 2) - 1;

	float t;
	size_t startRow;
	PiecewiseCurveTime(totalTime, c_numCurves, startRow, t);

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
	printf("  %f pixels per curve.  %f pixels per control point.\n", float(c_numPixels) / float(NUMCURVES), float(c_numPixels) / float(c_numControlPoints));

	// lambdas to convert between pixel index and texture coordinates
	auto TextureCoordinateToPixelIndex = [&] (size_t y, size_t x) -> size_t
	{
		return TextureCoordinateToPixelIndex2d(c_imageWidth, c_imageHeight, y, x);
	};
	auto pixelIndexToCoordinates = [&] (size_t pixelIndex, char pixelCoords[10])
	{
		size_t y, x;
		PixelIndexToTextureCoordinate2d(c_imageWidth, c_imageHeight, pixelIndex, y, x);
		sprintf(pixelCoords, "%zu%zu", y, x);
	};

	// create the equations
	TMatrix<c_numEquations, c_numPixels + c_numControlPoints> augmentedMatrix;
	for (size_t i = 0; i < c_numEquations; ++i)
	{
		TVector<c_numPixels + c_numControlPoints>& row = augmentedMatrix[i];

		// left side of the equation has a pattern like this:
		//   00
		//   01 10
		//
		// But, pattern index is added to the y index.
		// Also, the x coordinates flip from 0 to 1 on those after each pattern.
		// Also, left side coefficients must add up to 1.

		size_t patternIndex = i / 2;
		size_t xoff = patternIndex % 2 == 1;
		size_t xon = patternIndex % 2 == 0;
		switch (i % 2)
		{
			case 0:
			{
				row[TextureCoordinateToPixelIndex(patternIndex + 0, xoff)] = CRationalNumber(1, 1);
				break;
			}
			case 1:
			{
				row[TextureCoordinateToPixelIndex(patternIndex + 0, xon)] = CRationalNumber(1, 2);
				row[TextureCoordinateToPixelIndex(patternIndex + 1, xoff)] = CRationalNumber(1, 2);
				break;
			}
		}

		// right side of the equation is identity
		row[c_numPixels + i] = CRationalNumber(1);
	}
	
	// solve the matrix if possible and print out the equations
	std::unordered_set<size_t> freeVariables;
	if (!SolveMatrixAndPrintEquations(augmentedMatrix, c_numPixels, freeVariables, pixelIndexToCoordinates))
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
	printf("  %i Samples, Largest Error = %f\n\n", EQUALITY_TEST_SAMPLES, largestDifference);
}

void Test2DQuadraticsC0 ()
{
	printf("\nTesting 2D Textures / Quadratic Curves with C0 continuity\n\n");

	Test2DQuadraticC0<1>();
	Test2DQuadraticC0<2>();
	Test2DQuadraticC0<3>();
	Test2DQuadraticC0<4>();

	system("pause");
}

//===================================================================================================================================
//                                             3D Textures / Cubic Curves
//===================================================================================================================================
//
// Find the limitations of this pattern and show equivalence to Bernstein Polynomials (Bezier Curve Equations). Pattern details below.
//
//  --- For first curve, do:
//
//  P000 P001    P100 P101
//  P010 P011    P110 P111
//
//  P000 = C0                       0
//  P001 + P010 + P100 = 3 * C1     1 2 4
//  P011 + P101 + P110 = 3 * C2     3 5 6
//  P111 = C3                       7
//
//  --- For second curve, do:
//
//  P000 P001    P100 P101
//  P010 P011    P110 P111
//  P020 P021    P120 P121
//
//  P000 = C0                       0
//  P001 + P010 + P100 = 3 * C1     1 2 4
//  P011 + P101 + P110 = 3 * C2     3 7 8
//  P111 = C3                       9
//
//  P010 = C4                       2
//  P011 + P020 + P110 = 3 * C5     3 4 8
//  P021 + P111 + P120 = 3 * C6     5 9 10
//  P121 = C7                       11
//
//  and so on...
//  each equation is then multiplied by a value so the right side is identity and left side coefficients add up to 1.
//
//  --- Other details:
//  
//  * control points: 4 * NumCurves.
//  * image width it 2
//  * image depth is 2
//  * image height is 1 + NumCurves.
//  * equations: 4 * NumCurves.  This many rows in the augmented matrix.
//  * augmented matrix columns = num pixels (left columns) + num control points (right columns)
//

template <size_t N>
float EvaluateBernsteinPolynomial3DCubic (float totalTime, const std::array<float, N>& coefficients)
{
	const size_t c_numCurves = N / 4;

	float t;
	size_t startCurve;
	PiecewiseCurveTime(totalTime, c_numCurves, startCurve, t);

	size_t offset = startCurve * 4;

	float s = 1.0f - t;
	return
		coefficients[offset + 0] * s * s * s +
		coefficients[offset + 1] * s * s * t * 3.0f +
		coefficients[offset + 2] * s * t * t * 3.0f +
		coefficients[offset + 3] * t * t * t;
}

template <size_t N, typename LAMBDA>
float EvaluateLinearInterpolation3DCubic (float totalTime, const std::array<float, N>& pixels, LAMBDA& TextureCoordinateToPixelIndex)
{
	const size_t c_numCurves = (N / 4) - 1;

	float t;
	size_t startRow;
	PiecewiseCurveTime(totalTime, c_numCurves, startRow, t);

	//    rowZYX
	float row00x = lerp(t, pixels[TextureCoordinateToPixelIndex(0, startRow + 0, 0)], pixels[TextureCoordinateToPixelIndex(0, startRow + 0, 1)]);
	float row01x = lerp(t, pixels[TextureCoordinateToPixelIndex(0, startRow + 1, 0)], pixels[TextureCoordinateToPixelIndex(0, startRow + 1, 1)]);
	float row0yx = lerp(t, row00x, row01x);

	float row10x = lerp(t, pixels[TextureCoordinateToPixelIndex(1, startRow + 0, 0)], pixels[TextureCoordinateToPixelIndex(1, startRow + 0, 1)]);
	float row11x = lerp(t, pixels[TextureCoordinateToPixelIndex(1, startRow + 1, 0)], pixels[TextureCoordinateToPixelIndex(1, startRow + 1, 1)]);
	float row1yx = lerp(t, row10x, row11x);

	return lerp(t, row0yx, row1yx);
}

template <size_t NUMCURVES>
void Test3DCubic ()
{
	const size_t c_imageWidth = 2;
	const size_t c_imageHeight = NUMCURVES + 1;
	const size_t c_imageDepth = 2;
	const size_t c_numPixels = c_imageWidth * c_imageHeight * c_imageDepth;
	const size_t c_numControlPoints = NUMCURVES * 4;
	const size_t c_numEquations = NUMCURVES * 4;

	// report values for this test
	printf("  %zu curves.  %zu control points.  2x%zux2 texture = %zu pixels.\n", NUMCURVES, c_numControlPoints, c_imageHeight, c_numPixels);
	printf("  %f pixels per curve.  %f pixels per control point.\n", float(c_numPixels) / float(NUMCURVES), float(c_numPixels) / float(c_numControlPoints));

	// lambdas to convert between pixel index and texture coordinates
	auto TextureCoordinateToPixelIndex = [&] (size_t z, size_t y, size_t x) -> size_t
	{
		return TextureCoordinateToPixelIndex3d(c_imageWidth, c_imageHeight, c_imageDepth, z, y, x);
	};
	auto pixelIndexToCoordinates = [&] (size_t pixelIndex, char pixelCoords[10])
	{
		size_t z, y, x;
		PixelIndexToTextureCoordinate3d(c_imageWidth, c_imageHeight, c_imageDepth, pixelIndex, z, y, x);
		sprintf(pixelCoords, "%zu%zu%zu", z,y,x);
	};

	// create the equations
	TMatrix<c_numEquations, c_numPixels + c_numControlPoints> augmentedMatrix;
	for (size_t i = 0; i < c_numEquations; ++i)
	{
		TVector<c_numPixels + c_numControlPoints>& row = augmentedMatrix[i];

		// left side of the equation goes in this zyx coordinate pattern:
		//   000 
		//   001 010 100 
		//   011 101 110
		//   111
		// But, curve index is added to the y index.
		// Also, left side coefficients must add up to 1.
		size_t curveIndex = i / 4;
		switch (i % 4)
		{
			case 0:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 0, 0)] = CRationalNumber(1, 1);
				break;
			}
			case 1:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 0, 1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(0, curveIndex + 1, 0)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 0, 0)] = CRationalNumber(1, 3);
				break;
			}
			case 2:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 1, 1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 0, 1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 1, 0)] = CRationalNumber(1, 3);
				break;
			}
			case 3:
			{
				row[TextureCoordinateToPixelIndex(1, curveIndex + 1, 1)] = CRationalNumber(1, 1);
				break;
			}
		}

		// right side of the equation is identity
		row[c_numPixels + i] = CRationalNumber(1);
	}

	// solve the matrix if possible and print out the equations
	std::unordered_set<size_t> freeVariables;
	if (!SolveMatrixAndPrintEquations(augmentedMatrix, c_numPixels, freeVariables, pixelIndexToCoordinates))
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

		float value1 = EvaluateBernsteinPolynomial3DCubic(t, controlPoints);
		float value2 = EvaluateLinearInterpolation3DCubic(t, pixels, TextureCoordinateToPixelIndex);

		largestDifference = std::max(largestDifference, std::abs(value1 - value2));
	}
	printf("  %i Samples, Largest Error = %f\n\n", EQUALITY_TEST_SAMPLES, largestDifference);
}

void Test3DCubics ()
{
	printf("\nTesting 3D Textures / Cubic Curves\n\n");

	Test3DCubic<1>();
	Test3DCubic<2>();
	Test3DCubic<3>();
	Test3DCubic<4>();

	system("pause");
}

//===================================================================================================================================
//                                         3D Textures / Cubic Curves Multiple Curves
//===================================================================================================================================
//
// Find the limitations of this pattern and show equivalence to Bernstein Polynomials (Bezier Curve Equations). Pattern details below.
//
// This is the same as 3D Textures / Cubic Curves, but there is a second curve stored by flipping x coordinates.
//
//  --- Other details:
//  
//  * control points: 4 * NumCurves.
//  * image width it 2
//  * image depth is 2
//  * image height is 1 + (NumCurves/2).
//  * equations: 4 * NumCurves.  This many rows in the augmented matrix.
//  * augmented matrix columns = num pixels (left columns) + num control points (right columns)
//

template <size_t HALFNUMCURVES>
void Test3DCubicMulti ()
{
	const size_t NUMCURVES = HALFNUMCURVES * 2;
	const size_t c_imageWidth = 2;
	const size_t c_imageHeight = HALFNUMCURVES + 1;
	const size_t c_imageDepth = 2;
	const size_t c_numPixels = c_imageWidth * c_imageHeight * c_imageDepth;
	const size_t c_numControlPoints = NUMCURVES * 4;
	const size_t c_numEquations = NUMCURVES * 4;

	// report values for this test
	printf("  %zu curves.  %zu control points.  2x%zux2 texture = %zu pixels.\n", NUMCURVES, c_numControlPoints, c_imageHeight, c_numPixels);
	printf("  %f pixels per curve.  %f pixels per control point.\n", float(c_numPixels) / float(NUMCURVES), float(c_numPixels) / float(c_numControlPoints));

	// lambdas to convert between pixel index and texture coordinates
	auto TextureCoordinateToPixelIndex = [&] (size_t z, size_t y, size_t x) -> size_t
	{
		return TextureCoordinateToPixelIndex3d(c_imageWidth, c_imageHeight, c_imageDepth, z, y, x);
	};
	auto pixelIndexToCoordinates = [&] (size_t pixelIndex, char pixelCoords[10])
	{
		size_t z, y, x;
		PixelIndexToTextureCoordinate3d(c_imageWidth, c_imageHeight, c_imageDepth, pixelIndex, z, y, x);
		sprintf(pixelCoords, "%zu%zu%zu", z,y,x);
	};

	// create the first set of equations
	TMatrix<c_numEquations, c_numPixels + c_numControlPoints> augmentedMatrix;
	for (size_t i = 0; i < c_numEquations / 2; ++i)
	{
		TVector<c_numPixels + c_numControlPoints>& row = augmentedMatrix[i];

		// left side of the equation goes in this zyx coordinate pattern:
		//   000 
		//   001 010 100 
		//   011 101 110
		//   111
		// But, curve index is added to the y index.
		// Also, left side coefficients must add up to 1.
		size_t curveIndex = i / 4;
		switch (i % 4)
		{
			case 0:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 0, 0)] = CRationalNumber(1, 1);
				break;
			}
			case 1:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 0, 1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(0, curveIndex + 1, 0)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 0, 0)] = CRationalNumber(1, 3);
				break;
			}
			case 2:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 1, 1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 0, 1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 1, 0)] = CRationalNumber(1, 3);
				break;
			}
			case 3:
			{
				row[TextureCoordinateToPixelIndex(1, curveIndex + 1, 1)] = CRationalNumber(1, 1);
				break;
			}
		}

		// right side of the equation is identity
		row[c_numPixels + i] = CRationalNumber(1);
	}

	// create the second set of equations
	for (size_t i = 0; i < c_numEquations / 2; ++i)
	{
		TVector<c_numPixels + c_numControlPoints>& row = augmentedMatrix[i + c_numEquations / 2];

		// left side of the equation goes in this zyx coordinate pattern, which is the same as above but x axis flipped.
		//   001
		//   000 011 101 
		//   010 100 111
		//   110
		// But, curve index is added to the y index.
		// Also, left side coefficients must add up to 1.
		size_t curveIndex = i / 4;
		switch (i % 4)
		{
			case 0:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 0, 1)] = CRationalNumber(1, 1);
				break;
			}
			case 1:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 0, 0)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(0, curveIndex + 1, 1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 0, 1)] = CRationalNumber(1, 3);
				break;
			}
			case 2:
			{
				row[TextureCoordinateToPixelIndex(0, curveIndex + 1, 0)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 0, 0)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(1, curveIndex + 1, 1)] = CRationalNumber(1, 3);
				break;
			}
			case 3:
			{
				row[TextureCoordinateToPixelIndex(1, curveIndex + 1, 0)] = CRationalNumber(1, 1);
				break;
			}
		}

		// right side of the equation is identity
		row[c_numPixels + i + c_numEquations / 2] = CRationalNumber(1);
	}

	// solve the matrix if possible and print out the equations
	std::unordered_set<size_t> freeVariables;
	SolveMatrixAndPrintEquations(augmentedMatrix, c_numPixels, freeVariables, pixelIndexToCoordinates);
}

void Test3DCubicsMulti ()
{
	printf("\nTesting 3D Textures / Cubic Curves with Multiple Curves\n\n");

	Test3DCubicMulti<1>();

	system("pause");
}

//===================================================================================================================================
//                                       3D Textures / Cubic Curves With C0 Continuity
//===================================================================================================================================
//
// Find the limitations of this pattern and show equivalence to Bernstein Polynomials (Bezier Curve Equations). Pattern details below.
//
//  --- For first curve, do:
//
//  P000 P001    P100 P101
//  P010 P011    P110 P111
//
//  P000 = C0                       
//  P001 + P010 + P100 = 3 * C1     
//  P011 + P101 + P110 = 3 * C2     
//  P111 = C3                       
//
//  --- For second curve, do:
//
//  P000 P001    P100 P101
//  P010 P011    P110 P111
//  P020 P021    P120 P121
//
//  P000 = C0                       
//  P001 + P010 + P100 = 3 * C1     
//  P011 + P101 + P110 = 3 * C2     
//  P111 = C3                       
//                       
//  P011 + P110 + P121 = 3 * C4     
//  P010 + P021 + P110 = 3 * C5     
//  P020 = C6     
//
//  --- For third curve, do:
//
//  P000 P001    P100 P101
//  P010 P011    P110 P111
//  P020 P021    P120 P121
//  P030 P031    P130 P131
//
//  P000 = C0                       
//  P001 + P010 + P100 = 3 * C1     
//  P011 + P101 + P110 = 3 * C2     
//  P111 = C3                       
//                       
//  P011 + P110 + P121 = 3 * C4     
//  P010 + P021 + P110 = 3 * C5     
//  P020 = C6     
//
//  P021 + P030 + P120 = 3 * C7     
//  P031 + P121 + P130 = 3 * C8     
//  P131 = C9   
//
//  and so on...
//  each equation is then multiplied by a value so the right side is identity and left side coefficients add up to 1.
//
//  --- Other details:
//  
//  * control points: 1 + 3 * NumCurves.
//  * image width it 2
//  * image depth is 2
//  * image height is 1 + NumCurves.
//  * equations: 1 + 3 * NumCurves.  This many rows in the augmented matrix.
//  * augmented matrix columns = num pixels (left columns) + num control points (right columns)
//

template <size_t N>
float EvaluateBernsteinPolynomial3DCubicC0 (float totalTime, const std::array<float, N>& coefficients)
{
	const size_t c_numCurves = (N-1) / 3;

	float t;
	size_t startCurve;
	PiecewiseCurveTime(totalTime, c_numCurves, startCurve, t);

	size_t offset = startCurve * 3;

	float s = 1.0f - t;
	return
		coefficients[offset + 0] * s * s * s +
		coefficients[offset + 1] * s * s * t * 3.0f +
		coefficients[offset + 2] * s * t * t * 3.0f +
		coefficients[offset + 3] * t * t * t;
}

template <size_t N, typename LAMBDA>
float EvaluateLinearInterpolation3DCubicC0 (float totalTime, const std::array<float, N>& pixels, LAMBDA& TextureCoordinateToPixelIndex)
{
	const size_t c_numCurves = (N / 4) - 1;

	float t;
	size_t startRow;
	PiecewiseCurveTime(totalTime, c_numCurves, startRow, t);

	// Note we flip x and z axis direction every odd row to get the zig zag

	//    rowZYX
	float xzT = (startRow % 2) == 0 ? t : 1.0f - t;
	float row00x = lerp(xzT, pixels[TextureCoordinateToPixelIndex(0, startRow + 0, 0)], pixels[TextureCoordinateToPixelIndex(0, startRow + 0, 1)]);
	float row01x = lerp(xzT, pixels[TextureCoordinateToPixelIndex(0, startRow + 1, 0)], pixels[TextureCoordinateToPixelIndex(0, startRow + 1, 1)]);
	float row0yx = lerp(t, row00x, row01x);

	float row10x = lerp(xzT, pixels[TextureCoordinateToPixelIndex(1, startRow + 0, 0)], pixels[TextureCoordinateToPixelIndex(1, startRow + 0, 1)]);
	float row11x = lerp(xzT, pixels[TextureCoordinateToPixelIndex(1, startRow + 1, 0)], pixels[TextureCoordinateToPixelIndex(1, startRow + 1, 1)]);
	float row1yx = lerp(t, row10x, row11x);

	return lerp(xzT, row0yx, row1yx);
}

template <size_t NUMCURVES>
void Test3DCubicC0 ()
{

	const size_t c_imageWidth = 2;
	const size_t c_imageHeight = NUMCURVES + 1;
	const size_t c_imageDepth = 2;
	const size_t c_numPixels = c_imageWidth * c_imageHeight * c_imageDepth;
	const size_t c_numControlPoints = 1 + NUMCURVES * 3;
	const size_t c_numEquations = 1 + NUMCURVES * 3;

	// report values for this test
	printf("  %zu curves.  %zu control points.  2x%zux2 texture = %zu pixels.\n", NUMCURVES, c_numControlPoints, c_imageHeight, c_numPixels);
	printf("  %f pixels per curve.  %f pixels per control point.\n", float(c_numPixels) / float(NUMCURVES), float(c_numPixels) / float(c_numControlPoints));

	// lambdas to convert between pixel index and texture coordinates
	auto TextureCoordinateToPixelIndex = [&] (size_t z, size_t y, size_t x) -> size_t
	{
		return TextureCoordinateToPixelIndex3d(c_imageWidth, c_imageHeight, c_imageDepth, z, y, x);
	};
	auto pixelIndexToCoordinates = [&] (size_t pixelIndex, char pixelCoords[10])
	{
		size_t z, y, x;
		PixelIndexToTextureCoordinate3d(c_imageWidth, c_imageHeight, c_imageDepth, pixelIndex, z, y, x);
		sprintf(pixelCoords, "%zu%zu%zu", z,y,x);
	};

	// create the equations
	TMatrix<c_numEquations, c_numPixels + c_numControlPoints> augmentedMatrix;
	for (size_t i = 0; i < c_numEquations; ++i)
	{
		TVector<c_numPixels + c_numControlPoints>& row = augmentedMatrix[i];

		// left side of the equation has a pattern like this:
		//   000
		//   001 010 100
		//   011 101 110
		//
		// But, pattern index is added to the y index.
		// Also, the x and z coordinates flip from 0 to 1 on those after each pattern.
		// Also, left side coefficients must add up to 1.
		size_t patternIndex = i / 3;
		size_t xz0 = patternIndex % 2 == 1;
		size_t xz1 = patternIndex % 2 == 0;
		switch (i % 3)
		{
			case 0:
			{
				row[TextureCoordinateToPixelIndex(xz0, patternIndex + 0, xz0)] = CRationalNumber(1, 1);
				break;
			}
			case 1:
			{
				row[TextureCoordinateToPixelIndex(xz0, patternIndex + 0, xz1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(xz0, patternIndex + 1, xz0)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(xz1, patternIndex + 0, xz0)] = CRationalNumber(1, 3);
				break;
			}
			case 2:
			{
				row[TextureCoordinateToPixelIndex(xz0, patternIndex + 1, xz1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(xz1, patternIndex + 0, xz1)] = CRationalNumber(1, 3);
				row[TextureCoordinateToPixelIndex(xz1, patternIndex + 1, xz0)] = CRationalNumber(1, 3);
				break;
			}
		}

		// right side of the equation is identity
		row[c_numPixels + i] = CRationalNumber(1);
	}

	// solve the matrix if possible and print out the equations
	std::unordered_set<size_t> freeVariables;
	if (!SolveMatrixAndPrintEquations(augmentedMatrix, c_numPixels, freeVariables, pixelIndexToCoordinates))
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

		float value1 = EvaluateBernsteinPolynomial3DCubicC0(t, controlPoints);
		float value2 = EvaluateLinearInterpolation3DCubicC0(t, pixels, TextureCoordinateToPixelIndex);

		largestDifference = std::max(largestDifference, std::abs(value1 - value2));
	}
	printf("  %i Samples, Largest Error = %f\n\n", EQUALITY_TEST_SAMPLES, largestDifference);
}

void Test3DCubicsC0 ()
{

	printf("\nTesting 3D Textures / Cubic Curves with C0 continuity\n\n");

	Test3DCubicC0<1>();
	Test3DCubicC0<2>();
	Test3DCubicC0<3>();
	Test3DCubicC0<4>();
	Test3DCubicC0<5>();
	Test3DCubicC0<6>();

	system("pause");
}

//===================================================================================================================================
//                                                                 main
//===================================================================================================================================

int main (int agrc, char **argv)
{
	Test2DQuadratics();
	Test2DQuadraticsC0();
	Test3DCubics();
	Test3DCubicsMulti();
	Test3DCubicsC0();

	return 0;
}