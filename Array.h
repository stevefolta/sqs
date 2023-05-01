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

	Object* (*init)(struct Array* self);
	Object* (*at)(struct Array* self, size_t index);
	Object* (*set_at)(struct Array* self, size_t index, Object* value);
	Object* (*append)(struct Array* self, Object* value);
	} Array;


extern Object* Array_init(Array* self);

inline Array* new_Array()
{
	Array* self = (Array*) alloc_mem(sizeof(Array));
	self->init = Array_init;
	self->init(self);
	return self;
}

