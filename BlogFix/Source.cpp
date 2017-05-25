#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    FILE *srcFile = fopen("blogexport.xml","rt");
    FILE *destFile = fopen("blogexport_fixed.xml", "w+t");

    bool inside = false;
    int matchIndex = 0;

    unsigned char c = fgetc(srcFile);
    while (!feof(srcFile))
    {
        fputc(c, destFile);

        if (inside)
        {
            if (c == '\\')
                fputc(c, destFile);
            else if (c == '$')
            {
                inside = false;
                matchIndex = 0;
            }
        }
        else
        {
            static const char* startString = "$latex";
            if (c == startString[matchIndex])
            {
                ++matchIndex;
                if (matchIndex == strlen(startString))
                    inside = true;
            }
            else
                matchIndex = 0;
        }
        c = fgetc(srcFile);
    }    

    fclose(srcFile);
    fclose(destFile);
}