#include "String.h"
#include "Boolean.h"
#include <string.h>


extern void String_init(String* self, const char* str, size_t size);

String* new_String(const char* str, size_t size)
{
	String* string = (String*) alloc_mem(sizeof(String));
	string->init = String_init;
	string->init(string, str, size);
	return string;
}


static Object* String_equals(struct String* self, struct String* other)
{
	return make_bool(self->size == other->size && memcmp(self->str, other->str, self->size) == 0);
}

static Object* String_less_than(struct String* self, struct String* other)
{
	if (self->size == other->size)
		return make_bool(memcmp(self->str, other->str, self->size) < 0);
	else if (self->size < other->size) {
		int result = memcmp(self->str, other->str, self->size);
		// "abc" < "abcdef": true
		// "abb" < "abcdef": true
		// "abd" < "abcdef": false
		return make_bool(result <= 0);
		}
	else {
		int result = memcmp(self->str, other->str, other->size);
		// "abbdef" < "abc": true
		// "abcdef" < "abc": false
		// "abddef" < "abc:: false
		return make_bool(result < 0);
		}
}


void String_init(String* self, const char* str, size_t size)
{
	self->class_ = NULL; 	// TODO
	self->equals = String_equals;
	self->less_than = String_less_than;

	if (size == 0)
		size = strlen(str);
	self->size = size;
	self->str = alloc_mem(size);
	memcpy((char*) self->str, str, size);
}


