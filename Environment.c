#include "Environment.h"
#include "ParseNode.h"
#include "ClassStatement.h"
#include "BuiltinMethod.h"
#include "MethodBuilder.h"
#include "Upvalues.h"
#include "String.h"
#include "Dict.h"
#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include "Error.h"


Class* Environment_find_class_for_superclass(Environment* self, struct String* name, struct MethodBuilder* method)
{
	Environment* env = self;
	while (env) {
		if (env->get_class_for_superclass) {
			Class* the_class = env->get_class_for_superclass(env, name, method);
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

Class* GlobalEnvironment_get_class_for_superclass(Environment* super, String* name, MethodBuilder* method)
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
	global_environment.environment.get_class_for_superclass = GlobalEnvironment_get_class_for_superclass;
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
	if (function) {
		// If this is in the same function, we could return a RawLoc of the local.
		// But it might be in an enclosed function, so we'll use an
		// UpvalueFunction, which works anywhere.
		return (ParseNode*) new_UpvalueFunction(function);
		}
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

Class* BlockContext_get_class_for_superclass(Environment* super, String* name, MethodBuilder* method)
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
	self->environment.get_class_for_superclass = BlockContext_get_class_for_superclass;
	self->block = block;
}



ParseNode* BlockUpvalueContext_find(Environment* super, String* name)
{
	BlockUpvalueContext* self = (BlockUpvalueContext*) super;

	FunctionStatement* function = Block_get_function(self->block, name);
	if (function)
		return (ParseNode*) new_UpvalueFunction(function);
	ClassStatement* class_statement = Block_get_class(self->block, name);
	if (class_statement)
		return ClassStatement_make_reference(class_statement);

	ParseNode* node = self->environment.parent->find(self->environment.parent, name);
	if (node && node->type == PN_Local) {
		// It's a local in the block's method, turn it into an upvalue.
		// If it were a local in an enclosing method, it would already be an
		// UpvalueLocal.
		Local* local = (Local*) node;
		return (ParseNode*) new_UpvalueLocal(
			self->method_builder->method,
			local->block->locals_base + local->block_index);
		}
	return node;
}

Class* BlockUpvalueContext_get_class_for_superclass(Environment* super, String* name, MethodBuilder* method)
{
	BlockContext* self = (BlockContext*) super;
	ClassStatement* class_statement = Block_get_class(self->block, name);
	if (class_statement) {
		if (!class_statement->is_built)
			Error("A class can't have a superclass (\"%s\") that hasn't been defined yet.", String_c_str(name));
		return class_statement->built_class;
		}
	return NULL;
}

void BlockUpvalueContext_init(
	BlockUpvalueContext* self,
	struct Block* block, struct MethodBuilder* method_builder,
	Environment* parent)
{
	self->environment.parent = parent;
	self->environment.find = BlockUpvalueContext_find;
	self->environment.find_autodeclaring = BlockUpvalueContext_find;
	self->environment.get_class_for_superclass = BlockUpvalueContext_get_class_for_superclass;
	self->block = block;
	self->method_builder = method_builder;
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
	self->environment.get_class_for_superclass = NULL;
	self->variable_name = variable_name;
	self->variable_loc = variable_loc;
}



