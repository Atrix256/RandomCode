#include <stdio.h>
#include <array>
#include <vector>

//=================================================================================
void WaitForEnter()
{
    printf("\nPress Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
void AddSolutions(std::vector<int>& solutions, int add, int multiply, int mod)
{
    int Z = 0;
    while (1)
    {
        int value = multiply * Z + add;
        if (value < mod)
            solutions.push_back(value);
        else
            return;
        ++Z;
    }
}

//=================================================================================
void AddSolutionsFromSolutions (std::vector<int>& solutions, const std::vector<int>& adds, int multiply, int mod)
{
    std::for_each(
        adds.begin(),
        adds.end(),
        [&solutions, multiply, mod] (int add)
        {
            AddSolutions(solutions, add, multiply, mod);
        }
    );
}

//=================================================================================
int main(int argc, char **argv)
{
    // a,b,c,d...
    // ((x % a) % b) % c = d
    // etc.
    //const int c_values[] = {7, 5, 2, 1};
    const int c_values[] = { 12, 9, 7, 5, 3, 2 };
    const size_t c_numValues = sizeof(c_values) / sizeof(c_values[0]);
    
    // print the equation
    printf("Solving for x:\n");
    for (size_t i = 0; i < c_numValues - 1; ++i)
        printf("(");
    printf("x");
    for (size_t i = 0; i < c_numValues - 1; ++i)
        printf(" %% %i)", c_values[i]);
    printf(" = %i\n\n", c_values[c_numValues-1]);

    // print the solution equations
    for (size_t i = 0; i < c_numValues - 2; ++i)
    {
        char eqn = i > 0 ? 'A' + i - 1 : 'x';
        char eq = i > 0 ? '=' : 0xF0;

        printf("%c %c %c + %i*Z", eqn, eq, 'B' + i - 1, c_values[i]);
        if (i > 0)
            printf(" (in Z_%i)\n", c_values[i-1]);
        else
            printf(" (in Z)\n");
    }
    printf("%c = %i + %i*Z (in Z_%i)\n\n", 'A' + c_numValues - 2 - 1, c_values[c_numValues - 1], c_values[c_numValues - 2], c_values[c_numValues - 3]);

    // gather up the permutation of solutions for each equation, starting with the lowest equation which has only constants
    std::array<std::vector<int>, c_numValues - 2> solutions;
    AddSolutions(solutions[c_numValues - 3], c_values[c_numValues - 1], c_values[c_numValues - 2], c_values[c_numValues - 3]);
    for (size_t i = c_numValues - 3; i > 0; --i)
        AddSolutionsFromSolutions(solutions[i-1], solutions[i], c_values[i], c_values[i-1]);

    // Detect empty set
    if (solutions[0].size() == 0)
    {
        printf("No solutions!\n");
        WaitForEnter();
        return 0;
    }

    // Print the more specific solution equations
    printf("x = ");
    for (size_t i = 0, c = solutions[0].size(); i < c; ++i)
    {
        if (i < c - 1)
            printf("%c U ", 'a' + i);
        else
            printf("%c\n", 'a' + i);
    }
    std::sort(solutions[0].begin(), solutions[0].end());
    for (size_t i = 0, c = solutions[0].size(); i < c; ++i)
        printf("%c = %i + %iZ\n", 'a' + i, solutions[0][i], c_values[0]);
  
    // Print specific examples of solutions (first few numbers in each)
    std::vector<int> xValues;
    printf("\n");
    for (size_t i = 0, c = solutions[0].size(); i < c; ++i)
    {
        printf("%c = {", 'a' + i);
        for (int z = 0; z < 3; ++z)
        {
            printf("%i, ", solutions[0][i] + z * c_values[0]);
            xValues.push_back(solutions[0][i] + z * c_values[0]);
        }
        printf("...}\n");
    }

    // Show the list of specific values of X
    std::sort(xValues.begin(), xValues.end());
    printf("\nx = {");
    std::for_each(xValues.begin(), xValues.end(), [](int v) {printf("%i, ", v); });
    printf("...}\n");

    // Test the solutions to verify that they are valid!
    bool valuesOK = true;
    for (size_t i = 0, c = xValues.size(); i < c; ++i)
    {
        int value = xValues[i];
        for (size_t j = 0; j < c_numValues - 1; ++j)
            value = value % c_values[j];

        if (value != c_values[c_numValues - 1])
        {
            printf("Solution %i is invalid!!\n", xValues[i]);
            valuesOK = false;
        }
    }
    if (valuesOK)
        printf("\nAll solutions shown tested valid!\n");

    WaitForEnter();
    return 0;
}

/*

TODO:
* test it with 1 or 2 items.  test it with lots of items.
* make the solutions be a set instead of a vector to make sure dupes don't show up
 ? see if this is needed, when doing more complex ones?

Blog:
* get examples from ones you did on paper perhaps?
* mention it's a "save point" ? :p
* show it with 1 modulus level
* show it with 2 levels
* show it with more levels
* show it when there is no solution
* note that you are unsure if there are other ways to solve these equations?

Links:
http://math.stackexchange.com/questions/1434181/how-to-solve-nested-congruences

quotient remainder theorem:
https://www.khanacademy.org/computing/computer-science/cryptography/modarithmetic/a/the-quotient-remainder-theorem

*/