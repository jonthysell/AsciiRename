// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#ifndef HELPERS_H
#define HELPERS_H

#include <string>

namespace AsciiRename
{

bool TryGetUtf8(
#ifdef _WIN32
    std::wstring const &input,
#else
    std::string const &input,
#endif
    std::string &output);

bool TryGetAscii(
#ifdef _WIN32
    std::wstring const &input,
#else
    std::string const &input,
#endif
    std::string &output);

} // namespace AsciiRename

#endif