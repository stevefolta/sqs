#include "ParseNode.h"
#include "MethodBuilder.h"
#include "Environment.h"
#include "ByteArray.h"
#include "Array.h"
#include "String.h"
#include "Dict.h"
#include "Object.h"
#include "ByteCode.h"
#include "Error.h"


void Block_resolve_names(struct ParseNode* super, struct MethodBuilder* method)
{
	Block* self = (Block*) super;

	size_t size = self->statements->size;
	for (int i = 0; i < size; ++i) {
		ParseNode* statement = (ParseNode*) Array_at(self->statements, i);
		if (statement->resolve_names)
			statement->resolve_names(statement, method);
		}
}

int Block_emit(struct ParseNode* super, struct MethodBuilder* method)
{
	// Push our context.
	Block* self = (Block*) super;
	BlockContext context;
	BlockContext_init(&context, self, method->environment);
	method->environment = &context.environment;

	Block_resolve_names(super, method);

	if (self->locals)
		self->locals_base = MethodBuilder_reserve_locals(method, self->locals->size);
	else
		self->locals_base = method->cur_num_variables;

	size_t size = self->statements->size;
	for (int i = 0; i < size; ++i) {
		ParseNode* statement = (ParseNode*) Array_at(self->statements, i);
		statement->emit(statement, method);
		}

	// Pop our context.
	method->environment = context.environment.parent;

	method->cur_num_variables = self->locals_base;
	return -1;
}

Block* new_Block()
{
	Block* block = alloc_obj(Block);
	block->parse_node.emit = Block_emit;
	block->parse_node.resolve_names = Block_resolve_names;
	block->statements = new_Array();
	return block;
}

void Block_append(Block* self, ParseNode* statement)
{
	Array_append(self->statements, (Object*) statement);
}

ParseNode* Block_get_local(Block* self, String* name)
{
	if (self->locals == NULL)
		return NULL;

	Local* local = (Local*) Dict_at(self->locals, name);
	return (ParseNode*) local;
}

ParseNode* Block_autodeclare(Block* self, String* name)
{
	if (self->locals == NULL)
		self->locals = new_Dict();

	Local* local = new_Local(self, self->locals->size);
	Dict_set_at(self->locals, name, (Object*) local);
	return (ParseNode*) local;
}


int IfStatement_emit(ParseNode* super, MethodBuilder* method)
{
	IfStatement* self = (IfStatement*) super;

	int orig_locals = method->cur_num_variables;
	if (self->condition->resolve_names)
		self->condition->resolve_names(self->condition, method);
	int condition_reg = self->condition->emit(self->condition, method);

	if (self->if_block) {
		// Branch if false.
		MethodBuilder_add_bytecode(method, BC_BRANCH_IF_FALSE);
		MethodBuilder_add_bytecode(method, condition_reg);
		int false_patch_point = MethodBuilder_add_offset8(method);
		method->cur_num_variables = orig_locals;

		self->if_block->emit(self->if_block, method);

		if (self->else_block) {
			// Finish "if" block".
			MethodBuilder_add_bytecode(method, BC_BRANCH);
			int end_patch_point = MethodBuilder_add_offset8(method);

			// "else" block.
			MethodBuilder_patch_offset8(method, false_patch_point);
			self->else_block->emit(self->else_block, method);

			// Finish.
			MethodBuilder_patch_offset8(method, end_patch_point);
			}
		else
			MethodBuilder_patch_offset8(method, false_patch_point);
		}

	else {
		// *Only* an "else" block!
		MethodBuilder_add_bytecode(method, BC_BRANCH_IF_TRUE);
		MethodBuilder_add_bytecode(method, condition_reg);
		int end_patch_point = MethodBuilder_add_offset8(method);
		method->cur_num_variables = orig_locals;

		self->else_block->emit(self->else_block, method);
		MethodBuilder_patch_offset8(method, end_patch_point);
		}

	return 0;
}

IfStatement* new_IfStatement()
{
	IfStatement* if_statement = alloc_obj(IfStatement);
	if_statement->parse_node.emit = IfStatement_emit;
	return if_statement;
}


int WhileStatement_emit(ParseNode* super, MethodBuilder* method)
{
	WhileStatement* self = (WhileStatement*) super;

	// Condition.
	if (self->condition->resolve_names)
		self->condition->resolve_names(self->condition, method);
	int orig_locals = method->cur_num_variables;
	int loop_point = MethodBuilder_get_offset(method);
	int condition_loc = self->condition->emit(self->condition, method);

	// Branch out if false.
	MethodBuilder_add_bytecode(method, BC_BRANCH_IF_FALSE);
	MethodBuilder_add_bytecode(method, condition_loc);
	int end_patch_point = MethodBuilder_add_offset8(method);
	method->cur_num_variables = orig_locals;

	// Body.
	if (self->body)
		self->body->emit(self->body, method);

	// Jump back to beginning.
	MethodBuilder_add_bytecode(method, BC_BRANCH);
	MethodBuilder_add_back_offset8(method, loop_point);

	// Finish.
	MethodBuilder_patch_offset8(method, end_patch_point);
	return 0;
}

WhileStatement* new_WhileStatement()
{
	WhileStatement* self = alloc_obj(WhileStatement);
	self->parse_node.emit = WhileStatement_emit;
	return self;
}


int ForStatement_emit(ParseNode* super, MethodBuilder* method)
{
	ForStatement* self = (ForStatement*) super;
	/*** TODO ***/
}

ForStatement* new_ForStatement()
{
	ForStatement* self = alloc_obj(ForStatement);
	self->parse_node.emit = ForStatement_emit;
	return self;
}


