#include "ParseNode.h"
#include "MethodBuilder.h"
#include "ByteArray.h"
#include "Array.h"
#include "Object.h"


int Block_emit(struct ParseNode* super, struct MethodBuilder* method)
{
	Block* self = (Block*) super;
	size_t size = self->statements->size;
	for (int i = 0; i < size; ++i) {
		ParseNode* statement = (ParseNode*) Array_at(self->statements, i);
		statement->emit(statement, method);
		}
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


int IfStatement_emit(ParseNode* super, MethodBuilder* method)
{
	IfStatement* self = (IfStatement*) super;
	int condition_reg = self->condition->emit(self->condition, method);
	/*** TODO ***/
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
	/*** TODO ***/
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

SetExpr* new_SetExpr()
{
	SetExpr* self = alloc_obj(SetExpr);
	self->parse_node.emit = SetExpr_emit;
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


int GlobalExpr_emit(ParseNode* super, MethodBuilder* method)
{
	GlobalExpr* self = (GlobalExpr*) super;
	int name_literal = MethodBuilder_add_literal(method, (Object*) self->name);
}

GlobalExpr* new_GlobalExpr(struct String* name)
{
	GlobalExpr* self = alloc_obj(GlobalExpr);
	self->parse_node.emit = GlobalExpr_emit;
	self->name = name;
	return self;
}



