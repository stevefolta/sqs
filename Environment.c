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
	global_environment.dict = new_Dict();
}

void GlobalEnvironment_add(struct String* name, struct Object* value)
{
	Dict_set_at(global_environment.dict, name, value);
}



