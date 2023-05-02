#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "Array.h"
#include "ByteArray.h"
#include "Memory.h"


MethodBuilder* new_MethodBuilder()
{
	MethodBuilder* self = alloc_obj(MethodBuilder);
	self->method = new_Method();
	self->cur_num_variables = self->max_num_variables = 0;
	self->environment = &global_environment.environment;
	return self;
}


int MethodBuilder_add_literal(MethodBuilder* self, struct Object* literal)
{
	Array_append(self->method->literals, literal);
	return self->method->literals->size - 1;
}


void MethodBuilder_add_bytecode(MethodBuilder* self, uint8_t bytecode)
{
	ByteArray_append(self->method->bytecode, bytecode);
}


int MethodBuilder_reserve_locals(MethodBuilder* self, int num_locals)
{
	int base_index = self->cur_num_variables;
	self->cur_num_variables += num_locals;
	if (self->cur_num_variables > self->max_num_variables)
		self->max_num_variables = self->cur_num_variables;
	return base_index;
}


void MethodBuilder_release_locals(MethodBuilder* self, int num_locals)
{
	self->cur_num_variables -= num_locals;
}



