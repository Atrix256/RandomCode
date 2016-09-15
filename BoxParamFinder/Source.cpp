#include <stdio.h>
#include <array>

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

void PrintPoint(const TPoint& p)
{
    printf("(%0.2f, %0.2f, %0.2f)", p[0], p[1], p[2]);
}

int main (int argc, char **argv)
{
    TQuad top =    { { { 130.0f, 165.0f, 65.0f }, { 82.0f, 165.0f, 225.0f }, { 240.0f, 165.0f, 272.0f }, { 290.0f, 165.0f, 114.0f } } };
    TQuad bottom = { { { 130.0f,   0.0f, 65.0f }, { 82.0f,   0.0f, 225.0f }, { 240.0f,   0.0f, 272.0f }, { 290.0f,   0.0f, 114.0f } } };

    TQuad a = { { { 290.0f, 0.0f, 114.0f }, { 290.0f, 165.0f, 114.0f }, { 240.0f, 165.0f, 272.0f }, { 240.0f, 0.0f, 272.0f } } };
    TQuad b = { { { 130.0f, 0.0f, 65.0f }, { 130.0f, 165.0f, 65.0f }, { 290.0f, 165.0f, 114.0f }, { 290.0f, 0.0f, 114.0f } } };
    TQuad c = { { { 82.0f, 0.0f, 225.0f }, { 82.0f, 165.0f, 225.0f }, { 130.0f, 165.0f, 65.0f }, { 130.0f, 0.0f, 65.0f } } };
    TQuad d = { { { 240.0f, 0.0f, 272.0f }, { 240.0f, 165.0f, 272.0f }, { 82.0f, 165.0f, 225.0f }, { 82.0f, 0.0f, 225.0f } } };

    TPoint pointTop = AveragePoints(top);
    TPoint pointBottom = AveragePoints(bottom);
    TPoint pointA = AveragePoints(a);
    TPoint pointB = AveragePoints(a);
    TPoint pointC = AveragePoints(a);
    TPoint pointD = AveragePoints(a);

    const std::array<TPoint, 6> boxPoints = { pointTop, pointBottom, pointA, pointB, pointC, pointD };
    TPoint pointCenter = AveragePoints(boxPoints);

    printf("-----Center Points-----\n");

    printf("\nTop = ");
    PrintPoint(pointTop);

    printf("\nBottom = ");
    PrintPoint(pointBottom);

    printf("\n\n-----Center Of Box-----\n");
    PrintPoint(pointCenter);

    // TODO: figure out which of a,b,c,d are pairs
    // TODO: figure out width, height, depth of box
    // TODO: figure out box rotation

    return 0;
}