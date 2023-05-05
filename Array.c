#include "Array.h"
#include "Class.h"
#include "Object.h"
#include "String.h"
#include "Int.h"
#include "ByteCode.h"
#include "Memory.h"
#include "Error.h"
#include <string.h>

struct ArrayIterator;
extern struct ArrayIterator* new_ArrayIterator(Array* array);
extern void ArrayIterator_init_class();

#define capacity_increment 16

static Class Array_class;


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
	Array* stringized_items = new_Array();

	// Get the total size.
	size_t total_size = 0;
	for (int i = 0; i < self->size; ++i) {
		Object* item = self->items[i];
		String* str;
		if (item == NULL || item->class_ != &String_class) {
			String name_str;
			String_init_static_c(&name_str, "string");
			str = (String*) call_method(item, &name_str, NULL);
			Array_append(stringized_items, (Object*) str);
			}
		else
			str = (String*) item;
		total_size += str->size;
		}
	if (joiner && self->size > 0)
		total_size += (self->size - 1) * joiner->size;

	// Do the join.
	char* joined = alloc_mem(total_size);
	char* out = joined;
	bool need_joiner = false;
	Object** next_stringized_item = stringized_items->items;
	for (int i = 0; i < self->size; ++i) {
		if (need_joiner && joiner) {
			memcpy(out, joiner->str, joiner->size);
			out += joiner->size;
			}
		else
			need_joiner = true;

		Object* item = self->items[i];
		if (item == NULL || item->class_ != &String_class)
			item = *next_stringized_item++;
		String* str = (String*) item;
		memcpy(out, str->str, str->size);
		out += str->size;
		}

	// Return the string.
	return new_static_String(joined, total_size);
}


static Object* Array_size_builtin(Object* super, Object** args)
{
	Array* self = (Array*) super;
	return (Object*) new_Int(self->size);
}

static Object* Array_at_builtin(Object* super, Object** args)
{
	Array* self = (Array*) super;
	Int_enforce(args[0], "Array.[]");
	return Array_at(self, ((Int*) args[0])->value);
}

static Object* Array_at_set_builtin(Object* super, Object** args)
{
	Array* self = (Array*) super;
	Int_enforce(args[0], "Array.[]=");
	return Array_set_at(self, ((Int*) args[0])->value, args[1]);
}

static Object* Array_append_builtin(Object* super, Object** args)
{
	Array* self = (Array*) super;
	Array_append(self, args[0]);
	return args[0];
}

static Object* Array_iterator_builtin(Object* super, Object** args)
{
	Array* self = (Array*) super;
	return (Object*) new_ArrayIterator(self);
}

static Object* Array_join_builtin(Object* super, Object** args)
{
	Array* self = (Array*) super;
	String* joiner = (String*) args[0];
	if (joiner && joiner->class_ != &String_class)
		Error("Argument to Array.join() must be a String.");
	return (Object*) Array_join(self, joiner);
}


void Array_init_class()
{
	init_static_class(Array);

	static BuiltinMethodSpec builtin_methods[] = {
		{ "size", 0, Array_size_builtin },
		{ "[]", 1, Array_at_builtin },
		{ "[]=", 2, Array_at_set_builtin },
		{ "append", 1, Array_append_builtin },
		{ "iterator", 0, Array_iterator_builtin },
		{ "join", 1, Array_join_builtin },
		{ NULL },
		};
	Class_add_builtin_methods(&Array_class, builtin_methods);

	ArrayIterator_init_class();
}




typedef struct ArrayIterator {
	Class* class_;
	Array* array;
	int index;
	} ArrayIterator;
Class ArrayIterator_class;

ArrayIterator* new_ArrayIterator(Array* array)
{
	ArrayIterator* self = alloc_obj(ArrayIterator);
	self->class_ = &ArrayIterator_class;
	self->array = array;
	self->index = 0;
	return self;
}


Object* ArrayIterator_next(Object* super, Object** args)
{
	ArrayIterator* self = (ArrayIterator*) super;
	if (self->index >= self->array->size)
		return NULL;
	return self->array->items[self->index++];
}

void ArrayIterator_init_class()
{
	init_static_class(ArrayIterator);

	static BuiltinMethodSpec builtin_methods[] = {
		{ "next", 0, ArrayIterator_next },
		{ NULL },
		};
	Class_add_builtin_methods(&ArrayIterator_class, builtin_methods);
}



