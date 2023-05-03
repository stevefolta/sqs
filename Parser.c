#include "Parser.h"
#include "Lexer.h"
#include "ParseNode.h"
#include "String.h"
#include "Memory.h"
#include "Error.h"
#include <stdbool.h>

extern ParseNode* Parser_parse_statement(Parser* self);
extern ParseNode* Parser_parse_if_statement(Parser* self);
extern ParseNode* Parser_parse_while_statement(Parser* self);
extern ParseNode* Parser_parse_for_statement(Parser* self);
extern ParseNode* Parser_parse_expression(Parser* self);
extern ParseNode* Parser_parse_logical_or_expression(Parser* self);
extern ParseNode* Parser_parse_logical_and_expression(Parser* self);
extern ParseNode* Parser_parse_not_expression(Parser* self);
extern ParseNode* Parser_parse_inclusive_or_expression(Parser* self);
extern ParseNode* Parser_parse_exclusive_or_expression(Parser* self);
extern ParseNode* Parser_parse_and_expression(Parser* self);
extern ParseNode* Parser_parse_equality_expression(Parser* self);
extern ParseNode* Parser_parse_relational_expression(Parser* self);
extern ParseNode* Parser_parse_shift_expression(Parser* self);
extern ParseNode* Parser_parse_additive_expression(Parser* self);
extern ParseNode* Parser_parse_multiplicative_expression(Parser* self);
extern ParseNode* Parser_parse_unary_expression(Parser* self);
extern ParseNode* Parser_parse_postfix_expression(Parser* self);
extern ParseNode* Parser_parse_primary_expression(Parser* self);
extern ParseNode* Parser_parse_string_literal(Parser* self);


extern Parser* new_Parser(const char* text, size_t size)
{
	Parser* parser = (Parser*) alloc_mem(sizeof(Parser));
	parser->lexer = new_Lexer(text, size);
	return parser;
}


ParseNode* Parser_parse_block(Parser* self)
{
	// This is used to compile indented blocks, as well as raw code.  So if used
	// for an indented block, the Indent must be consumed before this is called.

	Block* block = NULL;

	while (true) {
		Token next_token = Lexer_peek(self->lexer);
		if (next_token.type == Unindent || next_token.type == EndOfText) {
			Lexer_next(self->lexer);
			break;
			}
		else if (next_token.type == EOL) {
			// Empty line.
			Lexer_next(self->lexer);
			continue;
			}

		ParseNode* statement = Parser_parse_statement(self);
		if (statement == NULL) {
			Error("Statement expected at line %d.", next_token.line_number);
			return NULL;
			}
		if (block == NULL)
			block = new_Block();
		Block_append(block, statement);
		}

	return (ParseNode*) block;
}


typedef struct {
	const char* name;
	ParseNode* (*fn)(Parser* self);
	} StatementParser;
static StatementParser statement_parsers[] = {
	{ "if", &Parser_parse_if_statement },
	{ "while", &Parser_parse_while_statement },
	{ "for", &Parser_parse_for_statement },
	};

ParseNode* Parser_parse_statement(Parser* self)
{
	Token next_token = Lexer_peek(self->lexer);
	if (next_token.type == EOL) {
		Lexer_next(self->lexer);
		return NULL;
		}

	if (next_token.type == Identifier) {
		for (int i = 0; i < sizeof(statement_parsers) / sizeof(statement_parsers[0]); ++i) {
			if (String_equals_c(next_token.token, statement_parsers[i].name))
				return statement_parsers[i].fn(self);
			}
		}

	ParseNode* expression = Parser_parse_expression(self);
	if (expression == NULL)
		return NULL;
	next_token = Lexer_next(self->lexer);
	if (next_token.type != EOL)
		Error("Extra characters after expression at line %d.", next_token.line_number);
	return (ParseNode*) new_ExpressionStatement(expression);
}


ParseNode* Parser_parse_if_statement(Parser* self)
{
	int line_number = Lexer_next(self->lexer).line_number;

	ParseNode* condition = Parser_parse_expression(self);
	if (condition == NULL)
		Error("Missing expression in \"if\" statement at line %d.", line_number);
	if (Lexer_next(self->lexer).type != EOL)
		Error("Extra characters after expression at line %d.", line_number);
	IfStatement* statement = new_IfStatement();
	statement->condition = condition;
	if (Lexer_peek(self->lexer).type == Indent) {
		Lexer_next(self->lexer);
		statement->if_block = Parser_parse_block(self);
		}
	Token next_token = Lexer_peek(self->lexer);
	if (next_token.type == Identifier && String_equals_c(next_token.token, "else")) {
		Lexer_next(self->lexer);
		next_token = Lexer_peek(self->lexer);
		if (next_token.type == Identifier && String_equals_c(next_token.token, "if"))
			statement->else_block = Parser_parse_if_statement(self);
		else {
			if (Lexer_peek(self->lexer).type == Indent) {
				Lexer_next(self->lexer);
				statement->else_block = Parser_parse_block(self);
				}
			}
		}
	return (ParseNode*) statement;
}


