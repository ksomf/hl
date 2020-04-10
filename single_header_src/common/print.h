//---------------------------------------------------------//
//-- LICENSE -- See end of file for license information  --//
//---------------------------------------------------------//

#if !defined( _HL_PRINT )

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

HL_FUN_DEF i64 hl_snprintf(  c8 *buffer, u64 buffer_length, c8 *fmt, ...           );
HL_FUN_DEF i64 hl_vsnprintf( c8 *buffer, u64 buffer_length, c8 *fmt, va_list vargs );

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
  c8 *buffer_position = 0;
  u64 str_len = hl_cstr_len( cstr );

  if( str_len < buffer_end - buffer_start  ){
    buffer_position = buffer_start;
    while( *cstr ){
      *buffer_position++ = *cstr++;
    }
  }

  return buffer_position;
}

HL_FUN_DEF c8 *hl_u64_to_chars( c8 *buffer_start, c8 *buffer_end, u64 number ){
  c8 *buffer_position = 0;
  u64 base10_digits = hl_u64_base10_digits( number );

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
  return hl_u64_to_chars( buffer_start, buffer_end, (u64)number );
}

HL_FUN_DEF c8 *hl_u64_to_hex( c8 *buffer_start, c8 *buffer_end, u64 number, b use_upper_hex ){
  c8 *buffer_position = 0;
  c8 lower_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x' };
  c8 upper_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X' };
  HL_ASSERT( HL_ARRAY_COUNT( lower_hex ) == HL_ARRAY_COUNT( upper_hex ) ); //TODO MAKE STATIC
  c8 *hex_code = use_upper_hex ? upper_hex : lower_hex;
  
  u64 base2_digits  = hl_u64_base2_digits( number );
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

#define R64_BIAS          1023
#define R64_MANTISSA_BITS 52
#define R64_EXPONENT_BITS 11
HL_STATIC_ASSERT( R64_MANTISSA_BITS + R64_EXPONENT_BITS + 1 == 8*sizeof(r64) );
typedef union {
  r64 real;
  struct {
    u64 mantissa : R64_MANTISSA_BITS;
    u64 exponent : R64_EXPONENT_BITS;
    u64 sign : 1;
  } real_bits;
  u64 raw_bits;
} hl_ieee754_r64_representation;

#include <stdio.h>
HL_FUN_DEF c8 *hl_r64_to_chars( c8 *buffer_start, c8 *buffer_end, r64 number, hl_print_real_types real_format, b use_upper_case ){
  u64 maxed_11_bit_number = (1 << 11) - 1;

  hl_ieee754_r64_representation r64_representation = {};
                                r64_representation.real = number;
  b   is_negative = r64_representation.real_bits.sign;
  u64 exponent    = r64_representation.real_bits.exponent;
  u64 mantissa    = r64_representation.real_bits.mantissa;
  //printf( "f2s: s:%i, e:%llu, m:%llu\n", is_negative, exponent, mantissa );

  c8 *buffer_position = buffer_start;
  if( is_negative ){
    buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "-" );
  }
  if( exponent == 0 && mantissa == 0 ){
    c8 *zero_format = 0;
    c8 *fixed_zero            = "0.000000";
    c8 *scientific_upper_zero = "0.000000E+00";
    c8 *scientific_lower_zero = "0.000000e+00";
    c8 *shortest_zero         = "0";
    if( real_format == HL_REAL_FIXED ){
      zero_format = fixed_zero;
    }else if( real_format == HL_REAL_SCIENTIFIC ){
      if( use_upper_case ){
        zero_format = scientific_upper_zero;
      }else{
        zero_format = scientific_lower_zero;
      }
    }else if( real_format == HL_REAL_SHORTEST ){
      zero_format = shortest_zero;
    }
    buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, zero_format );
  }else if( exponent == maxed_11_bit_number && mantissa == 0 ){
    if( use_upper_case ){
      buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "INF" );
    }else{
      buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "inf" );
    }
  }else if( exponent == 0 ){
    if( use_upper_case ){
      buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "NAN" );
    }else{
      buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "nan" );
    }
  }else{
    // Normalise exponent and mantissa between normal and subnormal before, as well as removing bias
    u64 normalised_mantissa;
    i64 normalised_exponent;
    if( exponent == 0 ){
      normalised_mantissa = mantissa;
      normalised_exponent = 1 - R64_BIAS - R64_MANTISSA_BITS;
    }else{
      normalised_mantissa = (1ull << R64_MANTISSA_BITS) | mantissa;
      normalised_exponent = exponent - R64_BIAS - R64_MANTISSA_BITS;
    }
    //printf( "normalised: m: %llu, e: %lli\n", normalised_mantissa, normalised_exponent );

    // Convert to a decimal base with a mantissa of n decimal digits 
    // a * 2^b -> a 5^-b 10^l x 10^(b-l) or a 2^b 10^(l-b) x 10^(b-l)
    // where
    // l = n + floor( b log10(5) - m log10(2) )
    // n = number of decimal digits
    // m = number of original binary mantissa digits
    u64 n = 7;
    u64 m = hl_u64_base2_digits( normalised_mantissa );
    i64 shifted_logs = (normalised_exponent*732923-m*315653);
    i64 l = n + (shifted_logs >> 20);
    //printf( "target_dec:%llu, current_bin:%llu, l:%lli\n", n, m, l );

    u64 target_mantissa = normalised_mantissa;
    i64 powers_of_two   = normalised_exponent;
    i64 powers_of_ten   = l - normalised_exponent;
    //printf( "Modifying by 10^(%+lli) 2^(%+lli)\n", powers_of_ten, powers_of_two );
    if( powers_of_two <= 0 ){
      while( powers_of_ten > 0 ){
        while( powers_of_two && target_mantissa > HL_U64_MAX / 10 ){
          target_mantissa >>= 1;
          ++powers_of_two;
        }
        target_mantissa *= 10;
        --powers_of_ten;
      }
      target_mantissa >>= -powers_of_two;
      while( powers_of_ten < 0 ){
        target_mantissa /= 10;
        ++powers_of_ten;
      }
    }else{
      while( powers_of_two > 0 ){
        while( powers_of_ten && target_mantissa > HL_U64_MAX / 1000 ){
          u64 remainder = target_mantissa % 10;
          target_mantissa /= 10;
          //printf( "Step back: %llu\n", garget_mantissa );
          ++powers_of_ten;
        }
        target_mantissa <<= 1;
        //printf( "Step forward: %llu\n", target_mantissa );
        --powers_of_two;
      }
      while( powers_of_ten < 0 ){
        target_mantissa = (target_mantissa + 5) / 10;
        //printf( "Equalize: %llu\n", target_mantissa );
        ++powers_of_ten;
      }
      while( powers_of_ten > 0 ){
        target_mantissa *= 10;
        //printf( "Equalize: %llu\n", target_mantissa );
        ++powers_of_ten;
      }
    }

    i64 target_exponent = normalised_exponent - l + n - 1;
    //printf( "target m: %llu, e:%lli\n", target_mantissa, target_exponent );
    c8 *buffer_start = buffer_position;
    buffer_position = hl_u64_to_chars( buffer_position+1, buffer_end, target_mantissa );
    buffer_start[0] = buffer_start[1];
    buffer_start[1] = '.';
    *buffer_position++ = 'e';
    if( target_exponent >= 0 ){
      *buffer_position++ = '+';
    }
    c8 *exponent_start = buffer_position;
    buffer_position = hl_i64_to_chars( buffer_position, buffer_end, target_exponent );
    if( buffer_position - exponent_start == 1 ){
      buffer_position[0] = buffer_position[-1];
      buffer_position[-1] = '0';
      ++buffer_position;
    }
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
