#include "Float.h"
#include "Int.h"
#include "Class.h"
#include "Object.h"
#include "String.h"
#include "Nil.h"
#include "Boolean.h"
#include "Memory.h"
#include "UTF8.h"
#include "Error.h"
#include <stdio.h>

Class Float_class;


Float* new_Float(double value)
{
	Float* self = alloc_obj(Float);
	self->class_ = &Float_class;
	self->value = value;
	return self;
}


double Float_enforce(Object* object, const char* name)
{
	if (object != NULL) {
		if (object->class_ == &Float_class)
			return Float_value(object);
		else if (object->class_ == &Int_class)
			return Int_value(object);
		}
	Error("Float required, but got a %s, in \"%s\".", String_c_str(object->class_->name), name);
	return 0.0;
}


Object* Float_init(Object* super, Object** args)
{
	Float* self = (Float*) super;
	if (args[0] == NULL)
		self->value = 0;
	else if (args[0]->class_ == &Float_class)
		self->value = ((Float*) args[0])->value;
	else if (args[0]->class_ == &Int_class)
		self->value = Int_value(args[0]);
	else if (args[0]->class_ == &String_class) {
		char* end_ptr = NULL;
		self->value = strtod(String_c_str((String*) args[0]), &end_ptr);
		if (*end_ptr != 0)
			Error("Invalid conversion from string \"%s\" to Float.", String_c_str((String*) args[0]));
		}
	else
		Error("Float.init() takes a String, a Float, or another Int.");
	return (Object*) self;
}

Object* Float_string(Object* super, Object** args)
{
	char str[64];
	sprintf(str, "%g", ((Float*) super)->value);
	return (Object*) new_c_String(str);
}

Object* Float_plus(Object* super, Object** args)
{
	return (Object*) new_Float(Float_value(super) + Float_enforce(args[0], "Float.+"));
}

Object* Float_minus(Object* super, Object** args)
{
	if (args[0] == NULL)
		return (Object*) new_Float(-Float_value(super));
	return (Object*) new_Float(Float_value(super) - Float_enforce(args[0], "Float.-"));
}

Object* Float_times(Object* super, Object** args)
{
	return (Object*) new_Float(Float_value(super) * Float_enforce(args[0], "Float.*"));
}

Object* Float_divide(Object* super, Object** args)
{
	return (Object*) new_Float(Float_value(super) / Float_enforce(args[0], "Float./"));
}

static bool is_floatable(Object* object)
{
	if (object == NULL)
		return false;
	return object->class_ == &Float_class || object->class_ == &Int_class;
}

Object* Float_equals(Object* super, Object** args)
{
	if (!is_floatable(args[0]))
		return &false_obj;
	return make_bool(Float_value(super) == Float_enforce(args[0], ""));
}

Object* Float_not_equals(Object* super, Object** args)
{
	if (!is_floatable(args[0]))
		return &true_obj;
	return make_bool(Float_value(super) != Float_enforce(args[0], ""));
}

Object* Float_less_than(Object* super, Object** args)
{
	if (!is_floatable(args[0]))
		return &false_obj;
	return make_bool(Float_value(super) < Float_enforce(args[0], ""));
}

Object* Float_greater_than(Object* super, Object** args)
{
	if (!is_floatable(args[0]))
		return &false_obj;
	return make_bool(Float_value(super) > Float_enforce(args[0], ""));
}

Object* Float_less_than_or_equal(Object* super, Object** args)
{
	if (!is_floatable(args[0]))
		return &false_obj;
	return make_bool(Float_value(super) <= Float_enforce(args[0], ""));
}

Object* Float_greater_than_or_equal(Object* super, Object** args)
{
	if (!is_floatable(args[0]))
		return &false_obj;
	return make_bool(Float_value(super) >= Float_enforce(args[0], ""));
}


void Float_init_class()
{
	init_static_class(Float);

	BuiltinMethodSpec builtin_methods[] = {
		{ "init", 1, Float_init },
		{ "string", 0, Float_string },
		{ "+", 1, Float_plus },
		{ "-", 1, Float_minus },
		{ "*", 1, Float_times },
		{ "/", 1, Float_divide },
		{ "==", 1, Float_equals },
		{ "!=", 1, Float_not_equals },
		{ "<", 1, Float_less_than },
		{ ">", 1, Float_greater_than },
		{ "<=", 1, Float_less_than_or_equal },
		{ ">=", 1, Float_greater_than_or_equal },
		{ NULL },
		};
	Class_add_builtin_methods(&Float_class, builtin_methods);
}



