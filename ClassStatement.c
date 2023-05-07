#include "ClassStatement.h"
#include "Parser.h"
#include "Lexer.h"
#include "MethodBuilder.h"
#include "Environment.h"
#include "String.h"
#include "Array.h"
#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include "Error.h"


ParseNode* Parser_parse_class_statement(Parser* self)
{
	Lexer_next(self->lexer); 	// Consume "class".

	// Make sure new functions and classes don't end up in the enclosing block.
	Block* inner_block = self->inner_block;
	self->inner_block = NULL;

	// Name.
	Token token = Lexer_next(self->lexer);
	if (token.type != Identifier)
		Error("Expected class name on line %d.", token.line_number);
	String* name = token.token;

	ClassStatement* class_statement = new_ClassStatement(name);

	// Superclass.
	token = Lexer_peek(self->lexer);
	if (token.type == Operator && String_equals_c(token.token, ":")) {
		Lexer_next(self->lexer);
		token = Lexer_next(self->lexer);
		if (token.type != Identifier)
			Error("Expected a class name as the superclass on line %d.", token.line_number);
		class_statement->superclass_name = token.token;
		}

	// Ivars.
	class_statement->ivars = Parser_parse_names_list(self, "instance variable");

	token = Lexer_next(self->lexer);
	if (token.type != EOL)
		Error("Extra characters after class definition on line %d.", token.line_number);

	// Functions and classes.
	token = Lexer_peek(self->lexer);
	if (token.type == Indent) {
		Lexer_next(self->lexer);
		while (true) {
			token = Lexer_next(self->lexer);
			if (token.type == Unindent)
				break;
			else if (token.type == EOL)
				continue;

			// "class"
			else if (token.type == Identifier && String_equals_c(token.token, "class")) {
				//***
				}

			// Anything else is a function.

			// It might be preceded by "fn", or not.
			if (token.type == Identifier && String_equals_c(token.token, "fn")) {
				token = Lexer_next(self->lexer);
				if (token.type != Identifier && token.type != Operator)
					Error("Bad function definition on line %d.", token.line_number);
				}

			ParseNode* function = Parser_parse_fn_statement_raw(self);
			if (function == NULL)
				Error("Expected function definition on line %d.", token.line_number);
			Array_append(class_statement->functions, (Object*) function);
			}
		}

	if (inner_block)
		Block_add_class(inner_block, class_statement);

	self->inner_block = inner_block;
	return (ParseNode*) class_statement;
}


int ClassStatement_emit(ParseNode* super, MethodBuilder* method)
{
	ClassStatement* self = (ClassStatement*) super;

	// Superclass.
	if (self->superclass_name) {
		Class* superclass = Environment_find_class(method->environment, self->superclass_name);
		if (superclass == NULL)
			Error("Couldn't find a superclass named \"%s\".", self->superclass_name);
		self->built_class->superclass = superclass;
		}

	// Calculate "num_ivars".
	int num_ivars = (self->ivars ? self->ivars->size : 0);
	Class* ancestor = self->built_class->superclass;
	while (ancestor) {
		num_ivars += ancestor->num_ivars;
		ancestor = ancestor->superclass;
		}

	/***/

	return 0;
}

ClassStatement* new_ClassStatement(String* name)
{
	ClassStatement* self = alloc_obj(ClassStatement);
	self->parse_node.emit = ClassStatement_emit;
	self->built_class = new_Class(name);
	self->functions = new_Array();
	return self;
}


String* ClassStatement_get_name(ClassStatement* self)
{
	return self->built_class->name;
}


ParseNode* ClassStatement_make_reference(ClassStatement* self)
{
	return (ParseNode*) new_GlobalExpr((Object*) self->built_class);
}



