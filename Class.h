#pragma once

struct String;
struct Dict;
struct Object;
struct Array;

typedef struct Class {
	struct Class* class_;
	struct String* name;
	struct Class* superclass;
	struct Dict* methods;
	int num_ivars;
	struct Array* slot_names;
	} Class;


typedef struct BuiltinMethodSpec {
	const char* name;
	int num_args;
	struct Object* (*fn)(struct Object* self, struct Object** args);
	} BuiltinMethodSpec;

extern void Class_init_static(Class* self, const char* name, int num_ivars);
extern void Class_add_builtin_methods(Class* self, const BuiltinMethodSpec* specs);
	// "specs" is a list, terminated by a NULL entry.
extern Class* new_Class(struct String* name);
extern struct Object* Class_instantiate(Class* self);
extern struct Object* Class_find_super_method(Class* self, struct String* name);

extern Class Class_class;
extern void Class_init_class();

#define NumIvarsFor(type) ((sizeof(type) + sizeof(struct Object*) - 1) / sizeof(struct Object*) - 1)
#define init_static_class(type) (Class_init_static(&type##_class, #type, NumIvarsFor(type)))


