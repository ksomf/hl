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

#if !defined( _fltused ) // USE FLOATING POINT 
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

#if defined( hlWINDOWS )
  #define hlASSERT( exp ) if( !(exp) ){ *(i32 *)0 = 0; }
#else
  #define hlASSERT( exp ) if( !(exp) ){ *(volatile i32 *)0 = 0; }
#endif

#define _hlSTR( str ) #str
#define hlSTR( str ) _hlSTR( str )

#include <stdarg.h> // va_arg, ..

#define hlARRAY_COUNT( arr ) sizeof(arr) / sizeof(arr[0])

#include <stddef.h> // size_t
#include <stdint.h> // int8_t, ...
#if !defined( i8 )
  typedef int8_t  i8;
  typedef int16_t i16;
  typedef int32_t i32;
  typedef int64_t i64;
  hlSTATIC_ASSERT( sizeof( i8  ) == 1 );
  hlSTATIC_ASSERT( sizeof( i16 ) == 2 );
  hlSTATIC_ASSERT( sizeof( i32 ) == 4 );
  hlSTATIC_ASSERT( sizeof( i64 ) == 8 );
#endif
#if !defined( u8 )
  typedef uint8_t  u8;
  typedef uint16_t u16;
  typedef uint32_t u32;
  typedef uint64_t u64;
  hlSTATIC_ASSERT( sizeof( u8  ) == 1 );
  hlSTATIC_ASSERT( sizeof( u16 ) == 2 );
  hlSTATIC_ASSERT( sizeof( u32 ) == 4 );
  hlSTATIC_ASSERT( sizeof( u64 ) == 8 );
#endif
#if !defined( c8 )
  typedef char c8;
  hlSTATIC_ASSERT( sizeof( c8 ) == 1 );
#endif
#if !defined( r32 )
  typedef float r32;
  hlSTATIC_ASSERT( sizeof( r32 ) == 4 );
#endif
#if !defined( r64 )
  typedef double r64;
  hlSTATIC_ASSERT( sizeof( r64 ) == 8 );
#endif

#define hlR64_DIGIT_PRECISION 15                      //-- NUMBER OF DIGITS IN PRECISION --//
#define hlR64_DIGIT_MANTISSA  53                      //-- NUMBER OF DIGITS IN MANTISSA  --//
#define hlR64_EPSILON         2.2204460492503131E-16  //-- 1.0 + REAL64_EPSILON != 1.0   --//
#define hlR64_MIN             2.2250738585072014E-308 //-- MINIMUM REAL64                --//
#define hlR64_MAX             1.7976931348623157E+308 //-- MAXIMUM REAL64                --//

#define hlR32_LARGEST_ODD     16777215                //-- POINT AT WHICH (u)int32 MORE PRECISE THAN real32 --//
#define hlR32_DIGIT_PRECISION 6                       //-- NUMBER OF DIGITS IN PRECISION                    --//
#define hlR32_DIGIT_MANTISSA  24                      //-- NUMBER OF DIGITS IN MANTISSA                     --//
#define hlR32_EPSILON         1.19209290E-07f         //-- 1.0 + REAL32_EPSILON != 1.0                      --//
#define hlR32_MIN             1.17549435E-38f         //-- MINIMUM REAL32                                   --//
#define hlR32_MAX             3.40282347E+38f         //-- MAXIMUM REAL32                                   --//

#define hlKILOBYTES( a ) (( 1024LL *              a   ))
#define hlMEGABYTES( a ) (( 1024LL * hlKILOBYTES( a ) ))
#define hlGIGABYTES( a ) (( 1024LL * hlMEGABYTES( a ) ))

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
