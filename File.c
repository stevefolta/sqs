#include "File.h"
#include "String.h"
#include "Path.h"
#include "LinesIterator.h"
#include "ByteArray.h"
#include "Int.h"
#include "ByteCode.h"
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
	const char* path = NULL;
	if (args[0] == NULL)
		Error("File() needs a path.");
	else if (args[0]->class_ == &Path_class)
		path = ((Path*) args[0])->path;
	else if (args[0]->class_ == &String_class)
		path = String_c_str((String*) args[0]);
	else {
		String* obj_string = (String*) call_object(args[0], new_c_static_String("string"), NULL);
		Error("File()'s path argument must be a Path or a String (got %s).", String_c_str(obj_string));
		}
	const char* mode = "r";
	if (args[1] && args[1]->class_ == &String_class)
		mode = String_c_str((String*) args[1]);

	self->file = fopen(path, mode);
	if (self->file == NULL)
		Error("Error opening file \"%s\" (%s).", path, strerror(errno));

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
		Error("File.read() requires a ByteArray.");
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



