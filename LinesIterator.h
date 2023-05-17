#pragma once

#include <stddef.h>

struct Class;
struct Object;


typedef struct LinesIterator {
	struct Class* class_;
	struct Object* stream;
	size_t buffer_size;
	char* buffer;
	size_t start_point, bytes_read;
	} LinesIterator;
extern struct Class LinesIterator_class;

LinesIterator* new_LinesIterator(struct Object* stream);

extern void LinesIterator_init_class();

