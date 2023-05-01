#include "Error.h"
#include <stdlib.h>
#include <stdio.h>


void Error(const char* message)
{
	fprintf(stderr, "%s\n", message);
	exit(1);
}





