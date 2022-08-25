// Copyright (c) Jon Thysell <http://jonthysell.com>
// Licensed under the MIT License.

#include <cstddef>
#include <stdint.h>

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
#include <utf8.h>

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

// Adapted from https://github.com/anyascii/anyascii/blob/0.3.1/impl/c/test.c
static void anyascii_string(const char *in, char *out)
{
    uint32_t utf32;
    uint32_t state = 0;
    size_t rlen;
    for (; *in; in++)
    {
        utf8_decode(&state, &utf32, (unsigned char)*in);
        switch (state)
        {
        case UTF8_ACCEPT:;
            const char *r;
            rlen = anyascii(utf32, &r);
            memcpy(out, r, rlen);
            out += rlen;
            break;
        case UTF8_REJECT:
            state = UTF8_ACCEPT;
            break;
        }
    }
    *out = 0;
}

bool TryGetAscii(std::string const &utf8Input, std::string &output)
{
    char *buffer = new char[utf8Input.length()];
    try
    {
        anyascii_string(utf8Input.c_str(), buffer);

        output = std::string(buffer);
        //delete buffer;
        return true;
    }
    catch (...)
    {
        output = nullptr;
        //delete buffer;
        return false;
    }
}

} // namespace AsciiRename
