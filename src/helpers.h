// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#ifndef HELPERS_H
#define HELPERS_H

#include <string>

namespace AsciiRename
{

void TrimTrailingPathSeparator(
#ifdef _WIN32
    std::wstring &s
#else
    std::string &s
#endif
);

bool TryGetUtf8(
#ifdef _WIN32
    std::wstring const &input,
#else
    std::string const &input,
#endif
    std::string &output);

bool TryGetAscii(std::string const &utf8Input, std::string &output);

bool IsValidWindowsFileName(std::string const &input);

} // namespace AsciiRename

#endif