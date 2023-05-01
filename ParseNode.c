#include "ParseNode.h"
#include "ByteArray.h"
#include "Array.h"


int Block_emit_method(struct ParseNode* super, struct ByteArray* bytecode)
{
	Block* self = (Block*) super;
	size_t size = self->statements->size;
	for (int i = 0; i < size; ++i) {
		ParseNode* statement = (ParseNode*) Array_at(self->statements, i);
		statement->emit_method(statement, bytecode);
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


int IfStatement_emit_method(struct ParseNode* super, struct ByteArray* bytecode)
{
	IfStatement* self = (IfStatement*) super;
	int condition_reg = self->condition->emit_method(self->condition, bytecode);
	/*** TODO ***/
}

IfStatement* new_IfStatement()
{
	IfStatement* if_statement = alloc_obj(IfStatement);
	if_statement->parse_node.emit_method = IfStatement_emit_method;
	return if_statement;
}



