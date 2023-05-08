#include "Print.h"
#include "String.h"
#include "Object.h"
#include "Dict.h"
#include "ByteCode.h"
#include "Error.h"
#include <stdio.h>


struct Object* Print(struct Object* self, struct Object** args)
{
	// Get the options.
	String* end_string = NULL;
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		String option_name;
		String_init_static_c(&option_name, "end");
		end_string = (String*) Dict_at(options, &option_name);
		if (end_string)
			String_enforce((Object*) end_string, "print() \"end\" option");
		}

	if (args[0]) {
		if (args[0]->class_ != &String_class)
			args[0] = call_object(args[0], new_c_static_String("string"), NULL);
		String* str = (String*) args[0];
		fwrite(str->str, str->size, 1, stdout);
		}

	if (end_string)
		fwrite(end_string->str, end_string->size, 1, stdout);
	else
		printf("\n");
	return NULL;
}


