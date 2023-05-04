#pragma once

struct String;
struct ParseNode;
struct Dict;
struct Object;
struct Block;


typedef struct Environment {
	struct Environment* parent;
	struct ParseNode* (*find)(struct Environment* self, struct String* name);
	struct ParseNode* (*find_autodeclaring)(struct Environment* self, struct String* name);
	} Environment;

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

typedef struct BlockContext {
	Environment environment;
	struct Block* block;
	} BlockContext;
extern void BlockContext_init(BlockContext* self, struct Block* block, Environment* parent);

