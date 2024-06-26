#pragma once

struct String;
struct ParseNode;
struct Dict;
struct Object;
struct Class;
struct Block;
struct MethodBuilder;


typedef struct Environment {
	struct Environment* parent;
	struct ParseNode* (*find)(struct Environment* self, struct String* name);
	struct ParseNode* (*find_autodeclaring)(struct Environment* self, struct String* name);
	struct Class* (*get_class_for_superclass)(struct Environment* self, struct String* name, struct MethodBuilder* builder);
	struct Class* on_class;
	} Environment;
extern struct Class* Environment_find_class_for_superclass(Environment* self, struct String* name, struct MethodBuilder* builder);
extern struct Class* Environment_find_function_class(Environment* self);

typedef struct GlobalEnvironment {
	Environment environment;
	struct Dict* dict;
	} GlobalEnvironment;
extern GlobalEnvironment global_environment;
extern void GlobalEnvironment_init();
extern void GlobalEnvironment_add(struct String* name, struct Object* value);
extern void GlobalEnvironment_add_c(const char* name, struct Object* value);
extern void GlobalEnvironment_add_fn(
	const char* name,
	int num_args,
	struct Object* (*fn)(struct Object* self, struct Object** args));
extern void GlobalEnvironment_add_class(struct Class* the_class);

typedef struct MethodEnvironment {
	Environment environment;
	struct MethodBuilder* method;
	} MethodEnvironment;
extern MethodEnvironment* new_MethodEnvironment(struct MethodBuilder* builder, Environment* parent);

typedef struct BlockContext {
	Environment environment;
	struct Block* block;
	} BlockContext;
extern void BlockContext_init(BlockContext* self, struct Block* block, Environment* parent);
extern struct ParseNode* BlockContext_find(Environment* super, struct String* name);
extern struct ParseNode* BlockContext_find_autodeclaring(Environment* super, struct String* name);

typedef struct BlockUpvalueContext {
	Environment environment;
	struct Block* block;
	struct MethodBuilder* method_builder;
	} BlockUpvalueContext;
extern void BlockUpvalueContext_init(
	BlockUpvalueContext* self,
	struct Block* block, struct MethodBuilder* method_builder,
	Environment* parent);

typedef struct ForStatementContext {
	Environment environment;
	struct String* variable_name;
	int variable_loc;
	} ForStatementContext;
extern void ForStatementContext_init(ForStatementContext* self, struct String* variable_name, int variable_loc);


