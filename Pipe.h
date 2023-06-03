#pragma once

#include <stdbool.h>
#include <stddef.h>

struct Class;

typedef struct Pipe {
	struct Class* class_;
	int read_fd, write_fd;
	} Pipe;
extern Pipe* new_Pipe();
extern void Pipe_close(Pipe* self);
extern struct Object* Pipe_capture(Pipe* self, bool as_string, size_t size_limit);

extern void Pipe_init_class();

