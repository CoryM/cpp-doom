//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      Miscellaneous.
//

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <filesystem>
#include <fmt/core.h>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <string_view>

#include <sys/stat.h>
#include <sys/types.h>

#include "doomtype.hpp"

#include "deh_str.hpp"

#include "../utils/memory.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "i_video.hpp"
#include "m_misc.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

//
// Create a directory
//

void M_MakeDirectory(const char *path)
{
    mkdir(path, 0755);
}

// Check if a file exists
bool M_FileExists(std::filesystem::path path)
{
    return std::filesystem::is_regular_file(path);
}

// Check if a directory exists
bool M_DirExists(std::filesystem::path path)
{
    return std::filesystem::is_directory(path);
}


// Check if a file exists by probing for common case variation of its filename.
// Returns a newly allocated string that the caller is responsible for freeing.
std::string M_FileCaseExists(const std::string_view sv_path)
{
    std::vector<std::filesystem::path> pathList;

    // 0: actual path
    auto path = pathList.emplace_back(std::filesystem::path(sv_path));

    auto transformFileName = [](const auto &path, auto trans) {
        auto q = path; // Copy Path
        return q.replace_filename(trans(path.filename().string()));
    };

    // 1: lowercase filename, e.g. doom2.wad
    pathList.emplace_back(transformFileName(path, S_ForceLowercase));

    // 2: uppercase filename, e.g. DOOM2.WAD
    pathList.emplace_back(transformFileName(path, S_ForceUppercase));

    // 3. uppercase basename with lowercase extension, e.g. DOOM2.wad
    pathList.emplace_back(transformFileName(path, [](const std::string &str) {
        auto p = std::filesystem::path(S_ForceUppercase(str));
        return p.replace_extension(S_ForceLowercase(p.extension().string()));
    }));

    // 4. lowercase filename with uppercase first letter, e.g. Doom2.wad
    pathList.emplace_back(transformFileName(path, [](const std::string &str) {
        auto s = S_ForceLowercase(str);
        s[0]   = std::toupper(s[0]);
        return s;
    }));

    for (auto &p : pathList)
    {
        if (M_FileExists(p))
        {
            return p.string();
        }
    }

    // 5. no luck
    return std::string();
}


//
// Determine the length of an open file.
//
long M_FileLength(FILE *handle)
{
    long savedpos;
    long length;

    // save the current position in the file
    savedpos = ftell(handle);

    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}

//
// M_WriteFile
//

bool M_WriteFile(const char *name, const void *source, int length)
{
    FILE *handle;
    int   count;

    handle = fopen(name, "wb");

    if (handle == NULL)
        return false;

    count = fwrite(source, 1, length, handle);
    fclose(handle);

    if (count < length)
        return false;

    return true;
}


//
// M_ReadFile
//

int M_ReadFile(const char *name, byte **buffer)
{
    FILE *handle;
    int   count, length;
    byte *buf;

    handle = fopen(name, "rb");
    if (handle == NULL)
        S_Error(fmt::format("Couldn't read file {}", name));

    // find the size of the file by seeking to the end and
    // reading the current position

    length = M_FileLength(handle);

    buf   = zmalloc<byte *>(length + 1, PU::STATIC, NULL);
    count = fread(buf, 1, length, handle);
    fclose(handle);

    if (count < length)
    {
        S_Error(fmt::format("Couldn't read file {}", name));
    }

    buf[length] = '\0';
    *buffer     = buf;
    return length;
}

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.

char *M_TempFile(const char *s)
{
    // In Unix, just use /tmp.
    return M_StringJoin({ "/tmp", DIR_SEPARATOR_S, s });
}

bool M_StrToInt(const char *str, int *result)
{
    errno = 0;
    char *str_end;
    *result = std::strtol(str, &str_end, 0);
    return (str != str_end && errno == 0);
}

