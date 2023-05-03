#pragma once

struct Class;
struct Object;

typedef struct BuiltinMethod {
	struct Class* class_;
	int num_args; 	// Put this in the same position in both Method and BuiltinMethod.
	struct Object* (*fn)(struct Object* self, struct Object** args);
	} BuiltinMethod;

extern struct Class BuiltinMethod_class;
extern void BuiltinMethod_init_class();

