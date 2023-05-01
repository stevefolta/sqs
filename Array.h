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


extern Array* new_Array();
extern Object* Array_at(struct Array* self, size_t index);
extern Object* Array_set_at(struct Array* self, size_t index, Object* value);
extern Object* Array_append(struct Array* self, Object* value);



