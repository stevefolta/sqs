#include "Environment.h"
#include "ParseNode.h"
#include "String.h"
#include "Dict.h"
#include "Memory.h"


GlobalEnvironment global_environment;

ParseNode* GlobalEnvironment_find(Environment* super, String* name)
{
	GlobalEnvironment* self = (GlobalEnvironment*) super;
	// Replace "name" with the same string the Dict is using, to save memory.
	name = Dict_key_at(self->dict, name);
	if (name == NULL)
		return NULL;
	return (ParseNode*) new_GlobalExpr(name);
}

void GlobalEnvironment_init()
{
	global_environment.environment.find = GlobalEnvironment_find;
	global_environment.environment.find_autodeclaring = GlobalEnvironment_find;
	global_environment.dict = new_Dict();
}

void GlobalEnvironment_add(struct String* name, struct Object* value)
{
	Dict_set_at(global_environment.dict, name, value);
}



ParseNode* BlockContext_find(Environment* super, String* name)
{
	BlockContext* self = (BlockContext*) super;

	ParseNode* expr = Block_get_local(self->block, name);
	if (expr)
		return expr;

	return self->environment.parent->find(self->environment.parent, name);
}

ParseNode* BlockContext_find_autodeclaring(Environment* super, String* name)
{
	BlockContext* self = (BlockContext*) super;

	ParseNode* expr = Block_get_local(self->block, name);
	if (expr)
		return expr;

	// Not declared here, search up the chain.
	expr = self->environment.parent->find_autodeclaring(self->environment.parent, name);
	if (expr)
		return expr;

	// Doesn't exist, so declare it.
	return Block_autodeclare(self->block, name);
}

void BlockContext_init(BlockContext* self, struct Block* block, Environment* parent)
{
	self->environment.parent = parent;
	self->environment.find = BlockContext_find;
	self->environment.find_autodeclaring = BlockContext_find_autodeclaring;
	self->block = block;
}



