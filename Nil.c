#include "Nil.h"
#include "Class.h"
#include "String.h"
#include "Object.h"

Class Nil_class;

Object* Nil_string(Object* self, Object** args)
{
	return (Object*) new_c_static_String("nil");
}


void Nil_init_class()
{
	Class_init_static(&Nil_class, "Nil", 0);

	static BuiltinMethodSpec builtin_methods[] = {
		{ "string", 0, Nil_string },
		{ NULL },
		};
	Class_add_builtin_methods(&Nil_class, builtin_methods);
}


