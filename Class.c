#include "Class.h"
#include "String.h"

static Class Class_class;


void Class_init_static(Class* self, const char* name, int total_num_slots)
{
	self->class_ = &Class_class;
	self->name = new_static_String(name);
	self->total_num_slots = total_num_slots;
}


void Class_init_class()
{
	Class_init_static(&Class_class, "Class", 4);
}



