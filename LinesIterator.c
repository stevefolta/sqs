#include "LinesIterator.h"
#include "String.h"
#include "ByteArray.h"
#include "Array.h"
#include "Int.h"
#include "Object.h"
#include "Class.h"
#include "ByteCode.h"
#include "Memory.h"
#include "Error.h"
#include <string.h>

Class LinesIterator_class;

#define INITIAL_BUFFER_SIZE 1024
#define MAX_BUFFER_SIZE 65536


LinesIterator* new_LinesIterator(Object* stream)
{
	LinesIterator* self = (LinesIterator*) alloc_obj(LinesIterator);
	self->class_ = &LinesIterator_class;
	self->stream = stream;
	self->buffer_size = INITIAL_BUFFER_SIZE;
	self->buffer = alloc_mem_no_pointers(self->buffer_size);
	self->start_point = self->bytes_read = 0;
	return self;
}

Object* LinesIterator_next(Object* super, Object** args)
{
	LinesIterator* self = (LinesIterator*) super;

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
					if (self->start_point == 0) {
						// We've reached the EOF, so this is the end of the last line.
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
		// Is the buffer full?
		if (self->start_point == 0 && self->bytes_read >= self->buffer_size) {
			// Grow the buffer... up to a limit.
			size_t new_buffer_size = self->buffer_size * 2;
			if (new_buffer_size > MAX_BUFFER_SIZE)
				Error("LinesIterator.next: tried to read a line that's too long.");
			char* new_buffer = alloc_mem_no_pointers(new_buffer_size);
			memcpy(new_buffer, self->buffer, self->bytes_read);
			self->buffer = new_buffer;
			self->buffer_size = new_buffer_size;
			}
		// Move the remaining read bytes to the beginning of the buffer.
		if (self->start_point > 0) {
			size_t bytes_left = self->bytes_read - self->start_point;
			memmove(self->buffer, self->buffer + self->start_point, bytes_left);
			self->start_point = 0;
			self->bytes_read = bytes_left;
			}
		// Read the rest of the buffer.
		size_t bytes_to_read = self->buffer_size - self->bytes_read;
		ByteArray buffer = {
			&ByteArray_class,
			bytes_to_read, bytes_to_read,
			(uint8_t*) self->buffer + self->bytes_read };
		Object* args_array[] = { (Object*) &buffer };
		Array args = { &Array_class, 1, 1, args_array };
		Object* result = call_object(self->stream, new_c_static_String("read"), &args);
		int bytes_read = Int_enforce(result, "LinesIterator.next");
		if (bytes_read == 0) {
			if (self->bytes_read == 0)
				return NULL;
			else {
				// Last line didn't have a line ending.
				String* last_line = new_String(self->buffer, self->bytes_read);
				self->bytes_read = 0;
				return (Object*) last_line;
				}
			}
		self->bytes_read += bytes_read;
		}

	return NULL;
}


void LinesIterator_init_class()
{
	init_static_class(LinesIterator);
	static const BuiltinMethodSpec lines_methods[] = {
		{ "next", 0, LinesIterator_next },
		{ "iterator", 0, Object_identity },
		{ NULL },
		};
	Class_add_builtin_methods(&LinesIterator_class, lines_methods);
}

