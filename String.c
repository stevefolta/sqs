#include "String.h"
#include "Class.h"
#include "Array.h"
#include "Object.h"
#include "Boolean.h"
#include "Int.h"
#include "Nil.h"
#include "ByteArray.h"
#include "Memory.h"
#include "UTF8.h"
#include "Error.h"
#include <string.h>
#include <stdbool.h>

Class String_class;
String empty_string = { &String_class, NULL, 0 };
declare_string(iterator_string, "iterator");
declare_string(next_string, "next");

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

bool String_starts_with(String* self, String* other)
{
	if (self->size < other->size)
		return false;
	return memcmp(self->str, other->str, other->size) == 0;
}

bool String_ends_with(String* self, String* other)
{
	if (self->size < other->size)
		return false;
	return memcmp(self->str + self->size - other->size, other->str, other->size) == 0;
}


const char* String_c_str(struct String* self)
{
	// Useful for debugging.

	char* str = (char*) alloc_mem_no_pointers(self->size + 1);
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


String* String_copy(String* other)
{
	return new_String(other->str, other->size);
}


String* String_add(String* self, String* other)
{
	String* result = alloc_obj(String);
	result->class_ = &String_class;
	int total_size = self->size + other->size;
	result->size = total_size;
	char* result_str = alloc_mem_no_pointers(total_size);
	memcpy(result_str, self->str, self->size);
	memcpy(result_str + self->size, other->str, other->size);
	result->str = result_str;
	return result;
}


void String_init(String* self, const char* str, size_t size)
{
	self->class_ = &String_class;

	self->size = size;
	self->str = alloc_mem_no_pointers(size);
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

static Object* String_less_than_builtin(Object* self, Object** args)
{
	return make_bool(String_cmp((String*) self, String_enforce(args[0], "<")) < 0);
}

static Object* String_greater_than_builtin(Object* self, Object** args)
{
	return make_bool(String_cmp((String*) self, String_enforce(args[0], ">")) > 0);
}

static Object* String_less_than_equals_builtin(Object* self, Object** args)
{
	return make_bool(String_cmp((String*) self, String_enforce(args[0], "<=")) <= 0);
}

static Object* String_greater_than_equals_builtin(Object* self, Object** args)
{
	return make_bool(String_cmp((String*) self, String_enforce(args[0], ">=")) >= 0);
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

static Object* String_split_builtin(Object* super, Object** args)
{
	String* self = (String*) super;
	Array* result = new_Array();
	const char* p = self->str;
	const char* end = p + self->size;

	// Whiltespace splitting.
	if (args[0] == NULL) {
		while (p < end) {
			// Skip initial whitespace.
			while (p < end) {
				char c = *p;
				if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
					p += 1;
				else
					break;
				}

			// Add the non-whitespace run.
			const char* word_start = p;
			while (p < end) {
				char c = *p;
				if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
					break;
				p += 1;
				}
			if (p > word_start)
				Array_append(result, (Object*) new_static_String(word_start, p - word_start));
			}
		}

	// Splitting by delimiter string.
	else {
		String* delimiter = String_enforce(args[0], "String.split");
		size_t delimiter_size = delimiter->size;
		while (true) {
			// Find the start of the delimiter.
			// (We'd use memmem() but it's not part of POSIX.)
			const char* delimiter_start = NULL;
			const char* search_p = p;
			while (search_p < end) {
				if (memcmp(search_p, delimiter->str, delimiter_size) == 0) {
					delimiter_start = search_p;
					break;
					}
				search_p += 1;
				}

			if (delimiter_start) {
				Array_append(result, (Object*) new_static_String(p, delimiter_start - p));
				p = delimiter_start + delimiter_size;
				}
			else {
				Array_append(result, (Object*) new_static_String(p, end - p));
				break;
				}
			}
		}

	return (Object*) result;
}

Object* String_starts_with_builtin(Object* super, Object** args)
{
	String* other = String_enforce(args[0], "String.starts-with");
	return make_bool(String_starts_with((String*) super, other));
}

Object* String_ends_with_builtin(Object* super, Object** args)
{
	String* other = String_enforce(args[0], "String.ends-with");
	return make_bool(String_ends_with((String*) super, other));
}

Object* String_contains_builtin(Object* super, Object** args)
{
	String* self = (String*) super;
	String* other = String_enforce(args[0], "String.contains");
	// memmem() is not part of Posix, so we have to do it ourself.
	const char* haystack_end = self->str + self->size - other->size + 1;
	const char* needle_end = other->str + other->size;
	for (const char* haystack_p = self->str; haystack_p < haystack_end; ++haystack_p) {
		const char* needle_p = other->str;
		const char* cur_haystack_p = haystack_p;
		for (; needle_p < needle_end; ++needle_p, ++cur_haystack_p) {
			if (*cur_haystack_p != *needle_p)
				break;
			}
		if (needle_p == needle_end)
			return &true_obj;
		}
	return &false_obj;
}

Object* String_is_valid_builtin(Object* super, Object** args)
{
	String* self = (String*) super;
	return make_bool(is_valid_utf8(self->str, self->size));
}

Object* String_decode_8859_1_builtin(Object* super, Object** args)
{
	String* self = (String*) super;
	return (Object*) decode_8859_1((uint8_t*) self->str, self->size);
}

Object* String_bytes(Object* super, Object** args)
{
	String* self = (String*) super;
	ByteArray* byte_array = new_ByteArray();
	byte_array->size = byte_array->capacity = self->size;
	byte_array->array = (uint8_t*) self->str;
	return (Object*) byte_array;
}

Object* String_size(Object* super, Object** args)
{
	String* self = (String*) super;
	int num_chars = chars_in_utf8(self->str, self->size);
	if (num_chars < 0)
		Error("Invalid UTF-8 (in string.size).");
	return (Object*) new_Int(num_chars);
}

Object* String_slice(Object* super, Object** args)
{
	String* self = (String*) super;
	int num_chars = chars_in_utf8(self->str, self->size);
	if (num_chars < 0)
		Error("Invalid UTF-8 (in string.slice())");
	int start = args[0] ? Int_enforce(args[0], "String.slice") : 0;
	int end = args[1] ? Int_enforce(args[1], "String.slice") : num_chars;
	if (start < 0)
		start += num_chars;
	if (start >= num_chars)
		return (Object*) &empty_string;
	if (end < 0)
		end += num_chars;
	if (end < start)
		return (Object*) &empty_string;
	else if (end > num_chars)
		end = num_chars;

	String* result = alloc_obj(String);
	result->class_ = &String_class;
	result->str = self->str + bytes_in_n_characters(self->str, start);
	result->size = bytes_in_n_characters(result->str, end - start);
	return (Object*) result;
}


void String_init_class()
{
	init_static_class(String);

	static const BuiltinMethodSpec specs[] = {
		{ "+", 1, String_add_builtin, },
		{ "string", 0, Object_identity },
		{ "==", 1, String_equals_builtin },
		{ "!=", 1, String_not_equals_builtin },
		{ "<", 1, String_less_than_builtin },
		{ ">", 1, String_greater_than_builtin },
		{ "<=", 1, String_less_than_equals_builtin },
		{ ">=", 1, String_greater_than_equals_builtin },
		{ "strip", 0, String_strip_builtin },
		{ "lstrip", 0, String_lstrip_builtin },
		{ "rstrip", 0, String_rstrip_builtin },
		{ "trim", 0, String_strip_builtin },
		{ "ltrim", 0, String_lstrip_builtin },
		{ "rtrim", 0, String_rstrip_builtin },
		{ "split", 1, String_split_builtin },
		{ "starts-with", 1, String_starts_with_builtin },
		{ "ends-with", 1, String_ends_with_builtin },
		{ "contains", 1, String_contains_builtin },
		{ "is-valid", 0, String_is_valid_builtin },
		{ "decode-8859-1", 0, String_decode_8859_1_builtin },
		{ "bytes", 0, String_bytes },
		{ "size", 0, String_size },
		{ "slice", 0, String_slice },
		{ NULL, 0, NULL },
		};
	Class_add_builtin_methods(&String_class, specs);
}



