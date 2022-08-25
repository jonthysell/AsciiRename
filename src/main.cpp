// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#include <filesystem>
#include <iostream>
#include <list>
#include <string>

#include <libpu8.h>

#include "helpers.h"

#ifndef VERSION_STR
#define VERSION_STR "0.0.0"
#endif

#ifdef _WIN32
#define L(s) L##s
#define ArgEquals(X, Y, Z) (X == L(Y) || X == L(Z))
#define ArgStartsWith(X, Y) (X.rfind(L(Y)) == 0)
#else
#define ArgEquals(X, Y, Z) (X == Y || X == Z)
#define ArgStartsWith(X, Y) (X.rfind(Y) == 0)
#endif

void ShowVersion()
{
    std::cout << "ascii-rename " << VERSION_STR << "\n";
}

void ShowHelp()
{
    std::cout << "Usage: ascii-rename [options...] [paths...]\n";
    std::cout << "-h, --help       Show this help and exit\n";
    std::cout << "-n, --no-op      Show what would happen but don't actually rename path(s)\n";
    std::cout << "-o, --overwrite  Overwrite existing paths(s)\n";
    std::cout << "-r, --recursive  Rename files and subdirectories recursively\n";
    std::cout << "-v, --verbose    Make the output more verbose\n";
    std::cout << "-V, --version    Show version number and exit\n";
}

struct PathItem
{
#ifdef _WIN32
    std::wstring Path;
#else
    std::string Path;
#endif
    bool SubsScanned;
};

int main_utf8(int argc, char **argv)
{
    if (argc <= 1)
    {
        std::cout << "ascii-rename: try \'ascii-rename --help\' for more information\n";
        return 0;
    }

    // Process arguments
    auto pathItems = std::list<PathItem>();

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
            pathItems.push_back({arg, false});
        }
    }

    // Process pathQueue
    int renames = 0;
    int skipped = 0;

    while (!pathItems.empty())
    {
        auto rawItem = pathItems.front();
        pathItems.pop_front();

        auto originalPathStr = std::string();
        AsciiRename::TryGetUtf8(rawItem.Path, originalPathStr);

        if (verbose)
        {
            std::cout << "Processing \"" << originalPathStr << "\"...\n";
        }

        auto originalPath = std::filesystem::path(rawItem.Path);

        auto asciiPathStr = std::string();
        AsciiRename::TryGetAscii(originalPathStr, asciiPathStr);

        auto asciiPath = std::filesystem::path(asciiPathStr);

        std::cout << "Ascii pathstr: \"" << asciiPathStr << "\"\n";
        std::cout << "Ascii path: \"" << asciiPath.string() << "\"\n";

        bool skip = false;
        bool skipForNow = false;

        if (!std::filesystem::exists(originalPath))
        {
            std::cerr << "ERROR: \"" << originalPathStr << "\" doesn't exist.\n";
            skip = true;
        }
        else
        {
            // Original path exists, get new path
            auto newPath = originalPath.parent_path() / asciiPath.filename();

            auto newPathStr = std::string();

            AsciiRename::TryGetUtf8(
#ifdef _WIN32
                newPath.wstring(),
#else
                newPath.string(),
#endif
                newPathStr);

            
            std::cout << "New pathstr: \"" << newPathStr << "\"\n";

            if (std::filesystem::is_directory(originalPath) && recursive && !rawItem.SubsScanned)
            {
                // Looking at a directory and recursive is true, so:
                // 1. Push item itself back onto front of list but with scanning disabled
                // 2. Push children ahead of parent on the list, so they'll get processed first

                if (verbose)
                {
                    std::cout << "Re-adding \"" << originalPathStr << "\" and children to queue...\n";
                }

                pathItems.push_front({rawItem.Path, true});

                for (const auto &child : std::filesystem::directory_iterator(originalPath))
                {
                    pathItems.push_front({
#ifdef _WIN32
                        child.path().wstring(),
#else
                        child.path().string(),
#endif
                        false});
                }

                skipForNow = true;
            }
            else if (originalPathStr == newPathStr)
            {
                // Path doesn't change with ASCII transliteration
                if (verbose)
                {
                    std::cout << "No need to rename \"" << originalPathStr << "\".\n";
                }
                skip = true;
            }
            else if (std::filesystem::exists(newPath) && !overwrite)
            {
                // New path already exists, but overwrite is false
                std::cerr << "ERROR: \"" << newPathStr << "\" already exists.\n";
                std::cerr << "ERROR: Specify --overwrite to overwrite.\n";
                skip = true;
            }
            else
            {
                // Just a single path rename
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

        if (skipForNow)
        {
            if (verbose)
            {
                std::cout << "Skipping \"" << originalPathStr << "\" for now...\n";
            }
        }
        else if (skip)
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
