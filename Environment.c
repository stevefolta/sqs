#include "Environment.h"
#include "ParseNode.h"
#include "ClassStatement.h"
#include "BuiltinMethod.h"
#include "MethodBuilder.h"
#include "String.h"
#include "Dict.h"
#include "Class.h"
#include "Object.h"
#include "Memory.h"


Class* Environment_find_class(Environment* self, struct String* name)
{
	Environment* env = self;
	while (env) {
		if (env->get_class) {
			Class* the_class = env->get_class(env, name);
			if (the_class)
				return the_class;
			}
		env = env->parent;
		}
	return NULL;
}


GlobalEnvironment global_environment;

ParseNode* GlobalEnvironment_find(Environment* super, String* name)
{
	GlobalEnvironment* self = (GlobalEnvironment*) super;
	Object* value = Dict_at(self->dict, name);
	if (value == NULL)
		return NULL;
	return (ParseNode*) new_GlobalExpr(value);
}

Class* GlobalEnvironment_get_class(Environment* super, String* name)
{
	GlobalEnvironment* self = (GlobalEnvironment*) super;
	Object* value = Dict_at(self->dict, name);
	if (value == NULL || value->class_ != &Class_class)
		return NULL;
	return (Class*) value;
}

void GlobalEnvironment_init()
{
	global_environment.environment.find = GlobalEnvironment_find;
	global_environment.environment.find_autodeclaring = GlobalEnvironment_find;
	global_environment.environment.get_class = GlobalEnvironment_get_class;
	global_environment.dict = new_Dict();
}

void GlobalEnvironment_add(struct String* name, struct Object* value)
{
	Dict_set_at(global_environment.dict, name, value);
}

void GlobalEnvironment_add_c(const char* name, struct Object* value)
{
	Dict_set_at(global_environment.dict, new_c_static_String(name), value);
}

void GlobalEnvironment_add_fn(
	const char* name,
	int num_args,
	struct Object* (*fn)(struct Object* self, struct Object** args))
{
	BuiltinMethod* method = new_BuiltinMethod(num_args, fn);
	GlobalEnvironment_add(new_c_static_String(name), (struct Object*) method);
}

void GlobalEnvironment_add_class(Class* the_class)
{
	GlobalEnvironment_add(the_class->name, (struct Object*) the_class);
}



ParseNode* MethodEnvironment_find(Environment* super, String* name)
{
	MethodEnvironment* self = (MethodEnvironment*) super;

	int loc = MethodBuilder_find_argument(self->method, name);
	if (loc != 0)
		return (ParseNode*) new_RawLoc(loc);
	return self->environment.parent->find(self->environment.parent, name);
}

MethodEnvironment* new_MethodEnvironment(struct MethodBuilder* method, Environment* parent)
{
	MethodEnvironment* self = alloc_obj(MethodEnvironment);
	self->environment.parent = parent;
	self->environment.find = MethodEnvironment_find;
	self->environment.find_autodeclaring = MethodEnvironment_find;
	self->method = method;
	return self;
}



ParseNode* BlockContext_find(Environment* super, String* name)
{
	BlockContext* self = (BlockContext*) super;

	ParseNode* expr = Block_get_local(self->block, name);
	if (expr)
		return expr;
	FunctionStatement* function = Block_get_function(self->block, name);
	if (function)
		return (ParseNode*) new_RawLoc(function->loc);
	ClassStatement* class_statement = Block_get_class(self->block, name);
	if (class_statement)
		return ClassStatement_make_reference(class_statement);

	return self->environment.parent->find(self->environment.parent, name);
}

ParseNode* BlockContext_find_autodeclaring(Environment* super, String* name)
{
	BlockContext* self = (BlockContext*) super;

	ParseNode* expr = Block_get_local(self->block, name);
	if (expr)
		return expr;

	// Not declared here, search up the chain.
	expr = self->environment.parent->find(self->environment.parent, name);
	if (expr)
		return expr;

	// Doesn't exist, so declare it.
	return Block_autodeclare(self->block, name);
}

Class* BlockContext_get_class(Environment* super, String* name)
{
	BlockContext* self = (BlockContext*) super;
	ClassStatement* class_statement = Block_get_class(self->block, name);
	if (class_statement)
		return class_statement->built_class;
	return NULL;
}

void BlockContext_init(BlockContext* self, struct Block* block, Environment* parent)
{
	self->environment.parent = parent;
	self->environment.find = BlockContext_find;
	self->environment.find_autodeclaring = BlockContext_find_autodeclaring;
	self->environment.get_class = BlockContext_get_class;
	self->block = block;
}



ParseNode* BlockUpvalueContext_find(Environment* super, String* name)
{
	BlockContext* self = (BlockContext*) super;

	FunctionStatement* function = Block_get_function(self->block, name);
	if (function)
		return (ParseNode*) new_UpvalueFunction(function);
	ClassStatement* class_statement = Block_get_class(self->block, name);
	if (class_statement)
		return ClassStatement_make_reference(class_statement);

	return self->environment.parent->find(self->environment.parent, name);
}

Class* BlockUpvalueContext_get_class(Environment* super, String* name)
{
	BlockContext* self = (BlockContext*) super;
	ClassStatement* class_statement = Block_get_class(self->block, name);
	if (class_statement)
		return class_statement->built_class;
	return NULL;
}

void BlockUpvalueContext_init(BlockUpvalueContext* self, struct Block* block, Environment* parent)
{
	self->environment.parent = parent;
	self->environment.find = BlockUpvalueContext_find;
	self->environment.find_autodeclaring = BlockUpvalueContext_find;
	self->environment.get_class = BlockUpvalueContext_get_class;
	self->block = block;
}



ParseNode* ForStatementContext_find(Environment* super, String* name)
{
	ForStatementContext* self = (ForStatementContext*) super;

	if (String_equals(name, self->variable_name))
		return (ParseNode*) new_RawLoc(self->variable_loc);

	return self->environment.parent->find(self->environment.parent, name);
}

void ForStatementContext_init(ForStatementContext* self, struct String* variable_name, int variable_loc)
{
	self->environment.find = ForStatementContext_find;
	self->environment.find_autodeclaring = NULL;
	self->environment.get_class = NULL;
	self->variable_name = variable_name;
	self->variable_loc = variable_loc;
}



