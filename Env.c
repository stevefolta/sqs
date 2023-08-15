#include "Env.h"
#include "String.h"
#include "Dict.h"
#include "Class.h"
#include "Object.h"
#include <stdlib.h>
#include <string.h>

Class Env_class;
Env env_obj = { &Env_class };


Object* Env_at(Object* super, Object** args)
{
	String* key = String_enforce(args[0], "env.[]");
	char* result = getenv(String_c_str(key));
	return (result ? (Object*) new_c_String(result) : NULL);
}

Object* Env_as_dict(Object* super, Object** args)
{
	Dict* dict = new_Dict();
	extern char** environ;
	for (char** e = environ; *e; ++e) {
		const char* entry = *e;
		char* delim = strchr(entry, '=');
		if (delim == NULL)
			continue;
		String* key = new_String(entry, delim - entry);
		String* value = new_c_String(delim + 1);
		Dict_set_at(dict, key, (Object*) value);
		}
	return (Object*) dict;
}


void Env_init_class()
{
	init_static_class(Env);

	static const BuiltinMethodSpec builtin_methods[] = {
		{ "[]", 1, Env_at },
		{ "as-dict", 0, Env_as_dict },
		{ NULL },
		};
	Class_add_builtin_methods(&Env_class, builtin_methods);
}



