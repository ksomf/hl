//-----------------------------------------------//
//-- hl.h -- v0.02 -- My Single Header Library --//
//-----------------------------------------------//
//-----------------------------------------------//
//-- USE                                       --//
//--     Use                                   --//
//--     #define HL_IMPLEMENTATION             --//
//--     in exactly ONE file before inclusion  --//
//-----------------------------------------------//


#if !( defined(__x86_64__) || defined(_M_AMD64) )
	#error CODEBASE CURRENTLY SUPPORTS ONLY 64 BIT x86
#endif

#if defined( __clang__ )
	#define HL_CLANG
#elif defined( _MSC_VER )
	#define HL_MSVC
#elif defined( __GNUC__ )
	#define HL_GCC
#endif

#if defined( __cplusplus )
	#define HL_EXTERN extern "C"
#else
	#define HL_EXTERN extern
#endif

#if defined( HL_STATIC )
	#define HL_FUN_DEF static
	#define HL_INLINE  static inline
#else
	#define HL_FUN_DEF HL_EXTERN 
	#define HL_INLINE  inline
#endif

#if !defined( _fltused ) // USE FLOATING POINT
	#if defined( HL_WINDOWS )
		HL_EXTERN int _fltused = 0;
	#elif defined( HL_GCC )
		HL_EXTERN int _fltused;
	#endif
#endif

#define _HL_STATIC_ASSERT3( expr, msg ) typedef char static_assertion_##msg[(!!( expr ) ) * 2 - 1]
#define _HL_STATIC_ASSERT2( expr, ln ) _HL_STATIC_ASSERT3( expr, hl_static_insertion_failure_##ln )
#define _HL_STATIC_ASSERT1( expr, ln ) _HL_STATIC_ASSERT2( expr, ln )
#define HL_STATIC_ASSERT( expression ) _HL_STATIC_ASSERT1( expression, __LINE__ )

#if defined( HL_WINDOWS )
	#define HL_ASSERT( exp ) if( !(exp) ){ *(i32 *)0 = 0; }
#else
	#define HL_ASSERT( exp ) if( !(exp) ){ *(volatile i32 *)0 = 0; }
#endif

#define _HL_STR( str ) #str
#define HL_STR( str ) _HL_STR( str )

#include <stdarg.h> // va_arg, ..
#include <stddef.h> // size_t
#include <stdint.h> // int8_t, ...

#define HL_ARRAY_COUNT( arr ) sizeof(arr) / sizeof(arr[0])

#if !defined( i8 )
	typedef int8_t  i8;
	typedef int16_t i16;
	typedef int32_t i32;
	typedef int64_t i64;
	HL_STATIC_ASSERT( sizeof( i8  ) == 1 );
	HL_STATIC_ASSERT( sizeof( i16 ) == 2 );
	HL_STATIC_ASSERT( sizeof( i32 ) == 4 );
	HL_STATIC_ASSERT( sizeof( i64 ) == 8 );
#endif
#if !defined( u8 )
	typedef uint8_t  u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;
	typedef __uint128_t u128;
	HL_STATIC_ASSERT( sizeof( u8   ) == 1 );
	HL_STATIC_ASSERT( sizeof( u16  ) == 2 );
	HL_STATIC_ASSERT( sizeof( u32  ) == 4 );
	HL_STATIC_ASSERT( sizeof( u64  ) == 8 );
	HL_STATIC_ASSERT( sizeof( u128 ) == 16 );
#endif
#if !defined( c8 )
	typedef char c8;
	HL_STATIC_ASSERT( sizeof( c8 ) == 1 );
#endif
#if !defined( r32 )
	typedef float r32;
	HL_STATIC_ASSERT( sizeof( r32 ) == 4 );
#endif
#if !defined( r64 )
	typedef double r64;
	HL_STATIC_ASSERT( sizeof( r64 ) == 8 );
#endif
#if !defined( bool )
	typedef _Bool bool;
	#define true 1
	#define false 0
	#if !defined( b )
		typedef bool b;
	#endif
#endif

#define HL_R64_DIGIT_PRECISION 15                      //-- NUMBER OF DIGITS IN PRECISION --//
#define HL_R64_DIGIT_MANTISSA  53                      //-- NUMBER OF DIGITS IN MANTISSA  --//
#define HL_R64_EPSILON         2.2204460492503131E-16  //-- 1.0 + REAL64_EPSILON != 1.0   --//
#define HL_R64_MIN             2.2250738585072014E-308 //-- MINIMUM REAL64                --//
#define HL_R64_MAX             1.7976931348623157E+308 //-- MAXIMUM REAL64                --//

#define HL_R32_LARGEST_ODD     16777215                //-- POINT AT WHICH (u)int32 MORE PRECISE THAN real32 --//
#define HL_R32_DIGIT_PRECISION 6                       //-- NUMBER OF DIGITS IN PRECISION                    --//
#define HL_R32_DIGIT_MANTISSA  24                      //-- NUMBER OF DIGITS IN MANTISSA                     --//
#define HL_R32_EPSILON         1.19209290E-07f         //-- 1.0 + REAL32_EPSILON != 1.0                      --//
#define HL_R32_MIN             1.17549435E-38f         //-- MINIMUM REAL32                                   --//
#define HL_R32_MAX             3.40282347E+38f         //-- MAXIMUM REAL32                                   --//

