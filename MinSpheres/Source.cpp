#define _CRT_SECURE_NO_WARNINGS

#include <random>
#include <stdio.h>
#include <vector>
#include <array>
#include <numeric>

//----------------------------------------------------------------------------
//                                SETTINGS
//----------------------------------------------------------------------------

static const size_t c_numSpheres = 1000;

static const size_t c_pointDimension = 2;

static const size_t c_numSolutions = 100;

static const float c_minRadius = 0.0f;
static const float c_maxRadius = 1.0f;

static const float c_worldSize = 100.0f;

//----------------------------------------------------------------------------

typedef std::array<float, c_pointDimension> TPoint;

struct TSphere
{
    TPoint position;
    float radius;
};

//----------------------------------------------------------------------------

// random numbers
std::random_device g_rd;
std::mt19937 g_mt(g_rd());
std::uniform_real_distribution<float> g_dist_coordinate(0.0f, c_worldSize);
std::uniform_real_distribution<float> g_dist_radius(c_minRadius, c_maxRadius);

//----------------------------------------------------------------------------

float length(const TPoint& p)
{
    float lenSquared = 0.0f;
    for (float f : p)
        lenSquared += f * f;
    return sqrt(lenSquared);
}

TPoint normalize(const TPoint& p)
{
    float lenSquared = 0.0f;
    for (float f : p)
        lenSquared += f * f;
    float len = sqrt(lenSquared);

    TPoint ret = p;
    for (float& f : ret)
        f /= len;
    return ret;
}

float dot(const TPoint& a, const TPoint& b)
{
    float ret = 0.0f;
    for (size_t i = 0; i < c_pointDimension; ++i)
        ret += a[i] * b[i];
    return ret;
}

TPoint operator - (const TPoint& a, const TPoint& b)
{
    TPoint ret;
    for (size_t i = 0; i < c_pointDimension; ++i)
        ret[i] = a[i] - b[i];
    return ret;
}

TPoint operator + (const TPoint& a, const TPoint& b)
{
    TPoint ret;
    for (size_t i = 0; i < c_pointDimension; ++i)
        ret[i] = a[i] + b[i];
    return ret;
}

TPoint operator * (const TPoint& a, float b)
{
    TPoint ret;
    for (size_t i = 0; i < c_pointDimension; ++i)
        ret[i] = a[i] * b;
    return ret;
}

//----------------------------------------------------------------------------

TSphere Solve(const std::vector<TSphere>& spheres)
{
    // choose a random order to process the points in.
    // This isn't a requirement of the algorithm, just here to show whether processing order matters or not
    std::vector<size_t> processingOrder;
    processingOrder.resize(spheres.size());
    std::iota(processingOrder.begin(), processingOrder.end(), 0);
    std::shuffle(processingOrder.begin(), processingOrder.end(), g_mt);

    // incrementally merge spheres
    TSphere result = spheres[processingOrder[0]];
    for (size_t i = 1; i < processingOrder.size(); ++i)
    {
        const TSphere& sphere = spheres[processingOrder[i]];

        TPoint direction = normalize(sphere.position - result.position);

        // get 1D locations relative to the result
        float resultMin = 0.0f - result.radius;
        float resultMax = 0.0f + result.radius;

        float spherePos = length(sphere.position - result.position);
        float sphereMin = spherePos - sphere.radius;
        float sphereMax = spherePos + sphere.radius;

        // get the min and max 1d positions
        float minPos = std::min(resultMin, sphereMin);
        float maxPos = std::max(resultMax, sphereMax);

        // make the result be at the center of the min and max, with a radius large enough to encompass both of them
        result.radius = (maxPos - minPos) / 2.0f;
        result.position = result.position + direction * ((maxPos + minPos) / 2.0f);
    }

    // return the results
    return result;
}

//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    // generate the points
    std::vector<TSphere> spheres;
    spheres.resize(c_numSpheres);
    for (TSphere& sphere : spheres)
    {
        sphere.radius = g_dist_radius(g_mt);
        for (float& p : sphere.position)
            p = g_dist_coordinate(g_mt);
    }

    // Solve
    std::vector<TSphere> solutions;
    solutions.resize(c_numSolutions);
    size_t solve = 0;
    for (TSphere& solution : solutions)
    {
        solution = Solve(spheres);
        ++solve;
        printf("\rSolution %zu / %zu", solve, c_numSolutions);
    }

    // write out the points and the solutions
    printf("\n\nSaving Results...\n\n");
    FILE* file = fopen("out.csv", "w+t");
    for (size_t i = 0; i < c_pointDimension; ++i)
        fprintf(file, "\"P_%zu\",",i);
    fprintf(file, "\"radius\"\n");
    for (const TSphere& sphere : spheres)
    {
        for (float f : sphere.position)
            fprintf(file, "%f,", f);
        fprintf(file, "%f\n", sphere.radius);
    }
    fprintf(file, "\n\nSolutions\n");
    for (const TSphere& sphere : solutions)
    {
        for (float f : sphere.position)
            fprintf(file, "%f,", f);
        fprintf(file, "%f\n", sphere.radius);
    }
    fclose(file);

    system("pause");
    return 1;
}

/*
TODO:
* getting inconsistent results need to understand why
* maybe report the min and max sized solution?

Notes:
* Share w/ Alan Hickman. Probably blog post, whether or not it works out
* Note: haven't tried to optimize yet, just trying to make it readable and work!
* Note: paralelizable since you can distribute combining work to unlimited threads. Limited only by work to do.

*/