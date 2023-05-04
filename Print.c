#include "Print.h"
#include "String.h"
#include "Object.h"
#include "Error.h"
#include <stdio.h>


struct Object* Print(struct Object* self, struct Object** args)
{
	if (args[0]) {
		if (args[0]->class_ != &String_class) {
			//*** TODO: call "string" method.
			Error("Attempt to print a non-string.");
			}
		String* str = (String*) args[0];
		fwrite(str->str, str->size, 1, stdout);
		}

	printf("\n");
	return NULL;
}


