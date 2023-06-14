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


int Int_enforce(Object* object, const char* name)
{
	if (object == NULL || object->class_ != &Int_class) {
		Class* class_ = (object ? object->class_ : &Nil_class);
		Error("Int required, but got a %s, in \"%s\".", String_c_str(class_->name), name);
		}
	return Int_value(object);
}


Object* Int_init(Object* super, Object** args)
{
	Int* self = (Int*) super;
	if (args[0] == NULL)
		self->value = 0;
	else if (args[0]->class_ == &Int_class)
		self->value = ((Int*) args[0])->value;
	else if (args[0]->class_ == &String_class) {
		char* end_ptr = NULL;
		self->value = strtol(String_c_str((String*) args[0]), &end_ptr, 0);
		if (*end_ptr != 0)
			Error("Invalid conversion from string \"%s\" to Int.", String_c_str((String*) args[0]));
		}
	else
		Error("Int.init() takes a String or another Int.");
	return (Object*) self;
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
	if (args[0] == NULL)
		return (Object*) new_Int(-Int_value(super));
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

Object* Int_not(Object* super, Object** args)
{
	return (Object*) new_Int(~Int_value(super));
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
	if (args[0] == NULL || args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) == Int_value(args[0]));
}

Object* Int_not_equals(Object* super, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &Int_class)
		return &true_obj;
	return make_bool(Int_value(super) != Int_value(args[0]));
}

Object* Int_less_than(Object* super, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) < Int_value(args[0]));
}

Object* Int_greater_than(Object* super, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) > Int_value(args[0]));
}

Object* Int_less_than_or_equal(Object* super, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) <= Int_value(args[0]));
}

Object* Int_greater_than_or_equal(Object* super, Object** args)
{
	if (args[0] == NULL || args[0]->class_ != &Int_class)
		return &false_obj;
	return make_bool(Int_value(super) >= Int_value(args[0]));
}


void Int_init_class()
{
	init_static_class(Int);

	BuiltinMethodSpec builtin_methods[] = {
		{ "init", 1, Int_init },
		{ "string", 0, Int_string },
		{ "+", 1, Int_plus },
		{ "-", 1, Int_minus },
		{ "*", 1, Int_times },
		{ "/", 1, Int_divide },
		{ "|", 1, Int_or },
		{ "^", 1, Int_exclusive_or },
		{ "&", 1, Int_and },
		{ "~", 0, Int_not },
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



