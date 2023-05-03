#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "Array.h"
#include "ByteArray.h"
#include "Memory.h"


MethodBuilder* new_MethodBuilder(int num_args)
{
	MethodBuilder* self = alloc_obj(MethodBuilder);
	self->method = new_Method(num_args);
	self->cur_num_variables = self->max_num_variables = num_args + 1;
		// "self" and the arguments count as a variables here.
	self->environment = &global_environment.environment;
	return self;
}


void MethodBuilder_finish(MethodBuilder* self)
{
	self->method->stack_size = self->max_num_variables;
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


int MethodBuilder_add_offset8(MethodBuilder* self)
{
	int patch_point = self->method->bytecode->size;
	ByteArray_append(self->method->bytecode, 0);
	return patch_point;
}


void MethodBuilder_add_back_offset8(MethodBuilder* self, int patch_point)
{
	int offset = patch_point - self->method->bytecode->size - 1;
	ByteArray_append(self->method->bytecode, offset);
}


void MethodBuilder_patch_offset8(MethodBuilder* self, int patch_point)
{
	ByteArray* bytecode = self->method->bytecode;
	int offset = bytecode->size - patch_point - 1;
	ByteArray_set_at(bytecode, patch_point, offset);
}


int MethodBuilder_get_offset(MethodBuilder* self)
{
	return self->method->bytecode->size;
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



