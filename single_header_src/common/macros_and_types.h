//---------------------------------------------------------//
//-- LICENSE -- See end of file for license information  --//
//---------------------------------------------------------//

#if !( defined(__x86_64__) || defined(_M_AMD64) )
  #error CODEBASE CURRENTLY SUPPORTS ONLY 64 BIT x86
#endif

#if defined( __clang__ )
  #define hlCLANG
#elif defined( _MSC_VER )
  #define hlMSVC
#elif defined( __GNUC__ )
  #define hlGCC
#endif

#if defined( __cplusplus )
  #define hlEXTERN extern "C"
#else
  #define hlEXTERN extern
#endif

#if defined( hlSTATIC )
  #define hlFUN_DEF static
#else
  #define hlFUN_DEF hlEXTERN
#endif

#if !defined( _fltused )
  #if defined( hlWINDOWS )
    hlEXTERN int _fltused = 0;
  #elif defined( hlGCC ) 
    hlEXTERN int _fltused;
  #endif
#endif

#define __hlSTATIC_ASSERT3( expr, msg ) typedef char static_assertion_##msg[(!!( expr ) ) * 2 - 1]
#define __hlSTATIC_ASSERT2( expr, ln ) __hlSTATIC_ASSERT3( expr, hl_static_insertion_failure_##ln )
#define __hlSTATIC_ASSERT1( expr, ln ) __hlSTATIC_ASSERT2( expr, ln )
#define hlSTATIC_ASSERT( expression )  __hlSTATIC_ASSERT1( expression, __LINE__ )

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer 
------------------------------------------------------------------------------
Alternative A -- MIT License
Copyright (c) 2019 Kim Somfleth

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
Alternative B -- Public Domain
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
------------------------------------------------------------------------------
*/
