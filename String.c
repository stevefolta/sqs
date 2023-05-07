#include "String.h"
#include "Class.h"
#include "Object.h"
#include "Boolean.h"
#include "Nil.h"
#include "Memory.h"
#include "Error.h"
#include <string.h>

Class String_class;

extern void String_init(String* self, const char* str, size_t size);

String* new_String(const char* str, size_t size)
{
	String* string = alloc_obj(String);
	String_init(string, str, size);
	return string;
}

String* new_c_String(const char* str)
{
	String* string = alloc_obj(String);
	String_init(string, str, strlen(str));
	return string;
}

String* new_c_static_String(const char* str)
{
	String* string = alloc_obj(String);
	String_init_static_c(string, str);
	return string;
}

String* new_static_String(const char* str, size_t size)
{
	String* string = alloc_obj(String);
	String_init_static(string, str, size);
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

int String_cmp(String* self, String* other)
{
	int size = self->size;
	if (other->size < size)
		size = other->size;
	int cmp = memcmp(self->str, other->str, size);
	if (cmp == 0)
		cmp = self->size - other->size;
	return cmp;
}



const char* String_c_str(struct String* self)
{
	// Useful for debugging.

	char* str = (char*) alloc_mem(self->size + 1);
	memcpy(str, self->str, self->size);
	str[self->size] = 0;
	return str;
}


String* String_enforce(Object* object, const char* name)
{
	if (object == NULL || object->class_ != &String_class) {
		Class* class_ = (object ? object->class_ : &Nil_class);
		Error("String required, but got a %s, in \"%s\".", String_c_str(class_->name), name);
		}
	return (String*) object;
}


String* String_add(String* self, String* other)
{
	String* result = alloc_obj(String);
	result->class_ = &String_class;
	int total_size = self->size + other->size;
	result->size = total_size;
	char* result_str = alloc_mem(total_size);
	memcpy(result_str, self->str, self->size);
	memcpy(result_str + self->size, other->str, other->size);
	result->str = result_str;
	return result;
}


void String_init(String* self, const char* str, size_t size)
{
	self->class_ = &String_class;

	self->size = size;
	self->str = alloc_mem(size);
	memcpy((char*) self->str, str, size);
}


void String_init_static(String* self, const char* str, size_t size)
{
	self->class_ = &String_class;
	self->str = str;
	self->size = size;
}

void String_init_static_c(String* self, const char* str)
{
	self->class_ = &String_class;
	self->str = str;
	self->size = strlen(str);
}


static Object* String_add_builtin(Object* self, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &String_class)
		Error("Attempt to add a non-string to a string.");

	return (Object*) String_add((String*) self, (String*) args[0]);
}

static Object* String_string(Object* self, Object** args)
{
	return (Object*) self;
}

static Object* String_equals_builtin(Object* self, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &String_class)
		return &false_obj;
	return make_bool(String_equals((String*) self, (String*) args[0]));
}

static Object* String_not_equals_builtin(Object* self, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &String_class)
		return &false_obj;
	return make_bool(!String_equals((String*) self, (String*) args[0]));
}

static Object* String_strip_builtin(Object* super, Object** args)
{
	String* self = (String*) super;
	const char* new_start = self->str;
	const char* end = new_start + self->size;
	while (new_start < end) {
		char c = *new_start;
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			new_start += 1;
		else
			break;
		}
	if (new_start >= end)
		return (Object*) new_static_String(NULL, 0);
	const char* new_end = end;
	while (new_end > new_start) {
		char c = new_end[-1];
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			new_end -= 1;
		else
			break;
		}
	if (new_start >= new_end)
		return (Object*) new_static_String(NULL, 0);
	return (Object*) new_static_String(new_start, new_end - new_start);
}

static Object* String_lstrip_builtin(Object* super, Object** args)
{
	String* self = (String*) super;
	const char* new_start = self->str;
	const char* end = new_start + self->size;
	while (new_start < end) {
		char c = *new_start;
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			new_start += 1;
		else
			break;
		}
	if (new_start >= end)
		return (Object*) new_static_String(NULL, 0);
	return (Object*) new_static_String(new_start, end - new_start);
}

static Object* String_rstrip_builtin(Object* super, Object** args)
{
	String* self = (String*) super;
	const char* start = self->str;
	const char* new_end = start + self->size;
	while (new_end > start) {
		char c = new_end[-1];
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			new_end -= 1;
		else
			break;
		}
	if (start >= new_end)
		return (Object*) new_static_String(NULL, 0);
	return (Object*) new_static_String(start, new_end - start);
}

void String_init_class()
{
	init_static_class(String);

	static const BuiltinMethodSpec specs[] = {
		{ "+", 1, String_add_builtin, },
		{ "string", 0, String_string },
		{ "==", 1, String_equals_builtin },
		{ "!=", 1, String_not_equals_builtin },
		{ "strip", 0, String_strip_builtin },
		{ "lstrip", 0, String_lstrip_builtin },
		{ "rstrip", 0, String_rstrip_builtin },
		{ "trim", 0, String_strip_builtin },
		{ "ltrim", 0, String_lstrip_builtin },
		{ "rtrim", 0, String_rstrip_builtin },
		{ NULL, 0, NULL },
		};
	Class_add_builtin_methods(&String_class, specs);
}



