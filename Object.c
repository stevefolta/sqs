#include "Object.h"
#include "Class.h"

Class Object_class;
void Object_init_class()
{
	Class_init_static(&Object_class, "Object", 0);
}


