#pragma once

#include <stdbool.h>

struct Parser;
struct ParseNode;
struct BlockContext;
struct BlockUpvalueContext;
struct String;
struct Dict;


extern struct ParseNode* Parser_parse_export(struct Parser* self);
extern struct ParseNode* Parser_parse_import(struct Parser* self);

typedef struct Module {
	struct Block* block;
	struct Method* method;
	struct Dict* exported_classes;
	struct Dict* exported_functions;
	struct Object** locals;
	bool is_building;
	} Module;
extern Module* Module_get_module(struct String* name);
extern struct FunctionStatement* Module_exported_function(Module* self, struct String* name);
extern struct ClassStatement* Module_exported_class(Module* self, struct String* name);
extern void Module_create_module_locals(Module* self, int num_locals);
extern void Module_build(Module* module);

extern void BlockContext_make_module_context(struct BlockContext* context);
extern void BlockUpvalueContext_make_module_context(struct BlockUpvalueContext* context);

