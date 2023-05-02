#pragma once

#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include <stdlib.h>
#include <stdbool.h>


typedef struct String {
	Class* class_;
	const char* str;
	size_t size;
	} String;


extern String* new_String(const char* str, size_t size);
extern bool String_equals(struct String* self, struct String* other);
extern bool String_equals_c(struct String* self, const char* other);
extern bool String_less_than(struct String* self, struct String* other);
extern const char* String_c_str(struct String* self);

#define make_string(str) (new_String(str, 0))


