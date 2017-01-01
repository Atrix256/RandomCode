#include <stdio.h>
#include <array>

#define FILTER_ZERO_COEFFICIENTS true // if false, will show terms which have a coefficient of 0

//====================================================================
template<size_t N>
using TVector = std::array<float, N>;

template<size_t M, size_t N>
using TMatrix = std::array<TVector<N>, M>;

//====================================================================
// Specify a degree per axis.
// 1 = linear, 2 = quadratic, etc
template <size_t... DEGREES>
class COnlineLeastSquaresFitter
{
public:
    COnlineLeastSquaresFitter ()
    {
        // initialize our sums to zero
        std::fill(m_SummedPowers.begin(), m_SummedPowers.end(), 0.0f);
        std::fill(m_SummedPowersTimesValues.begin(), m_SummedPowersTimesValues.end(), 0.0f);
    }

	// Calculate how many summed powers we need.
	// Product of degree*2+1 for each axis.
	template <class T>
	constexpr static size_t NumSummedPowers(T degree)
	{
		return degree * 2 + 1;
	}
	template <class T, class... DEGREES>
	constexpr static size_t NumSummedPowers(T first, DEGREES... degrees)
	{
		return NumSummedPowers(first) * NumSummedPowers(degrees...);
	}

	// Calculate how many coefficients we have for our equation.
	// Product of degree+1 for each axis.
	template <class T>
	constexpr static size_t NumCoefficients(T degree)
	{
		return (degree + 1);
	}
	template <class T, class... DEGREES>
	constexpr static size_t NumCoefficients(T first, DEGREES... degrees)
	{
		return NumCoefficients(first) * NumCoefficients(degrees...);
	}

	// Helper function to get degree of specific axis
	static size_t Degree (size_t axisIndex)
	{
		static const std::array<size_t, c_dimension-1> c_degrees = { DEGREES... };
		return c_degrees[axisIndex];
	}
	
	// static const values
	static const size_t c_dimension = sizeof...(DEGREES) + 1; 
	static const size_t c_numCoefficients = NumCoefficients(DEGREES...);
	static const size_t c_numSummedPowers = NumSummedPowers(DEGREES...);

	// Typedefs
	typedef TVector<c_numCoefficients> TCoefficients;
	typedef TVector<c_dimension> TDataPoint;

	// Function for converting from an index to a specific power permutation
	static void IndexToPowers (size_t index, std::array<size_t, c_dimension-1>& powers, size_t maxDegreeMultiply, size_t maxDegreeAdd)
	{
		for (int i = c_dimension-2; i >= 0; --i)
		{
			size_t degree = Degree(i) * maxDegreeMultiply + maxDegreeAdd;
			powers[i] = index % degree;
			index = index / degree;
		}
	}

	// Function for converting from a specific power permuation back into an index
	static size_t PowersToIndex (std::array<size_t, c_dimension - 1>& powers, size_t maxDegreeMultiply, size_t maxDegreeAdd)
	{
		size_t ret = 0;
		for (int i = 0; i < c_dimension - 1; ++i)
		{
			ret *= Degree(i) * maxDegreeMultiply + maxDegreeAdd;
			ret += powers[i];
		}
		return ret;
	}

	// Add a datapoint to our fitting
    void AddDataPoint (const TDataPoint& dataPoint)
    {
		// Note: It'd be a good idea to memoize the powers and calculate them through repeated
		// multiplication, instead of calculating them on demand each time, using std::pow.

        // add the summed powers of the input values
		std::array<size_t, c_dimension-1> powers;
        for (size_t i = 0; i < m_SummedPowers.size(); ++i)
        {
			IndexToPowers(i, powers, 2, 1);
			float valueAdd = 1.0;
			for (size_t j = 0; j < c_dimension - 1; ++j)
				valueAdd *= (float)std::pow(dataPoint[j], powers[j]);
			m_SummedPowers[i] += valueAdd;
        }

        // add the summed powers of the input value, multiplied by the output value
        for (size_t i = 0; i < m_SummedPowersTimesValues.size(); ++i)
        {
			IndexToPowers(i, powers, 1, 1);
			float valueAdd = dataPoint[c_dimension - 1];
			for (size_t j = 0; j < c_dimension-1; ++j)
				valueAdd *= (float)std::pow(dataPoint[j], powers[j]);
			m_SummedPowersTimesValues[i] += valueAdd;
        }
    }

