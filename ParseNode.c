#include "ParseNode.h"
#include "MethodBuilder.h"
#include "Environment.h"
#include "ByteArray.h"
#include "Array.h"
#include "String.h"
#include "Dict.h"
#include "Object.h"
#include "Int.h"
#include "ByteCode.h"
#include "Error.h"
#include <stdlib.h>


int Block_emit(struct ParseNode* super, struct MethodBuilder* method)
{
	Block* self = (Block*) super;

	// Before we do anything, compile our functions and classes and get them into
	// the literals.
	if (self->functions) {
		DictIterator* it = new_DictIterator(self->functions);
		while (true) {
			DictIteratorResult kv = DictIterator_next(it);
			if (kv.key == NULL)
				break;
			FunctionStatement* function = (FunctionStatement*) kv.value;
			Object* compiled_method = FunctionStatement_compile(function);
			function->loc = -MethodBuilder_add_literal(method, compiled_method) - 1;
			}
		}

	// Push our context.
	BlockContext context;
	BlockContext_init(&context, self, method->environment);
	method->environment = &context.environment;

	// First, resolve names, so our locals get autodeclared.
	// This is a per-block operation.  The ParseNodes don't need to descend into
	// their subblocks, if they have them.
	size_t size = self->statements->size;
	for (int i = 0; i < size; ++i) {
		ParseNode* statement = (ParseNode*) Array_at(self->statements, i);
		if (statement->resolve_names)
			statement->resolve_names(statement, method);
		}

	// Allocate locals on the stack.
	if (self->locals)
		self->locals_base = MethodBuilder_reserve_locals(method, self->locals->size);
	else
		self->locals_base = method->cur_num_variables;

	// Emit.
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

ParseNode* Block_get_function(Block* self, struct String* name)
{
	if (self->functions == NULL)
		return NULL;
	
	FunctionStatement* function = (FunctionStatement*) Dict_at(self->functions, name);
	if (function)
		return (ParseNode*) new_RawLoc(function->loc);
	return NULL;
}

ParseNode* Block_autodeclare(Block* self, String* name)
{
	if (self->locals == NULL)
		self->locals = new_Dict();

	Local* local = new_Local(self, self->locals->size);
	Dict_set_at(self->locals, name, (Object*) local);
	return (ParseNode*) local;
}

void Block_add_function(Block* self, struct FunctionStatement* function)
{
	if (self->functions == NULL)
		self->functions = new_Dict();
	Dict_set_at(self->functions, function->name, (Object*) function);
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

void IfStatement_resolve_names(ParseNode* super, MethodBuilder* method)
{
	IfStatement* self = (IfStatement*) super;
	if (self->condition->resolve_names)
		self->condition->resolve_names(self->condition, method);
	// The "else_block" could also be an IfStatement.
	if (self->else_block && self->else_block->type == PN_IfStatement)
		self->else_block->resolve_names(self->else_block, method);
}

IfStatement* new_IfStatement()
{
	IfStatement* if_statement = alloc_obj(IfStatement);
	if_statement->parse_node.type = PN_IfStatement;
	if_statement->parse_node.emit = IfStatement_emit;
	if_statement->parse_node.resolve_names = IfStatement_resolve_names;
	return if_statement;
}


int WhileStatement_emit(ParseNode* super, MethodBuilder* method)
{
	WhileStatement* self = (WhileStatement*) super;

	// Start the loop.
	int loop_point = MethodBuilder_get_offset(method);
	MethodBuilder_push_loop_points(method);

	// Condition.
	int orig_locals = method->cur_num_variables;
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
	MethodBuilder_pop_loop_points(method, loop_point, MethodBuilder_get_offset(method));
	return 0;
}

void WhileStatement_resolve_names(ParseNode* super, MethodBuilder* method)
{
	WhileStatement* self = (WhileStatement*) super;
	if (self->condition->resolve_names)
		self->condition->resolve_names(self->condition, method);
}

WhileStatement* new_WhileStatement()
{
	WhileStatement* self = alloc_obj(WhileStatement);
	self->parse_node.emit = WhileStatement_emit;
	self->parse_node.resolve_names = WhileStatement_resolve_names;
	return self;
}


int emit_call(int receiver_loc, char* name, int num_args, int* args_locs, MethodBuilder* method)
{
	// Allocate stack space for the new frame.
	int orig_locals =
		MethodBuilder_reserve_locals(
			method,
			frame_saved_area_size + 1 /* receiver's "self" */ + num_args);
	int args_start = orig_locals + frame_saved_area_size;

	// Emit receiver and args, and put them into the new frame's arguments.
	MethodBuilder_add_move(method, receiver_loc, args_start);
	for (int i = 0; i < num_args; ++i)
		MethodBuilder_add_move(method, args_locs[i], args_start + i + 1);

	// Emit the call itself.
	MethodBuilder_add_bytecode(method, BC_CALL_0 + num_args);
	int name_literal = MethodBuilder_add_literal(method, (Object*) new_c_String(name));
	MethodBuilder_add_bytecode(method, -name_literal - 1);
	MethodBuilder_add_bytecode(method, args_start);

	method->cur_num_variables = orig_locals + 1;
	return orig_locals;
}

int ForStatement_emit(ParseNode* super, MethodBuilder* method)
{
	ForStatement* self = (ForStatement*) super;

	int result_loc = MethodBuilder_reserve_locals(method, 1);

	// Collection.
	int collection_loc = self->collection->emit(self->collection, method);

	// "iterator" call.
	int iterator_loc = emit_call(collection_loc, "iterator", 0, NULL, method);

	// Start the loop.
	MethodBuilder_push_loop_points(method);
	int loop_point = MethodBuilder_get_offset(method);

	// "next" call.
	int value_loc = emit_call(iterator_loc, "next", 0, NULL, method);

	// Context.
	ForStatementContext context;
	ForStatementContext_init(&context, self->variable_name, value_loc);
	MethodBuilder_push_environment(method, &context.environment);

	// Test.
	MethodBuilder_add_bytecode(method, BC_BRANCH_IF_NIL);
	MethodBuilder_add_bytecode(method, value_loc);
	int end_patch_point = MethodBuilder_add_offset8(method);
	method->cur_num_variables = value_loc + 1;

	// Body.
	if (self->body)
		self->body->emit(self->body, method);

	// Loop back.
	MethodBuilder_add_bytecode(method, BC_BRANCH);
	MethodBuilder_add_back_offset8(method, loop_point);

	MethodBuilder_patch_offset8(method, end_patch_point);
	MethodBuilder_pop_loop_points(method, loop_point, MethodBuilder_get_offset(method));
	MethodBuilder_pop_environment(method);
	method->cur_num_variables = result_loc + 1;
	return result_loc;
}

void ForStatement_resolve_names(ParseNode* super, MethodBuilder* method)
{
	ForStatement* self = (ForStatement*) super;
	if (self->collection->resolve_names)
		self->collection->resolve_names(self->collection, method);
}

ForStatement* new_ForStatement()
{
	ForStatement* self = alloc_obj(ForStatement);
	self->parse_node.emit = ForStatement_emit;
	self->parse_node.resolve_names = ForStatement_resolve_names;
	return self;
}


int ContinueStatement_emit(ParseNode* self, MethodBuilder* method)
{
	MethodBuilder_add_bytecode(method, BC_BRANCH);
	MethodBuilder_add_continue_offset8(method);
	return 0;
}

ParseNode* new_ContinueStatement()
{
	ParseNode* self = alloc_obj(ParseNode);
	self->emit = ContinueStatement_emit;
	return self;
}


int BreakStatement_emit(ParseNode* self, MethodBuilder* method)
{
	MethodBuilder_add_bytecode(method, BC_BRANCH);
	MethodBuilder_add_break_offset8(method);
	return 0;
}

ParseNode* new_BreakStatement()
{
	ParseNode* self = alloc_obj(ParseNode);
	self->emit = BreakStatement_emit;
	return self;
}


int FunctionStatement_emit(ParseNode* super, MethodBuilder* method)
{
	// Nothing to do here; everything was taken care of at the start of the
	// enclosing Block.
	return 0;
}

FunctionStatement* new_FunctionStatement(struct String* name)
{
	FunctionStatement* self = alloc_obj(FunctionStatement);
	self->parse_node.emit = FunctionStatement_emit;
	self->name = name;
	self->arguments = new_Array();
	return self;
}

void FunctionStatement_add_argument(FunctionStatement* self, String* name)
{
	Array_append(self->arguments, (Object*) name);
}

Object* FunctionStatement_compile(FunctionStatement* self)
{
	MethodBuilder* builder = new_MethodBuilder(self->arguments->size);
	if (self->body)
		self->body->emit(self->body, builder);
	MethodBuilder_finish(builder);
	return (Object*) builder->method;
}


int ExpressionStatement_emit(ParseNode* super, MethodBuilder* method)
{
	ExpressionStatement* self = (ExpressionStatement*) super;
	int orig_locals = method->cur_num_variables;
	int result = self->expression->emit(self->expression, method);
	method->cur_num_variables = orig_locals;
	return result;
}

void ExpressionStatement_resolve_names(ParseNode* super, MethodBuilder* method)
{
	ExpressionStatement* self = (ExpressionStatement*) super;
	if (self->expression->resolve_names)
		self->expression->resolve_names(self->expression, method);
}

ExpressionStatement* new_ExpressionStatement(ParseNode* expression)
{
	ExpressionStatement* self = alloc_obj(ExpressionStatement);
	self->parse_node.emit = ExpressionStatement_emit;
	self->parse_node.resolve_names = ExpressionStatement_resolve_names;
	self->expression = expression;
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
	if (self->left->resolve_names_autodeclaring)
		self->left->resolve_names_autodeclaring(self->left, method);
	else if (self->left->resolve_names)
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

void ShortCircuitExpr_resolve_names(ParseNode* super, MethodBuilder* method)
{
	ShortCircuitExpr* self = (ShortCircuitExpr*) super;
	if (self->expr1->resolve_names)
		self->expr1->resolve_names(self->expr1, method);
	if (self->expr2->resolve_names)
		self->expr2->resolve_names(self->expr2, method);
}

ShortCircuitExpr* new_ShortCircuitExpr(ParseNode* expr1, ParseNode* expr2, bool is_and)
{
	ShortCircuitExpr* self = alloc_obj(ShortCircuitExpr);
	self->parse_node.emit = ShortCircuitExpr_emit;
	self->parse_node.resolve_names = ShortCircuitExpr_resolve_names;
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


int InterpolatedStringLiteral_emit(ParseNode* super, MethodBuilder* method)
{
	InterpolatedStringLiteral* self = (InterpolatedStringLiteral*) super;

	int result_loc = MethodBuilder_reserve_locals(method, 2);
	int array_loc = result_loc + 1;

	// Create array.
	MethodBuilder_add_bytecode(method, BC_NEW_ARRAY);
	MethodBuilder_add_bytecode(method, array_loc);

	// Add components.
	for (int i = 0; i < self->components->size; ++i) {
		ParseNode* component = (ParseNode*) Array_at(self->components, i);
		int component_loc = component->emit(component, method);
		MethodBuilder_add_bytecode(method, BC_ARRAY_APPEND);
		MethodBuilder_add_bytecode(method, array_loc);
		MethodBuilder_add_bytecode(method, component_loc);
		method->cur_num_variables = array_loc + 1;
		}

	// Join.
	MethodBuilder_add_bytecode(method, BC_ARRAY_JOIN);
	MethodBuilder_add_bytecode(method, array_loc);
	MethodBuilder_add_bytecode(method, result_loc);

	method->cur_num_variables = result_loc + 1;
	return result_loc;
}

void InterpolatedStringLiteral_resolve_names(ParseNode* super, MethodBuilder* method)
{
	InterpolatedStringLiteral* self = (InterpolatedStringLiteral*) super;
	for (int i = 0; i < self->components->size; ++i) {
		ParseNode* component = (ParseNode*) Array_at(self->components, i);
		if (component->resolve_names)
			component->resolve_names(component, method);
		}
}

InterpolatedStringLiteral* new_InterpolatedStringLiteral(struct Array* components)
{
	InterpolatedStringLiteral* self = alloc_obj(InterpolatedStringLiteral);
	self->parse_node.emit = InterpolatedStringLiteral_emit;
	self->parse_node.resolve_names = InterpolatedStringLiteral_resolve_names;
	self->components = components;
	return self;
}


int IntLiteralExpr_emit(ParseNode* super, MethodBuilder* method)
{
	IntLiteralExpr* self = (IntLiteralExpr*) super;
	return -MethodBuilder_add_literal(method, (Object*) new_Int(self->value)) - 1;
}

IntLiteralExpr* new_IntLiteralExpr(String* value_str)
{
	IntLiteralExpr* self = alloc_obj(IntLiteralExpr);
	self->parse_node.emit = IntLiteralExpr_emit;
	self->value = strtol(String_c_str(value_str), NULL, 0);
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
	int name_literal = MethodBuilder_add_literal(method, self->object);
	return -name_literal - 1;
}

GlobalExpr* new_GlobalExpr(struct Object* object)
{
	GlobalExpr* self = alloc_obj(GlobalExpr);
	self->parse_node.emit = GlobalExpr_emit;
	self->object = object;
	return self;
}


void Variable_resolve_names(ParseNode* super, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	if (self->resolved == NULL) {
		self->resolved = method->environment->find(method->environment, self->name);
		if (!self->resolved)
			Error("Couldn't find name \"%s\" on line %d.", String_c_str(self->name), self->line_number);
		}
}

void Variable_resolve_names_autodeclaring(ParseNode* super, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	if (self->resolved == NULL) {
		self->resolved = method->environment->find_autodeclaring(method->environment, self->name);
		if (!self->resolved) {
			// Probably won't happen, because of the autodeclaration.
			Error("Couldn't find name \"%s\" on line %d.", String_c_str(self->name), self->line_number);
			}
		}
}

int Variable_emit(ParseNode* super, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	return self->resolved->emit(self->resolved, method);
}

int Variable_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* method)
{
	Variable* self = (Variable*) super;
	return self->resolved->emit_set(self->resolved, value, method);
}

Variable* new_Variable(struct String* name, int line_number)
{
	Variable* self = alloc_obj(Variable);
	self->parse_node.emit = Variable_emit;
	self->parse_node.emit_set = Variable_emit_set;
	self->parse_node.resolve_names = Variable_resolve_names;
	self->parse_node.resolve_names_autodeclaring = Variable_resolve_names_autodeclaring;
	self->name = name;
	self->line_number = line_number;
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


int RawLoc_emit(ParseNode* super, MethodBuilder* method)
{
	RawLoc* self = (RawLoc*) super;
	return self->loc;
}

RawLoc* new_RawLoc(int loc)
{
	RawLoc* self = alloc_obj(RawLoc);
	self->parse_node.emit = RawLoc_emit;
	self->loc = loc;
	return self;
}


int ArrayLiteral_emit(ParseNode* super, MethodBuilder* method)
{
	ArrayLiteral* self = (ArrayLiteral*) super;

	int array_loc = MethodBuilder_reserve_locals(method, 1);
	MethodBuilder_add_bytecode(method, BC_NEW_ARRAY);
	MethodBuilder_add_bytecode(method, array_loc);

	for (int i = 0; i < self->items->size; ++i) {
		ParseNode* item = (ParseNode*) Array_at(self->items, i);
		int item_loc = item->emit(item, method);
		MethodBuilder_add_bytecode(method, BC_ARRAY_APPEND);
		MethodBuilder_add_bytecode(method, array_loc);
		MethodBuilder_add_bytecode(method, item_loc);
		method->cur_num_variables = array_loc + 1;
		}

	return array_loc;
}

void ArrayLiteral_resolve_names(ParseNode* super, MethodBuilder* method)
{
	ArrayLiteral* self = (ArrayLiteral*) super;

	for (int i = 0; i < self->items->size; ++i) {
		ParseNode* item = (ParseNode*) Array_at(self->items, i);
		if (item->resolve_names)
			item->resolve_names(item, method);
		}
}

ArrayLiteral* new_ArrayLiteral()
{
	ArrayLiteral* self = alloc_obj(ArrayLiteral);
	self->parse_node.emit = ArrayLiteral_emit;
	self->parse_node.resolve_names = ArrayLiteral_resolve_names;
	self->items = new_Array();
	return self;
}

void ArrayLiteral_add_item(ArrayLiteral* self, ParseNode* item)
{
	Array_append(self->items, (Object*) item);
}



int DictLiteral_emit(ParseNode* super, MethodBuilder* method)
{
	DictLiteral* self = (DictLiteral*) super;

	// Create the dict.
	int dict_loc = MethodBuilder_reserve_locals(method, 1);
	MethodBuilder_add_bytecode(method, BC_NEW_DICT);
	MethodBuilder_add_bytecode(method, dict_loc);

	// Add the values.
	DictIterator* it = new_DictIterator(self->items);
	while (true) {
		DictIteratorResult item = DictIterator_next(it);
		if (item.key == NULL)
			break;
		ParseNode* value = (ParseNode*) item.value;

		int value_loc = value->emit(value, method);

		MethodBuilder_add_bytecode(method, BC_DICT_ADD);
		MethodBuilder_add_bytecode(method, dict_loc);
		int name_literal = MethodBuilder_add_literal(method, (Object*) item.key);
		MethodBuilder_add_bytecode(method, -name_literal - 1);
		MethodBuilder_add_bytecode(method, value_loc);
		}

	return dict_loc;
}

void DictLiteral_resolve_names(ParseNode* super, MethodBuilder* method)
{
	DictLiteral* self = (DictLiteral*) super;

	DictIterator* it = new_DictIterator(self->items);
	while (true) {
		DictIteratorResult item = DictIterator_next(it);
		if (item.key == NULL)
			break;
		ParseNode* value = (ParseNode*) item.value;
		if (value->resolve_names)
			value->resolve_names(value, method);
		}
}

DictLiteral* new_DictLiteral()
{
	DictLiteral* self = alloc_obj(DictLiteral);
	self->parse_node.emit = DictLiteral_emit;
	self->parse_node.resolve_names = DictLiteral_resolve_names;
	self->items = new_Dict();
	return self;
}


void DictLiteral_add_item(DictLiteral* self, String* key, ParseNode* value)
{
	Dict_set_at(self->items, key, (Object*) value);
}



int CallExpr_emit(ParseNode* super, MethodBuilder* method)
{
	CallExpr* self = (CallExpr*) super;

	int num_args = self->arguments->size;
	if (num_args > 15)
		Error("Too many arguments in call to \"%s\".", String_c_str(self->name));

	// Allocate stack space for the new frame.
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

int CallExpr_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* method)
{
	CallExpr* self = (CallExpr*) super;

	// Make a copy, turn it into the setter, and emit from that.
	CallExpr setter = *self;
	String equals_string;
	String_init_static_c(&equals_string, "=");
	setter.name = String_add(setter.name, &equals_string);
	setter.arguments = Array_copy(setter.arguments);
	CallExpr_add_argument(&setter, value);
	return CallExpr_emit((ParseNode*) &setter, method);
}

void CallExpr_resolve_names(ParseNode* super, MethodBuilder* method)
{
	CallExpr* self = (CallExpr*) super;
	if (self->receiver->resolve_names)
		self->receiver->resolve_names(self->receiver, method);
	int num_args = self->arguments->size;
	for (int i = 0; i < num_args; ++i) {
		ParseNode* arg = (ParseNode*) Array_at(self->arguments, i);
		if (arg->resolve_names)
			arg->resolve_names(arg, method);
		}
}

CallExpr* new_CallExpr(ParseNode* receiver, String* name)
{
	CallExpr* self = alloc_obj(CallExpr);
	self->parse_node.emit = CallExpr_emit;
	self->parse_node.emit_set = CallExpr_emit_set;
	self->parse_node.resolve_names = CallExpr_resolve_names;
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


int FunctionCallExpr_emit(ParseNode* super, MethodBuilder* method)
{
	FunctionCallExpr* self = (FunctionCallExpr*) super;

	// Allocate stack space for the new frame.
	int num_args = self->arguments->size;
	int orig_locals =
		MethodBuilder_reserve_locals(
			method,
			frame_saved_area_size + 1 /* receiver's "self" */ + num_args);
	int args_start = orig_locals + frame_saved_area_size;

	// Emit the function.
	int fn_loc = self->fn->emit(self->fn, method);

	// Set up the new frame's arguments.
	// "receiver" is nil.
	MethodBuilder_add_bytecode(method, BC_NIL);
	MethodBuilder_add_bytecode(method, args_start);
	for (int i = 0; i < num_args; ++i) {
		ParseNode* arg = (ParseNode*) Array_at(self->arguments, i);
		int arg_loc = arg->emit(arg, method);
		MethodBuilder_add_move(method, arg_loc, args_start + i + 1);
		}

	// Emit the function call itself.
	MethodBuilder_add_bytecode(method, BC_FN_CALL);
	MethodBuilder_add_bytecode(method, fn_loc);
	MethodBuilder_add_bytecode(method, num_args);
	MethodBuilder_add_bytecode(method, args_start);

	method->cur_num_variables = orig_locals + 1;
	return orig_locals;
}

void FunctionCallExpr_resolve_names(ParseNode* super, MethodBuilder* method)
{
	FunctionCallExpr* self = (FunctionCallExpr*) super;
	if (self->fn->resolve_names)
		self->fn->resolve_names(self->fn, method);
	int num_args = self->arguments->size;
	for (int i = 0; i < num_args; ++i) {
		ParseNode* arg = (ParseNode*) Array_at(self->arguments, i);
		if (arg->resolve_names)
			arg->resolve_names(arg, method);
		}
}

FunctionCallExpr* new_FunctionCallExpr(ParseNode* fn, struct Array* arguments)
{
	FunctionCallExpr* self = alloc_obj(FunctionCallExpr);
	self->parse_node.emit = FunctionCallExpr_emit;
	self->parse_node.resolve_names = FunctionCallExpr_resolve_names;
	self->fn = fn;
	self->arguments = arguments;
	return self;
}



