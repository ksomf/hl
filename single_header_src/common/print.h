//---------------------------------------------------------//
//-- LICENSE -- See end of file for license information  --//
//---------------------------------------------------------//

#if !defined( _HL_PRINT )

HL_FUN_DEF i64 hl_snprintf(  c8 *buffer, u64 buffer_length, c8 *fmt, ...           );
HL_FUN_DEF i64 hl_vsnprintf( c8 *buffer, u64 buffer_length, c8 *fmt, va_list vargs );

typedef enum {
  HL_REAL_FIXED
, HL_REAL_SCIENTIFIC
, HL_REAL_SHORTEST
} hl_print_real_types;

HL_FUN_DEF c8 *hl_c8ptr_to_chars(   c8 *buffer_start, c8 *buffer_end, c8 *cstr );
HL_FUN_DEF c8 *hl_u64_to_chars(     c8 *buffer_start, c8 *buffer_end, u64 number );
HL_FUN_DEF c8 *hl_u64_to_chars_hex( c8 *buffer_start, c8 *buffer_end, u64 number, b use_upper_hex );
HL_FUN_DEF c8 *hl_i64_to_chars(     c8 *buffer_start, c8 *buffer_end, i64 number );
HL_FUN_DEF c8 *hl_r64_to_chars(     c8 *buffer_start, c8 *buffer_end, r64 number, hl_print_real_types real_format, b use_upper_case );

#define _HL_PRINT
#endif 

#if defined( HL_IMPLEMENTATION )

typedef struct {
  u64 width;
  u64 decimals;
  u64 flags;
} hl_parsed_print_format;

typedef enum {
  HL_LEFT_JUSTIFY     = 1
, HL_LEADING_PLUS     = 2
, HL_LEADING_SPACE    = 4
, HL_ZERO_PADDING     = 8
, HL_ALTERNATIVE_FORM = 16
, HL_32BYTE           = 32
} hl_print_parse_flags;

HL_FUN_DEF c8 *hl_c8ptr_to_chars( c8 *buffer_start, c8 *buffer_end, c8 *cstr ){
  u64 str_len = hl_cstr_len( cstr );
  HL_ASSERT( str_len < buffer_end - buffer_start  );
  for( u64 buffer_index = 0; buffer_index < str_len; ++buffer_index ){
     buffer_start[buffer_index] = cstr[buffer_index];
  }
  return buffer_start + str_len;
}

HL_FUN_DEF c8 *hl_u64_to_chars( c8 *buffer_start, c8 *buffer_end, u64 number ){
  c8 *buffer_position = 0;
  u64 base2_digits  = sizeof(u64)*HL_BITS_IN_BYTE - __builtin_clzll( number );
  u64 base10_digits = (( base2_digits + 1 ) * 1233 >> 12) + 1; //1233 >> 12 is approximately log2(10)^-1

  if( base10_digits < buffer_end - buffer_start ){
    buffer_position = buffer_start + base10_digits;
    for( c8 *write_ptr = buffer_position - 1; write_ptr >= buffer_start; --write_ptr ){
      *write_ptr = (number % 10) + '0';
      number /= 10;
    }
  }
  return buffer_position;
}

HL_FUN_DEF c8 *hl_i64_to_chars( c8 *buffer_start, c8 *buffer_end, i64 number ){
  HL_ASSERT( buffer_start != buffer_end );
  if( number < 0 ){
    *buffer_start++ = '-';
    number = -number;
  }
  c8 *buffer_position = hl_u64_to_chars( buffer_start, buffer_end, (u64)number );
  return buffer_position;
}

HL_FUN_DEF c8 *hl_u64_to_hex( c8 *buffer_start, c8 *buffer_end, u64 number, b use_upper_hex ){
  c8 *buffer_position = 0;
  c8 lower_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x' };
  c8 upper_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X' };
  HL_ASSERT( HL_ARRAY_COUNT( lower_hex ) == HL_ARRAY_COUNT( upper_hex ) ); //TODO MAKE STATIC
  c8 *hex_code = use_upper_hex ? upper_hex : lower_hex;
  
  u64 base2_digits  = sizeof(u64)*HL_BITS_IN_BYTE - __builtin_clzll( number ); //_lzcnt_u64( abs_integer );
  u64 base16_digits = (base2_digits+3) / 4;

  if( base16_digits < buffer_end - buffer_start ){
    buffer_position = buffer_start + base16_digits;
    for( c8 *write_ptr = buffer_position - 1; write_ptr >= buffer_start; --write_ptr ){
      *write_ptr = hex_code[number % 16];
      number /= 16;
    }
  }
  return buffer_position;
}

