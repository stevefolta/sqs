#pragma once

#include <stddef.h>
#include <stdlib.h>

struct Object;
struct Class;


typedef struct Array {
	struct Class* class_;
	size_t size, capacity;
	struct Object** items;
	} Array;


extern Array* new_Array();
extern struct Object* Array_at(Array* self, size_t index);
extern struct Object* Array_set_at(Array* self, size_t index, struct Object* value);
extern struct Object* Array_append(Array* self, struct Object* value);
extern Array* Array_copy(Array* self);


extern void Array_init_class();

