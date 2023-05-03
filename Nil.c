#include "Nil.h"
#include "Class.h"

Class Nil_class;

void Nil_init_class()
{
	Class_init_static(&Nil_class, "Nil", 0);
}