typedef union {
  r64 real;
  struct {
    u64 mantissa : 52;
    u64 exponent : 11;
    u64 sign : 1;
  } real_bits;
  u64 raw_bits;
} hl_ieee754_r64_representation;

HL_FUN_DEF c8 *hl_r64_to_chars( c8 *buffer_start, c8 *buffer_end, r64 number, hl_print_real_types real_format, b use_upper_case ){
  u64 maxed_11_bit_number = (2 << 12) - 1;
  u64 bias = 1023;

  hl_ieee754_r64_representation r64_representation = {};
                                r64_representation.real = number;
  b   negative = r64_representation.real_bits.sign;
  u64 exponent = r64_representation.real_bits.exponent;
  u64 mantissa = r64_representation.real_bits.mantissa;

  c8 *buffer_position = buffer_start;
  if( exponent == 0 || exponent == maxed_11_bit_number ){
    c8 *code;
    c8 *inf_codes[]  = { "inf", "INF" };
    c8 *zero_codes[] = { "0"  , "0"   };
    c8 *nan_codes[]  = { "nan", "NAN" };
    if( mantissa == 0 && negative ){
      buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "-" );
    }
    if( exponent == 0 && mantissa == 0 ){
      code = zero_codes[use_upper_case];
    }else if( exponent == maxed_11_bit_number && mantissa == 0 ){
      code = inf_codes[use_upper_case];
    }else{
      code = nan_codes[use_upper_case];
    }
    buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, code );
  }else{
    // normal subnormal real
    buffer_position = buffer_start;
  }

  return buffer_position;
}

