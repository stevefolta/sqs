#include "Pipe.h"
#include "Class.h"
#include "Object.h"
#include "String.h"
#include "ByteArray.h"
#include "Memory.h"
#include "Error.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>

Class Pipe_class;


Object* Pipe_init(Object* super, Object** args)
{
	Pipe* self = (Pipe*) super;
	self->class_ = &Pipe_class;

	int fds[2];
	int result = pipe(fds);
	if (result != 0)
		Error("Error creating a pipe (%s).", strerror(errno));
	self->read_fd = fds[0];
	self->write_fd = fds[1];

	extern void close_pipe(void* ptr, void* data);
	mem_add_finalizer(self, close_pipe, NULL);

	return (Object*) self;
}

Pipe* new_Pipe()
{
	Pipe* self = alloc_obj(Pipe);
	Pipe_init((Object*) self, NULL);
	return self;
}


void Pipe_close(Pipe* self)
{
	if (self->read_fd >= 0)
		close(self->read_fd);
	if (self->write_fd >= 0)
		close(self->write_fd);
	self->read_fd = -1;
	self->write_fd = -1;
}


Object* Pipe_capture(Pipe* self, bool as_string, size_t size_limit)
{
	ByteArray* bytes = new_ByteArray();

	int buf_size = 1024;
	uint8_t* buffer = alloc_mem_no_pointers(buf_size);
	while (true) {
		ssize_t bytes_read = read(self->read_fd, buffer, buf_size);
		if (bytes_read < 0)
			Error("Error while capturing run() output (%s).", strerror(errno));
		else if (bytes_read == 0) {
			// EOF.
			break;
			}
		ByteArray_append_bytes(bytes, buffer, bytes_read);
		if (size_limit != 0 && bytes->size >= size_limit)
			Error("Capturing too much output in Pipe.capture().");
		}

	return (as_string ? (Object*) ByteArray_as_string(bytes) : (Object*) bytes);
}


void close_pipe(void* ptr, void* data)
{
	Pipe_close((Pipe*) ptr);
}


Object* Pipe_close_builtin(Object* super, Object** arg)
{
	Pipe_close((Pipe*) super);
	return NULL;
}

Object* Pipe_capture_builtin(Object* super, Object** arg)
{
	Pipe* self = (Pipe*) super;
	return Pipe_capture(self, true, 0);
}


void Pipe_init_class()
{
	init_static_class(Pipe);
	static const BuiltinMethodSpec builtin_methods[] = {
		{ "init", 0, Pipe_init },
		{ "close", 0, Pipe_close_builtin },
		{ "capture", 0, Pipe_capture_builtin },
		{ NULL },
		};
	Class_add_builtin_methods(&Pipe_class, builtin_methods);
}



