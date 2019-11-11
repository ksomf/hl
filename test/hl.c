#define HL_IMPLEMENTATION
#include "../single_header_src/proto_hl.h"

#include <stdio.h>

u64 test_print( c8 *fmt, ... ){
  u64 result;
  c8 sprintf_result[512];
  c8 hlPrint_result[512];
  va_list args;
  va_start(args,fmt);
  vsnprintf( sprintf_result, hlARRAY_COUNT( sprintf_result ), fmt, args );
  va_end(args);
  va_start(args,fmt);
  hlVPrint( hlPrint_result, hlARRAY_COUNT( hlPrint_result ), fmt, args );
  va_end(args);
  if( hlCStrEqual( sprintf_result, hlPrint_result ) ){
    result = 1;
  }else{
    printf( "Mismatch - vsnprintf: %s", sprintf_result );
    printf( "         - hlVPrint : %s", hlPrint_result );
    result = 0;
  }
  return result;
}
i32 main( i32 argc, c8 **argv ){
  test_print( "Hello, World!\n" );
  test_print( "%cello, %corld!\n", 'H', 'W' );
  test_print( "%sello, %sorld!\n", "H", "W" );

  test_print( "%lld, %10lld, %010lld\n",  10,  10,  10 );
  test_print( "%lld, %10lld, %010lld\n", -10, -10, -10 );

  test_print( "%x, %X, %#x, %#X\n"    , 0x12345678, 10, 10, 10 );
  test_print( "%0x, %0X, %0#x, %#0X\n", 0x12345678, 10, 10, 10 );
  return 0;
}
