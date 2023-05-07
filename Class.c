#include "Class.h"
#include "BuiltinMethod.h"
#include "String.h"
#include "Dict.h"
#include "Object.h"
#include "Memory.h"

Class Class_class;


void Class_init_static(Class* self, const char* name, int num_ivars)
{
	self->class_ = &Class_class;
	self->superclass = &Object_class;
	self->name = new_c_static_String(name);
	self->num_ivars = num_ivars;
}


Class* new_Class(struct String* name)
{
	Class* self = alloc_obj(Class);
	self->class_ = &Class_class;
	self->superclass = &Object_class;
	self->name = name;
	return self;
}


Object* Class_instantiate(Class* self)
{
	Object* object = alloc_mem(self->num_ivars * sizeof(Object*));
	object->class_ = self;
	return object;
}


void Class_add_builtin_methods(Class* self, const BuiltinMethodSpec* specs)
{
	if (self->methods == NULL)
		self->methods = new_Dict();

	// Count the specs.
	int num_specs = 0;
	for (const BuiltinMethodSpec* spec = specs; spec->name; ++spec)
		num_specs += 1;

	// We'll make them as one big chunk with all the objects in it.
	BuiltinMethod* methods = (BuiltinMethod*) alloc_mem(num_specs * sizeof(BuiltinMethod));
	BuiltinMethod* method = methods;
	for (const BuiltinMethodSpec* spec = specs; spec->name; ++spec, ++method) {
		method->class_ = &BuiltinMethod_class;
		method->num_args = spec->num_args;
		method->fn = spec->fn;
		Dict_set_at(self->methods, new_c_static_String(spec->name), (Object*) method);
		}
}


void Class_init_class()
{
	init_static_class(Class);
}



