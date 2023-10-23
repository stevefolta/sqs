#include "Class.h"
#include "BuiltinMethod.h"
#include "String.h"
#include "Dict.h"
#include "Object.h"
#include "Int.h"
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
	Object* object = alloc_mem((self->num_ivars + 1) * sizeof(Object*));
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


Object* Class_string(Object* super, Object** args)
{
	Class* self = (Class*) super;
	declare_static_string(class_suffix_string, " class");
	return (Object*) String_add(self->name, &class_suffix_string);
}

Object* Class_name(Object* super, Object** args)
{
	return (Object*) ((Class*) super)->name;
}

Object* Class_superclass(Object* super, Object** args)
{
	return (Object*) ((Class*) super)->superclass;
}

Object* Class_num_ivars(Object* super, Object** args)
{
	return (Object*) new_Int(((Class*) super)->num_ivars);
}


void Class_init_class()
{
	init_static_class(Class);

	static BuiltinMethodSpec builtin_methods[] = {
		{ "string", 0, Class_string },
		{ "name", 0, Class_name },
		{ "superclass", 0, Class_superclass },
		{ "num-ivars", 0, Class_num_ivars },
		{ NULL, 0, NULL },
		};
	Class_add_builtin_methods(&Class_class, builtin_methods);
}



