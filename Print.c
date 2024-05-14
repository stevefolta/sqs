#include "Print.h"
#include "String.h"
#include "Object.h"
#include "Dict.h"
#include "Array.h"
#include "ByteCode.h"
#include "File.h"
#include "Boolean.h"
#include "Error.h"
#include <stdio.h>
#include <stdbool.h>

declare_static_string(end_option, "end");
declare_static_string(file_option, "file");
declare_static_string(out_option, "out");
declare_static_string(flush_option, "flush");


struct Object* Print(struct Object* self, struct Object** args)
{
	// Get the options.
	String* end_string = NULL;
	Object* file_object = NULL;
	bool flush = false;
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		// "end"
		end_string = (String*) Dict_at(options, &end_option);
		if (end_string)
			String_enforce((Object*) end_string, "print() \"end\" option");

		// "file" or "out"
		file_object = Dict_at(options, &file_option);
		if (file_object == NULL)
			file_object = Dict_at(options, &out_option);

		// "flush"
		Object* flush_object = Dict_at(options, &flush_option);
		flush = IS_TRUTHY(flush_object);
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

	if (flush) {
		if (file_object) {
			declare_static_string(flush_string, "flush");
			Array args = { &Array_class, 0, 0, NULL };
			call_object(file_object, &flush_string, &args);
			}
		else
			fflush(stdout);
		}

	return NULL;
}