// Returns the directory portion of the given path, without the trailing
// slash separator character. If no directory is described in the path,
// the string "." is returned. In either case, the result is newly allocated
// and must be freed by the caller after use.
std::string M_DirName(const std::string_view path)
{
    const auto p = path.find(DIR_SEPARATOR);
    if (p == std::string::npos)
    {
        return std::string(".");
    }
    else
    {
        return std::string(path).substr(0, p - 1);
    }
}

// Returns the base filename described by the given path (without the
// directory name). The result points inside path and nothing new is
// allocated.
const char *M_BaseName(const char *path)
{
    const char *p;

    p = strrchr(path, DIR_SEPARATOR);
    if (p == NULL)
    {
        return path;
    }
    else
    {
        return p + 1;
    }
}

void M_ExtractFileBase(const char *path, char *dest)
{
    const char *src;
    const char *filename;
    int         length;

    src = path + strlen(path) - 1;

    // back up until a \ or the start
    while (src != path && *(src - 1) != DIR_SEPARATOR)
    {
        src--;
    }

    filename = src;

    // Copy up to eight characters
    // Note: Vanilla Doom exits with an error if a filename is specified
    // with a base of more than eight characters.  To remove the 8.3
    // filename limit, instead we simply truncate the name.

    length = 0;
    memset(dest, 0, 8);

    while (*src != '\0' && *src != '.')
    {
        if (length >= 8)
        {
            printf("Warning: Truncated '%s' lump name to '%.8s'.\n",
                filename, dest);
            break;
        }

        dest[length++] = toupper((int)*src++);
    }
}

//---------------------------------------------------------------------------
//
// PROC M_ForceUppercase
//
// Change string to uppercase.
//
//---------------------------------------------------------------------------
void M_ForceUppercase(char *text)
{
    char *p;

    for (p = text; *p != '\0'; ++p)
    {
        *p = toupper(*p);
    }
}
std::string S_ForceUppercase(const std::string_view source)
{
    auto dest = std::string(source.size(), '\0');
    std::transform(source.begin(), source.end(), dest.begin(),
        [](unsigned char c) -> unsigned char { return std::toupper(c); });
    return dest;
}

//---------------------------------------------------------------------------
//
// PROC M_ForceLowercase
//
// Change string to lowercase.
//
//---------------------------------------------------------------------------
void M_ForceLowercase(char *text)
{
    char *p;

    for (p = text; *p != '\0'; ++p)
    {
        *p = tolower(*p);
    }
}

std::string S_ForceLowercase(const std::string_view source)
{
    auto dest = std::string(source.size(), '\0');
    std::transform(source.begin(), source.end(), dest.begin(),
        [](unsigned char c) -> unsigned char { return std::tolower(c); });
    return dest;
}


//
// M_StrCaseStr
//
// Case-insensitive version of strstr()
//

const char *M_StrCaseStr(const char *haystack, const char *needle)
{
    unsigned int haystack_len;
    unsigned int needle_len;
    unsigned int len;
    unsigned int i;

    haystack_len = strlen(haystack);
    needle_len   = strlen(needle);

    if (haystack_len < needle_len)
    {
        return nullptr;
    }

    len = haystack_len - needle_len;

    for (i = 0; i <= len; ++i)
    {
        if (!strncasecmp(haystack + i, needle, needle_len))
        {
            return haystack + i;
        }
    }

    return nullptr;
}

//
// Safe version of strdup() that checks the string was successfully
// allocated.
//
auto M_StringDuplicate(const std::string_view orig) -> char *
{
    auto *result        = static_cast<char *>(malloc(orig.size() + 1));
    result[orig.size()] = '\0';

    if (std::strncpy(result, orig.data(), orig.size()) == nullptr)
    {
        S_Error(fmt::format("Failed to duplicate string (length {})\n", orig.size()));
    }

    return result;
}

// In trans to using strings / string_view instead of char*
auto S_StringDuplicate(const std::string_view orig) -> std::string
{
    return std::string(orig);
}

