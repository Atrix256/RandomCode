#include <stdio.h>
#include <array>

//====================================================================
template<size_t N>
using TVector = std::array<float, N>;

template<size_t M, size_t N>
using TMatrix = std::array<TVector<N>, M>;

template<size_t N>
using TSquareMatrix = TMatrix<N,N>;

typedef TVector<2> TDataPoint; 

//====================================================================
template <size_t N>
float DotProduct (const TVector<N>& A, const TVector<N>& B)
{
    float ret = 0.0f;
    for (size_t i = 0; i < N; ++i)
        ret += A[i] * B[i];
    return ret;
}

//====================================================================
template <size_t M, size_t N>
void TransposeMatrix (const TMatrix<M, N>& in, TMatrix<N, M>& result)
{
    for (size_t j = 0; j < M; ++j)
        for (size_t k = 0; k < N; ++k)
            result[k][j] = in[j][k];
}

//====================================================================
template <size_t M, size_t N>
void MinorMatrix (const TMatrix<M, N>& in, TMatrix<M-1, N-1>& out, size_t excludeI, size_t excludeJ)
{
    size_t destI = 0;
    for (size_t i = 0; i < N; ++i)
    {
        if (i != excludeI)
        {
            size_t destJ = 0;
            for (size_t j = 0; j < N; ++j)
            {
                if (j != excludeJ)
                {
                    out[destI][destJ] = in[i][j];
                    ++destJ;
                }
            }
            ++destI;
        }
    }
}

//====================================================================
template <size_t M, size_t N>
float Determinant (const TMatrix<M,N>& in)
{
    float determinant = 0.0f;
	TMatrix<M - 1, N - 1> minor;
    for (size_t j = 0; j < N; ++j)
    {
        MinorMatrix(in, minor, 0, j);

        float minorDeterminant = Determinant(minor);
        if (j % 2 == 1)
            minorDeterminant *= -1.0f;

        determinant += in[0][j] * minorDeterminant;
    }
    return determinant;
}

//====================================================================
template <>
float Determinant<1> (const TMatrix<1,1>& in)
{
	return in[0][0];
}

//====================================================================
template <size_t N>
bool InvertMatrix (const TSquareMatrix<N>& in, TSquareMatrix<N>& out)
{
    // calculate the cofactor matrix and determinant
    float determinant = 0.0f;
    TSquareMatrix<N> cofactors;
    TSquareMatrix<N-1> minor;
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            MinorMatrix(in, minor, i, j);

            cofactors[i][j] = Determinant(minor);
            if ((i + j) % 2 == 1)
                cofactors[i][j] *= -1.0f;

            if (i == 0)
                determinant += in[i][j] * cofactors[i][j];
        }
    }

    // matrix cant be inverted if determinant is zero
    if (determinant == 0.0f)
        return false;

    // calculate the adjoint matrix into the out matrix
    TransposeMatrix(cofactors, out);

    // divide by determinant
    float oneOverDeterminant = 1.0f / determinant;
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            out[i][j] *= oneOverDeterminant;
    return true;
}

//====================================================================
template <>
bool InvertMatrix<2> (const TSquareMatrix<2>& in, TSquareMatrix<2>& out)
{
    float determinant = Determinant(in);
    if (determinant == 0.0f)
        return false;

    float oneOverDeterminant = 1.0f / determinant;
    out[0][0] =  in[1][1] * oneOverDeterminant;
    out[0][1] = -in[0][1] * oneOverDeterminant;
    out[1][0] = -in[1][0] * oneOverDeterminant;
    out[1][1] =  in[0][0] * oneOverDeterminant;
    return true;
}

//====================================================================
template <size_t DEGREE>  // 1 = linear, 2 = quadratic, etc
class COnlineLeastSquaresFitter
{
public:
    COnlineLeastSquaresFitter ()
    {
        // initialize our sums to zero
        std::fill(m_SummedPowersX.begin(), m_SummedPowersX.end(), 0.0f);
        std::fill(m_SummedPowersXTimesY.begin(), m_SummedPowersXTimesY.end(), 0.0f);
    }

    void AddDataPoint (const TDataPoint& dataPoint)
    {
        // add the summed powers of the x value
        for (size_t i = 0; i < m_SummedPowersX.size(); ++i)
            m_SummedPowersX[i] += std::pow(dataPoint[0], float(i));

        // add the summed powers of the x value, multiplied by the y value
        for (size_t i = 0; i < m_SummedPowersXTimesY.size(); ++i)
            m_SummedPowersXTimesY[i] += std::pow(dataPoint[0], float(i)) * dataPoint[1];
    }

    bool CalculateCoefficients (TVector<DEGREE+1>& coefficients) const
    {
        // initialize all coefficients to zero
        std::fill(coefficients.begin(), coefficients.end(), 0.0f);

        // calculate the coefficients
        return CalculateCoefficientsInternal<DEGREE>(coefficients);
    }

private:

