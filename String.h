#pragma once

#include <stdlib.h>
#include <stdbool.h>

struct Class;
struct Object;

typedef struct String {
	struct Class* class_;
	const char* str;
	size_t size;
	} String;


extern String* new_String(const char* str, size_t size);
extern String* new_c_String(const char* str);
extern String* new_c_static_String(const char* str);
extern String* new_static_String(const char* str, size_t size);
extern bool String_equals(String* self, String* other);
extern bool String_equals_c(String* self, const char* other);
extern bool String_less_than(String* self, String* other);
extern int String_cmp(String* self, String* other);
extern bool String_starts_with(String* self, String* other);
extern bool String_ends_with(String* self, String* other);
extern const char* String_c_str(String* self);
extern String* String_enforce(struct Object* object, const char* name);

extern String* String_add(String* self, String* other);

#define make_string(str) (new_String(str, 0))

extern void String_init_static(String* self, const char* str, size_t size);
extern void String_init_static_c(String* self, const char* str);

extern void String_init_class();

extern struct Class String_class;

#define declare_static_string(name, value) 	\
	static const char name##_chars[] = value; 	\
	static String name = { &String_class, name##_chars, sizeof(value) - 1 };
#define declare_string(name, value) 	\
	static const char name##_chars[] = value; 	\
	String name = { &String_class, name##_chars, sizeof(value) - 1 };

// A few widely-used strings.
extern String iterator_string;
extern String next_string;