//
// String replace function.
//

auto M_StringReplace(const char *haystack, const char *needle,
    const char *replacement) -> char *
{
    // Iterate through occurrences of 'needle' and calculate the size of
    // the new string.
    auto  needle_len = strlen(needle);
    auto  result_len = strlen(haystack) + 1;
    auto *p          = haystack;

    for (;;)
    {
        p = strstr(p, needle);
        if (p == nullptr)
        {
            break;
        }

        p += needle_len;
        result_len += strlen(replacement) - needle_len;
    }

    // Construct new string.

    auto *result = static_cast<char *>(malloc(result_len));
    if (result == nullptr)
    {
        S_Error("M_StringReplace: Failed to allocate new string");
        return nullptr;
    }

    auto *dst     = result;
    auto  dst_len = result_len;
    p             = haystack;

    while (*p != '\0')
    {
        if (!strncmp(p, needle, needle_len))
        {
            M_StringCopy(dst, replacement, dst_len);
            p += needle_len;
            dst += strlen(replacement);
            dst_len -= strlen(replacement);
        }
        else
        {
            *dst = *p;
            ++dst;
            --dst_len;
            ++p;
        }
    }

    *dst = '\0';

    return result;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

bool M_StringCopy(char *dest, const char *src, size_t dest_size)
{
    size_t len;

    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
    else
    {
        return false;
    }

    len = strlen(dest);
    return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

bool M_StringConcat(char *dest, const char *src, size_t dest_size)
{
    size_t offset;

    offset = strlen(dest);
    if (offset > dest_size)
    {
        offset = dest_size;
    }

    return M_StringCopy(dest + offset, src, dest_size - offset);
}


// Returns true if 's' begins with the specified prefix.

bool M_StringStartsWith(const char *s, const char *prefix)
{
    return strlen(s) >= strlen(prefix)
           && strncmp(s, prefix, strlen(prefix)) == 0;
}

// Returns true if 's' ends with the specified suffix.

bool M_StringEndsWith(const char *s, const char *suffix)
{
    return strlen(s) >= strlen(suffix)
           && strcmp(s + strlen(s) - strlen(suffix), suffix) == 0;
}

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.

[[deprecated("replaced by S_StringJoin")]] char *M_StringJoin(const std::initializer_list<const std::string_view> &il_S)
{
    // Find out how much space we need
    const auto result_len = std::accumulate(il_S.begin(), il_S.end(), 1,
        [](const size_t &len, const auto &sv) { return len + sv.size(); });

    // allocate the space
    auto result = static_cast<char *>(malloc(result_len));

    // Copy the strings
    auto index = result;
    for (const auto &s : il_S)
    {
        index = std::copy(s.begin(), s.end(), index);
    };
    *index = '\0'; // Null terminate

    return result;
}

// Builds a string and returns a unique_ptr for the char * with needed std::free deleter
std::string S_StringJoin(const std::initializer_list<const std::string_view> &il_S)
{
    return std::string(M_StringJoin(il_S));
}

// Safe, portable vsnprintf().
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
    int result;

    if (buf_len < 1)
    {
        return 0;
    }

    // Windows (and other OSes?) has a vsnprintf() that doesn't always
    // append a trailing \0. So we must do it, and write into a buffer
    // that is one byte shorter; otherwise this function is unsafe.
    result = vsnprintf(buf, buf_len, s, args);

    // If truncated, change the final char in the buffer to a \0.
    // A negative result indicates a truncated buffer on Windows.
    if (result < 0 || result >= static_cast<int>(buf_len))
    {
        buf[buf_len - 1] = '\0';
        result           = buf_len - 1;
    }

    return result;
}

// Safe, portable snprintf().
int M_snprintf(char *buf, size_t buf_len, const char *s, ...)
{
    va_list args;
    int     result;
    va_start(args, s);
    result = M_vsnprintf(buf, buf_len, s, args);
    va_end(args);
    return result;
}
