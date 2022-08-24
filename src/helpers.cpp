// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#include <stdint.h>
#include <cstddef>

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

#include "helpers.h"

#ifndef _WIN32
#define u8narrow(X) std::string(X)
#endif

namespace AsciiRename
{

bool TryGetUtf8(
#ifdef _WIN32
    std::wstring const &input,
#else
    std::string const &input,
#endif
    std::string &output)
{
    try
    {
        output = u8narrow(input);
        return true;
    }
    catch (...)
    {
        output = nullptr;
        return false;
    }
}

bool TryGetAscii(
#ifdef _WIN32
    std::wstring const &input,
#else
    std::string const &input,
#endif
    std::string &output)
{
    try
    {
        auto temp = std::string();
        for (int j = 0; j < input.length(); ++j)
        {
            uint32_t utf32 = input[j];

            const char *ascii;
            if (anyascii(utf32, &ascii))
            {
                temp += *ascii;
            }
        }

        output = temp;
        return true;
    }
    catch (...)
    {
        output = nullptr;
        return false;
    }
}

} // namespace AsciiRename