HL_FUN_DEF c8 *hl_formatted_input_to_chars( c8 *buffer_start, c8 *buffer_end, c8 num_type, hl_parsed_print_format *fmt, va_list vargs ){
  HL_ASSERT( buffer_start != buffer_end );
  c8 scratch[512]; //CARRIES ANY 64 BIT OR LOWER STR FORMAT OF A NUMBER
  c8 lead[8] = {};

  c8 *buffer_position = buffer_start;
  
  b negative = false;
  c8 *scratch_start = scratch;
  c8 *scratch_end = scratch_start + HL_ARRAY_COUNT( scratch );
  switch( num_type ){
    case 'c':{
      c8 c = (c8)va_arg( vargs, i32 );
      scratch[0] = c;
      scratch_end = scratch_start + 1;
    }break;

    case 's':{
      scratch_start = va_arg( vargs, c8 * );
      scratch_end   = scratch_start + hl_cstr_len( scratch_start );
    }break;

    case 'u':{
      u64 number = fmt->flags & HL_32BYTE ? va_arg( vargs, u32 ) : va_arg( vargs, u64 );
      scratch_end = hl_u64_to_chars( scratch_start, scratch_end, number );
    }break;

    case 'X':
    case 'x':{
      u64 number = fmt->flags & HL_32BYTE ? va_arg( vargs, u32 ) : va_arg( vargs, u64 );
      scratch_end = hl_u64_to_hex ( scratch_start, scratch_end, number, num_type == 'X' );
      if( fmt->flags & HL_ALTERNATIVE_FORM ){
        lead[0] = '0';
        lead[1] = num_type;
      }
    }break;

    case 'i':
    case 'd':{
      i64 number = fmt->flags & HL_32BYTE ? va_arg( vargs, i32 ) : va_arg( vargs, i64 );
      scratch_end = hl_i64_to_chars( scratch_start, scratch_end, number );
      negative = number < 0;
    }break;

    case 'f':
    case 'F':{
      r64 number = va_arg( vargs, r64 ); 
      scratch_end = hl_r64_to_chars( scratch_start, scratch_end, number, HL_REAL_FIXED     , num_type == 'F' );
    }break;
    case 'e':
    case 'E':{
      r64 number = va_arg( vargs, r64 ); 
      scratch_end = hl_r64_to_chars( scratch_start, scratch_end, number, HL_REAL_SCIENTIFIC, num_type == 'E' );
    }break;
    case 'g':
    case 'G':{
      r64 number = va_arg( vargs, r64 ); 
      scratch_end = hl_r64_to_chars( scratch_start, scratch_end, number, HL_REAL_SHORTEST  , num_type == 'G' );

    }break;
  }
  u64 scratch_length  = scratch_end - scratch_start;

  if( negative ){
    *buffer_position++ = '-';
  }else if( fmt->flags & HL_LEADING_PLUS ){
    *buffer_position++ = '+';
  }else if( fmt->flags & HL_LEADING_SPACE ){
    *buffer_position++ = ' ';
  }

  u64 padding_bytes = fmt->width > scratch_length ? fmt->width - scratch_length : 0;
  if( !(fmt->flags & HL_LEFT_JUSTIFY) ){
    c8 padding_char   = fmt->flags & HL_ZERO_PADDING ? '0' : ' ';
    HL_ASSERT( padding_bytes < buffer_end - buffer_position );
    for( u64 padding_index = 0; padding_index < padding_bytes; ++padding_index ){
      *buffer_position++ = padding_char;
    }
  }

  HL_ASSERT( hl_cstr_len( lead ) < buffer_end - buffer_position );
  for( c8 *lead_position = lead; *lead_position; ++lead_position ){
    *buffer_position++ = *lead_position;
  }
  
  if( fmt->flags & HL_LEFT_JUSTIFY ){
    c8 padding_char   = fmt->flags & HL_ZERO_PADDING ? '0' : ' ';
    HL_ASSERT( padding_bytes < buffer_end - buffer_position );
    for( u64 padding_index = 0; padding_index < padding_bytes; ++padding_index ){
      *buffer_position++ = padding_char;
    }
  }


  HL_ASSERT( scratch_length < buffer_end - buffer_position );
  for( c8 *scratch_position = scratch_start; scratch_position < scratch_end; ++scratch_position ){
    *buffer_position++ = *scratch_position;
  }

  return buffer_position;
}
HL_FUN_DEF u64 _hl_printFormattedString( c8 *buffer, u64 buffer_length, c8 *string, hl_parsed_print_format* fmt ){
  if( string == 0 ){
    string = (c8 *)"null";
  }
  u64 written_bytes = 0;
  u64 str_len = hl_cstr_len( string );
  u64 padding_width = fmt->width > str_len ? fmt->width - str_len : 0;
  if( !(fmt->flags & HL_LEFT_JUSTIFY) ){
    for( u64 pad_byte = 0; pad_byte < padding_width && written_bytes < buffer_length; ++pad_byte, ++written_bytes ){
      *buffer++ = ' ';
    }
  }
  for( u64 string_byte = 0; string_byte < str_len && written_bytes < buffer_length; ++string_byte, ++written_bytes ){
    *buffer++ = string[string_byte];
  }
  if( fmt->flags & HL_LEFT_JUSTIFY ){
    for( u64 pad_byte = 0; pad_byte < padding_width && written_bytes < buffer_length; ++pad_byte, ++written_bytes ){
      *buffer++ = ' ';
    }
  }
  return written_bytes;
}

HL_FUN_DEF i64 hl_snprintf( c8 *buffer, u64 buffer_length, c8 *fmt, ... ){
  va_list vargs;
  va_start(vargs,fmt);
  i64 result = hl_vsnprintf( buffer, buffer_length, fmt, vargs );
  va_end(vargs);
  return result;
}

HL_FUN_DEF i64 hl_vsnprintf( c8 *buffer, u64 buffer_length, c8 *fmt, va_list vargs ){
  HL_ASSERT( buffer_length != 0 );
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
              print_fmt.flags |= HL_LEFT_JUSTIFY;
              ++fmt;
            }break;
            case '+':{
              print_fmt.flags |= HL_LEADING_PLUS;
              ++fmt;
            }break;
            case '0':{
              print_fmt.flags |= HL_ZERO_PADDING;
              ++fmt;
            }break;
            case '#':{
              print_fmt.flags |= HL_ALTERNATIVE_FORM;
              ++fmt;
            }break;
            case ' ':{
              print_fmt.flags |= HL_LEADING_SPACE;
              ++fmt;
            }break;
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
            print_fmt.flags |= HL_32BYTE;
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

  
        write_ptr = hl_formatted_input_to_chars( write_ptr, buffer+buffer_length-1, fmt[0], &print_fmt, vargs );
        ++fmt;
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