#define HL_I64_MIN ( -(1ll << 63)     )
#define HL_I64_MAX (  (1ll << 63) - 1 )

#define HL_U64_MAX ( 1ull << 63 )

#define HL_BITS_IN_BYTE 8

#define HL_KILOBYTES( a ) (( 1024LL *              a   ))
#define HL_MEGABYTES( a ) (( 1024LL * HL_KILOBYTES( a ) ))
#define HL_GIGABYTES( a ) (( 1024LL * HL_MEGABYTES( a ) ))

#define HL_64_BYTE_CEIL( a ) ( (a + 0x3f) & ~0x3f )


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

#if !defined( _HL_CSTRING )

HL_FUN_DEF u64 hl_cstr_len( c8 *str );

HL_FUN_DEF u64 hl_cstr_equ( c8 *str1, c8 *str2 );

HL_FUN_DEF bool hl_cstr_startswith( c8 *start, c8 *str );

#define _HL_CSTRING
#endif 

#if defined( HL_IMPLEMENTATION )

HL_FUN_DEF u64 hl_cstr_len( c8 * str ){
	u64 result = 0;
	while( *str++ ){
		++result;
	}
	return result;
}

HL_FUN_DEF u64 hl_cstr_equ( c8 *str1, c8 *str2 ){
	u64 result = 1;
	while( result && (*str1 || *str2) ){
		result = *str1++ == *str2++;
	}
	return result;
}

HL_FUN_DEF bool hl_cstr_startswith( c8 *start, c8 *str ){
	bool result;
	while( *start && *start == *str ){
		++start;
		++str;
	}
	result = *start == 0;
	return result;
}


#endif


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

	if( str_len < buffer_end - buffer_start ){
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

HL_FUN_DEF c8 *hl_r64_to_chars( c8 *buffer_start, c8 *buffer_end, r64 number, hl_print_real_types real_format, b use_upper_case ){
	u64 maxed_11_bit_number = (1 << 11) - 1;

	hl_ieee754_r64_representation r64_representation = {};
	                              r64_representation.real = number;
	b   is_negative = r64_representation.real_bits.sign;
	u64 exponent    = r64_representation.real_bits.exponent;
	u64 mantissa    = r64_representation.real_bits.mantissa;

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

		u64 target_mantissa = normalised_mantissa;
		i64 powers_of_two   = normalised_exponent;
		i64 powers_of_ten   = l - normalised_exponent;
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
					++powers_of_ten;
				}
				target_mantissa <<= 1;
				--powers_of_two;
			}
			while( powers_of_ten < 0 ){
				target_mantissa = (target_mantissa + 5) / 10;
				++powers_of_ten;
			}
			while( powers_of_ten > 0 ){
				target_mantissa *= 10;
				++powers_of_ten;
			}
		}

		i64 target_exponent = normalised_exponent - l + n - 1;
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


#if !defined( _HL_MEMORY )

typedef struct {
	u8 *base;
	u64 size;
	u64 used;
} hl_memory_pool;

HL_FUN_DEF void *_hl_push_to_memory_pool		 ( hl_memory_pool *pool, u64 size );
HL_FUN_DEF void *_hl_push_to_memory_pool_safe( hl_memory_pool *pool, u64 size );

typedef struct {
	hl_memory_pool transient;
	hl_memory_pool permanent;
} hl_program_memory;

HL_FUN_DEF hl_program_memory hl_create_program_memory( u64 base, u64 transient_size, u64 permanent_size );

#define _HL_MEMORY
#endif 

#if defined( HL_IMPLEMENTATION )

HL_FUN_DEF void *_hl_push_to_memory_pool( hl_memory_pool *pool, u64 size ){
	u64 push_size = HL_64_BYTE_CEIL( size );
	HL_ASSERT( pool->size - pool->used >= push_size );

	void *result = pool->base + pool->used;
	pool->used += push_size;
	return result;
}

HL_FUN_DEF void *_hl_push_to_memory_pool_safe( hl_memory_pool *pool, u64 size ){
	u64 push_size = HL_64_BYTE_CEIL( size );
	void *result = 0;
	if( pool->size - pool->used >= push_size ){
		result = pool->base + pool->used;
		pool->used += push_size;
	}
	return result;
}

#include <sys/mman.h>
HL_FUN_DEF hl_program_memory hl_create_program_memory( u64 base, u64 transient_size, u64 permanent_size ){
	HL_ASSERT( base == HL_64_BYTE_CEIL( base ) );
	hl_program_memory result = {};

	u64 pool_size = transient_size + permanent_size;
	u8 *memory_block = (u8 *)mmap( (void *)base, pool_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANON, -1, 0 );
	HL_ASSERT( memory_block != (void *)-1 );

	result.transient.base = memory_block;
	result.transient.size = transient_size;
	result.permanent.base = memory_block + transient_size;
	result.permanent.size = permanent_size;

	return result;
}

#endif