	// Get the coefficients of the equation fit to the points
    bool CalculateCoefficients (TCoefficients& coefficients) const
    {
		// make the ATA matrix
		std::array<size_t, c_dimension - 1> powersi;
		std::array<size_t, c_dimension - 1> powersj;
		std::array<size_t, c_dimension - 1> summedPowers;
		TMatrix<c_numCoefficients, c_numCoefficients> ATA;
		for (size_t j = 0; j < c_numCoefficients; ++j)
		{
			IndexToPowers(j, powersj, 1, 1);

			for (size_t i = 0; i < c_numCoefficients; ++i)
			{
				IndexToPowers(i, powersi, 1, 1);

				for (size_t k = 0; k < c_dimension - 1; ++k)
					summedPowers[k] = powersi[k] + powersj[k];

				size_t summedPowersIndex = PowersToIndex(summedPowers, 2, 1);
				ATA[j][i] = m_SummedPowers[summedPowersIndex];
			}
		}

		// solve: ATA * coefficients = m_SummedPowers
		// for the coefficients vector, using Gaussian elimination.
		coefficients = m_SummedPowersTimesValues;
		for (size_t i = 0; i < c_numCoefficients; ++i)
		{
			for (size_t j = 0; j < c_numCoefficients; ++j)
			{
				if (ATA[i][i] == 0.0f)
					return false;

				float c = ((i == j) - ATA[j][i]) / ATA[i][i];
				coefficients[j] += c*coefficients[i];
				for (size_t k = 0; k < c_numCoefficients; ++k)
					ATA[j][k] += c*ATA[i][k];
			}
		}

		// Note: this is the old, "bad" way using matrix inversion. It's a worse choice for larger matrices and
		// surfaces and volumes definitely use larger matrices.
		/*
		// Inverse the ATA matrix
		TMatrix<c_numCoefficients, c_numCoefficients> ATAInverse;
		if (!InvertMatrix(ATA, ATAInverse))
			return false;

		// calculate the coefficients
		for (size_t i = 0; i < c_numCoefficients; ++i)
			coefficients[i] = DotProduct(ATAInverse[i], m_SummedPowersTimesValues);
		*/

		return true;
    }

private:
	//Storage Requirements:
	// Summed Powers = Product of degree*2+1 for each axis.
	// Summed Powers Times Values = Product of degree+1 for each axis.
    TVector<c_numSummedPowers>		m_SummedPowers;
	TVector<c_numCoefficients>		m_SummedPowersTimesValues;
};

//====================================================================
char AxisIndexToLetter (size_t axisIndex)
{
	// x,y,z,w,v,u,t,....
	if (axisIndex < 3)
		return 'x' + char(axisIndex);
	else
		return 'x' + 2 - char(axisIndex);
}

//====================================================================
template <class T, size_t M, size_t N>
float EvaluateFunction (const T& fitter, const TVector<M>& dataPoint, const TVector<N>& coefficients)
{
	float ret = 0.0f;
	for (size_t i = 0; i < coefficients.size(); ++i)
	{
		// start with the coefficient
		float term = coefficients[i];

		// then the powers of the input variables
		std::array<size_t, T::c_dimension - 1> powers;
		fitter.IndexToPowers(i, powers, 1, 1);
		for (size_t j = 0; j < powers.size(); ++j)
			term *= (float)std::pow(dataPoint[j], powers[j]);

		// add this term to our return value
		ret += term;
	}
	return ret;
}

