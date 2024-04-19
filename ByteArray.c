#include "ByteArray.h"
#include "Int.h"
#include "String.h"
#include "Boolean.h"
#include "Memory.h"
#include "UTF8.h"
#include "Error.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define capacity_increment 16

Class ByteArray_class;
ByteArray empty_byte_array = { &ByteArray_class, 0, 0, NULL };


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


void ByteArray_ensure_capacity(struct ByteArray* self, size_t needed_size)
{
	if (needed_size <= self->capacity)
		return;

	size_t old_capacity = self->capacity;
	self->capacity = needed_size + capacity_increment - (needed_size % capacity_increment);
	uint8_t* new_array = (uint8_t*) alloc_mem_no_pointers(self->capacity * sizeof(uint8_t));
	memset(new_array + old_capacity, 0, (self->capacity - old_capacity) * sizeof(uint8_t));
	if (self->array) {
		// We'd use realloc_mem(), but this might be a slice of another ByteArray.
		memcpy(new_array, self->array, old_capacity * sizeof(uint8_t));
		}
	self->array = new_array;
}

void ByteArray_set_at(struct ByteArray* self, size_t index, uint8_t value)
{
	if (index >= self->size) {
		ByteArray_ensure_capacity(self, index + 1);
		self->size = index + 1;
		}

	self->array[index] = value;
}


void ByteArray_append(struct ByteArray* self, uint8_t value)
{
	ByteArray_set_at(self, self->size, value);
}


void ByteArray_append_bytes(struct ByteArray* self, uint8_t* values, size_t size)
{
	size_t needed_size = self->size + size;
	ByteArray_ensure_capacity(self, needed_size);
	memcpy(self->array + self->size, values, size);
	self->size = needed_size;
}


String* ByteArray_as_string(ByteArray* self)
{
	return new_static_String((char*) self->array, self->size);
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

Object* ByteArray_append_builtin(Object* super, Object** args)
{
	ByteArray_append((ByteArray*) super, Int_enforce(args[0], "ByteArray.append"));
	return NULL;
}

Object* ByteArray_as_string_builtin(Object* super, Object** args)
{
	return (Object*) ByteArray_as_string((ByteArray*) super);
}

Object* ByteArray_slice(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;

	int start = args[0] ? Int_enforce(args[0], "ByteArray.slice") : 0;
	int end = args[1] ? Int_enforce(args[1], "ByteArray.slice") : self->size;
	if (start < 0)
		start += self->size;
	if (start >= self->size)
		return (Object*) &empty_byte_array;
	if (end < 0)
		end += self->size;
	if (end < start)
		return (Object*) &empty_byte_array;
	else if (end > self->size)
		end = self->size;

	ByteArray* result = alloc_obj(ByteArray);
	result->class_ = &ByteArray_class;
	result->size = result->capacity = end - start;
	result->array = self->array + start;
	return (Object*) result;
}

Object* ByteArray_is_valid_utf8(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;
	return make_bool(is_valid_utf8((const char*) self->array, self->size));
}

Object* ByteArray_decode_8859_1(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;
	return (Object*) decode_8859_1(self->array, self->size);
}


typedef struct ByteArrayIterator {
	Class* class_;
	ByteArray* byte_array;
	size_t index;
	} ByteArrayIterator;
Class ByteArrayIterator_class;

Object* ByteArrayIterator_next(Object* super, Object** args)
{
	ByteArrayIterator* self = (ByteArrayIterator*) super;
	if (self->index >= self->byte_array->size)
		return NULL;
	return (Object*) new_Int(self->byte_array->array[self->index++]);
}

Object* ByteArray_iterator(Object* super, Object** args)
{
	ByteArray* self = (ByteArray*) super;
	ByteArrayIterator* iterator = alloc_obj(ByteArrayIterator);
	iterator->class_ = &ByteArrayIterator_class;
	iterator->byte_array = self;
	iterator->index = 0;
	return (Object*) iterator;
}


void ByteArray_init_class()
{
	init_static_class(ByteArray);
	static const BuiltinMethodSpec builtin_methods[] = {
		{ "init", 1, ByteArray_init_builtin },
		{ "size", 0, ByteArray_size_builtin },
		{ "[]", 1, ByteArray_at_builtin },
		{ "[]=", 1, ByteArray_set_at_builtin },
		{ "append", 1, ByteArray_append_builtin },
		{ "as-string", 0, ByteArray_as_string_builtin },
		{ "slice", 2, ByteArray_slice },
		{ "is-valid-utf8", 0, ByteArray_is_valid_utf8 },
		{ "decode-8859-1", 0, ByteArray_decode_8859_1 },
		{ "iterator", 0, ByteArray_iterator },
		{ NULL },
		};
	Class_add_builtin_methods(&ByteArray_class, builtin_methods);

	init_static_class(ByteArrayIterator);
	static const BuiltinMethodSpec iterator_methods[] = {
		{ "next", 0, ByteArrayIterator_next },
		{ NULL },
		};
	Class_add_builtin_methods(&ByteArrayIterator_class, iterator_methods);
}


