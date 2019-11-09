#define HL_IMPLEMENTATION
#include "../hl.h"

#include <stdio.h>
i32 main( i32 argc, c8 **argv ){
  c8 result[512];
  hlPrint( result, hlARRAY_COUNT(result), "Hello, World!\n" );
  printf( "%s", result );
  hlPrint( result, hlARRAY_COUNT(result), "%cello, %corld!\n" , 'H', 'W' );
  printf( "%s", result );
  hlPrint( result, hlARRAY_COUNT(result), "%sello,%2sorld!\n", "H", "W" );
  printf( "%s", result );

  hlPrint( result, hlARRAY_COUNT(result), "%d, %10d, %010d\n", 10, 10, 10 );
  printf( "%s", result );
}