//====================================================================
template <size_t... DEGREES>
void DoTest (const std::initializer_list<TVector<sizeof...(DEGREES)+1>>& data)
{
	// say what we are are going to do
	printf("Fitting a function of degree (");
	for (size_t i = 0; i < COnlineLeastSquaresFitter<DEGREES...>::c_dimension - 1; ++i)
	{
		if (i > 0)
			printf(",");
		printf("%zi", COnlineLeastSquaresFitter<DEGREES...>::Degree(i));
	}
	printf(") to %zi data points: \n", data.size());

	// show input data points
	for (const COnlineLeastSquaresFitter<DEGREES...>::TDataPoint& dataPoint : data)
	{
		printf("  (");
		for (size_t i = 0; i < dataPoint.size(); ++i)
		{
			if (i > 0)
				printf(", ");
			printf("%0.2f", dataPoint[i]);
		}
		printf(")\n");
	}

	// fit data
	COnlineLeastSquaresFitter<DEGREES...> fitter;
    for (const COnlineLeastSquaresFitter<DEGREES...>::TDataPoint& dataPoint : data)
        fitter.AddDataPoint(dataPoint);

	// calculate coefficients if we can
	COnlineLeastSquaresFitter<DEGREES...>::TCoefficients coefficients;
	bool success = fitter.CalculateCoefficients(coefficients);
	if (!success)
	{
		printf("Could not calculate coefficients!\n\n");
		return;
	}

	// print the polynomial
	bool firstTerm = true;
	printf("%c = ", AxisIndexToLetter(sizeof...(DEGREES)));
    bool showedATerm = false;
	for (int i = (int)coefficients.size() - 1; i >= 0; --i)
	{
		// don't show zero terms
		if (FILTER_ZERO_COEFFICIENTS && std::abs(coefficients[i]) < 0.00001f)
			continue;

        showedATerm = true;

		// show an add or subtract between terms
		float coefficient = coefficients[i];
		if (firstTerm)
			firstTerm = false;
		else if (coefficient >= 0.0f)
			printf(" + ");
		else
		{
			coefficient *= -1.0f;
			printf(" - ");
		}

		printf("%0.2f", coefficient);

		std::array<size_t, COnlineLeastSquaresFitter<DEGREES...>::c_dimension - 1> powers;
		fitter.IndexToPowers(i, powers, 1, 1);

		for (size_t j = 0; j < powers.size(); ++j)
		{
			if (powers[j] > 0)
				printf("%c", AxisIndexToLetter(j));
			if (powers[j] > 1)
				printf("^%zi", powers[j]);
		}
	}
    if (!showedATerm)
        printf("0");
	printf("\n");

	// Calculate and show R^2 value.
	float rSquared = 1.0f;
	if (data.size() > 0)
	{
		float mean = 0.0f;
		for (const COnlineLeastSquaresFitter<DEGREES...>::TDataPoint& dataPoint : data)
			mean += dataPoint[sizeof...(DEGREES)];
		mean /= data.size();
		float SSTot = 0.0f;
		float SSRes = 0.0f;
		for (const COnlineLeastSquaresFitter<DEGREES...>::TDataPoint& dataPoint : data)
		{
			float value = dataPoint[sizeof...(DEGREES)] - mean;
			SSTot += value*value;

			value = dataPoint[sizeof...(DEGREES)] - EvaluateFunction(fitter, dataPoint, coefficients);
			SSRes += value*value;
		}
		if (SSTot != 0.0f)
			rSquared = 1.0f - SSRes / SSTot;
	}
	printf("R^2 = %0.4f\n\n", rSquared);
}