    template <size_t EFFECTIVEDEGREE>
    bool CalculateCoefficientsInternal (TVector<DEGREE + 1>& coefficients) const
    {
        // if we don't have enough data points for this degree, try one degree less
        if (m_SummedPowersX[0] <= EFFECTIVEDEGREE)
            return CalculateCoefficientsInternal<EFFECTIVEDEGREE - 1>(coefficients);

        // Make the ATA matrix
        TMatrix<EFFECTIVEDEGREE + 1, EFFECTIVEDEGREE + 1> ATA;
        for (size_t i = 0; i < EFFECTIVEDEGREE + 1; ++i)
            for (size_t j = 0; j < EFFECTIVEDEGREE + 1; ++j)
                ATA[i][j] = m_SummedPowersX[i + j];

        // calculate inverse of ATA matrix
        TMatrix<EFFECTIVEDEGREE + 1, EFFECTIVEDEGREE + 1> ATAInverse;
        if (!InvertMatrix(ATA, ATAInverse))
            return false;

        // calculate the coefficients for this degree. The higher ones are already zeroed out.
        TVector<EFFECTIVEDEGREE + 1> summedPowersXTimesY;
        std::copy(m_SummedPowersXTimesY.begin(), m_SummedPowersXTimesY.begin() + EFFECTIVEDEGREE + 1, summedPowersXTimesY.begin());
        for (size_t i = 0; i < EFFECTIVEDEGREE + 1; ++i)
            coefficients[i] = DotProduct(ATAInverse[i], summedPowersXTimesY);
        return true;
    }

    // Base case when no points are given, or if you are fitting a degree 0 curve to the data set.
    template <>
    bool CalculateCoefficientsInternal<0> (TVector<DEGREE + 1>& coefficients) const
    {
        if (m_SummedPowersX[0] > 0.0f)
            coefficients[0] = m_SummedPowersXTimesY[0] / m_SummedPowersX[0];
        return true;
    }

    // Total storage space (# of floats) needed is 3 * DEGREE + 2
    // Where y is number of values that need to be stored and x is the degree of the polynomial
    TVector<(DEGREE + 1) * 2 - 1>   m_SummedPowersX;
    TVector<DEGREE + 1>             m_SummedPowersXTimesY;
};

//====================================================================
template <size_t DEGREE>
void DoTest(const std::initializer_list<TDataPoint>& data)
{
	printf("Fitting a curve of degree %zi to %zi data points:\n", DEGREE, data.size());

    COnlineLeastSquaresFitter<DEGREE> fitter;

	// show data
    for (const TDataPoint& dataPoint : data)
		printf("  (%0.2f, %0.2f)\n", dataPoint[0], dataPoint[1]);

	// fit data
    for (const TDataPoint& dataPoint : data)
        fitter.AddDataPoint(dataPoint);

	// calculate coefficients if we can
	TVector<DEGREE+1> coefficients;
	bool success = fitter.CalculateCoefficients(coefficients);
	if (!success)
	{
		printf("ATA Matrix could not be inverted!\n");
		return;
	}

	// print the polynomial
	bool firstTerm = true;
	printf("y = ");
    bool showedATerm = false;
	for (int i = (int)coefficients.size() - 1; i >= 0; --i)
	{
		// don't show zero terms
		if (std::abs(coefficients[i]) < 0.00001f)
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

		if (i > 0)
			printf("x");

		if (i > 1)
			printf("^%i", i);
	}
    if (!showedATerm)
        printf("0");
	printf("\n\n");
}

//====================================================================
int main (int argc, char **argv)
{
	// Point - 1 data points
    DoTest<0>(
        {
            TDataPoint{ 1.0f, 2.0f },
        }
    );

	// Point - 2 data points
    DoTest<0>(
        {
            TDataPoint{ 1.0f, 2.0f },
            TDataPoint{ 2.0f, 4.0f },
        }
    );

	// Linear - 2 data points
    DoTest<1>(
        {
            TDataPoint{ 1.0f, 2.0f },
            TDataPoint{ 2.0f, 4.0f },
        }
    );

	// Linear - 3 colinear data points
    DoTest<1>(
        {
            TDataPoint{ 1.0f, 2.0f },
            TDataPoint{ 2.0f, 4.0f },
            TDataPoint{ 3.0f, 6.0f },
        }
    );

	// Linear - 3 non colinear data points
    DoTest<1>(
        {
            TDataPoint{ 1.0f, 2.0f },
            TDataPoint{ 2.0f, 4.0f },
            TDataPoint{ 3.0f, 5.0f },
        }
    );

	// Quadratic - 3 colinear data points
    DoTest<2>(
        {
            TDataPoint{ 1.0f, 2.0f },
            TDataPoint{ 2.0f, 4.0f },
            TDataPoint{ 3.0f, 6.0f },
        }
    );

	// Quadratic - 3 data points
    DoTest<2>(
        {
            TDataPoint{ 1.0f, 5.0f },
            TDataPoint{ 2.0f, 16.0f },
            TDataPoint{ 3.0f, 31.0f },
        }
    );

	// Cubic - 4 data points
    DoTest<3>(
        {
            TDataPoint{ 1.0f, 5.0f },
            TDataPoint{ 2.0f, 16.0f },
            TDataPoint{ 3.0f, 31.0f },
            TDataPoint{ 4.0f, 16.0f },
        }
    );

	// Cubic - 2 data points
    DoTest<3>(
        {
            TDataPoint{ 1.0f, 7.0f },
            TDataPoint{ 3.0f, 17.0f },
        }
    );

	// Cubic - 1 data point
    DoTest<3>(
        {
            TDataPoint{ 1.0f, 7.0f },
        }
    );

	// Cubic - 0 data points
    DoTest<3>(
        {
        }
    );

    system("pause");
    return 0;
}
