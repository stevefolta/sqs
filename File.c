#include "File.h"
#include "String.h"
#include "ByteArray.h"
#include "Error.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

Class File_class;

typedef struct File {
	struct Class* class_;
	FILE* file;
	} File;


Object* File_init(Object* super, Object** args)
{
	File* self = (File*) super;

	// Arguments.
	String* path = String_enforce(args[0], "File.init");
	const char* mode = "r";
	if (args[1] && args[1]->class_ == &String_class)
		mode = String_c_str((String*) args[1]);

	self->file = fopen(String_c_str(path), mode);
	if (self->file == NULL)
		Error("Error opening file \"%s\" (%s).", String_c_str(path), strerror(errno));

	return super;
}

Object* File_write(Object* super, Object** args)
{
	File* self = (File*) super;
	if (self->file == NULL)
		Error("Attempt to write to a closed file.");
	if (args[0] == NULL)
		Error("Missing argument to File.write().");

	if (args[0]->class_ == &String_class) {
		String* str = (String*) args[0];
		fwrite(str->str, str->size, 1, self->file);
		}
	
	return super;
}

Object* File_close(Object* super, Object** args)
{
	File* self = (File*) super;
	if (self->file) {
		fclose(self->file);
		self->file = NULL;
		}

	return NULL;
}


void File_init_class()
{
	init_static_class(File);
	static const BuiltinMethodSpec file_methods[] = {
		{ "init", 2, File_init },
		{ "write", 1, File_write },
		{ "close", 0, File_close },
		{ NULL },
		};
	Class_add_builtin_methods(&File_class, file_methods);
}


FILE* File_get_file(File* file)
{
	return file->file;
}



