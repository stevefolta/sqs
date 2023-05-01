#pragma once

#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include <stddef.h>
#include <stdlib.h>


typedef struct Array {
	Class* class_;
	size_t size, capacity;
	Object** items;
	} Array;


extern Object* Array_init(Array* self);
extern Object* Array_at(struct Array* self, size_t index);
extern Object* Array_set_at(struct Array* self, size_t index, Object* value);
extern Object* Array_append(struct Array* self, Object* value);


inline Array* new_Array()
{
	Array* self = (Array*) alloc_mem(sizeof(Array));
	Array_init(self);
	return self;
}

