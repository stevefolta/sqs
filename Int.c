#include "Int.h"
#include "Class.h"
#include "Object.h"
#include "String.h"
#include "Nil.h"
#include "Boolean.h"
#include "Memory.h"
#include "Error.h"
#include <stdio.h>

Class Int_class;


Int* new_Int(int value)
{
	Int* self = alloc_obj(Int);
	self->class_ = &Int_class;
	self->value = value;
	return self;
}


void Int_enforce(Object* object, const char* name)
{
	if (object == NULL || object->class_ != &Int_class) {
		Class* class_ = (object ? object->class_ : &Nil_class);
		Error("Int required, but got a %s, in \"%s\".", String_c_str(class_->name), name);
		}
}


Object* Int_string(Object* super, Object** args)
{
	char str[64];
	sprintf(str, "%d", ((Int*) super)->value);
	return (Object*) new_c_String(str);
}

Object* Int_plus(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.+");
	return (Object*) new_Int(Int_value(super) + Int_value(args[0]));
}

Object* Int_minus(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.-");
	return (Object*) new_Int(Int_value(super) - Int_value(args[0]));
}

Object* Int_times(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.*");
	return (Object*) new_Int(Int_value(super) * Int_value(args[0]));
}

Object* Int_divide(Object* super, Object** args)
{
	Int_enforce(args[0], "Int./");
	return (Object*) new_Int(Int_value(super) / Int_value(args[0]));
}

Object* Int_or(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.+");
	return (Object*) new_Int(Int_value(super) | Int_value(args[0]));
}

Object* Int_exclusive_or(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.+");
	return (Object*) new_Int(Int_value(super) ^ Int_value(args[0]));
}

Object* Int_and(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.+");
	return (Object*) new_Int(Int_value(super) & Int_value(args[0]));
}

Object* Int_left_shift(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.+");
	return (Object*) new_Int(Int_value(super) << Int_value(args[0]));
}

Object* Int_right_shift(Object* super, Object** args)
{
	Int_enforce(args[0], "Int.+");
	return (Object*) new_Int(Int_value(super) >> Int_value(args[0]));
}

Object* Int_equals(Object* super, Object** args)
{
	if (args[0] && args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) == Int_value(args[0]));
}

Object* Int_not_equals(Object* super, Object** args)
{
	if (args[0] && args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) != Int_value(args[0]));
}

Object* Int_less_than(Object* super, Object** args)
{
	if (args[0] && args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) < Int_value(args[0]));
}

Object* Int_greater_than(Object* super, Object** args)
{
	if (args[0] && args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) > Int_value(args[0]));
}

Object* Int_less_than_or_equal(Object* super, Object** args)
{
	if (args[0] && args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) <= Int_value(args[0]));
}

Object* Int_greater_than_or_equal(Object* super, Object** args)
{
	if (args[0] && args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) >= Int_value(args[0]));
}


void Int_init_class()
{
	init_static_class(Int);

	BuiltinMethodSpec builtin_methods[] = {
		{ "string", 0, Int_string },
		{ "+", 1, Int_plus },
		{ "-", 1, Int_minus },
		{ "*", 1, Int_times },
		{ "/", 1, Int_divide },
		{ "|", 1, Int_or },
		{ "^", 1, Int_exclusive_or },
		{ "&", 1, Int_and },
		{ "==", 1, Int_equals },
		{ "!=", 1, Int_not_equals },
		{ "<", 1, Int_less_than },
		{ ">", 1, Int_greater_than },
		{ "<=", 1, Int_less_than_or_equal },
		{ ">=", 1, Int_greater_than_or_equal },
		{ "<<", 1, Int_left_shift },
		{ ">>", 1, Int_right_shift },
		{ NULL },
		};
	Class_add_builtin_methods(&Int_class, builtin_methods);
}



