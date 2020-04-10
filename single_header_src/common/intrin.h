//---------------------------------------------------------//
//-- LICENSE -- See end of file for license information  --//
//---------------------------------------------------------//

#if !defined( _HL_INTRIN )

#include <immintrin.h>

static inline u64 hl_u64_base2_digits(  u64 number );
static        u64 hl_u64_base10_digits( u64 number );

#define _HL_INTRIN
#endif

#if defined( HL_IMPLEMENTATION )
static inline u64 hl_u64_base2_digits( u64 number ){
  u64 base2_digits = sizeof(u64)*HL_BITS_IN_BYTE - __builtin_clzll( number );
  return base2_digits;
}

//-- BRUTE FORCE METHOD FOR NOW --//
static u64 hl_u64_base10_digits( u64 number ){
  u64 base10_digits = 0; 
  if( number < 10 ){
    base10_digits = 1;
  }else if( number < 100 ){
    base10_digits = 2;
  }else if( number < 1000 ){
    base10_digits = 3;
  }else if( number < 10000 ){
    base10_digits = 4;
  }else if( number < 100000 ){
    base10_digits = 5;
  }else if( number < 1000000 ){
    base10_digits = 6;
  }else if( number < 10000000 ){
    base10_digits = 7;
  }else if( number < 100000000 ){
    base10_digits = 8;
  }else if( number < 1000000000 ){
    base10_digits = 9;
  }else if( number < 10000000000 ){
    base10_digits = 10;
  }else if( number < 100000000000 ){
    base10_digits = 11;
  }else if( number < 1000000000000 ){
    base10_digits = 12;
  }else if( number < 10000000000000 ){
    base10_digits = 13;
  }else if( number < 100000000000000 ){
    base10_digits = 14;
  }else if( number < 1000000000000000 ){
    base10_digits = 15;
  }else if( number < 10000000000000000 ){
    base10_digits = 16;
  }else if( number < 100000000000000000 ){
    base10_digits = 17;
  }else if( number < 1000000000000000000 ){
    base10_digits = 18;
  }
  return base10_digits;
}
#endif
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
