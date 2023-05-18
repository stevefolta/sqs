#include "Print.h"
#include "String.h"
#include "Object.h"
#include "Dict.h"
#include "Array.h"
#include "ByteCode.h"
#include "File.h"
#include "Error.h"
#include <stdio.h>
#include <stdbool.h>

static String end_option, file_option;
static bool Print_initialized = false;

static void Print_init()
{
	String_init_static_c(&end_option, "end");
	String_init_static_c(&file_option, "file");
	Print_initialized = true;
}

struct Object* Print(struct Object* self, struct Object** args)
{
	if (!Print_initialized)
		Print_init();

	// Get the options.
	String* end_string = NULL;
	Object* file_object = NULL;
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		// "end"
		end_string = (String*) Dict_at(options, &end_option);
		if (end_string)
			String_enforce((Object*) end_string, "print() \"end\" option");

		// "file"
		file_object = Dict_at(options, &file_option);
		}

	if (args[0]) {
		if (args[0]->class_ != &String_class)
			args[0] = call_object(args[0], new_c_static_String("string"), NULL);
		String* str = (String*) args[0];
		if (file_object) {
			Object* args_array[] = { args[0] };
			Array args = { &Array_class, 1, 1, args_array };
			call_object(file_object, new_c_static_String("write"), &args);
			}
		else
			fwrite(str->str, str->size, 1, stdout);
		}

	if (end_string == NULL)
		end_string = new_c_static_String("\n");
	if (file_object) {
		Object* args_array[] = { (Object*) end_string };
		Array args = { &Array_class, 1, 1, args_array };
		call_object(file_object, new_c_static_String("write"), &args);
		}
	else
		fwrite(end_string->str, end_string->size, 1, stdout);

	return NULL;
}


