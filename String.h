#pragma once

#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include <stdlib.h>


typedef struct String {
	Class* class_;
	const char* str;
	size_t size;
	} String;


extern String* new_String(const char* str, size_t size);
extern Object* String_equals(struct String* self, struct String* other);
extern Object* String_equals_c(struct String* self, const char* other);
extern Object* String_less_than(struct String* self, struct String* other);

#define make_string(str) (new_String(str, 0))


