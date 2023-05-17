#include "File.h"
#include "String.h"
#include "LinesIterator.h"
#include "ByteArray.h"
#include "Int.h"
#include "Memory.h"
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

	else if (args[0]->class_ == &ByteArray_class) {
		ByteArray* byte_array = (ByteArray*) args[0];
		fwrite(byte_array->array, byte_array->size, 1, self->file);
		}

	else
		Error("File.write() needs a String or a ByteArray.");
	
	return super;
}

Object* File_read(Object* super, Object** args)
{
	File* self = (File*) super;
	if (self->file == NULL)
		Error("Attempt to read from a closed file.");
	if (args[0] == NULL || args[0]->class_ != &ByteArray_class)
		Error("File.write() requires a ByteArray.");
	ByteArray* buffer = (ByteArray*) args[0];

	size_t bytes_read = fread(buffer->array, 1, buffer->size, self->file);
	if (bytes_read == 0) {
		if (ferror(self->file))
			Error("Error while reading a file (%s).", strerror(errno));
		}

	return (Object*) new_Int(bytes_read);
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


Object* File_lines(Object* super, Object** args)
{
	return (Object*) new_LinesIterator(super);
}


void File_init_class()
{
	init_static_class(File);
	static const BuiltinMethodSpec file_methods[] = {
		{ "init", 2, File_init },
		{ "write", 1, File_write },
		{ "read", 1, File_read },
		{ "close", 0, File_close },
		{ "lines", 0, File_lines },
		{ NULL },
		};
	Class_add_builtin_methods(&File_class, file_methods);
}


FILE* File_get_file(File* file)
{
	return file->file;
}



