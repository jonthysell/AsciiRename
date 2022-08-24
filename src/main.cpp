// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <libpu8.h>

#ifndef VERSION_STR
#define VERSION_STR "0.0.0"
#endif

#include "helpers.h"

void ShowVersion()
{
    std::cout << "ascii-rename " << VERSION_STR << "\n";
}

void ShowHelp()
{
    std::cout << "Usage: ascii-rename [options...] [paths...]\n";
    std::cout << "-h, --help       Show this help and exit\n";
    std::cout << "-n, --no-op      Don't actually rename files\n";
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
    bool noop = false;

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
        else if (ArgEquals(arg, "-n", "--no-op"))
        {
            noop = true;
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
        auto oldPathStr = std::string();
        auto newPathStr = std::string();

        AsciiRename::TryGetUtf8(filePaths[i], oldPathStr);
        AsciiRename::TryGetAscii(filePaths[i], newPathStr);

        if (verbose)
        {
            std::cout << "Processing \"" << oldPathStr << "\"...\n";
        }

        auto oldPath = std::filesystem::path(filePaths[i]);
        auto oldDir = oldPath.parent_path();
        auto oldFile = oldPath.filename();

        auto newPath = std::filesystem::path(newPathStr);
        auto newDir = newPath.parent_path();
        auto newFile = newPath.filename();

        bool skip = false;

        if (!std::filesystem::exists(oldPath))
        {
            std::cerr << "ERROR: \"" << oldPathStr << "\" doesn't exist.\n";
            skip = true;
        }
        else if (std::filesystem::is_directory(oldPath))
        {
            std::cerr << "ERROR: \"" << oldPathStr << "\" is a directory.\n";
            skip = true;
        }
        else if (std::filesystem::exists(newPath) && !overwrite)
        {
            std::cerr << "ERROR: \"" << newPath.string() << "\" already exists.\n";
            std::cerr << "ERROR: Specify --overwrite to overwrite.\n";
            skip = true;
        }
        else if (!std::filesystem::exists(newDir))
        {
            if (parents)
            {
                if (noop)
                {
                    if (verbose)
                    {
                        std::cout << "Would have created \"" << newDir.string() << "\"...\n";
                    }
                }
                else
                {
                    if (verbose)
                    {
                        std::cout << "Creating \"" << newDir.string() << "\"...\n";
                    }
                    std::filesystem::create_directories(newDir);
                }
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
                std::cout << "Skipping \"" << oldPathStr << "\"...\n";
            }
            ++skipped;
        }
        else if (noop)
        {
            std::cout << "Would have renamed \"" << oldPathStr << "\" to \"" << newPathStr << "\"...\n";
        }
        else
        {
            std::cout << "Renaming \"" << oldPathStr << "\" to \"" << newPathStr << "\"...\n";

            if (std::filesystem::exists(newPath))
            {
                std::filesystem::remove(newPath);
            }

            std::filesystem::rename(oldPath, newPath);
        }
    }

    return skipped;
}
