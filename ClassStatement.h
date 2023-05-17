#pragma once

#include "ParseNode.h"

struct Parser;
struct String;
struct Array;
struct Dict;
struct Class;

extern ParseNode* Parser_parse_class_statement(struct Parser* self);

typedef struct ClassStatement {
	ParseNode parse_node;
	struct String* superclass_name;
	struct Array* ivars;
	struct Dict* functions;
	struct Dict* enclosed_classes;

	struct Class* built_class;
	bool is_building, is_built;
	} ClassStatement;
ClassStatement* new_ClassStatement(struct String* name);

struct String* ClassStatement_get_name(ClassStatement* self);
ParseNode* ClassStatement_make_reference(ClassStatement* self);


