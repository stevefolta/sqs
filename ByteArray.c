#include "ByteArray.h"
#include "Error.h"
#include <string.h>

#define capacity_increment 16


ByteArray* new_ByteArray()
{
	ByteArray* self = (ByteArray*) alloc_mem(sizeof(ByteArray));
	ByteArray_init(self);
	return self;
}


Object* ByteArray_init(struct ByteArray* self)
{
	self->class_ = NULL; 	// TODO
	self->size = self->capacity = 0;
	self->array = NULL;
	return (Object*) self;
}


uint8_t ByteArray_at(struct ByteArray* self, size_t index)
{
	if (index > self->size)
		Error("ByteArray index out of bounds.");
	return self->array[index];
}


void ByteArray_set_at(struct ByteArray* self, size_t index, uint8_t value)
{
	if (index >= self->size) {
		if (index >= self->capacity) {
			size_t needed_size = index + 1;
			size_t old_capacity = self->capacity;
			self->capacity = needed_size + capacity_increment - (needed_size % capacity_increment);
			if (self->array) {
				self->array = (uint8_t*) realloc_mem(self->array, self->capacity * sizeof(uint8_t));
				memset(self->array + old_capacity, 0, (self->capacity - old_capacity) * sizeof(uint8_t));
				}
			else
				self->array = (uint8_t*) alloc_mem(self->capacity * sizeof(uint8_t));
			}
		self->size = index + 1;
		}

	self->array[index] = value;
}


void ByteArray_append(struct ByteArray* self, uint8_t value)
{
	ByteArray_set_at(self, self->size, value);
}