ParseNode* Parser_parse_while_statement(Parser* self)
{
	int line_number = Lexer_next(self->lexer).line_number;

	ParseNode* condition = Parser_parse_expression(self);
	if (condition == NULL)
		Error("Missing expression in \"while\" statement at line %d.", line_number);
	if (Lexer_next(self->lexer).type != EOL)
		Error("Extra characters after expression at line %d.", line_number);
	WhileStatement* statement = new_WhileStatement();
	statement->condition = condition;
	if (Lexer_peek(self->lexer).type == Indent) {
		Lexer_next(self->lexer);
		statement->body = Parser_parse_block(self);
		}
	return (ParseNode*) statement;
}


ParseNode* Parser_parse_for_statement(Parser* self)
{
	int line_number = Lexer_next(self->lexer).line_number;

	Token token = Lexer_next(self->lexer);
	if (token.type != Identifier)
		Error("Need identifier for \"for\" statement (line %d).", line_number);
	ForStatement* statement = new_ForStatement();
	statement->variable_name = token.token;
	token = Lexer_next(self->lexer);
	if (token.type != Operator || !String_equals_c(token.token, ":"))
		Error("Missing \":\" in \"for\" statement (line %d).", line_number);
	statement->collection = Parser_parse_expression(self);
	if (statement->collection == NULL)
		Error("Missing expression in \"for\" statement (line %d).", line_number);
	if (Lexer_next(self->lexer).type != EOL)
		Error("Extra characters after expression at line %d.", line_number);
	if (Lexer_peek(self->lexer).type == Indent) {
		Lexer_next(self->lexer);
		statement->body = Parser_parse_block(self);
		}

	return (ParseNode*) statement;
}


ParseNode* Parser_parse_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_logical_or_expression(self);
	if (expr == NULL)
		return NULL;

	Token next_token = Lexer_peek(self->lexer);
	if (next_token.type == Operator && String_equals_c(next_token.token, "=")) {
		Lexer_next(self->lexer);
		if (!expr->emit_set)
			Error("Attempt to set something that isn't settable (line %d).", next_token.line_number);
		ParseNode* right = Parser_parse_expression(self);
		if (right == NULL)
			Error("Missing expression after \"=\" (line %d).", next_token.line_number);
		SetExpr* setter = new_SetExpr();
		setter->left = expr;
		setter->right = right;
		expr = (ParseNode*) setter;
		}

	// TODO: +=, etc.

	return expr;
}


ParseNode* Parser_parse_logical_or_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_logical_and_expression(self);
	if (expr == NULL)
		return NULL;

	while (true) {
		Token next_token = Lexer_peek(self->lexer);
		if (next_token.type != Operator || !String_equals_c(next_token.token, "||"))
			break;
		Lexer_next(self->lexer);

		ParseNode* expr2 = Parser_parse_logical_and_expression(self);
		if (expr2 == NULL)
			Error("Missing expression after \"||\" (line %d).", next_token.line_number);
		expr = (ParseNode*) new_ShortCircuitExpr(expr, expr2, false);
		}

	return expr;
}


ParseNode* Parser_parse_logical_and_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_not_expression(self);
	if (expr == NULL)
		return NULL;

	while (true) {
		Token next_token = Lexer_peek(self->lexer);
		if (next_token.type != Operator || !String_equals_c(next_token.token, "&&"))
			break;
		Lexer_next(self->lexer);

		ParseNode* expr2 = Parser_parse_not_expression(self);
		if (expr2 == NULL)
			Error("Missing expression after \"&&\" (line %d).", next_token.line_number);
		expr = (ParseNode*) new_ShortCircuitExpr(expr, expr2, true);
		}

	return expr;
}


ParseNode* Parser_parse_not_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_inclusive_or_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_inclusive_or_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_exclusive_or_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_exclusive_or_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_and_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_and_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_equality_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_equality_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_relational_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_relational_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_shift_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_shift_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_additive_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_additive_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_multiplicative_expression(self);
	if (expr == NULL)
		return NULL;

	Token op = Lexer_peek(self->lexer);
	if (op.type != Operator || (!String_equals_c(op.token, "+") && !String_equals_c(op.token, "-")))
		return expr;
	Lexer_next(self->lexer);
	ParseNode* expr2 = Parser_parse_multiplicative_expression(self);
	if (expr2 == NULL)
		Error("Missing expression after \"%s\" in line %d.", op.token, op.line_number);

	return (ParseNode*) new_CallExpr_binop(expr, expr2, op.token);
}


ParseNode* Parser_parse_multiplicative_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_unary_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_unary_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_postfix_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_postfix_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_primary_expression(self);
	if (expr == NULL)
		return NULL;

	/***/

	return expr;
}


ParseNode* Parser_parse_primary_expression(Parser* self)
{
	Token next_token = Lexer_peek(self->lexer);

	if (next_token.type == StringLiteral)
		return Parser_parse_string_literal(self);

	else if (next_token.type == Identifier) {
		Lexer_next(self->lexer);
		if (String_equals_c(next_token.token, "true"))
			return (ParseNode*) new_BooleanLiteral(true);
		else if (String_equals_c(next_token.token, "false"))
			return (ParseNode*) new_BooleanLiteral(false);
		else if (String_equals_c(next_token.token, "nil"))
			return new_NilLiteral();
		return (ParseNode*) new_Variable(next_token.token);
		}

	/***/

	return NULL;
}


ParseNode* Parser_parse_string_literal(Parser* self)
{
	Token token = Lexer_next(self->lexer);

	/*** TODO: process interpolations and escapes ***/
	/***/

	return (ParseNode*) new_StringLiteralExpr(token.token);
}



