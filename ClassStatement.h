#pragma once

#include "ParseNode.h"

struct Parser;
struct String;
struct Array;
struct Class;

extern ParseNode* Parser_parse_class_statement(struct Parser* self);

typedef struct ClassStatement {
	ParseNode parse_node;
	struct String* superclass_name;
	struct Array* ivars;
	struct Array* functions;

	struct Class* built_class;
	} ClassStatement;
ClassStatement* new_ClassStatement(struct String* name);

struct String* ClassStatement_get_name(ClassStatement* self);
ParseNode* ClassStatement_make_reference(ClassStatement* self);


