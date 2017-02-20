#include <stdio.h>
#include <cmath>
#include <array>
#include <algorithm>
 
#define PI 3.14159265359f

#define EPSILON 0.001f  // for numeric derivatives calculation

template <size_t NUMVARIABLES>
class CDualNumber
{
public:

	// constructor to make a constant
	CDualNumber (float f = 0.0f) {
		m_real = f;
		std::fill(m_dual.begin(), m_dual.end(), 0.0f);
	}

	// constructor to make a variable value.  It sets the derivative to 1.0 for whichever variable this is a value for.
	CDualNumber (float f, size_t variableIndex) {
		m_real = f;
		std::fill(m_dual.begin(), m_dual.end(), 0.0f);
		m_dual[variableIndex] = 1.0f;
	}

	// storage for real and dual values
	float							m_real;
	std::array<float, NUMVARIABLES> m_dual;
};
 
//----------------------------------------------------------------------
// Math Operations
//----------------------------------------------------------------------
template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> operator + (const CDualNumber<NUMVARIABLES> &a, const CDualNumber<NUMVARIABLES> &b)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = a.m_real + b.m_real;
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = a.m_dual[i] + b.m_dual[i];
	return ret;
}
 
template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> operator - (const CDualNumber<NUMVARIABLES> &a, const CDualNumber<NUMVARIABLES> &b)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = a.m_real - b.m_real;
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = a.m_dual[i] - b.m_dual[i];
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> operator * (const CDualNumber<NUMVARIABLES> &a, const CDualNumber<NUMVARIABLES> &b)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = a.m_real * b.m_real;
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = a.m_real * b.m_dual[i] + a.m_dual[i] * b.m_real;
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> operator / (const CDualNumber<NUMVARIABLES> &a, const CDualNumber<NUMVARIABLES> &b)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = a.m_real / b.m_real;
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = (a.m_dual[i] * b.m_real - a.m_real * b.m_dual[i]) / (b.m_real * b.m_real);
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> sqrt (const CDualNumber<NUMVARIABLES> &a)
{
	CDualNumber<NUMVARIABLES> ret;
	float sqrtReal = sqrt(a.m_real);
	ret.m_real = sqrtReal;
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = 0.5f * a.m_dual[i] / sqrtReal;
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> pow (const CDualNumber<NUMVARIABLES> &a, float y)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = pow(a.m_real, y);
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = y * a.m_dual[i] * pow(a.m_real, y - 1.0f);
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> sin (const CDualNumber<NUMVARIABLES> &a)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = sin(a.m_real);
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = a.m_dual[i] * cos(a.m_real);
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> cos (const CDualNumber<NUMVARIABLES> &a)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = cos(a.m_real);
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = -a.m_dual[i] * sin(a.m_real);
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> tan (const CDualNumber<NUMVARIABLES> &a)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = tan(a.m_real);
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = a.m_dual[i] / (cos(a.m_real) * cos(a.m_real));
	return ret;
}

template <size_t NUMVARIABLES>
inline CDualNumber<NUMVARIABLES> atan (const CDualNumber<NUMVARIABLES> &a)
{
	CDualNumber<NUMVARIABLES> ret;
	ret.m_real = tan(a.m_real);
	for (size_t i = 0; i < NUMVARIABLES; ++i)
		ret.m_dual[i] = a.m_dual[i] / (1.0f + a.m_real * a.m_real);
	return ret;
}

// templated so it can work for both a CDualNumber<1> and a float
template <typename T>
inline T SmoothStep (const T& x)
{
	return x * x * (T(3.0f) - T(2.0f) * x);
}
 
//----------------------------------------------------------------------
// Test Functions
//----------------------------------------------------------------------
 
void TestSmoothStep (float input)
{
	// create a dual number as the value of x
	CDualNumber<1> x(input, 0);

	// calculate value and derivative using dual numbers
	CDualNumber<1> y = SmoothStep(x);

	// calculate numeric derivative using central differences
	float derivNumeric = (SmoothStep(input + EPSILON) - SmoothStep(input - EPSILON)) / (2.0f * EPSILON);

	// calculate actual derivative
	float derivActual = 6.0f * input - 6.0f * input * input;

	// show value and derivatives
	printf("(smoothstep) y=3x^2-2x^3  (x=%0.4f)\n", input);
	printf("  y = %0.4f\n", y.m_real);
	printf("  dual# dy/dx = %0.4f\n", y.m_dual[0]);
	printf("  actual dy/dx = %0.4f\n", derivActual);
	printf("  numeric dy/dx = %0.4f\n\n", derivNumeric);
}

