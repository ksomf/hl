//---------------------------------------------------------//
//-- LICENSE -- See end of file for license information  --//
//---------------------------------------------------------//

#if !defined( _HL_PRINT )

hlFUN_DEF i32 hlPrint( c8 *buffer, i32 buffer_length, c8 *fmt, ... );

#define _HL_PRINT
#endif 

#if defined( HL_IMPLEMENTATION )
typedef struct {
  u64 fmt_length;
  u64 width;
  u64 decimals;
  u64 flags;
} hlParsed_Print_Format;
typedef enum {
  hlLEFT_JUSTIFY = 1
, hlLEADING_PLUS = 2
, hlZERO_PADDING = 4
, hlALTERNATIVE_FORM = 8
} hlPrint_Parse_Flags;

#include <stdio.h>
hlFUN_DEF i32 hlPrint( c8 *buffer, i32 buffer_length, c8 *fmt, ... ){
   hlASSERT( buffer_length != 0 );
   va_list args;
   va_start(args,fmt);
   c8 *write_ptr = buffer;
   while( fmt[0] && write_ptr < buffer + buffer_length ){
     if( fmt[0] == '%' ){
       if( fmt[1] == '%' ){
         *write_ptr++ = '%';
         fmt += 2;
       }else{
         ++fmt;
         hlParsed_Print_Format result = {};
         u64 parsed_flags = 0;
         while( !parsed_flags ){
           switch( fmt[0] ){
             case '-':{
               result.flags |= hlLEFT_JUSTIFY;
               ++fmt;
             }break;
             case '+':{
               result.flags |= hlLEADING_PLUS;
               ++fmt;
             }break;
             case '0':{
               result.flags |= hlZERO_PADDING;
               ++fmt;
             }break;
             case '#':{
               result.flags |= hlALTERNATIVE_FORM;
               ++fmt;
             }
             default:{
               parsed_flags = 1;
             }
           }
         }

         if( fmt[0] == '*' ){
           result.width = va_arg( args, u32 );
           ++fmt;
         }else{
           while( fmt[0] >= '0' && fmt[0] <= '9' ){
             result.width = 10 * result.width + fmt[0] - '0';
             ++fmt;
           }
         }

         if( fmt[0] == '.' ){
           ++fmt;
           if( fmt[0] == '*' ){
             result.decimals = va_arg( args, u32 );
           }else{
             while( fmt[0] >= '0' && fmt[0] <= '9' ){
               result.decimals = 10 * result.decimals + fmt[0] - '0';
               ++fmt;
             }
           }
         }

         switch( fmt[0] ){
           case 'h':{
             ++fmt;
             if( fmt[0] == 'h' ){
               ++fmt;
             }
           }break;
           case 'l':{
             ++fmt;
             if( fmt[0] == 'l' ){
               ++fmt;
             }
           }break;
           case 'L':{
             ++fmt;
           }break;
           case 'z':{
             ++fmt;
           }break;
           case 'j':{
             ++fmt;
           }break;
           case 't':{
             ++fmt;
           }break;
         }

         switch( fmt[0] ){
           case 'c':{
             c8 c = (c8)va_arg( args, i32 );
             *write_ptr++ = c;
             ++fmt;
           }break;
           case 's':{
             c8 *str = va_arg( args, c8 * );
             if( str == 0 ){
                str = (c8 *)"null";
             }
             ++fmt;
             while( *str && write_ptr < buffer + buffer_length ){
               *write_ptr++ = *str++;
             }
           }
         }

         //printf( "%s\n", fmt );

       }
     }else{
       *write_ptr++ = *fmt++;
     }
   }
   va_end(args);
   if( write_ptr == buffer + buffer_length ){
     write_ptr[-1] = 0;
   }else{
     write_ptr[0] = 0;
   }
   return buffer_length - (i32)(buffer - write_ptr);
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
