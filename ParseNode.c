#include "ParseNode.h"
#include "MethodBuilder.h"
#include "ByteArray.h"
#include "Array.h"


int Block_emit_method(struct ParseNode* super, struct MethodBuilder* method)
{
	Block* self = (Block*) super;
	size_t size = self->statements->size;
	for (int i = 0; i < size; ++i) {
		ParseNode* statement = (ParseNode*) Array_at(self->statements, i);
		statement->emit_method(statement, method);
		}
	return -1;
}

Block* new_Block()
{
	Block* block = alloc_obj(Block);
	block->parse_node.emit_method = Block_emit_method;
	block->statements = new_Array();
	return block;
}

void Block_append(Block* self, ParseNode* statement)
{
	Array_append(self->statements, (Object*) statement);
}


int IfStatement_emit_method(ParseNode* super, MethodBuilder* method)
{
	IfStatement* self = (IfStatement*) super;
	int condition_reg = self->condition->emit_method(self->condition, method);
	/*** TODO ***/
}

IfStatement* new_IfStatement()
{
	IfStatement* if_statement = alloc_obj(IfStatement);
	if_statement->parse_node.emit_method = IfStatement_emit_method;
	return if_statement;
}


int WhileStatement_emit_method(ParseNode* super, MethodBuilder* method)
{
	WhileStatement* self = (WhileStatement*) super;
	/*** TODO ***/
}

WhileStatement* new_WhileStatement()
{
	WhileStatement* self = alloc_obj(WhileStatement);
	self->parse_node.emit_method = WhileStatement_emit_method;
	return self;
}


int ForStatement_emit_method(ParseNode* super, MethodBuilder* method)
{
	ForStatement* self = (ForStatement*) super;
	/*** TODO ***/
}

ForStatement* new_ForStatement()
{
	ForStatement* self = alloc_obj(ForStatement);
	self->parse_node.emit_method = ForStatement_emit_method;
	return self;
}



