#include "String.h"
#include "Boolean.h"
#include <string.h>


extern void String_init(String* self, const char* str, size_t size);

String* new_String(const char* str, size_t size)
{
	String* string = (String*) alloc_mem(sizeof(String));
	String_init(string, str, size);
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


Object* String_less_than(struct String* self, struct String* other)
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


const char* String_c_str(struct String* self)
{
	char* str = (char*) alloc_mem(self->size + 1);
	memcpy(str, self->str, self->size);
	str[self->size] = 0;
	return str;
}


void String_init(String* self, const char* str, size_t size)
{
	self->class_ = NULL; 	// TODO

	if (size == 0)
		size = strlen(str);
	self->size = size;
	self->str = alloc_mem(size);
	memcpy((char*) self->str, str, size);
}


