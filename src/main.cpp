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

#ifndef VERSION_STR
#define VERSION_STR "0.0.0"
#endif

void ShowVersion()
{
    std::cout << "ascii-rename " << VERSION_STR << "\n";
}

void ShowHelp()
{
    std::cout << "Usage: ascii-rename [--version] [--help]"
              << "\n";
    std::cout << "                    [<options>] path1 [path2]... [pathN]"
              << "\n";
}

int wmain(int argc, wchar_t *argv[])
{
    if (argc < 1)
    {
        ShowHelp();
        return 0;
    }

    // Process arguments

    auto filePaths = std::vector<wchar_t *>();

    // Options
    bool verbose = false;
    bool recursive = false;
    bool createDirs = false;
    bool overwrite = false;

    for (int i = 1; i < argc; ++i)
    {
        if (wcscmp(argv[i], L"--help") == 0)
        {
            ShowHelp();
            return 0;
        }
        else if (wcscmp(argv[i], L"--version") == 0)
        {
            ShowVersion();
            return 0;
        }
        else if (wcscmp(argv[i], L"--verbose") == 0)
        {
            verbose = true;
        }
        else if (wcscmp(argv[i], L"--recursive") == 0)
        {
            recursive = true;
        }
        else if (wcscmp(argv[i], L"--createDirs") == 0)
        {
            createDirs = true;
        }
        else if (wcscmp(argv[i], L"--overwrite") == 0)
        {
            overwrite = true;
        }
        else
        {
            filePaths.push_back(argv[i]);
        }
    }

    // Perform renames
    int skipped = 0;

    for (int i = 0; i < filePaths.size(); ++i)
    {
        auto filename = std::string();
        auto asciiFilename = std::string();
        for (int j = 0; j < wcslen(filePaths[i]); ++j)
        {
            uint32_t utf32 = filePaths[i][j];
            filename += static_cast<char>(utf32);

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
            if (createDirs)
            {
                if (verbose)
                {
                    std::cout << "Creating directory \"" << newDir.string() << "\"...\n";
                }
                std::filesystem::create_directories(newDir);
            }
            else
            {
                std::cerr << "ERROR: Need to create directory \"" << newDir.string() << "\".\n";
                std::cerr << "ERROR: Specify --createDirs to create directories.\n";
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
            if (verbose)
            {
                std::cout << "Renaming \"" << filename << "\" to \"" << asciiFilename << "\"...\n";
            }

            if (std::filesystem::exists(newPath))
            {
                std::filesystem::remove(newPath);
            }

            std::filesystem::rename(oldPath, newPath);
        }
    }

    return skipped;
}