//====================================================================
int main (int argc, char **argv)
{
	// bilinear - 4 data points
	DoTest<1, 1>(
		{
			TVector<3>{ 0.0f, 0.0f, 5.0f },
			TVector<3>{ 0.0f, 1.0f, 3.0f },
			TVector<3>{ 1.0f, 0.0f, 8.0f },
			TVector<3>{ 1.0f, 1.0f, 2.0f },
		}
	);

	// biquadratic - 9 data points
	DoTest<2, 2>(
		{
			TVector<3>{ 0.0f, 0.0f, 8.0f },
			TVector<3>{ 0.0f, 1.0f, 4.0f },
			TVector<3>{ 0.0f, 2.0f, 6.0f },
			TVector<3>{ 1.0f, 0.0f, 5.0f },
			TVector<3>{ 1.0f, 1.0f, 2.0f },
			TVector<3>{ 1.0f, 2.0f, 1.0f },
			TVector<3>{ 2.0f, 0.0f, 7.0f },
			TVector<3>{ 2.0f, 1.0f, 9.0f },
			TVector<3>{ 2.0f, 2.5f, 12.0f },
		}
	);

	// trilinear - 8 data points
	DoTest<1, 1, 1>(
		{
			TVector<4>{ 0.0f, 0.0f, 0.0f, 8.0f },
			TVector<4>{ 0.0f, 0.0f, 1.0f, 4.0f },
			TVector<4>{ 0.0f, 1.0f, 0.0f, 6.0f },
			TVector<4>{ 0.0f, 1.0f, 1.0f, 5.0f },
			TVector<4>{ 1.0f, 0.0f, 0.0f, 2.0f },
			TVector<4>{ 1.0f, 0.0f, 1.0f, 1.0f },
			TVector<4>{ 1.0f, 1.0f, 0.0f, 7.0f },
			TVector<4>{ 1.0f, 1.0f, 1.0f, 9.0f },
		}
	);

	// trilinear - 9 data points
	DoTest<1, 1, 1>(
		{
			TVector<4>{ 0.0f, 0.0f, 0.0f, 8.0f },
			TVector<4>{ 0.0f, 0.0f, 1.0f, 4.0f },
			TVector<4>{ 0.0f, 1.0f, 0.0f, 6.0f },
			TVector<4>{ 0.0f, 1.0f, 1.0f, 5.0f },
			TVector<4>{ 1.0f, 0.0f, 0.0f, 2.0f },
			TVector<4>{ 1.0f, 0.0f, 1.0f, 1.0f },
			TVector<4>{ 1.0f, 1.0f, 0.0f, 7.0f },
			TVector<4>{ 1.0f, 1.0f, 1.0f, 9.0f },
			TVector<4>{ 0.5f, 0.5f, 0.5f, 12.0f },
		}
	);

	// Linear - 2 data points
    DoTest<1>(
        {
            TVector<2>{ 1.0f, 2.0f },
            TVector<2>{ 2.0f, 4.0f },
        }
    );

	// Quadratic - 4 data points
    DoTest<2>(
        {
            TVector<2>{ 1.0f, 5.0f },
			TVector<2>{ 2.0f, 16.0f },
			TVector<2>{ 3.0f, 31.0f },
			TVector<2>{ 4.0f, 16.0f },
        }
    );

	// Cubic - 4 data points
    DoTest<3>(
        {
            TVector<2>{ 1.0f, 5.0f },
            TVector<2>{ 2.0f, 16.0f },
			TVector<2>{ 3.0f, 31.0f },
			TVector<2>{ 4.0f, 16.0f },
        }
    );

    system("pause");
    return 0;
}

/*

This Post:
* talk about R^2 calculation
* Mention that this code doesn't fall back if it doesn't have enough points.
 * Say that there may or may not be a good way to fall back to either lower degrees or lower dimensions, but I didn't look into it.
* mention how you do gauss elimination instead of matrix inversion which is more numerically stable, and faster. point out that the examples are slightly different, but this is more correct.

Last Demo:
* Better matrix solving. see if it helps with numerical issues (it should!)
* if it works better (wikipedia said it should be better numerically), let some higher degrees in again!

WegGL demo:
* Start with the surface demo from before
* make it able to take degree on each axis and coefficients
* ray march the function
* calculate partial derivatives for normal (maybe not even needed to do it numerically?)
* do a draw per data point. Show data points different color when they are above or below surface.
* show R^2 and equation calculated.
* better matrix solving

*/