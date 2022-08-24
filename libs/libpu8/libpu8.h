/*
Copyright 2019 Johannes Feulner

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef libpu8_h__
#define libpu8_h__

/*
Enables portable utf-8. See also http://utf8everywhere.org/

Defines the functions u8widen and u8narrow that convert between utf-8 and wchar_t on windows
and do nothing elsewhere.
Defines the macro main_utf8 that can be used instead of main.
main_utf8 will then be called with utf8 arguments.

On windows, the streambuffers of cin, cerr and cout are replaced if attached to a console window
so that utf8 input and output works also on the console. This also works in DLLs as long as both the exe
and the DLL uses the DLL version of the C++ runtime.
If cin/cerr/cout are not attached to a console, is is assumed that in/output is already utf8, and no conversion takes place.

Make sure that the linker uses /ENTRY:WmainCRTStartup when building a non-console-windows application.

Usage example: my_main.cpp:

#include <libpu8.h>
// other includes were omitted

// argv are utf-8 strings
int main_utf8(int argc, char** argv)
{
  std::ofstream f(u8widen(argv[1])); // will also work on a non-windows OS that supports utf-8 natively
  if (!f)
    MessageBoxW(0, u8widen(string("Failed to open file ") + argv[1] + " for writing").c_str(), 0, 0);
  std::string line;
  std::getline(cin, line); // line will be utf-8 encoded
  std::cout << "You said: " << line;
  return 0;
}

Caveats:
C-input-output functions (like scanf, printf) will not work as expected on windows if attached to a console.
You need to stick to C++ cin/cout/cerr.
C functions that expect filenames (such as fopen) will also not work on windows.
You need to use C++ fstream.
*/


#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

class U8ConversionError : public std::runtime_error
{
public:
    U8ConversionError(const std::string& s) : std::runtime_error(s) {}
};

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <streambuf>
#include <cassert>
#include <cwchar>
#include <cstring>

static const bool u8_default_throw = true;

std::wstring u8widen(const char* s, size_t len, bool throw_on_inv_chars = u8_default_throw);

// u8widen and u8narrow throw a U8ConversionError if conversion failed and if throw_on_inv_chars is true.
// If throw_on_inv_chars is false, then invalid characters are silently replaced by a replacement character

inline std::wstring u8widen(const std::string& s, bool throw_on_inv_chars = u8_default_throw)
{
    return u8widen(s.data(), s.size(), throw_on_inv_chars);
}
inline std::wstring u8widen(const char* s, bool throw_on_inv_chars = u8_default_throw)
{
    return u8widen(s, std::strlen(s), throw_on_inv_chars);
}


std::string u8narrow(const wchar_t *s, size_t len, bool throw_on_inv_chars = u8_default_throw);

inline std::string u8narrow(const wchar_t *s, bool throw_on_inv_chars = u8_default_throw)
{
    return u8narrow(s, std::wcslen(s), throw_on_inv_chars);
}
inline std::string u8narrow(const std::wstring& s, bool throw_on_inv_chars = u8_default_throw)
{
    return u8narrow(s.data(), s.size(), throw_on_inv_chars);
}


class U8ConsoleOstreamBufWin32 : public std::stringbuf
{
public:
    U8ConsoleOstreamBufWin32(DWORD handleId) : m_handle(::GetStdHandle(handleId)) {}

    int sync()override;
private:
    static bool is_utf8_continuation(unsigned char c)
    {
        return ((c & 192) == 128);
    }
    static bool is_utf8_leading(unsigned char c)
    {
        return ((c & 192) == 192);
    }

    // may return more bytes than necessary
    static size_t num_trailing_partial_bytes(const std::string& s);
    std::string m_unwritten_partial_bytes;
    HANDLE m_handle;
};

class U8ConsoleIstreamBufWin32 : public std::streambuf
{
public:
    U8ConsoleIstreamBufWin32(DWORD handleId) : m_handle(::GetStdHandle(handleId)) 
    {
        setg(0, 0, 0);
    }
private:
    int_type underflow() override;
    std::string m_buffer;
    HANDLE m_handle;
};


