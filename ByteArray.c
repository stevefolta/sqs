#include "ByteArray.h"
#include "Int.h"
#include "Memory.h"
#include "Error.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define capacity_increment 16

Class ByteArray_class;


ByteArray* new_ByteArray()
{
	ByteArray* self = (ByteArray*) alloc_mem(sizeof(ByteArray));
	ByteArray_init(self);
	return self;
}


Object* ByteArray_init(struct ByteArray* self)
{
	self->class_ = &ByteArray_class;
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
				self->array = (uint8_t*) alloc_mem_no_pointers(self->capacity * sizeof(uint8_t));
			}
		self->size = index + 1;
		}

	self->array[index] = value;
}


void ByteArray_append(struct ByteArray* self, uint8_t value)
{
	ByteArray_set_at(self, self->size, value);
}



Object* ByteArray_init_builtin(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;
	ByteArray_init(self);

	if (args[0]) {
		size_t size = Int_enforce(args[0], "ByteArray.init");
		self->size = self->capacity = size;
		self->array = (uint8_t*) alloc_mem_no_pointers(size * sizeof(uint8_t));
		}

	return (Object*) self;
}

Object* ByteArray_size_builtin(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;
	return (Object*) new_Int(self->size);
}

Object* ByteArray_at_builtin(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;
	return (Object*) new_Int(ByteArray_at(self, Int_enforce(args[0], "ByteArray.[]")));
}

Object* ByteArray_set_at_builtin(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;
	int value = Int_enforce(args[1], "ByteArray[]= value");
	ByteArray_set_at(self, Int_enforce(args[0], "ByteArray.[]="), value);
	return args[1];
}


void ByteArray_init_class()
{
	init_static_class(ByteArray);
	static const BuiltinMethodSpec builtin_methods[] = {
		{ "init", 1, ByteArray_init_builtin },
		{ "size", 0, ByteArray_size_builtin },
		{ "[]", 1, ByteArray_at_builtin },
		{ "[]=", 1, ByteArray_set_at_builtin },
		{ NULL },
		};
	Class_add_builtin_methods(&ByteArray_class, builtin_methods);
}


