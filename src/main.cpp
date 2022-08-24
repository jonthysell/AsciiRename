// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C"
{
#endif
    size_t anyascii(uint_least32_t utf32, const char **ascii);
#ifdef __cplusplus
}
#endif

#include <anyascii.h>
#include <libpu8.h>

#ifndef VERSION_STR
#define VERSION_STR "0.0.0"
#endif

void ShowVersion()
{
    std::cout << "ascii-rename " << VERSION_STR << "\n";
}

void ShowHelp()
{
    std::cout << "Usage: ascii-rename [options...] [paths...]\n";
    std::cout << "-h, --help       Show this help and exit\n";
    std::cout << "-o, --overwrite  Overwrite existing file(s)\n";
    std::cout << "-p, --parents    Make parent directory(ies) as needed\n";
    std::cout << "-v, --verbose    Make the output more verbose\n";
    std::cout << "-V, --version    Show version number and exit\n";
}

#ifdef _WIN32
#define L(s) L##s
#define ArgEquals(X, Y, Z) (X == L(Y) || X == L(Z))
#else
#define ArgEquals(X, Y, Z) (X == Y || X == Z)
#define u8narrow(X) (X)
#endif

int main_utf8(int argc, char **argv)
{
    if (argc < 1)
    {
        ShowHelp();
        return 0;
    }

    // Process arguments
#ifdef _WIN32
    auto filePaths = std::vector<std::wstring>();
#else
    auto filePaths = std::vector<std::string>();
#endif

    // Options
    bool verbose = false;
    bool parents = false;
    bool overwrite = false;

    for (int i = 1; i < argc; ++i)
    {
        const auto arg = u8widen(argv[i]);

        if (ArgEquals(arg, "-h", "--help"))
        {
            ShowHelp();
            return 0;
        }
        else if (ArgEquals(arg, "-V", "--version"))
        {
            ShowVersion();
            return 0;
        }
        else if (ArgEquals(arg, "-v", "--verbose"))
        {
            verbose = true;
        }
        else if (ArgEquals(arg, "-p", "--parents"))
        {
            parents = true;
        }
        else if (ArgEquals(arg, "-o", "--overwrite"))
        {
            overwrite = true;
        }
        else
        {
            filePaths.push_back(arg);
        }
    }

    // Perform renames
    int skipped = 0;

    for (int i = 0; i < filePaths.size(); ++i)
    {
        auto filename = u8narrow(filePaths[i]);
        auto asciiFilename = std::string();
        for (int j = 0; j < filePaths[i].length(); ++j)
        {
            uint32_t utf32 = filePaths[i][j];

            const char *ascii;
            if (anyascii(utf32, &ascii))
            {
                asciiFilename += *ascii;
            }
        }

        if (verbose)
        {
            std::cout << "Processing \"" << filename << "\"...\n";
        }

        auto oldPath = std::filesystem::path(filePaths[i]);
        auto newPath = std::filesystem::path(asciiFilename);

        auto newDir = newPath.parent_path();

        bool skip = false;

        if (!std::filesystem::exists(oldPath))
        {
            std::cerr << "ERROR: File \"" << filename << "\" doesn't exist.\n";
            skip = true;
        }
        else if (std::filesystem::exists(newPath) && !overwrite)
        {
            std::cerr << "ERROR: File \"" << newPath.string() << "\" already exists.\n";
            std::cerr << "ERROR: Specify --overwrite to overwrite.\n";
            skip = true;
        }
        else if (!std::filesystem::exists(newDir))
        {
            if (parents)
            {
                if (verbose)
                {
                    std::cout << "Creating directory \"" << newDir.string() << "\"...\n";
                }
                std::filesystem::create_directories(newDir);
            }
            else
            {
                std::cerr << "ERROR: Need to create parent directory(ies) \"" << newDir.string() << "\".\n";
                std::cerr << "ERROR: Specify --parents to create parent directory(ies).\n";
                skip = true;
            }
        }

        if (skip)
        {
            if (verbose)
            {
                std::cout << "Skipping \"" << filename << "\"...\n";
            }
            ++skipped;
        }
        else
        {
            std::cout << "Renaming \"" << filename << "\" to \"" << asciiFilename << "\"...\n";

            if (std::filesystem::exists(newPath))
            {
                std::filesystem::remove(newPath);
            }

            std::filesystem::rename(oldPath, newPath);
        }
    }

    return skipped;
}
