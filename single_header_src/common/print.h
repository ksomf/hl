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

#include <stdio.h>
HL_FUN_DEF c8 *hl_u64_to_chars( c8 *buffer_start, c8 *buffer_end, u64 number ){
  c8 *buffer_position = 0;
  u64 base10_digits = hl_u64_base10_digits( number );
  printf( "%llu:  %llu\n", number, base10_digits );

  if( base10_digits < buffer_end - buffer_start ){
    buffer_position = buffer_start + base10_digits;
    for( c8 *write_ptr = buffer_position - 1; write_ptr >= buffer_start; --write_ptr ){
      *write_ptr = (number % 10) + '0';
      number /= 10;
    }
  }
  return buffer_position;
}

HL_FUN_DEF c8 *hl_u128_to_chars( c8 *buffer_start, c8 *buffer_end, u128 number ){
  c8 *buffer_position = 0;
  u64 base2_digits  = number > HL_U64_MAX ? 64 + hl_u64_base2_digits( (u64)(number >> 64) ) : hl_u64_base2_digits( (u64)number );
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
  printf( "f2s: s:%i, e:%llu, m:%llu\n", is_negative, exponent, mantissa );

  c8 *buffer_position = buffer_start;
  if( exponent == 0 && mantissa == 0 ){
    c8 *zero_format = 0;
    c8 *fixed_zero            = "0.000000";
    c8 *scientific_upper_zero = "0.000000E+00";
    c8 *scientific_lower_zero = "0.000000e+00";
    c8 *shortest_zero         = "0";
    if( is_negative ){
      buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "-" );
    }
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
  }else if( exponent == 0 || exponent == maxed_11_bit_number ){
    c8 *code;
    c8 *inf_codes[]  = { "inf", "INF" };
    c8 *nan_codes[]  = { "nan", "NAN" };
    if( exponent == maxed_11_bit_number && mantissa == 0 ){
      code = inf_codes[use_upper_case];
    }else{
      if( is_negative ){
        buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, "-" );
      }
      code = nan_codes[use_upper_case];
    }
    buffer_position = hl_c8ptr_to_chars( buffer_position, buffer_end, code );
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
    printf( "normalised: m: %llu, e: %lli\n", normalised_mantissa, normalised_exponent );

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
    printf( "target_dec:%llu, current_bin:%llu, l:%lli\n", n, m, l );

    u64 target_mantissa = normalised_mantissa;
    i64 powers_of_two   = normalised_exponent;
    i64 powers_of_ten   = l - normalised_exponent;
    printf( "Modifying by 10^(%+lli) 2^(%+lli)\n", powers_of_ten, powers_of_two );
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
          //printf( "Step back: %llu\n", target_mantissa );
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
    printf( "target m: %llu, e:%lli\n", target_mantissa, target_exponent );
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


    //i64 target_exponent = -(normalised_exponent + l) + target_mantissa_decimal_digits + 1;
    //printf( "target m: %llu, e:%lli\n", target_mantissa, target_exponent );
    //c8 *double_start = buffer_position;
    //buffer_position = hl_u64_to_chars( buffer_position+1, buffer_end, target_mantissa );
    //double_start[0] = double_start[1];
    //double_start[1] = '.';
    //*buffer_position++ = 'e';
    //if( target_exponent >= 0 ){
    //  *buffer_position++ = '+';
    //}
    //c8 *exponent_start = buffer_position;
    //buffer_position = hl_i64_to_chars( buffer_position, buffer_end, target_exponent );
    //if( buffer_position - exponent_start == 1 ){
    //  buffer_position[0] = buffer_position[-1];
    //  buffer_position[-1] = '0';
    //  ++buffer_position;
    //}

    //// normal or subnormal real
    //// doi: 10.1145/3192366.3192369
    //// Step 1: Decode float into mantissa and exponent common between normal and subnormal
    //u128 normalised_mantissa;
    //i64  normalised_exponent;
    //if( exponent == 0 ){ //subnormal
    //  normalised_mantissa = mantissa;
    //  normalised_exponent = 1 - R64_BIAS - R64_MANTISSA_BITS;
    //}else{
    //  normalised_mantissa = (1ull << R64_MANTISSA_BITS) | mantissa;
    //  normalised_exponent = exponent - R64_BIAS - R64_MANTISSA_BITS;
    //}
    //printf( "normalised: m: %llu, e: %lli\n", (u64)normalised_mantissa, normalised_exponent );

    //// Step 2: Determine interval of information preserving
    //normalised_exponent = normalised_exponent - 2; // needed to garantee positivity of operations below
    //u64 lower_offset = mantissa == 0 && exponent > 1 ? 1 : 2;
    //u128 preserving_mantissa_base2_lower  = 4 * normalised_mantissa - lower_offset;
    //u128 preserving_mantissa_base2_middle = 4 * normalised_mantissa;
    //u128 preserving_mantissa_base2_upper  = 4 * normalised_mantissa + 2;
    //c8 buf21[256] = {};
    //c8 buf22[256] = {};
    //c8 buf23[256] = {};
    //hl_u128_to_chars( buf21, buf21 + HL_ARRAY_COUNT( buf21 ), preserving_mantissa_base2_lower  );
    //hl_u128_to_chars( buf22, buf22 + HL_ARRAY_COUNT( buf22 ), preserving_mantissa_base2_middle );
    //hl_u128_to_chars( buf23, buf23 + HL_ARRAY_COUNT( buf23 ), preserving_mantissa_base2_upper  );
    //printf( "e2,(u,v,w): %lli, (%s,%s,%s)\n", normalised_exponent, buf21, buf22, buf23 );

    //// Step 3: Convert interval and exponenent to decimal representation, but one that fits into memory after dropping no longer required precision
    //i64 exponent_base10;
    //u128 preserving_mantissa_base10_lower;
    //u128 preserving_mantissa_base10_middle;
    //u128 preserving_mantissa_base10_upper;
    ////if( normalised_exponent >= 0 ){
    ////  u64 q = (normalised_exponent * 631306) >> 20; // (1<<20)*log_10(2) = 315652.8
    ////      q -= 1;
    ////  u64 k = 125 + ((q * 2434718) >> 20);          // (1<<20)*ln(2)/ln(5) = 2434718
    ////  printf( "%llu,%llu\n", q, k );
    ////}else{
    //{
    //  exponent_base10 = normalised_exponent;
    //  u64 q = (-normalised_exponent * 451597) >> 20; // (1<<20)*log(2)/log(5) = 451597
    //      q -= 1;
    //  i64 k = ((q * 2434718) >> 20) + 1 - 125;          // (1<<20)*ln(2)/ln(5) = 2434718
    //  u64 factor = (((-normalised_exponent-q) * 2434718) >> 20) - q;
    //  printf( "%llu,%lli,%llu\n", q, k, factor );
    //  preserving_mantissa_base10_lower  = preserving_mantissa_base2_lower  << factor;
    //  preserving_mantissa_base10_middle = preserving_mantissa_base2_middle << factor;
    //  preserving_mantissa_base10_upper  = preserving_mantissa_base2_upper  << factor;
    //}
    //c8 buf1[256] = {};
    //c8 buf2[256] = {};
    //c8 buf3[256] = {};
    //hl_u128_to_chars( buf1, buf1 + HL_ARRAY_COUNT( buf1 ), preserving_mantissa_base10_lower  );
    //hl_u128_to_chars( buf2, buf2 + HL_ARRAY_COUNT( buf2 ), preserving_mantissa_base10_middle );
    //hl_u128_to_chars( buf3, buf3 + HL_ARRAY_COUNT( buf3 ), preserving_mantissa_base10_upper  );
    //printf( "e10,(a,b,c): %lli, (%s,%s,%s)\n", exponent_base10, buf1, buf2, buf3 );

    ////// Step 4: Compute the shortest representation
    ////b accept_smaller = false;
    ////b accept_larger  = false;
    ////u128 preserving_mantissa_base10_lower_i = preserving_mantissa_base10_lower;
    ////u128 preserving_mantissa_base10_upper_i = preserving_mantissa_base10_upper - (accept_larger ? 0 : 1);
    ////u128 preserving_mantissa_base10_lower_ip1 = preserving_mantissa_base10_lower_i / 10;
    ////u128 preserving_mantissa_base10_upper_ip1 = preserving_mantissa_base10_upper_i / 10;
    ////u128 lower_bound_remainder_is_zero = preserving_mantissa_base10_lower_i % 10 == 0; 
    ////b all_lower_bound_zero = true;
    ////u64 digits = 0;
    ////while( preserving_mantissa_base10_lower_ip1 < preserving_mantissa_base10_upper_ip1 ){
    ////  all_lower_bound_zero = all_lower_bound_zero & lower_bound_remainder_is_zero;
    ////  preserving_mantissa_base10_lower_i = preserving_mantissa_base10_lower_ip1;
    ////  preserving_mantissa_base10_upper_i = preserving_mantissa_base10_upper_ip1;
    ////  preserving_mantissa_base10_lower_ip1 = preserving_mantissa_base10_lower_i / 10;
    ////  preserving_mantissa_base10_upper_ip1 = preserving_mantissa_base10_upper_i / 10;
    ////  lower_bound_remainder_is_zero = preserving_mantissa_base10_lower_i % 10 == 0; 
    ////  ++digits;
    ////}
    ////if( accept_smaller && all_lower_bound_zero ){
    ////  while( lower_bound_remainder_is_zero ){
    ////    preserving_mantissa_base10_lower_i = preserving_mantissa_base10_lower_ip1;
    ////    preserving_mantissa_base10_upper_i = preserving_mantissa_base10_upper_ip1;
    ////    preserving_mantissa_base10_lower_ip1 = preserving_mantissa_base10_lower_i / 10;
    ////    preserving_mantissa_base10_upper_ip1 = preserving_mantissa_base10_upper_i / 10;
    ////    lower_bound_remainder_is_zero = preserving_mantissa_base10_lower_i % 10 == 0; 
    ////    ++digits;
    ////  }
    ////}
    ////HL_ASSERT( preserving_mantissa_base10_upper_i < HL_U64_MAX );
    ////u64  shortest_base10       = (u64)preserving_mantissa_base10_upper_i;
    ////u64  exponent_modification = digits;
    ////printf( "e10,e0,d0: %lli,%llu,%llu\n", exponent_base10, digits, shortest_base10 );

    ////// Step 5: Print the number d0 * 10**(e0+e10)
    ////buffer_position = hl_u64_to_chars( buffer_position, buffer_end, shortest_base10 );
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
