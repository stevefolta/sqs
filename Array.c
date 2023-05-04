#include "Array.h"
#include "Class.h"
#include "Object.h"
#include "String.h"
#include "Memory.h"
#include "Error.h"
#include <string.h>

#define capacity_increment 16

static Class Array_class;

void Array_init_class()
{
	init_static_class(Array);
}


Array* new_Array()
{
	Array* self = alloc_obj(Array);
	self->class_ = &Array_class;
	self->size = self->capacity = 0;
	self->items = NULL;
	return self;
}


Object* Array_at(struct Array* self, size_t index)
{
	if (index > self->size)
		Error("Array index out of bounds.");
	return self->items[index];
}


Object* Array_set_at(struct Array* self, size_t index, Object* value)
{
	if (index >= self->size) {
		if (index >= self->capacity) {
			size_t needed_size = index + 1;
			size_t old_capacity = self->capacity;
			self->capacity = needed_size + capacity_increment - (needed_size % capacity_increment);
			if (self->items) {
				self->items = (Object**) realloc_mem(self->items, self->capacity * sizeof(Object*));
				memset(self->items + old_capacity, 0, (self->capacity - old_capacity) * sizeof(Object*));
				}
			else
				self->items = (Object**) alloc_mem(self->capacity * sizeof(Object*));
			}
		self->size = index + 1;
		}

	self->items[index] = value;

	return value;
}


Object* Array_append(struct Array* self, Object* value)
{
	return Array_set_at(self, self->size, value);
}


Array* Array_copy(Array* self)
{
	Array* copy = alloc_obj(Array);
	copy->class_ = &Array_class;
	copy->size = self->size;
	copy->capacity = self->capacity;
	copy->items = NULL;
	if (self->items) {
		copy->items = (Object**) alloc_mem(self->capacity);
		memcpy(copy->items, self->items, self->size * sizeof(Object*));
		}
	return copy;
}


String* Array_join(Array* self, String* joiner)
{
	// Get the total size.
	size_t total_size = 0;
	for (int i = 0; i < self->size; ++i) {
		if (self->items[i]->class_ != &String_class)
			Error("Array.join() can't join non-strings... yet.");
		total_size += ((String*) self->items[i])->size;
		}
	if (joiner && self->size > 0)
		total_size += (self->size - 1) * joiner->size;

	// Do the join.
	char* joined = alloc_mem(total_size);
	char* out = joined;
	bool need_joiner = false;
	for (int i = 0; i < self->size; ++i) {
		if (need_joiner && joiner) {
			memcpy(out, joiner->str, joiner->size);
			out += joiner->size;
			}
		else
			need_joiner = true;

		String* str = (String*) self->items[i];
		memcpy(out, str->str, str->size);
		out += str->size;
		}

	// Return the string.
	return new_static_String(joined, total_size);
}