void TestTrig (float input)
{
	// create a dual number as the value of x
	CDualNumber<1> x(input, 0);

	// sin
	{
		// calculate value and derivative using dual numbers
		CDualNumber<1> y = sin(x);

		// calculate numeric derivative using central differences
		float derivNumeric = (sin(input + EPSILON) - sin(input - EPSILON)) / (2.0f * EPSILON);

		// calculate actual derivative
		float derivActual = cos(input);

		// show value and derivatives
		printf("sin(%0.4f) = %0.4f\n", input, y.m_real);
		printf("  dual# dy/dx = %0.4f\n", y.m_dual[0]);
		printf("  actual dy/dx = %0.4f\n", derivActual);
		printf("  numeric dy/dx = %0.4f\n\n", derivNumeric);
	}

	// cos
	{
		// calculate value and derivative using dual numbers
		CDualNumber<1> y = cos(x);

		// calculate numeric derivative using central differences
		float derivNumeric = (cos(input + EPSILON) - cos(input - EPSILON)) / (2.0f * EPSILON);

		// calculate actual derivative
		float derivActual = -sin(input);

		// show value and derivatives
		printf("cos(%0.4f) = %0.4f\n", input, y.m_real);
		printf("  dual# dy/dx = %0.4f\n", y.m_dual[0]);
		printf("  actual dy/dx = %0.4f\n", derivActual);
		printf("  numeric dy/dx = %0.4f\n\n", derivNumeric);
	}

	// tan
	{
		// calculate value and derivative using dual numbers
		CDualNumber<1> y = tan(x);

		// calculate numeric derivative using central differences
		float derivNumeric = (tan(input + EPSILON) - tan(input - EPSILON)) / (2.0f * EPSILON);

		// calculate actual derivative
		float derivActual = 1.0f / (cos(input)*cos(input));

		// show value and derivatives
		printf("tan(%0.4f) = %0.4f\n", input, y.m_real);
		printf("  dual# dy/dx = %0.4f\n", y.m_dual[0]);
		printf("  actual dy/dx = %0.4f\n", derivActual);
		printf("  numeric dy/dx = %0.4f\n\n", derivNumeric);
	}

	// atan
	{
		// calculate value and derivative using dual numbers
		CDualNumber<1> y = atan(x);

		// calculate numeric derivative using central differences
		float derivNumeric = (atan(input + EPSILON) - atan(input - EPSILON)) / (2.0f * EPSILON);

		// calculate actual derivative
		float derivActual = 1.0f / (1.0f + input * input);

		// show value and derivatives
		printf("atan(%0.4f) = %0.4f\n", input, y.m_real);
		printf("  dual# dy/dx = %0.4f\n", y.m_dual[0]);
		printf("  actual dy/dx = %0.4f\n", derivActual);
		printf("  numeric dy/dx = %0.4f\n\n", derivNumeric);
	}
}

void TestSimple (float input)
{
	// create a dual number as the value of x
	CDualNumber<1> x(input, 0);

	// sqrt
	{
		// calculate value and derivative using dual numbers
		CDualNumber<1> y = CDualNumber<1>(3.0f) / sqrt(x);

		// calculate numeric derivative using central differences
		float derivNumeric = ((3.0f / sqrt(input + EPSILON)) - (3.0f / sqrt(input - EPSILON))) / (2.0f * EPSILON);

		// calculate actual derivative
		float derivActual = -3.0f / (2.0f * pow(input, 3.0f / 2.0f));

		// show value and derivatives
		printf("3/sqrt(%0.4f) = %0.4f\n", input, y.m_real);
		printf("  dual# dy/dx = %0.4f\n", y.m_dual[0]);
		printf("  actual dy/dx = %0.4f\n", derivActual);
		printf("  numeric dy/dx = %0.4f\n\n", derivNumeric);
	}

	// pow
	{
		// calculate value and derivative using dual numbers
		CDualNumber<1> y = pow(x + CDualNumber<1>(1.0f), 1.337f);

		// calculate numeric derivative using central differences
		float derivNumeric = ((pow(input + 1.0f + EPSILON, 1.337f)) - (pow(input + 1.0f - EPSILON, 1.337f))) / (2.0f * EPSILON);

		// calculate actual derivative
		float derivActual = 1.337f * pow(input + 1.0f, 0.337f);

		// show value and derivatives
		printf("(%0.4f+1)^1.337 = %0.4f\n", input, y.m_real);
		printf("  dual# dy/dx = %0.4f\n", y.m_dual[0]);
		printf("  actual dy/dx = %0.4f\n", derivActual);
		printf("  numeric dy/dx = %0.4f\n\n", derivNumeric);
	}
}

