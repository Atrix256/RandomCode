#include <stdio.h>
#include <array>

const float c_pi = 3.14159265359;

typedef std::array<float, 3> TPoint;
typedef std::array<TPoint, 4> TQuad;

template <size_t N>
TPoint AveragePoints(const std::array<TPoint, N>& points)
{
    TPoint ret = { 0.0f, 0.0f, 0.0f };
    for (int i = 0; i < N; ++i)
    {
        ret[0] += (points[i])[0];
        ret[1] += (points[i])[1];
        ret[2] += (points[i])[2];
    }
    ret[0] /= float(N);
    ret[1] /= float(N);
    ret[2] /= float(N);
    return ret;
}

TPoint operator- (const TPoint& a, const TPoint& b)
{
    return
    {
        a[0] - b[0],
        a[1] - b[1],
        a[2] - b[2]
    };
}

float Length(const TPoint& v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

TPoint Normalize(const TPoint& v)
{
    float len = Length(v);
    return
    {
        v[0] / len,
        v[1] / len,
        v[2] / len
    };
}

void PrintPoint(const TPoint& p)
{
    printf("(%0.2f, %0.2f, %0.2f)", p[0], p[1], p[2]);
}

int main (int argc, char **argv)
{
    // small box

    TQuad top = { {{ 423.0f, 330.0f, 247.0f },{ 265.0f, 330.0f, 296.0f },{ 314.0f, 330.0f, 456.0f },{ 472.0f, 330.0f, 406.0f }} };
    TQuad bottom = { { { 423.0f, 0.0f, 247.0f },{ 265.0f, 0.0f, 296.0f },{ 314.0f, 0.0f, 456.0f },{ 472.0f, 0.0f, 406.0f } } };

    TQuad a = { {{ 423.0f,   0.0f, 247.0f },{ 423.0f, 330.0f, 247.0f },{ 472.0f, 330.0f, 406.0f },{ 472.0f,   0.0f, 406.0f }} };
    TQuad b = { {{ 472.0f,   0.0f, 406.0f },{ 472.0f, 330.0f, 406.0f },{ 314.0f, 330.0f, 456.0f },{ 314.0f,   0.0f, 456.0f }} };
    TQuad c = { {{ 314.0f,   0.0f, 456.0f },{ 314.0f, 330.0f, 456.0f },{ 265.0f, 330.0f, 296.0f },{ 265.0f,   0.0f, 296.0f }} };
    TQuad d = { {{ 265.0f,   0.0f, 296.0f },{ 265.0f, 330.0f, 296.0f },{ 423.0f, 330.0f, 247.0f },{ 423.0f,   0.0f, 247.0f }} };

    /*
    TQuad top =    { { { 130.0f, 165.0f, 65.0f }, { 82.0f, 165.0f, 225.0f }, { 240.0f, 165.0f, 272.0f }, { 290.0f, 165.0f, 114.0f } } };
    TQuad bottom = { { { 130.0f,   0.0f, 65.0f }, { 82.0f,   0.0f, 225.0f }, { 240.0f,   0.0f, 272.0f }, { 290.0f,   0.0f, 114.0f } } };
    TQuad a = { { { 290.0f, 0.0f, 114.0f }, { 290.0f, 165.0f, 114.0f }, { 240.0f, 165.0f, 272.0f }, { 240.0f, 0.0f, 272.0f } } };
    TQuad b = { { { 130.0f, 0.0f, 65.0f }, { 130.0f, 165.0f, 65.0f }, { 290.0f, 165.0f, 114.0f }, { 290.0f, 0.0f, 114.0f } } };
    TQuad c = { { { 82.0f, 0.0f, 225.0f }, { 82.0f, 165.0f, 225.0f }, { 130.0f, 165.0f, 65.0f }, { 130.0f, 0.0f, 65.0f } } };
    TQuad d = { { { 240.0f, 0.0f, 272.0f }, { 240.0f, 165.0f, 272.0f }, { 82.0f, 165.0f, 225.0f }, { 82.0f, 0.0f, 225.0f } } };
    */

    // tall box

    TPoint pointTop = AveragePoints(top);
    TPoint pointBottom = AveragePoints(bottom);
    TPoint pointA = AveragePoints(a);
    TPoint pointB = AveragePoints(b);
    TPoint pointC = AveragePoints(c);
    TPoint pointD = AveragePoints(d);

    const std::array<TPoint, 6> boxPoints = { pointTop, pointBottom, pointA, pointB, pointC, pointD };
    TPoint pointCenter = AveragePoints(boxPoints);

    printf("-----Center Points-----");

    printf("\nTop = ");
    PrintPoint(pointTop);

    printf("\nBottom = ");
    PrintPoint(pointBottom);

    printf("\nA = ");
    PrintPoint(pointA);

    printf("\nB = ");
    PrintPoint(pointB);

    printf("\nC = ");
    PrintPoint(pointC);

    printf("\nD = ");
    PrintPoint(pointD);

    // X
    TPoint x1 = pointA - pointCenter;
    float x1Len = Length(x1);
    TPoint x1Normed = Normalize(x1);

    TPoint x2 = pointC - pointCenter;
    float x2Len = Length(x2);
    TPoint x2Normed = Normalize(x2);

    // Y
    TPoint y1 = pointTop - pointCenter;
    float y1Len = Length(y1);
    TPoint y1Normed = Normalize(y1);

    TPoint y2 = pointBottom - pointCenter;
    float y2Len = Length(y2);
    TPoint y2Normed = Normalize(y2);

    // Z
    TPoint z1 = pointB - pointCenter;
    float z1Len = Length(z1);
    TPoint z1Normed = Normalize(z1);

    TPoint z2 = pointD - pointCenter;
    float z2Len = Length(z2);
    TPoint z2Normed = Normalize(z2);

    float yAxisRotation = atan2(z2Normed[2], z2Normed[0]) * 180.0f / c_pi;

    // print box info
    TPoint radius = {x1Len, y1Len, z1Len};
    printf("\n\n-----Box-----");
    printf("\nCenter = ");
    PrintPoint(pointCenter);
    printf("\nRadius = ");
    PrintPoint(radius);
    printf("\nRotation = %f degrees\n", yAxisRotation);

    return 0;
}