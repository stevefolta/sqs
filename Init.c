#include "Init.h"
#include "Object.h"
#include "Class.h"
#include "String.h"
#include "Boolean.h"
#include "Array.h"
#include "Dict.h"
#include "Nil.h"
#include "Method.h"
#include "BuiltinMethod.h"
#include "Environment.h"


void init_all()
{
	Class_init_class();
	Object_init_class();
	String_init_class();
	Boolean_init_class();
	Array_init_class();
	Dict_init_class();
	Method_init_class();
	BuiltinMethod_init_class();
	Nil_init_class();

	GlobalEnvironment_init();
}

