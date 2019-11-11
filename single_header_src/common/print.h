//---------------------------------------------------------//
//-- LICENSE -- See end of file for license information  --//
//---------------------------------------------------------//

#if !defined( _HL_PRINT )

hlFUN_DEF i64 hlPrint ( c8 *buffer, u64 buffer_length, c8 *fmt, ...           );
hlFUN_DEF i64 hlVPrint( c8 *buffer, u64 buffer_length, c8 *fmt, va_list vargs );

#define _HL_PRINT
#endif 

#if defined( HL_IMPLEMENTATION )

typedef struct {
  u64 width;
  u64 decimals;
  u64 flags;
} hl_parsed_print_format;

typedef enum {
  hlLEFT_JUSTIFY     = 1
, hlLEADING_PLUS     = 2
, hlLEADING_SPACE    = 4
, hlZERO_PADDING     = 8
, hlALTERNATIVE_FORM = 16
, hl32BYTE           = 32
, hlNEGATIVE         = 64
, hlUPPER_CASE       = 128
} hlPrint_Parse_Flags;

#include <stdio.h>
#define hlPRINT_NUMBER_BUFFER_SIZE 512
#define hlPRINT_MAX_I8_SIZE   4
#define hlPRINT_MAX_I16_SIZE  6
#define hlPRINT_MAX_I32_SIZE 10
#define hlPRINT_MAX_I64_SIZE 20
hlFUN_DEF u64 _hlPrintFormattedHex ( c8 *buffer, u64 buffer_length, u64 abs_integer, hl_parsed_print_format *fmt ){
  hlASSERT( buffer_length );
  u64 written_bytes  = 0;

  c8 lower_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x' };
  c8 upper_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X' };
  hlASSERT( hlARRAY_COUNT( lower_hex ) == hlARRAY_COUNT( upper_hex ) ); //TODO MAKE STATIC
  c8 *hex_code = fmt->flags & hlUPPER_CASE ? upper_hex : lower_hex;

  if( fmt->flags & hlALTERNATIVE_FORM ){
    hlASSERT( buffer_length - written_bytes >= 2 );
    *buffer++ = '0';
    *buffer++ = hex_code[hlARRAY_COUNT(lower_hex)-1];
    written_bytes += 2;
  }
  u64 log2_digits  = sizeof(u64)*hlBITS_IN_BYTE - __builtin_clzll( abs_integer ); //_lzcnt_u64( abs_integer );
  u64 log16_digits = (log2_digits+3) / 4;
  hlASSERT( log16_digits < buffer_length - written_bytes );
  for( u64 hex_byte = log16_digits; hex_byte; --hex_byte ){
    c8 digit    = hex_code[ abs_integer % 16 ];
    abs_integer /= 16;
    buffer[ hex_byte - 1 ] = digit;
  }
  written_bytes += log16_digits;
  return written_bytes;
}

hlFUN_DEF u64 _hlPrintFormattedInteger( c8 *buffer, u64 buffer_length, u64 abs_integer, hl_parsed_print_format *fmt ){
  hlASSERT( buffer_length );
  u64 written_bytes  = 0;
  u64 log2_digits    = sizeof(u64)*hlBITS_IN_BYTE - __builtin_clzll( abs_integer ); //_lzcnt_u64( abs_integer );
  u64 log10_digits   = (( log2_digits + 1 ) * 1233 >> 12) + 1; //1233 >> 12  is approximately log2(10)^-1
  u64 padding_width = fmt->width > log10_digits ? fmt->width - log10_digits : 0;
  c8  padding_char  = fmt->flags & hlZERO_PADDING ? '0' : ' ';
  if( fmt->flags & hlNEGATIVE ){
    *buffer++ = '-';
    --buffer_length;
  }else if( fmt->flags & hlLEADING_PLUS ){
    *buffer++ = '+';
    --buffer_length;
  }else if( fmt->flags & hlLEADING_SPACE ){
    *buffer++ = ' ';
    --buffer_length;
  }
  for( u64 pad_byte = 0; pad_byte< padding_width && written_bytes < buffer_length; ++pad_byte, ++written_bytes ){
    *buffer++ = padding_char;
  }
  hlASSERT( log10_digits < buffer_length - written_bytes ); //TODO FIX THIS CASE? OR SHOULD IT BE LEFT AS IS FOR EASY FAIL 
  for( u64 integer_byte = log10_digits; integer_byte; --integer_byte ){ 
    c8 digit = abs_integer % 10 + '0';
    abs_integer /= 10;
    buffer[ integer_byte - 1 ] = digit;
  }
  written_bytes += log10_digits;
  buffer        += log10_digits;
  return written_bytes;
}

