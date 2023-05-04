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


int Block_emit(struct ParseNode* super, struct MethodBuilder* method)
{
	// Push our context.
	Block* self = (Block*) super;
	BlockContext context;
	BlockContext_init(&context, self, method->environment);
	method->environment = &context.environment;

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


int ExpressionStatement_emit(ParseNode* super, MethodBuilder* method)
{
	ExpressionStatement* self = (ExpressionStatement*) super;
	int orig_locals = method->cur_num_variables;
	int result = self->expression->emit(self->expression, method);
	method->cur_num_variables = orig_locals;
	return result;
}

ExpressionStatement* new_ExpressionStatement(ParseNode* expression)
{
	ExpressionStatement* self = alloc_obj(ExpressionStatement);
	self->parse_node.emit = ExpressionStatement_emit;
	self->expression = expression;
	return self;
}


int SetExpr_emit(ParseNode* super, MethodBuilder* method)
{
	SetExpr* self = (SetExpr*) super;
	return self->left->emit_set(self->left, self->right, method);
}

SetExpr* new_SetExpr()
{
	SetExpr* self = alloc_obj(SetExpr);
	self->parse_node.emit = SetExpr_emit;
	return self;
}


int ShortCircuitExpr_emit(ParseNode* super, MethodBuilder* method)
{
	ShortCircuitExpr* self = (ShortCircuitExpr*) super;

	int result_slot = MethodBuilder_reserve_locals(method, 1);
	int orig_locals = method->cur_num_variables;

	// expr1
	int expr_loc = self->expr1->emit(self->expr1, method);
	MethodBuilder_add_bytecode(method, BC_SET_LOCAL);
	MethodBuilder_add_bytecode(method, expr_loc);
	MethodBuilder_add_bytecode(method, result_slot);
	method->cur_num_variables = orig_locals;

	// Test.
	MethodBuilder_add_bytecode(method, (self->is_and ? BC_BRANCH_IF_FALSE : BC_BRANCH_IF_TRUE));
	MethodBuilder_add_bytecode(method, result_slot);
	int patch_point = MethodBuilder_add_offset8(method);

	// expr2
	expr_loc = self->expr2->emit(self->expr2, method);
	MethodBuilder_add_bytecode(method, BC_SET_LOCAL);
	MethodBuilder_add_bytecode(method, expr_loc);
	MethodBuilder_add_bytecode(method, result_slot);
	method->cur_num_variables = orig_locals;

	// Finish.
	MethodBuilder_patch_offset8(method, patch_point);
	return 0;
}

ShortCircuitExpr* new_ShortCircuitExpr(ParseNode* expr1, ParseNode* expr2, bool is_and)
{
	ShortCircuitExpr* self = alloc_obj(ShortCircuitExpr);
	self->parse_node.emit = ShortCircuitExpr_emit;
	self->expr1 = expr1;
	self->expr2 = expr2;
	self->is_and = is_and;
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


void Variable_resolve_names(ParseNode* super, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	self->resolved = method->environment->find_autodeclaring(method->environment, self->name);
	if (!self->resolved)
		Error("Couldn't find name: \"%s\".\n", self->name);
}

int Variable_emit(ParseNode* super, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	if (self->resolved == NULL)
		Variable_resolve_names(super, method);
	return self->resolved->emit(self->resolved, method);
}

int Variable_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	if (self->resolved == NULL)
		Variable_resolve_names(super, method);
	return self->resolved->emit_set(self->resolved, value, method);
}

Variable* new_Variable(struct String* name)
{
	Variable* self = alloc_obj(Variable);
	self->parse_node.emit = Variable_emit;
	self->parse_node.emit_set = Variable_emit_set;
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



int CallExpr_emit(ParseNode* super, MethodBuilder* method)
{
	CallExpr* self = (CallExpr*) super;

	int num_args = self->arguments->size;
	if (num_args > 15)
		Error("Too many arguments in call to \"%s\".", String_c_str(self->name));

	int orig_locals =
		MethodBuilder_reserve_locals(
			method,
			frame_saved_area_size + 1 /* receiver's "self" */ + num_args);
	int args_start = orig_locals + frame_saved_area_size;

	// Emit receiver and args, and put them into the new frame's arguments.
	int receiver_loc = self->receiver->emit(self->receiver, method);
	MethodBuilder_add_move(method, receiver_loc, args_start);
	for (int i = 0; i < num_args; ++i) {
		ParseNode* arg = (ParseNode*) Array_at(self->arguments, i);
		int arg_loc = arg->emit(arg, method);
		MethodBuilder_add_move(method, arg_loc, args_start + i + 1);
		}

	// Emit the call itself.
	MethodBuilder_add_bytecode(method, BC_CALL_0 + num_args);
	int name_literal = MethodBuilder_add_literal(method, (Object*) self->name);
	MethodBuilder_add_bytecode(method, -name_literal - 1);
	MethodBuilder_add_bytecode(method, args_start);

	method->cur_num_variables = orig_locals + 1;
	return orig_locals;
}

CallExpr* new_CallExpr(ParseNode* receiver, String* name)
{
	CallExpr* self = alloc_obj(CallExpr);
	self->parse_node.emit = CallExpr_emit;
	self->receiver = receiver;
	self->name = name;
	self->arguments = new_Array();
	return self;
}

CallExpr* new_CallExpr_binop(ParseNode* receiver, ParseNode* arg, String* name)
{
	CallExpr* self = new_CallExpr(receiver, name);
	CallExpr_add_argument(self, arg);
	return self;
}

void CallExpr_add_argument(CallExpr* self, ParseNode* arg)
{
	Array_append(self->arguments, (Object*) arg);
}



