#include "Object.h"
#include "Class.h"
#include "Dict.h"
#include "String.h"
#include "Nil.h"
#include "Boolean.h"
#include "Error.h"
#include <stddef.h>
#include <string.h>


Object* Object_find_method(Object* self, struct String* name)
{
	Class* class_ = (self ? self->class_ : &Nil_class);
	while (class_) {
		if (class_->methods) {
			Object* method = Dict_at(class_->methods, name);
			if (method)
				return method;
			}
		class_ = class_->superclass;
		}

	return NULL;
}


Object* Object_find_super_method(Object* self, struct String* name)
{
	Class* class_ = (self ? self->class_ : &Nil_class);
	class_ = (class_->superclass ? class_->superclass : NULL);
	while (class_) {
		if (class_->methods) {
			Object* method = Dict_at(class_->methods, name);
			if (method)
				return method;
			}
		class_ = class_->superclass;
		}

	return NULL;
}


Object* Object_identity(Object* self, Object** args)
{
	return self;
}


Object* Object_string(Object* self, Object** args)
{
	String* class_name = self->class_->name;
	String* prefix =
		strchr("AEIOUaeiou", self->class_->name->str[0]) ?
		new_c_static_String("an ") :
		new_c_static_String("a ");
	return (Object*) String_add(prefix, class_name);
}

Object* Object_equals(Object* self, Object** args)
{
	// Default: is it the exact same object?
	return make_bool(self == args[0]);
}

Object* Object_not_equals(Object* self, Object** args)
{
	// Default: is it the exact same object?
	return make_bool(self != args[0]);
}


Class Object_class;
void Object_init_class()
{
	init_static_class(Object);
	Object_class.superclass = NULL;

	static BuiltinMethodSpec builtin_methods[] = {
		{ "string", 0, Object_string },
		{ "==", 1, Object_equals },
		{ "!=", 1, Object_not_equals },
		{ NULL, 0, NULL },
		};
	Class_add_builtin_methods(&Object_class, builtin_methods);
}


