#include "Boolean.h"
#include "String.h"

Object true_obj;
Object false_obj;
String true_name, false_name;


static Class Boolean_class;


Object* Boolean_string(Object* self, Object** args)
{
	return (self == &true_obj ? (Object*) &true_name : (Object*) &false_name);
}

void Boolean_init_class()
{
	Class_init_static(&Boolean_class, "Boolean", 0);
	true_obj.class_ = &Boolean_class;
	false_obj.class_ = &Boolean_class;
	String_init_static_c(&true_name, "true");
	String_init_static_c(&false_name, "false");

	static const BuiltinMethodSpec specs[] = {
		{ "string", 0, Boolean_string },
		{ NULL, 0, NULL },
		};
	Class_add_builtin_methods(&Boolean_class, specs);
}