class U8StdInStreamFixer
{
public:
    U8StdInStreamFixer(DWORD handleId, std::istream& stream)
        : m_stream(&stream), m_sb_backup(nullptr)
    {
        if (::GetFileType(::GetStdHandle(handleId)) == FILE_TYPE_CHAR)
        {
            m_csb.reset(new U8ConsoleIstreamBufWin32(handleId));
            m_sb_backup = stream.rdbuf();
            stream.rdbuf(m_csb.get());
        }
    }
    ~U8StdInStreamFixer()
    {
        if (m_sb_backup)
            m_stream->rdbuf(m_sb_backup);
    }
private:
    U8StdInStreamFixer(const U8StdInStreamFixer&) = delete;
    U8StdInStreamFixer& operator=(const U8StdInStreamFixer&) = delete;
    std::istream* m_stream;
    std::streambuf* m_sb_backup;
    std::unique_ptr<U8ConsoleIstreamBufWin32> m_csb;
};


class U8StdOutStreamFixer
{
public:
    U8StdOutStreamFixer(DWORD handleId, std::ostream& stream)
        : m_stream(&stream), m_sb_backup(nullptr)
    {
        if (::GetFileType(::GetStdHandle(handleId)) == FILE_TYPE_CHAR)
        {
            m_stream->flush();
            m_csb.reset(new U8ConsoleOstreamBufWin32(handleId));
            m_sb_backup = stream.rdbuf();
            stream.rdbuf(m_csb.get());
        }
    }
    ~U8StdOutStreamFixer()
    {
        if (m_sb_backup)
        {
            m_stream->flush();
            m_stream->rdbuf(m_sb_backup);
        }
    }
private:
    U8StdOutStreamFixer(const U8StdOutStreamFixer&) = delete;
    U8StdOutStreamFixer& operator=(const U8StdOutStreamFixer&) = delete;
    std::ostream* m_stream;
    std::streambuf* m_sb_backup;
    std::unique_ptr<U8ConsoleOstreamBufWin32> m_csb;
};

// for exception safety
struct u8_argv_buf
{
    u8_argv_buf(int _argc)
    {
        argc = _argc;
        argv = new char*[argc+1];
        for (int i=0; i <= argc; ++i)
            argv[i] = 0;
    }
    ~u8_argv_buf()
    {
        for (int i=0; i < argc; ++i)
            delete[] (argv[i]);
        delete[] argv;
    }
    int argc;
    char** argv;
};

struct u8_argv_copy
{
    u8_argv_copy(u8_argv_buf& ab)
    {
        argv = new char*[ab.argc + 1];
        memcpy(argv, ab.argv, (ab.argc + 1) * sizeof(char*));
    }
    ~u8_argv_copy()
    {
        delete[] argv;
    }
    char** argv;
};

#define main_utf8 \
main_utf8_(int argc, char** argv); \
int wmain(int argc, wchar_t *wargv[]) \
{ \
    U8StdInStreamFixer cinfix(STD_INPUT_HANDLE, std::cin); \
    U8StdOutStreamFixer coutfix(STD_OUTPUT_HANDLE, std::cout); \
    U8StdOutStreamFixer cerrfix(STD_ERROR_HANDLE, std::cerr); \
    u8_argv_buf ab(argc); \
    for (int i=0; i < argc; ++i) \
    { \
        int utf8_bytes = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0); \
        ab.argv[i] = new char[utf8_bytes]; \
        WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, ab.argv[i], utf8_bytes, 0, 0); \
    } \
    u8_argv_copy ab_copy(ab); \
    return main_utf8_(argc, ab_copy.argv); \
} \
int main_utf8_

#else
// non-windows systems usually support utf-8 natively, so narrow is not needed and widen does nothing.

#define main_utf8 main

inline std::string u8widen(const char* s, size_t len, bool = true)
{ return std::string(s, s+len); }
inline std::string u8widen(const std::string& s, bool = true)
{ return s; }
inline std::string u8widen(const char* s, bool = true)
{ return std::string(s); }


#endif

#endif //libpu8_h__
