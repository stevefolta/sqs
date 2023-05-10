#include "File.h"
#include "String.h"
#include "ByteArray.h"
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

Object* File_close(Object* super, Object** args)
{
	File* self = (File*) super;
	if (self->file) {
		fclose(self->file);
		self->file = NULL;
		}

	return NULL;
}


typedef struct FileLinesIterator {
	Class* class_;
	File* file;
	size_t buffer_size;
	char* buffer;
	size_t start_point, bytes_read;
	} FileLinesIterator;
Class FileLinesIterator_class;

FileLinesIterator* new_FileLinesIterator(File* file)
{
	FileLinesIterator* self = (FileLinesIterator*) alloc_obj(FileLinesIterator);
	self->class_ = &FileLinesIterator_class;
	self->file = file;
	self->buffer_size = 1024;
	self->buffer = alloc_mem_no_pointers(self->buffer_size);
	self->start_point = self->bytes_read = 0;
	return self;
}

Object* FileLinesIterator_next(Object* super, Object** args)
{
	FileLinesIterator* self = (FileLinesIterator*) super;

	while (true) {
		// Find the next end-of-line.
		const char* p = self->buffer + self->start_point;
		const char* end = self->buffer + self->bytes_read;
		const char* line_start = p;
		const char* line_end = NULL;
		while (p < end) {
			char c = *p++;
			if (c == '\n') {
				line_end = p - 1;
				self->start_point = p - self->buffer;
				break;
				}
			else if (c == '\r') {
				// Followed by '\n'?
				if (p >= end) {
					if (feof(self->file->file)) {
						line_end = p - 1;
						self->start_point = self->bytes_read = 0;
						}
					else {
						// Read some more.
						}
					break;
					}
				else if (*p == '\n') {
					line_end = p - 1;
					self->start_point = p + 1 - self->buffer;
					break;
					}
				}
			}
		if (line_end)
			return (Object*) new_String(line_start, line_end - line_start);

		// Read some more.
		if (self->start_point == 0 && self->bytes_read >= self->buffer_size)
			Error("File.lines: tried to read a line that's too long.");
		// Move the remaining read bytes to the beginning of the buffer.
		size_t bytes_left = self->bytes_read - self->start_point;
		memmove(self->buffer, self->buffer + self->start_point, bytes_left);
		self->start_point = 0;
		self->bytes_read = bytes_left;
		// Read the rest of the buffer.
		size_t bytes_read =
			fread(
				self->buffer + self->bytes_read, 1,
				self->buffer_size - self->bytes_read,
				self->file->file);
		if (bytes_read == 0) {
			if (feof(self->file->file)) {
				if (self->bytes_read == 0)
					return NULL;
				else {
					// Last line didn't have a line ending.
					}
				}
			Error("Error while reading a file's lines (%s).", strerror(errno));
			}
		self->bytes_read += bytes_read;
		}

	return NULL;
}

Object* File_lines(Object* super, Object** args)
{
	return (Object*) new_FileLinesIterator((File*) super);
}


void File_init_class()
{
	init_static_class(File);
	static const BuiltinMethodSpec file_methods[] = {
		{ "init", 2, File_init },
		{ "write", 1, File_write },
		{ "close", 0, File_close },
		{ "lines", 0, File_lines },
		{ NULL },
		};
	Class_add_builtin_methods(&File_class, file_methods);

	init_static_class(FileLinesIterator);
	static const BuiltinMethodSpec lines_methods[] = {
		{ "next", 0, FileLinesIterator_next },
		{ "iterator", 0, Object_identity },
		{ NULL },
		};
	Class_add_builtin_methods(&FileLinesIterator_class, lines_methods);
}


FILE* File_get_file(File* file)
{
	return file->file;
}



