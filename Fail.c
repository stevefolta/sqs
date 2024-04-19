#include "Fail.h"
#include "Object.h"
#include "String.h"
#include <unistd.h>
#include <stdlib.h>


Object* Fail(Object* self, Object** args)
{
	if (args[0]) {
		String* message = String_enforce(args[0], "fail()");
		write(STDERR_FILENO, message->str, message->size);
		write(STDERR_FILENO, "\n", 1);
		}
	exit(1);
	return NULL;
}



