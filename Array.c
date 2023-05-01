#include "Array.h"
#include "Error.h"
#include <string.h>

#define capacity_increment 16


Object* Array_init(struct Array* self)
{
	self->class_ = NULL; 	// TODO
	self->size = self->capacity = 0;
	self->items = NULL;
	return (Object*) self;
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



