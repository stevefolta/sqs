#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "ParseNode.h"
#include "Array.h"
#include "Dict.h"
#include "String.h"
#include "Int.h"
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
	self->unwindings = new_Array();
	self->string_literals = new_Dict();
	self->object_literals = new_Dict();
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


int MethodBuilder_emit_literal(MethodBuilder* self, Object* literal)
{
	// Deduplicate literals by object identity.  Often a method will refer to a
	// class or a function multiple times.
	int literal_number;
	Object* value = IdentityDict_at(self->object_literals, literal);
	if (value)
		literal_number = Int_enforce(value, "Internal error: MethodBuilder object_literals");
	else {
		literal_number = MethodBuilder_add_literal(self, literal);
		IdentityDict_set_at(self->object_literals, literal, (Object*) new_Int(literal_number));
		}

	return MethodBuilder_emit_literal_by_num(self, literal_number);
}

int MethodBuilder_emit_string_literal(MethodBuilder* self, String* literal)
{
	// Deduplicate string literals.
	int literal_number;
	Object* value = Dict_at(self->string_literals, literal);
	if (value)
		literal_number = Int_enforce(value, "Internal error: MethodBuilder string_literals");
	else {
		// Copy the string.  It might be a slice of source file, and we don't want
		// to make the garbage collector hold on to the whole source file.
		String* literal_string = String_copy(literal);
		literal_number = MethodBuilder_add_literal(self, (Object*) literal_string);
		Dict_set_at(self->string_literals, literal_string, (Object*) new_Int(literal_number));
		}

	return MethodBuilder_emit_literal_by_num(self, literal_number);
}

int MethodBuilder_emit_literal_by_num(MethodBuilder* self, int literal_num)
{
	if (literal_num < -INT8_MIN)
		return -literal_num - 1;

	// Won't fit in seven bits, need to move it to a temporary local.
	int loc = MethodBuilder_reserve_locals(self, 1);
	MethodBuilder_add_bytecode(self, BC_GET_LITERAL);
	MethodBuilder_add_bytecode(self, literal_num >> 8);
	MethodBuilder_add_bytecode(self, literal_num & 0xFF);
	MethodBuilder_add_bytecode(self, loc);
	return loc;
}

int MethodBuilder_add_literal(MethodBuilder* self, struct Object* literal)
{
	Array_append(self->method->literals, literal);
	if (self->method->literals->size > UINT16_MAX)
		Error("Internal error: Too many literals.");
	return self->method->literals->size - 1;
}

int MethodBuilder_reserve_literal_for(MethodBuilder* self, Object* literal)
{
	int literal_number;
	Object* value = IdentityDict_at(self->object_literals, literal);
	if (value)
		literal_number = Int_enforce(value, "Internal error: MethodBuilder_reserve_literal_for");
	else {
		literal_number = MethodBuilder_add_literal(self, NULL);
		IdentityDict_set_at(self->object_literals, literal, (Object*) new_Int(literal_number));
		}
	return literal_number;
}

int MethodBuilder_get_reserved_literal_for(MethodBuilder* self, Object* literal)
{
	Object* value = IdentityDict_at(self->object_literals, literal);
	if (value == NULL)
		return -1;
	return Int_enforce(value, "Internal error: MethodBuilder_get_reserved_literal_for");
}

void MethodBuilder_set_literal(MethodBuilder* self, int literal, Object* value)
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
	if (offset < INT8_MIN || offset > INT8_MAX)
		Error("Internal error: Offset out-of-bounds!");
	ByteArray_append(self->method->bytecode, offset);
}


void MethodBuilder_patch_offset8(MethodBuilder* self, int patch_point)
{
	ByteArray* bytecode = self->method->bytecode;
	int offset = bytecode->size - patch_point - 1;
	if (offset < INT8_MIN || offset > INT8_MAX)
		Error("Internal error: Offset out-of-bounds!");
	ByteArray_set_at(bytecode, patch_point, offset);
}


