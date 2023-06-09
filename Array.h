#pragma once

#include <stddef.h>
#include <stdlib.h>

struct Object;
struct Class;
struct String;


typedef struct Array {
	struct Class* class_;
	size_t size, capacity;
	struct Object** items;
	} Array;


extern Array* new_Array();
extern struct Object* Array_at(Array* self, size_t index);
extern struct Object* Array_set_at(Array* self, size_t index, struct Object* value);
extern struct Object* Array_append(Array* self, struct Object* value);
extern void Array_append_strings(Array* self, struct Object* value);
extern struct Object* Array_pop_back(Array* self);
extern struct Object* Array_back(Array* self);
extern Array* Array_copy(Array* self);
extern struct String* Array_join(Array* self, struct String* joiner);


extern struct Class Array_class;
extern void Array_init_class();

