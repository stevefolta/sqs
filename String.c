#include "String.h"
#include "Class.h"
#include "Memory.h"
#include <string.h>

Class String_class;

extern void String_init(String* self, const char* str, size_t size);

String* new_String(const char* str, size_t size)
{
	String* string = alloc_obj(String);
	String_init(string, str, size);
	return string;
}

String* new_static_String(const char* str)
{
	String* string = alloc_obj(String);
	String_init_static(string, str);
	return string;
}


bool String_equals(struct String* self, struct String* other)
{
	return self->size == other->size && memcmp(self->str, other->str, self->size) == 0;
}

bool String_equals_c(struct String* self, const char* other)
{
	return self->size == strlen(other) && memcmp(self->str, other, self->size) == 0;
}


bool String_less_than(struct String* self, struct String* other)
{
	if (self->size == other->size)
		return memcmp(self->str, other->str, self->size) < 0;
	else if (self->size < other->size) {
		int result = memcmp(self->str, other->str, self->size);
		// "abc" < "abcdef": true
		// "abb" < "abcdef": true
		// "abd" < "abcdef": false
		return result <= 0;
		}
	else {
		int result = memcmp(self->str, other->str, other->size);
		// "abbdef" < "abc": true
		// "abcdef" < "abc": false
		// "abddef" < "abc:: false
		return result < 0;
		}
}


const char* String_c_str(struct String* self)
{
	// Useful for debugging.

	char* str = (char*) alloc_mem(self->size + 1);
	memcpy(str, self->str, self->size);
	str[self->size] = 0;
	return str;
}


void String_init(String* self, const char* str, size_t size)
{
	self->class_ = &String_class;

	if (size == 0)
		size = strlen(str);
	self->size = size;
	self->str = alloc_mem(size);
	memcpy((char*) self->str, str, size);
}


void String_init_static(String* self, const char* str)
{
	self->class_ = &String_class;
	self->str = str;
	self->size = strlen(str);
}


void String_init_class()
{
	Class_init_static(&String_class, "String", 2);
}