int MethodBuilder_add_offset16(MethodBuilder* self)
{
	int patch_point = self->method->bytecode->size;
	ByteArray_append(self->method->bytecode, 0);
	ByteArray_append(self->method->bytecode, 0);
	return patch_point;
}


void MethodBuilder_add_back_offset16(MethodBuilder* self, int patch_point)
{
	int offset = patch_point - self->method->bytecode->size - 2;
	if (offset < INT16_MIN || offset > INT16_MAX)
		Error("Internal error: Offset out-of-bounds!");
	ByteArray_append(self->method->bytecode, offset >> 8);
	ByteArray_append(self->method->bytecode, offset & 0xFF);
}


void MethodBuilder_patch_offset16(MethodBuilder* self, int patch_point)
{
	ByteArray* bytecode = self->method->bytecode;
	int offset = bytecode->size - patch_point - 2;
	if (offset < INT16_MIN || offset > INT16_MAX)
		Error("Internal error: Offset out-of-bounds!");
	ByteArray_set_at(bytecode, patch_point, offset >> 8);
	ByteArray_set_at(bytecode, patch_point + 1, offset & 0xFF);
}


void MethodBuilder_patch_offset16_to(MethodBuilder* self, int patch_point, int dest_point)
{
	ByteArray* bytecode = self->method->bytecode;
	int offset = dest_point - patch_point - 2;
	if (offset < INT16_MIN || offset > INT16_MAX)
		Error("Internal error: Offset out-of-bounds!");
	ByteArray_set_at(bytecode, patch_point, offset >> 8);
	ByteArray_set_at(bytecode, patch_point + 1, offset & 0xFF);
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

	LoopPoints* loop_points = self->loop_points;
	for (int i = 0; i < loop_points->continue_patch_points->size; ++i) {
		size_t patch_point = (size_t) Array_at(loop_points->continue_patch_points, i);
		MethodBuilder_patch_offset16_to(self, patch_point, continue_point);
		}
	for (int i = 0; i < loop_points->break_patch_points->size; ++i) {
		size_t patch_point = (size_t) Array_at(loop_points->break_patch_points, i);
		MethodBuilder_patch_offset16_to(self, patch_point, break_point);
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


void MethodBuilder_add_continue_offset16(MethodBuilder* self)
{
	Array_append(
		self->loop_points->continue_patch_points,
		(Object*) (size_t) MethodBuilder_add_offset16(self));
}


void MethodBuilder_add_break_offset16(MethodBuilder* self)
{
	Array_append(
		self->loop_points->break_patch_points,
		(Object*) (size_t) MethodBuilder_add_offset16(self));
}



void MethodBuilder_push_unwind_point(MethodBuilder* self, ParseNode* node)
{
	Array_append(self->unwindings, (Object*) node);
}

void MethodBuilder_pop_unwind_point(MethodBuilder* self, ParseNode* node)
{
	ParseNode* popped_node = (ParseNode*) Array_pop_back(self->unwindings);
	if (popped_node != node)
		Error("Internal error: mismatch unwinding points.");
}

void MethodBuilder_unwind_all(MethodBuilder* self)
{
	for (int index = self->unwindings->size - 1; index >= 0; --index) {
		ParseNode* node = (ParseNode*) Array_at(self->unwindings, index);
		if (node->type == PN_WithStatement)
			WithStatement_emit_close((WithStatement*) node, self);
		}
}

void MethodBuilder_unwind_loop(MethodBuilder* self)
{
	for (int index = self->unwindings->size - 1; index >= 0; --index) {
		ParseNode* node = (ParseNode*) Array_at(self->unwindings, index);
		if (node->type == PN_WithStatement)
			WithStatement_emit_close((WithStatement*) node, self);
		else {
			// Anything else is a loop statement, so we're done unwinding the
			// innermost loop.
			break;
			}
		}
}



