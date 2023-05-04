#pragma once

#include <stdlib.h>
#include <stdbool.h>

struct Class;

typedef struct String {
	struct Class* class_;
	const char* str;
	size_t size;
	} String;


extern String* new_String(const char* str, size_t size);
extern String* new_c_static_String(const char* str);
extern String* new_static_String(const char* str, size_t size);
extern bool String_equals(struct String* self, struct String* other);
extern bool String_equals_c(struct String* self, const char* other);
extern bool String_less_than(struct String* self, struct String* other);
extern const char* String_c_str(struct String* self);

extern String* String_add(String* self, String* other);

#define make_string(str) (new_String(str, 0))

extern void String_init_static(String* self, const char* str, size_t size);
extern void String_init_static_c(String* self, const char* str);

extern void String_init_class();

extern struct Class String_class;

