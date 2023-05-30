#include "Env.h"
#include "String.h"
#include "Class.h"
#include "Object.h"
#include <stdlib.h>

Class Env_class;
Env env_obj = { &Env_class };


Object* Env_at(Object* super, Object** args)
{
	String* key = String_enforce(args[0], "env.[]");
	char* result = getenv(String_c_str(key));
	return (result ? (Object*) new_c_String(result) : NULL);
}


void Env_init_class()
{
	init_static_class(Env);

	static const BuiltinMethodSpec builtin_methods[] = {
		{ "[]", 1, Env_at },
		{ NULL },
		};
	Class_add_builtin_methods(&Env_class, builtin_methods);
}



