#ifndef _CHAR_UTIL_H_
#define _CHAR_UTIL_H_
#include "type.h"

class CharUtil
{
public:
    static u4char_t  utf8byteToUCS4Char(const char** ub);
    static int       ucs4CharToUTF16Byte(u32 uchr, u16* ub);
    static char*     ucs4StrToUTF8Str(const u32 *ucs, size_t* u8slen=NULL);
    static int       ucs4slen(const u32 *ucs);
    static u4char_t* utf8StrToUcs4Str(const char *u8s, size_t* u4slen=NULL);

    static char*    mbsrtoutf8s(const char *mbs);
    static wchar_t* utf8srtowcs(const char *u8s);
    static wchar_t  mbrtowc(char** mb);
    static wchar_t* mbsrtowcs(const char *mb);
    static char*    wcsrtombs(const wchar_t *wc);
    static int      wcrtomb(char* s, wchar_t *wc);
};

#endif
