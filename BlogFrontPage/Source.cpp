#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>

#define STRING_SIZE 102400

bool ReadLine(FILE* file, std::string& s)
{
    s = "";
    while (1)
    {
        char c = fgetc(file);

        s += c;

        if (feof(file))
        {
            return false;
        }

        if (c == '\r' || c=='\n')
        {
            return true;
        }
    }
}

int main(int argc, char** argv)
{
    FILE* inFile = fopen("export.xml", "rt");
    if (!inFile)
        return 1;

    FILE* outFile = fopen("out.txt", "w+t");
    if (!outFile)
        return 2;

    struct Item
    {
        std::string title;
        std::string link;
        std::string postDate;
    };
    std::vector<Item> items;

    std::string line;

    std::string title;
    bool hasTitle = false;

    std::string link;
    bool hasLink = false;

    std::string postType;
    bool hasPostType = false;

    std::string status;
    bool hasStatus = false;

    std::string postDate;
    bool hasPostDate = false;

    if (ReadLine(inFile, line))
    {
        do
        {
            if (!hasTitle)
            {
                const char* find1 = "<title>";
                const char* find2 = "</title>";

                const char* start = strstr(line.c_str(), find1);
                const char* end = strstr(line.c_str(), find2);

                assert((start == nullptr && end == nullptr) || (start != nullptr && end != nullptr));

                if (start)
                {
                    title = line.substr(start - line.c_str() + strlen(find1), end - start - strlen(find1));
                    hasTitle = true;
                }
            }

            if (!hasLink)
            {
                const char* find1 = "<link>";
                const char* find2 = "</link>";

                const char* start = strstr(line.c_str(), find1);
                const char* end = strstr(line.c_str(), find2);

                assert((start == nullptr && end == nullptr) || (start != nullptr && end != nullptr));

                if (start)
                {
                    link = line.substr(start - line.c_str() + strlen(find1), end - start - strlen(find1));
                    hasLink = true;
                }
            }

            if (!hasPostType)
            {
                const char* find1 = "<wp:post_type>";
                const char* find2 = "</wp:post_type>";

                const char* start = strstr(line.c_str(), find1);
                const char* end = strstr(line.c_str(), find2);

                assert((start == nullptr && end == nullptr) || (start != nullptr && end != nullptr));

                if (start)
                {
                    postType = line.substr(start - line.c_str() + strlen(find1), end - start - strlen(find1));
                    hasPostType = true;
                }
            }

            if (!hasStatus)
            {
                const char* find1 = "<wp:status>";
                const char* find2 = "</wp:status>";

                const char* start = strstr(line.c_str(), find1);
                const char* end = strstr(line.c_str(), find2);

                assert((start == nullptr && end == nullptr) || (start != nullptr && end != nullptr));

                if (start)
                {
                    status = line.substr(start - line.c_str() + strlen(find1), end - start - strlen(find1));
                    hasStatus = true;
                }
            }

            if (!hasPostDate)
            {
                const char* find1 = "<wp:post_date>";
                const char* find2 = "</wp:post_date>";

                const char* start = strstr(line.c_str(), find1);
                const char* end = strstr(line.c_str(), find2);

                assert((start == nullptr && end == nullptr) || (start != nullptr && end != nullptr));

                if (start)
                {
                    const char* end = strstr(line.c_str(), " ");
                    assert(end);
                    postDate = line.substr(start - line.c_str() + strlen(find1), end - start - strlen(find1));
                    hasPostDate = true;
                }
            }


            if (strstr(line.c_str(), "</item>"))
            {
                if (hasTitle && hasLink && hasPostType && hasStatus && hasPostDate)
                {
                    if (!strcmp(postType.c_str(), "post") && !strcmp(status.c_str(), "publish"))
                    {
                        items.push_back(
                            {
                                title,
                                link,
                                postDate
                            }
                        );
                    }
                }
                else
                {
                    assert(false);
                }
                hasTitle = false;
                hasLink = false;
                hasPostType = false;
                hasStatus = false;
                hasPostDate = false;
            }
        }
        while (ReadLine(inFile, line));
    }

    std::sort(items.begin(), items.end(),
        [] (const Item& a, const Item& b) -> bool
        {
            return a.postDate.compare(b.postDate) > 0;
        }
    );

    for (Item& item : items)
    {
        fprintf(outFile, "<a target=\"_blank\" href=\"%s\">%s %s</a>\r\n", item.link.c_str(), item.postDate.c_str(), item.title.c_str());
    }

    fclose(inFile);
    fclose(outFile);
    return 0;
}