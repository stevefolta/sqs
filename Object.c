#include "Object.h"
#include "Class.h"
#include "Dict.h"
#include "String.h"
#include "Array.h"
#include "BuiltinMethod.h"
#include "Nil.h"
#include "Boolean.h"
#include "Error.h"
#include <stddef.h>
#include <string.h>


extern Object* ivar_accessor(int index);

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

	// Check if it's accessing an ivar.
	class_ = (self ? self->class_ : &Nil_class);
	while (class_) {
		if (class_->slot_names) {
			for (int i = 0; i < class_->slot_names->size; ++i) {
				if (String_equals(name, (String*) Array_at(class_->slot_names, i)))
					return ivar_accessor(i + (class_->superclass ? class_->superclass->num_ivars : 0));
				}
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


#define IvarAccessor(index) 	\
	Object* ivar_accessor_##index(Object* self, Object** args) 	\
	{ 	\
		return ((Object**) self)[index + 1]; 	\
	}
#define max_ivar_accessors 32
IvarAccessor(0) IvarAccessor(1) IvarAccessor(2) IvarAccessor(3)
IvarAccessor(4) IvarAccessor(5) IvarAccessor(6) IvarAccessor(7)
IvarAccessor(8) IvarAccessor(9) IvarAccessor(10) IvarAccessor(11)
IvarAccessor(12) IvarAccessor(13) IvarAccessor(14) IvarAccessor(15)
IvarAccessor(16) IvarAccessor(17) IvarAccessor(18) IvarAccessor(19)
IvarAccessor(20) IvarAccessor(21) IvarAccessor(22) IvarAccessor(23)
IvarAccessor(24) IvarAccessor(25) IvarAccessor(26) IvarAccessor(27)
IvarAccessor(28) IvarAccessor(29) IvarAccessor(30) IvarAccessor(31)
#define IvarAccessorBuiltin(index) { &BuiltinMethod_class, 0, ivar_accessor_##index }
static BuiltinMethod ivar_accessors[max_ivar_accessors] = {
	IvarAccessorBuiltin(0), IvarAccessorBuiltin(1), IvarAccessorBuiltin(2), IvarAccessorBuiltin(3),
	IvarAccessorBuiltin(4), IvarAccessorBuiltin(5), IvarAccessorBuiltin(6), IvarAccessorBuiltin(7),
	IvarAccessorBuiltin(8), IvarAccessorBuiltin(9), IvarAccessorBuiltin(10), IvarAccessorBuiltin(11),
	IvarAccessorBuiltin(12), IvarAccessorBuiltin(13), IvarAccessorBuiltin(14), IvarAccessorBuiltin(15),
	IvarAccessorBuiltin(16), IvarAccessorBuiltin(17), IvarAccessorBuiltin(18), IvarAccessorBuiltin(19),
	IvarAccessorBuiltin(20), IvarAccessorBuiltin(21), IvarAccessorBuiltin(22), IvarAccessorBuiltin(23),
	IvarAccessorBuiltin(24), IvarAccessorBuiltin(25), IvarAccessorBuiltin(26), IvarAccessorBuiltin(27),
	IvarAccessorBuiltin(28), IvarAccessorBuiltin(29), IvarAccessorBuiltin(30), IvarAccessorBuiltin(31),
	};

Object* ivar_accessor(int index)
{
	if (index >= max_ivar_accessors)
		Error("Sorry, we don't support external access of more than %d ivars.", max_ivar_accessors);
	return (Object*) &ivar_accessors[index];
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

Object* Object_is_a(Object* self, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &Class_class)
		return &false_obj;
	Class* test_class = (Class*) args[0];
	Class* cur_class = self->class_;
	for (; cur_class; cur_class = cur_class->superclass) {
		if (cur_class == test_class)
			return &true_obj;
		}
	return &false_obj;
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
		{ "is-a", 1, Object_is_a },
		{ NULL, 0, NULL },
		};
	Class_add_builtin_methods(&Object_class, builtin_methods);
}



