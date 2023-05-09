#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "Array.h"
#include "String.h"
#include "ByteArray.h"
#include "ByteCode.h"
#include "Memory.h"
#include "Error.h"


MethodBuilder* new_MethodBuilder(Array* arguments, Environment* environment)
{
	MethodBuilder* self = alloc_obj(MethodBuilder);
	int num_args = arguments->size;
	self->method = new_Method(num_args);
	self->arguments = arguments;
	self->cur_num_variables = self->max_num_variables = num_args + 1;
		// "self" and the arguments count as a variables here.
	if (environment == NULL)
		environment = &global_environment.environment;
	self->environment = (Environment*) new_MethodEnvironment(self, environment);
	return self;
}


void MethodBuilder_finish(MethodBuilder* self)
{
	MethodBuilder_add_bytecode(self, BC_RETURN_NIL);
	self->method->stack_size = self->max_num_variables;
}

void MethodBuilder_finish_init(MethodBuilder* self)
{
	MethodBuilder_add_bytecode(self, BC_RETURN);
	MethodBuilder_add_bytecode(self, 0);
	self->method->stack_size = self->max_num_variables;
}


int MethodBuilder_add_literal(MethodBuilder* self, struct Object* literal)
{
	Array_append(self->method->literals, literal);
	return self->method->literals->size - 1;
}

int MethodBuilder_reserve_literal(MethodBuilder* self)
{
	Array_append(self->method->literals, NULL);
	return self->method->literals->size - 1;
}

void MethodBuilder_set_literal(MethodBuilder* self, int literal, struct Object* value)
{
	Array_set_at(self->method->literals, literal, value);
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
	if (offset < -127 || offset > 127)
		Error("Internal error: Offset out-of-bounds!");
	ByteArray_append(self->method->bytecode, offset);
}


void MethodBuilder_patch_offset8(MethodBuilder* self, int patch_point)
{
	ByteArray* bytecode = self->method->bytecode;
	int offset = bytecode->size - patch_point - 1;
	if (offset < -127 || offset > 127)
		Error("Internal error: Offset out-of-bounds!");
	ByteArray_set_at(bytecode, patch_point, offset);
}


int MethodBuilder_get_offset(MethodBuilder* self)
{
	return self->method->bytecode->size;
}


void MethodBuilder_add_move(MethodBuilder* self, int src, int dest)
{
	MethodBuilder_add_bytecode(self, BC_SET_LOCAL);
	MethodBuilder_add_bytecode(self, src);
	MethodBuilder_add_bytecode(self, dest);
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


int MethodBuilder_find_argument(MethodBuilder* self, String* name)
{
	for (int i = 0; i < self->arguments->size; ++i) {
		String* arg_name = (String*) Array_at(self->arguments, i);
		if (String_equals(arg_name, name))
			return i + 1;
		}
	return 0;
}


void MethodBuilder_push_environment(MethodBuilder* self, Environment* environment)
{
	environment->parent = self->environment;
	self->environment = environment;
}


void MethodBuilder_pop_environment(MethodBuilder* self)
{
	self->environment = self->environment->parent;
}


typedef struct LoopPoints {
	struct LoopPoints* parent;
	Array* continue_patch_points;
	Array* break_patch_points;
	} LoopPoints;

void MethodBuilder_push_loop_points(MethodBuilder* self)
{
	LoopPoints* loop_points = alloc_obj(LoopPoints);
	loop_points->parent = self->loop_points;
	loop_points->continue_patch_points = new_Array();
	loop_points->break_patch_points = new_Array();
	self->loop_points = loop_points;
}


void MethodBuilder_pop_loop_points(MethodBuilder* self, int continue_point, int break_point)
{
	if (self->loop_points == NULL) {
		// Error("Internal error: unmatched loop points.");
		return;
		}

	ByteArray* bytecode = self->method->bytecode;
	LoopPoints* loop_points = self->loop_points;
	for (int i = 0; i < loop_points->continue_patch_points->size; ++i) {
		size_t patch_point = (size_t) Array_at(loop_points->continue_patch_points, i);
		int offset = continue_point - patch_point - 1;
		ByteArray_set_at(bytecode, patch_point, offset);
		}
	for (int i = 0; i < loop_points->break_patch_points->size; ++i) {
		size_t patch_point = (size_t) Array_at(loop_points->break_patch_points, i);
		int offset = break_point - patch_point - 1;
		ByteArray_set_at(bytecode, patch_point, offset);
		}

	self->loop_points = loop_points->parent;
}


void MethodBuilder_add_continue_offset8(MethodBuilder* self)
{
	Array_append(
		self->loop_points->continue_patch_points,
		(Object*) (size_t) MethodBuilder_add_offset8(self));
}


void MethodBuilder_add_break_offset8(MethodBuilder* self)
{
	Array_append(
		self->loop_points->break_patch_points,
		(Object*) (size_t) MethodBuilder_add_offset8(self));
}



