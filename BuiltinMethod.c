#include "BuiltinMethod.h"
#include "Class.h"
#include "Memory.h"


BuiltinMethod* new_BuiltinMethod(int num_args, struct Object* (*fn)(struct Object* self, struct Object** args))
{
	BuiltinMethod* self = alloc_obj(BuiltinMethod);
	self->class_ = &BuiltinMethod_class;
	self->num_args = num_args;
	self->fn = fn;
	return self;
}


struct Class BuiltinMethod_class;

void BuiltinMethod_init_class()
{
	init_static_class(BuiltinMethod);
}


