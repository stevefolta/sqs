#pragma once

struct String;
struct Dict;
struct Object;

typedef struct Class {
	struct Class* class_;
	struct String* name;
	struct Class* superclass;
	int total_num_slots;
	struct Dict* methods;
	} Class;


typedef struct BuiltinMethodSpec {
	const char* name;
	int num_args;
	struct Object* (*fn)(struct Object* self, struct Object** args);
	} BuiltinMethodSpec;

extern void Class_init_static(Class* self, const char* name, int total_num_slots);
extern void Class_add_builtin_methods(Class* self, const BuiltinMethodSpec* specs);
	// "specs" is a list, terminated by a NULL entry.

extern void Class_init_class();

#define NumSlotsFor(type) ((sizeof(type) + sizeof(struct Object*) - 1) / sizeof(struct Object*) - 1)


