#include "ExprParser.h"
#include <stdio.h>
#include <stdarg.h>

void myprintf(const char* format, ... ) {
#ifdef DEBUG
  va_list args;
  va_start( args, format );
  vfprintf( stdout, format, args );
  va_end( args );

#endif
}

