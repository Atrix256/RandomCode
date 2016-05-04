#include <array>

typedef std::array<int, 2> TVector;

const float c_pi = 3.14159265359f;

//=================================================================
void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================
int main (int argc, char **argv)
{
    // (1 + i) * (0 + i) = -1 + i
    // 100 * 99 = 9900
    // 9900 % 9802 = 98
    // that decodes to 98 + 0i, but should be -1 + i. ambiguous!

    TVector A = { 1, 0 };
    TVector B = { 0, 1 };

    // show the vectors
    printf("A = (%i, %i), angle = %0.2f degrees\n", A[0], A[1], atan2(A[1], A[0]) * 180.0f / c_pi);
    printf("B = (%i, %i), angle = %0.2f degrees\n\n", B[0], B[1], atan2(B[1], B[0]) * 180.0f / c_pi);

    // show the intermediary value
    int value = ((A[0] + A[1] * 99) * (B[0] + B[1] * 99)) % 9802;
    printf("value = %i\n\n", value);

    // show the results of vector A rotated by vector B
    TVector C = {value % 99, value / 99};
    printf("C = (%i, %i), angle = %0.2f degrees\n\n", C[0], C[1], atan2(C[1], C[0]) * 180.0f / c_pi);

    value = value - 9802;
    TVector D = { value % 99, value / 99 };
    printf("D = (%i, %i), angle = %0.2f degrees\n\n", D[0], D[1], atan2(D[1], D[0]) * 180.0f / c_pi);

    WaitForEnter();
    return 0;
}