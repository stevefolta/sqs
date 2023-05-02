#pragma once

struct String;
struct ParseNode;
struct Dict;
struct Object;


typedef struct Environment {
	struct ParseNode* (*find)(struct Environment* self, struct String* name);
	} Environment;

typedef struct GlobalEnvironment {
	Environment environment;
	struct Dict* dict;
	} GlobalEnvironment;
extern GlobalEnvironment global_environment;
extern void GlobalEnvironment_init();
extern void GlobalEnvironment_add(struct String* name, struct Object* value);