hlFUN_DEF u64 _hlPrintFormattedString( c8 *buffer, u64 buffer_length, c8 *string, hl_parsed_print_format* fmt ){
  if( string == 0 ){
    string = (c8 *)"null";
  }
  u64 written_bytes = 0;
  u64 str_len = hlCStrLen( string );
  u64 padding_width = fmt->width > str_len ? fmt->width - str_len : 0;
  if( !(fmt->flags & hlLEFT_JUSTIFY) ){
    for( u64 pad_byte = 0; pad_byte < padding_width && written_bytes < buffer_length; ++pad_byte, ++written_bytes ){
      *buffer++ = ' ';
    }
  }
  for( u64 string_byte = 0; string_byte < str_len && written_bytes < buffer_length; ++string_byte, ++written_bytes ){
    *buffer++ = string[string_byte];
  }
  if( fmt->flags & hlLEFT_JUSTIFY ){
    for( u64 pad_byte = 0; pad_byte < padding_width && written_bytes < buffer_length; ++pad_byte, ++written_bytes ){
      *buffer++ = ' ';
    }
  }
  return written_bytes;
}

hlFUN_DEF i64 hlPrint( c8 *buffer, u64 buffer_length, c8 *fmt, ... ){
  va_list vargs;
  va_start(vargs,fmt);
  i64 result = hlVPrint( buffer, buffer_length, fmt, vargs );
  va_end(vargs);
  return result;
}

hlFUN_DEF i64 hlVPrint( c8 *buffer, u64 buffer_length, c8 *fmt, va_list vargs ){
  hlASSERT( buffer_length != 0 );
  c8 *write_ptr = buffer;
  while( fmt[0] && write_ptr < buffer + buffer_length ){
    if( fmt[0] == '%' ){
      if( fmt[1] == '%' ){
        *write_ptr++ = '%';
        fmt += 2;
      }else{
        ++fmt;
        hl_parsed_print_format print_fmt = {};
        u64 parsed_flags = 0;
        while( !parsed_flags ){
          switch( fmt[0] ){
            case '-':{
              print_fmt.flags |= hlLEFT_JUSTIFY;
              ++fmt;
            }break;
            case '+':{
              print_fmt.flags |= hlLEADING_PLUS;
              ++fmt;
            }break;
            case '0':{
              print_fmt.flags |= hlZERO_PADDING;
              ++fmt;
            }break;
            case '#':{
              print_fmt.flags |= hlALTERNATIVE_FORM;
              ++fmt;
            }
            case ' ':{
              print_fmt.flags |= hlLEADING_SPACE;
            }
            default:{
              parsed_flags = 1;
            }
          }
        }

        if( fmt[0] == '*' ){
          print_fmt.width = va_arg( vargs, u32 );
          ++fmt;
        }else{
          while( fmt[0] >= '0' && fmt[0] <= '9' ){
            print_fmt.width = 10 * print_fmt.width + fmt[0] - '0';
            ++fmt;
          }
        }

        if( fmt[0] == '.' ){
          ++fmt;
          if( fmt[0] == '*' ){
            print_fmt.decimals = va_arg( vargs, u32 );
          }else{
            while( fmt[0] >= '0' && fmt[0] <= '9' ){
              print_fmt.decimals = 10 * print_fmt.decimals + fmt[0] - '0';
              ++fmt;
            }
          }
        }

        switch( fmt[0] ){
          case 'h':{
            print_fmt.flags |= hl32BYTE;
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
            c8 c = (c8)va_arg( vargs, i32 );
            *write_ptr++ = c;
            ++fmt;
          }break;
          case 's':{
            c8 *str = va_arg( vargs, c8 * );
            u64 written_bytes = _hlPrintFormattedString( write_ptr, buffer_length - (write_ptr-buffer), str, &print_fmt );
            write_ptr += written_bytes;
            ++fmt;
          }break;
          case 'u':
          case 'i':
          case 'd': {
            i64 integer;
            if( print_fmt.flags & hl32BYTE ){
              integer = va_arg( vargs, i32 );
            }else{
              integer = va_arg( vargs, i64 );
            }
            u64 abs_integer = (u64)integer;
            if( fmt[0] != 'u' && integer < 0 ){
              abs_integer = (u64)-integer;
              print_fmt.flags |= hlNEGATIVE;
            }
            u64 written_bytes = _hlPrintFormattedInteger( write_ptr, buffer_length - (write_ptr-buffer), abs_integer, &print_fmt );
            write_ptr += written_bytes;
            ++fmt;
          }break;
          case 'X':
            print_fmt.flags |= hlUPPER_CASE;
          case 'x':{
            u64 abs_integer;
            if( print_fmt.flags & hl32BYTE ){
              abs_integer = va_arg( vargs, i32 );
            }else{
              abs_integer = va_arg( vargs, i64 );
            }
            u64 written_bytes = _hlPrintFormattedHex( write_ptr, buffer_length - (write_ptr-buffer), abs_integer, &print_fmt );
            write_ptr += written_bytes;
            ++fmt;
          }break;
        }


      }
    }else{
      *write_ptr++ = *fmt++;
    }
  }
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