void Test2D (float inputx, float inputy)
{
	// create dual numbers as the value of x and y
	CDualNumber<2> x(inputx, 0);
	CDualNumber<2> y(inputy, 1);

	// z = 3x^2 - 2y^3
	{
		// calculate value and partial derivatives using dual numbers
		CDualNumber<2> z = CDualNumber<2>(3.0f) * x * x - CDualNumber<2>(2.0f) * y * y * y;

		// calculate numeric partial derivatives using central differences
		auto f = [] (float x, float y) {
			return 3.0f * x * x - 2.0f * y * y * y;
		};
		float derivNumericX = (f(inputx + EPSILON, inputy) - f(inputx - EPSILON, inputy)) / (2.0f * EPSILON);
		float derivNumericY = (f(inputx, inputy + EPSILON) - f(inputx, inputy - EPSILON)) / (2.0f * EPSILON);

		// calculate actual partial derivatives
		float derivActualX = 6.0f * inputx;
		float derivActualY = -6.0f * inputy * inputy;

		// show value and derivatives
		printf("z=3x^2-2y^3 (x = %0.4f, y = %0.4f)\n", inputx, inputy);
		printf("  z = %0.4f\n", z.m_real);
		printf("  dual# dz/dx = %0.4f\n", z.m_dual[0]);
		printf("  dual# dz/dy = %0.4f\n", z.m_dual[1]);
		printf("  actual dz/dx = %0.4f\n", derivActualX);
		printf("  actual dz/dy = %0.4f\n", derivActualY);
		printf("  numeric dz/dx = %0.4f\n", derivNumericX);
		printf("  numeric dz/dy = %0.4f\n\n", derivNumericY);
	}
}

void Test3D (float inputx, float inputy, float inputz)
{
	// create dual numbers as the value of x and y
	CDualNumber<3> x(inputx, 0);
	CDualNumber<3> y(inputy, 1);
	CDualNumber<3> z(inputz, 2);

	// w = sin(x*cos(2*y)) / tan(z)
	{
		// calculate value and partial derivatives using dual numbers
		CDualNumber<3> w = sin(x * cos(CDualNumber<3>(2.0f)*y)) / tan(z);

		// calculate numeric partial derivatives using central differences
		auto f = [] (float x, float y, float z) {
			return sin(x*cos(2.0f*y)) / tan(z);
		};
		float derivNumericX = (f(inputx + EPSILON, inputy, inputz) - f(inputx - EPSILON, inputy, inputz)) / (2.0f * EPSILON);
		float derivNumericY = (f(inputx, inputy + EPSILON, inputz) - f(inputx, inputy - EPSILON, inputz)) / (2.0f * EPSILON);
		float derivNumericZ = (f(inputx, inputy, inputz + EPSILON) - f(inputx, inputy, inputz - EPSILON)) / (2.0f * EPSILON);

		// calculate actual partial derivatives
		float derivActualX = cos(inputx*cos(2.0f*inputy))*cos(2.0f * inputy) / tan(inputz);
		float derivActualY = cos(inputx*cos(2.0f*inputy)) *-2.0f*inputx*sin(2.0f*inputy) / tan(inputz);
		float derivActualZ = sin(inputx * cos(2.0f * inputy)) / -(sin(inputz) * sin(inputz));

		// show value and derivatives
		printf("w=sin(x*cos(2*y))/tan(z) (x = %0.4f, y = %0.4f, z = %0.4f)\n", inputx, inputy, inputz);
		printf("  w = %0.4f\n", w.m_real);
		printf("  dual# dw/dx = %0.4f\n", w.m_dual[0]);
		printf("  dual# dw/dy = %0.4f\n", w.m_dual[1]);
		printf("  dual# dw/dz = %0.4f\n", w.m_dual[2]);
		printf("  actual dw/dx = %0.4f\n", derivActualX);
		printf("  actual dw/dy = %0.4f\n", derivActualY);
		printf("  actual dw/dz = %0.4f\n", derivActualZ);
		printf("  numeric dw/dx = %0.4f\n", derivNumericX);
		printf("  numeric dw/dy = %0.4f\n", derivNumericY);
		printf("  numeric dw/dz = %0.4f\n\n", derivNumericZ);
	}
}

int main (int argc, char **argv)
{
	TestSmoothStep(0.5f);
	TestSmoothStep(0.75f);
	TestTrig(PI * 0.25f);
	TestSimple(3.0f);
	Test2D(1.5f, 3.28f);
	Test3D(7.12f, 8.93f, 12.01f);
    return 0;
}

/*

BLOG:
* the way to get partial derivatives is to have a dual number per variable.
 * The real number work is duplicated though, so might as well make a big vector
 * alternate way of looking at it: an epsilon per variable, keeping in mind that multiplying two different epsilons together still equals zero. (ends up being equivelant but more computationally expensive)
* link to dual numbers post
* link to finite differences post
* note that numeric derivatives are less accurate! Dual numbers are NOT a numeric method!

*/