#define HL_IMPLEMENTATION
#include "../hl.h"

#include <stdio.h>

u64 test_print( c8 *fmt, ... ){
	u64 result;
	c8 crt_vsnprintf_result[1024];
	c8  hl_vsnprintf_result[1024];
	va_list args;
	va_start(args,fmt);
	vsnprintf(    crt_vsnprintf_result, HL_ARRAY_COUNT( crt_vsnprintf_result ), fmt, args );
	va_end(args);
	va_start(args,fmt);
	hl_vsnprintf(  hl_vsnprintf_result, HL_ARRAY_COUNT(  hl_vsnprintf_result ), fmt, args );
	va_end(args);
	if( hl_cstr_equ( crt_vsnprintf_result, hl_vsnprintf_result ) ){
		result = 1;
	}else{
		printf( "Mismatch - fmt: %s", fmt                  );
		printf( "         - crt: %s", crt_vsnprintf_result );
		printf( "         - hl : %s",  hl_vsnprintf_result );
		result = 0;
	}
	return result;
}
i32 main( i32 argc, c8 **argv ){
	test_print( "Hello, World!\n" );
	test_print( "%cello, %corld!\n", 'H', 'W' );
	test_print( "%sello, %sorld!\n", "H", "W" );

	test_print( "%llu, %10llu, %010llu\n",  10,  10,  10 );
	test_print( "%lld, %10lld, %010lld\n",  10,  10,  10 );
	test_print( "%lld, %10lld, %010lld\n", -10, -10, -10 );

	test_print( "%x, %X, %#x, %#X\n"    , 0x12345678, 10, 10, 10 );
	test_print( "%0x, %0X, %0#x, %#0X\n", 0x12345678, 10, 10, 10 );

	test_print( "%f, %F\n",  0.0,  0.0 );
	test_print( "%e, %E\n",  0.0,  0.0 );
	test_print( "%g, %G\n",  0.0,  0.0 );
	test_print( "%f, %F\n", -0.0, -0.0 );
	test_print( "%e, %E\n", -0.0, -0.0 );
	test_print( "%g, %G\n", -0.0, -0.0 );
	test_print( "%f, %F\n", 1.0, 1.0 );
	test_print( "%e, %E\n", 1.0, 1.0 );
	test_print( "%g, %G\n", 1.0, 1.0 );
	test_print( "%f, %F\n", 42.0, 42.0 );
	test_print( "%e, %E\n", 42.0, 42.0 );
	test_print( "%g, %G\n", 42.0, 42.0 );
	test_print( "%f, %F\n", 42.0e14, 42.0e14 );
	test_print( "%e, %E\n", 42.0e14, 42.0e14 );
	test_print( "%g, %G\n", 42.0e14, 42.0e14 );
	test_print( "%f, %F\n", 42.0e33, 42.0e33 );
	test_print( "%e, %E\n", 42.0e33, 42.0e33 );
	test_print( "%g, %G\n", 42.0e33, 42.0e33 );
	test_print( "%f, %F\n", 42.0e-5, 42.0e-5 );
	test_print( "%e, %E\n", 42.0e-5, 42.0e-5 );
	test_print( "%g, %G\n", 42.0e-5, 42.0e-5 );

	test_print( "%e\n", 1.0 );
	test_print( "%e\n", 2.0 );
	r64 f = 1.0;
	//while( test_print( "%e\n", f ) ){ f*=10.0; }

	//u64 u = 0;
	//test_print( "%llu\n",  0 );
	//while( test_print( "%llu\n",  (1<<u) ) ) ++u;

	return 0;
}
