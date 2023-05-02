#include "Boolean.h"

Object true_obj;
Object false_obj;


static Class Boolean_class;

void Boolean_init_class()
{
	Class_init_static(&Boolean_class, "Boolean", 0);
	true_obj.class_ = &Boolean_class;
	false_obj.class_ = &Boolean_class;
}



