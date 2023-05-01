#include "Error.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


void Error(const char* message, ...)
{
	va_list args;
	va_start(args, message);

	vfprintf(stderr, message, args);
	fprintf(stderr, "\n");

	va_end(args);
	exit(1);
}





