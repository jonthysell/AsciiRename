/*
Copyright 2019 Johannes Feulner

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef _WIN32

#include "libpu8.h"

#include <cstdint>

std::wstring u8widen(const char *s, size_t len, bool throw_on_inv_chars)
{
    if (!len)
        return std::wstring();
    if (len < size_t(INT_MAX))
    {
        int ilen = int(len);
        DWORD flags = throw_on_inv_chars ? MB_ERR_INVALID_CHARS : 0;
        int num_wchars = MultiByteToWideChar(CP_UTF8, flags, s, ilen, 0, 0);
        if (num_wchars > 0)
        {
            std::wstring result;
            result.resize(size_t(num_wchars));
            wchar_t *pout = const_cast<wchar_t *>(result.data());
            MultiByteToWideChar(CP_UTF8, flags, s, ilen, pout, int(result.size()));
            return result;
        }
    }
    throw U8ConversionError("utf8 to wide-string conversion failed.");
}

std::string u8narrow(const wchar_t *s, size_t len, bool throw_on_inv_chars)
{
    if (!len)
        return std::string();
    if (len < size_t(INT_MAX))
    {
        int ilen = int(len);
        DWORD flags = throw_on_inv_chars ? WC_ERR_INVALID_CHARS : 0;
        int utf8_bytes = WideCharToMultiByte(CP_UTF8, flags, s, ilen, 0, 0, 0, 0);
        if (utf8_bytes > 0)
        {
            std::string result;
            result.resize(size_t(utf8_bytes));
            char *pout = const_cast<char *>(result.data());
            WideCharToMultiByte(CP_UTF8, flags, s, ilen, pout, utf8_bytes, 0, 0);
            return result;
        }
    }
    throw U8ConversionError("wide-string to utf8 conversion failed.");
}

unsigned num_succeeding_bytes(unsigned char c)
{
    switch (c >> 5)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        // ascii byte
        return 0;
    case 4:
    case 5:
        assert(false); // this is a continuation byte
        return 0;
    case 6:
        return 1;
    case 7:
        if (c & (1 << 5))
            return 3;
        return 2;
    default:
        assert(false);
        return unsigned(-1);
    }
}

U8ConsoleIstreamBufWin32::int_type U8ConsoleIstreamBufWin32::underflow()
{
    if (gptr() >= egptr())
    {
        wchar_t wideBuffer[128];
        DWORD readSize;
        if (!::ReadConsoleW(m_handle, wideBuffer, ARRAYSIZE(wideBuffer) - 2, &readSize, NULL))
            return traits_type::eof();
        if (readSize > 0 && IS_HIGH_SURROGATE(wideBuffer[readSize - 1]))
        {
            // try to read one more character
            DWORD extraReadSize;
            if (::ReadConsoleW(m_handle, wideBuffer + readSize, 1, &extraReadSize, NULL) && extraReadSize == 1)
                ++readSize;
        }

        wideBuffer[readSize] = L'\0';
        m_buffer = u8narrow(wideBuffer);

        setg(&m_buffer[0], &m_buffer[0], &m_buffer[0] + m_buffer.size());

        if (gptr() >= egptr())
        {
            return traits_type::eof();
        }
    }

    return sgetc();
}

int U8ConsoleOstreamBufWin32::sync()
{
    std::string buffer = m_unwritten_partial_bytes + str();
    size_t partial_trailing = num_trailing_partial_bytes(buffer);
    m_unwritten_partial_bytes = std::string(buffer.end() - partial_trailing, buffer.end());
    str(""); // clear stringbuf's buffer
    buffer.resize(buffer.size() - partial_trailing);
    if (buffer.empty())
        return 0;

    std::wstring wideBuffer = u8widen(buffer);
    DWORD writtenSize;
    ::WriteConsoleW(m_handle, wideBuffer.c_str(), DWORD(wideBuffer.size()), &writtenSize, NULL);

    return 0;
}
size_t U8ConsoleOstreamBufWin32::num_trailing_partial_bytes(const std::string &s)
{
    if (s.empty())
        return 0;
    if (is_utf8_leading(*s.rbegin()))
        return 1;
    size_t result = 0;
    // skip any trailing continuation bytes
    while (result < s.size() && is_utf8_continuation(s[s.size() - result - 1]))
        ++result;
    if (result && result < s.size())
    {
        // there is a non-continuation byte left
        if (num_succeeding_bytes(s[s.size() - result - 1]) == result)
            return 0; // s ends with a complete multi-byte sequence
        // s ends with an incomplete multi-byte sequence
        ++result;
    }
    return result;
}

#endif //_WIN32
