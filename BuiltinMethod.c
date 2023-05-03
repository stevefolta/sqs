#include "BuiltinMethod.h"
#include "Class.h"

struct Class BuiltinMethod_class;

void BuiltinMethod_init_class()
{
	Class_init_static(&BuiltinMethod_class, "BuiltinMethod", 2);
}


