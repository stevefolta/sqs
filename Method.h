#pragma once

struct ByteArray;
struct Array;
struct Class;


typedef struct Method {
	struct Class* class_;
	int num_args; 	// Put this in the same position in both Method and BuiltinMethod.
	struct ByteArray* bytecode;
	struct Array* literals;
	int stack_size;
	} Method;

Method* new_Method(int num_args);

extern struct Class Method_class;
extern void Method_init_class();