int SetExpr_emit(ParseNode* super, MethodBuilder* method)
{
	SetExpr* self = (SetExpr*) super;
	return self->left->emit_set(self->left, self->right, method);
}

void SetExpr_resolve_names(ParseNode* super, MethodBuilder* method)
{
	SetExpr* self = (SetExpr*) super;
	if (self->left->resolve_names)
		self->left->resolve_names(self->left, method);
	if (self->right->resolve_names)
		self->right->resolve_names(self->right, method);
}

SetExpr* new_SetExpr()
{
	SetExpr* self = alloc_obj(SetExpr);
	self->parse_node.emit = SetExpr_emit;
	self->parse_node.resolve_names = SetExpr_resolve_names;
	return self;
}


int ShortCircuitOrExpr_emit(ParseNode* super, MethodBuilder* method)
{
	ShortCircuitOrExpr* self = (ShortCircuitOrExpr*) super;
	/*** TODO ***/
}

ShortCircuitOrExpr* new_ShortCircutOrExpr(ParseNode* expr1, ParseNode* expr2)
{
	ShortCircuitOrExpr* self = alloc_obj(ShortCircuitOrExpr);
	self->parse_node.emit = ShortCircuitOrExpr_emit;
	self->expr1 = expr1;
	self->expr2 = expr2;
	return self;
}


int ShortCircuitAndExpr_emit(ParseNode* super, MethodBuilder* method)
{
	ShortCircuitAndExpr* self = (ShortCircuitAndExpr*) super;
	/*** TODO ***/
}

ShortCircuitAndExpr* new_ShortCircutAndExpr(ParseNode* expr1, ParseNode* expr2)
{
	ShortCircuitAndExpr* self = alloc_obj(ShortCircuitAndExpr);
	self->parse_node.emit = ShortCircuitAndExpr_emit;
	self->expr1 = expr1;
	self->expr2 = expr2;
	return self;
}


int StringLiteralExpr_emit(ParseNode* super, MethodBuilder* method)
{
	StringLiteralExpr* self = (StringLiteralExpr*) super;
	return -MethodBuilder_add_literal(method, (Object*) self->str) - 1;
}

StringLiteralExpr* new_StringLiteralExpr(struct String* str)
{
	StringLiteralExpr* self = alloc_obj(StringLiteralExpr);
	self->parse_node.emit = StringLiteralExpr_emit;
	self->str = str;
	return self;
}


int BooleanLiteral_emit(ParseNode* super, MethodBuilder* method)
{
	BooleanLiteral* self = (BooleanLiteral*) super;
	int slot = MethodBuilder_reserve_locals(method, 1);
	MethodBuilder_add_bytecode(method, self->value ? BC_TRUE : BC_FALSE);
	MethodBuilder_add_bytecode(method, slot);
	return slot;
}

BooleanLiteral* new_BooleanLiteral(bool value)
{
	BooleanLiteral* self = alloc_obj(BooleanLiteral);
	self->parse_node.emit = BooleanLiteral_emit;
	self->value = value;
	return self;
}


int NilLiteral_emit(ParseNode* self, MethodBuilder* method)
{
	int slot = MethodBuilder_reserve_locals(method, 1);
	MethodBuilder_add_bytecode(method, BC_NIL);
	MethodBuilder_add_bytecode(method, slot);
	return slot;
}

ParseNode* new_NilLiteral()
{
	ParseNode* self = alloc_obj(ParseNode);
	self->emit = NilLiteral_emit;
	return self;
}


int GlobalExpr_emit(ParseNode* super, MethodBuilder* method)
{
	GlobalExpr* self = (GlobalExpr*) super;
	int name_literal = MethodBuilder_add_literal(method, (Object*) self->name);
	return name_literal;
}

GlobalExpr* new_GlobalExpr(struct String* name)
{
	GlobalExpr* self = alloc_obj(GlobalExpr);
	self->parse_node.emit = GlobalExpr_emit;
	self->name = name;
	return self;
}


int Variable_emit(ParseNode* super, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	if (self->resolved == NULL)
		Error("Unresolved variable \"%s\".", String_c_str(self->name));
	return self->resolved->emit(self->resolved, method);
}

int Variable_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	if (self->resolved == NULL)
		Error("Unresolved variable \"%s\".", String_c_str(self->name));
	return self->resolved->emit_set(self->resolved, value, method);
}

void Variable_resolve_names(ParseNode* super, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	self->resolved = method->environment->find_autodeclaring(method->environment, self->name);
	if (!self->resolved)
		Error("Couldn't find name: \"%s\".\n", self->name);
}

Variable* new_Variable(struct String* name)
{
	Variable* self = alloc_obj(Variable);
	self->parse_node.emit = Variable_emit;
	self->parse_node.emit_set = Variable_emit_set;
	self->parse_node.resolve_names = Variable_resolve_names;
	self->name = name;
	return self;
}


int Local_emit(ParseNode* super, MethodBuilder* method)
{
	Local* self = (Local*) super;
	return self->block->locals_base + self->block_index;
}

int Local_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* method)
{
	Local* self = (Local*) super;

	int value_loc = value->emit(value, method);
	MethodBuilder_add_bytecode(method, BC_SET_LOCAL);
	MethodBuilder_add_bytecode(method, value_loc);
	MethodBuilder_add_bytecode(method, self->block->locals_base + self->block_index);

	return value_loc;
}

Local* new_Local(Block* block, int block_index)
{
	Local* self = alloc_obj(Local);
	self->parse_node.emit = Local_emit;
	self->parse_node.emit_set = Local_emit_set;
	self->block = block;
	self->block_index = block_index;
	return self;
}



