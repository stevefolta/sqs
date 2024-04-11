#include "Error.h"
#include "String.h"
#include "Array.h"
#include "Object.h"
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
	exit(EXIT_FAILURE);
}


const char* where(int line_number, struct String* filename)
{
	Array* strings = new_Array();
	declare_static_string(line_str, "on line ");
	Array_append(strings, (Object*) &line_str);
	char buf[32];
	sprintf(buf, "%d", line_number);
	Array_append(strings, (Object*) new_c_String(buf));
	if (filename) {
		declare_static_string(filename_start_str, " of \"");
		declare_static_string(filename_end_str, "\"");
		Array_append(strings, (Object*) &filename_start_str);
		Array_append(strings, (Object*) filename);
		Array_append(strings, (Object*) &filename_end_str);
		}
	return String_c_str(Array_join(strings, NULL));
}


