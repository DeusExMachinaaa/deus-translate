#include "encoding.h"

#include <string>

namespace enc {

// Extrae el número de code page de una cadena como "cp1251", "1251" o
// "windows-1251". Devuelve 0 si no encuentra dígitos.
static unsigned ParseCodepage(const std::string& charset) {
    unsigned cp = 0;
    bool any = false;
    for (size_t i = 0; i < charset.size(); ++i) {
        char c = charset[i];
        if (c >= '0' && c <= '9') { cp = cp * 10 + (unsigned)(c - '0'); any = true; }
        else if (any) break; // corta al primer no-dígito tras los dígitos
    }
    return any ? cp : 0;
}

} // namespace enc

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace enc {

bool Utf8ToCodepage(const std::string& utf8, const std::string& charset, std::string& out) {
    unsigned cp = ParseCodepage(charset);
    if (cp == 0) return false;

    // UTF-8 -> UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    if (wlen <= 0) { out = utf8; return true; }
    std::wstring wide((size_t)wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wide[0], wlen);

    // UTF-16 -> code page destino (caracteres sin representación -> '?')
    const char def = '?';
    BOOL usedDef = FALSE;
    int mlen = WideCharToMultiByte((UINT)cp, 0, wide.c_str(), wlen, NULL, 0, &def, &usedDef);
    if (mlen <= 0) return false; // code page no válido en este sistema
    out.assign((size_t)mlen, '\0');
    WideCharToMultiByte((UINT)cp, 0, wide.c_str(), wlen, &out[0], mlen, &def, &usedDef);
    return true;
}

} // namespace enc

#else // POSIX (Linux) -> iconv (provisto por glibc)

#include <iconv.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

namespace enc {

static bool ConvertWith(const char* tocode, const std::string& utf8, std::string& out) {
    iconv_t cd = iconv_open(tocode, "UTF-8");
    if (cd == (iconv_t)-1) return false;

    size_t inLeft = utf8.size();
    char* inBuf = const_cast<char*>(utf8.data());

    std::string result;
    char buf[4096];
    bool ok = true;
    while (inLeft > 0) {
        char* outBuf = buf;
        size_t outLeft = sizeof(buf);
        size_t r = iconv(cd, &inBuf, &inLeft, &outBuf, &outLeft);
        result.append(buf, sizeof(buf) - outLeft);
        if (r == (size_t)-1) {
            if (errno == E2BIG) continue;      // buffer lleno: seguir
            if (errno == EILSEQ || errno == EINVAL) {
                // carácter sin representación / secuencia incompleta -> '?'
                result.push_back('?');
                if (inLeft > 0) { ++inBuf; --inLeft; }
                continue;
            }
            ok = false;
            break;
        }
    }
    iconv_close(cd);
    if (ok) out.swap(result);
    return ok;
}

bool Utf8ToCodepage(const std::string& utf8, const std::string& charset, std::string& out) {
    unsigned cp = ParseCodepage(charset);
    if (cp == 0) return false;

    char tocode[32];
    // //TRANSLIT hace que iconv sustituya lo no representable en lugar de fallar.
    std::snprintf(tocode, sizeof(tocode), "CP%u//TRANSLIT", cp);
    if (ConvertWith(tocode, utf8, out)) return true;

    std::snprintf(tocode, sizeof(tocode), "CP%u", cp);
    return ConvertWith(tocode, utf8, out);
}

} // namespace enc

#endif
