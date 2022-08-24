// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#include <filesystem>
#include <iostream>
#include <string>
#include <queue>

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
    std::cout << "-r, --recursive  Rename files and subdirectories recursively\n";
    std::cout << "-v, --verbose    Make the output more verbose\n";
    std::cout << "-V, --version    Show version number and exit\n";
}

#ifdef _WIN32
#define L(s) L##s
#define ArgEquals(X, Y, Z) (X == L(Y) || X == L(Z))
#define ArgStartsWith(X, Y) (X.rfind(L(Y)) == 0)
#else
#define ArgEquals(X, Y, Z) (X == Y || X == Z)
#define ArgStartsWith(X, Y) (X.rfind(Y) == 0)
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
    auto pathQueue = std::queue<std::wstring>();
#else
    auto pathQueue = std::queue<std::string>();
#endif

    // Options
    bool noop = false;
    bool overwrite = false;
    bool recursive = false;
    bool verbose = false;

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
        else if (ArgEquals(arg, "-n", "--no-op"))
        {
            noop = true;
        }
        else if (ArgEquals(arg, "-o", "--overwrite"))
        {
            overwrite = true;
        }
        else if (ArgEquals(arg, "-r", "--recursive"))
        {
            recursive = true;
        }
        else if (ArgEquals(arg, "-v", "--verbose"))
        {
            verbose = true;
        }
        else if (ArgStartsWith(arg, "-"))
        {
            auto argStr = std::string();
            AsciiRename::TryGetUtf8(arg, argStr);
            std::cerr << "ERROR: \"" << argStr << "\" option not recognized. Run with --help for usage info.\n";
            return -1;
        }
        else
        {
            pathQueue.push(arg);
        }
    }

    // Process pathQueue
    int renames = 0;
    int skipped = 0;

    while (!pathQueue.empty())
    {
        auto rawPath = pathQueue.front();
        pathQueue.pop();

        auto originalPathStr = std::string();

        AsciiRename::TryGetUtf8(rawPath, originalPathStr);

        if (verbose)
        {
            std::cout << "Processing \"" << originalPathStr << "\"...\n";
        }

        auto originalPath = std::filesystem::path(rawPath);
        auto originalParentPath = originalPath.parent_path();
        auto originalFile = originalPath.filename();

        auto asciiPathStr = std::string();
        AsciiRename::TryGetAscii(rawPath, asciiPathStr);

        auto asciiPath = std::filesystem::path(asciiPathStr);
        auto asciiParentPath = asciiPath.parent_path();
        auto asciiFile = asciiPath.filename();

        bool skip = false;

        if (!std::filesystem::exists(originalPath))
        {
            std::cerr << "ERROR: \"" << originalPathStr << "\" doesn't exist.\n";
            skip = true;
        }
        else
        {
            // Original path exists, get new path

            auto newPath = originalParentPath / asciiFile;
            auto newPathStr = std::string();
#ifdef _WIN32
            AsciiRename::TryGetUtf8(newPath.wstring(), newPathStr);
#else
            AsciiRename::TryGetUtf8(newPath.string(), newPathStr);
#endif

            if (originalPathStr == newPathStr)
            {
                if (verbose)
                {
                    std::cout << "No need to rename \"" << originalPathStr << "\".\n";
                }
                skip = true;
            }
            else if (std::filesystem::exists(newPath) && !overwrite)
            {
                std::cerr << "ERROR: \"" << newPathStr << "\" already exists.\n";
                std::cerr << "ERROR: Specify --overwrite to overwrite.\n";
                skip = true;
            }
            else if (std::filesystem::is_directory(originalPath) && recursive)
            {
                // TODO: Implement recursive renaming
            }
            else
            {
                // Just a single item rename
                if (noop)
                {
                    std::cout << "Would have renamed \"" << originalPathStr << "\" to \"" << newPathStr << "\"...\n";
                }
                else
                {
                    std::cout << "Renaming \"" << originalPathStr << "\" to \"" << newPathStr << "\"...\n";
                    std::filesystem::rename(originalPath, newPath);
                }
                ++renames;
            }
        }

        if (skip)
        {
            if (verbose)
            {
                std::cout << "Skipping \"" << originalPathStr << "\"...\n";
            }
            ++skipped;
        }
    }

    if (verbose)
    {
        std::cout << "Renamed: " << renames << ", Skipped: " << skipped << ", Total: " << renames + skipped << "\n";
    }

    return skipped;
}
