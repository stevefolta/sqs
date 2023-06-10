#include "Upvalues.h"
#include "Method.h"
#include "MethodBuilder.h"
#include "ByteCode.h"
#include "Object.h"
#include "Memory.h"


int UpvalueLocal_emit(ParseNode* super, MethodBuilder* method)
{
	UpvalueLocal* self = (UpvalueLocal*) super;

	int loc = MethodBuilder_reserve_locals(method, 1);
	int literal_loc = MethodBuilder_emit_literal(method, (Object*) self->method);

	MethodBuilder_add_bytecode(method, BC_GET_UPVAL);
	MethodBuilder_add_bytecode(method, literal_loc);
	MethodBuilder_add_bytecode(method, self->local_index);
	MethodBuilder_add_bytecode(method, loc);

	method->cur_num_variables = loc + 1;
	return loc;
}

int UpvalueLocal_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* method)
{
	UpvalueLocal* self = (UpvalueLocal*) super;

	int value_loc = value->emit(value, method);
	int orig_locals = method->cur_num_variables;

	int literal_loc = MethodBuilder_emit_literal(method, (Object*) self->method);

	MethodBuilder_add_bytecode(method, BC_SET_UPVAL);
	MethodBuilder_add_bytecode(method, literal_loc);
	MethodBuilder_add_bytecode(method, self->local_index);
	MethodBuilder_add_bytecode(method, value_loc);

	method->cur_num_variables = orig_locals;
	return value_loc;
}

UpvalueLocal* new_UpvalueLocal(Method* method, int local_index)
{
	UpvalueLocal* self = alloc_obj(UpvalueLocal);
	self->parse_node.emit = UpvalueLocal_emit;
	self->parse_node.emit_set = UpvalueLocal_emit_set;
	self->method = method;
	self->local_index = local_index;
	return self;
}


