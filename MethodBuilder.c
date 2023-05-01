#include "MethodBuilder.h"
#include "Method.h"
#include "Memory.h"


MethodBuilder* new_MethodBuilder()
{
	MethodBuilder* self = alloc_obj(MethodBuilder);
	self->method = new_Method();
	self->cur_num_variables = self->max_num_variables = 0;
	return self;
}



