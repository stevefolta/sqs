#pragma once

#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include <stdlib.h>


typedef struct String {
	Class* class_;
	const char* str;
	size_t size;

	void (*init)(struct String* self, const char* str, size_t size);
	Object* (*equals)(struct String* self, struct String* other);
	Object* (*less_than)(struct String* self, struct String* other);
	} String;


extern String* new_String(const char* str, size_t size);

#define make_string(str) (new_String(str, 0))


