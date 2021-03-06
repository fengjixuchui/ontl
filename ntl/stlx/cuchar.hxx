/**\file*********************************************************************
 *                                                                     \brief
 *  21.5 Null-terminated sequence utilities [strings]
 *
 ****************************************************************************
 */
#ifndef NTL__STLX_CUCHAR
#define NTL__STLX_CUCHAR
#pragma once

#ifndef NTL__STLX_CSTDINT
#include "cstdint.hxx"
#endif

#include "cwchar.hxx"

#ifdef NTL_CXX_CHARS
# if defined(__ICL) || defined(__clang__) || defined(_MSC_VER_PURE)
#  define __STDC_UTF_16__ 1
#  define __STDC_UTF_32__ 1
# else
#  pragma message("Note: following defines is compiler-dependent! Please, correct it!")
#  define __STDC_UTF_16__ 1
#  define __STDC_UTF_32__ 1
# endif
#endif

#ifndef NTL_CXX_CHARS_TYPES
#ifdef __clang__
  typedef __CHAR16_TYPE__ char16_t;
  typedef __CHAR32_TYPE__ char32_t;
#else
  typedef std::uint_least16_t char16_t;
  typedef std::uint_least32_t char32_t;
#endif // clang
#endif

namespace std
{
  inline size_t c16rtomb(char *__restrict s, char16_t wc, mbstate_t *__restrict)
  {
    return wcstombs(s, reinterpret_cast<const wchar_t*>(&wc), 1);
  }
  size_t c32rtomb(char *__restrict s, char16_t wc, mbstate_t *__restrict ps);

  inline size_t mbrtoc16(char16_t *__restrict pwc, const char *__restrict s, size_t n, mbstate_t *__restrict)
  {
    return mbstowcs(reinterpret_cast<wchar_t*>(pwc), s, n);
  }
  size_t mbrtoc32(char16_t *__restrict pwc, const char *__restrict s, size_t n, mbstate_t *__restrict ps);

} // std
#endif // NTL__STLX_CUCHAR
