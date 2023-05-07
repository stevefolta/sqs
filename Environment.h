#pragma once

struct String;
struct ParseNode;
struct Dict;
struct Object;
struct Class;
struct Block;
struct MethodBulder;


typedef struct Environment {
	struct Environment* parent;
	struct ParseNode* (*find)(struct Environment* self, struct String* name);
	struct ParseNode* (*find_autodeclaring)(struct Environment* self, struct String* name);
	struct Class* (*get_class)(struct Environment* self, struct String* name);
	} Environment;
extern struct Class* Environment_find_class(Environment* self, struct String* name);

typedef struct GlobalEnvironment {
	Environment environment;
	struct Dict* dict;
	} GlobalEnvironment;
extern GlobalEnvironment global_environment;
extern void GlobalEnvironment_init();
extern void GlobalEnvironment_add(struct String* name, struct Object* value);
extern void GlobalEnvironment_add_fn(
	const char* name,
	int num_args,
	struct Object* (*fn)(struct Object* self, struct Object** args));
extern void GlobalEnvironment_add_class(struct Class* the_class);

typedef struct MethodEnvironment {
	Environment environment;
	struct MethodBuilder* method;
	} MethodEnvironment;
extern MethodEnvironment* new_MethodEnvironment(struct MethodBuilder* method, Environment* parent);

typedef struct BlockContext {
	Environment environment;
	struct Block* block;
	} BlockContext;
extern void BlockContext_init(BlockContext* self, struct Block* block, Environment* parent);

typedef struct BlockUpvalueContext {
	Environment environment;
	struct Block* block;
	} BlockUpvalueContext;
extern void BlockUpvalueContext_init(BlockUpvalueContext* self, struct Block* block, Environment* parent);

typedef struct ForStatementContext {
	Environment environment;
	struct String* variable_name;
	int variable_loc;
	} ForStatementContext;
extern void ForStatementContext_init(ForStatementContext* self, struct String* variable_name, int variable_loc);


