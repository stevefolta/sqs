#include "Parser.h"
#include "Lexer.h"
#include "ParseNode.h"
#include "ClassStatement.h"
#include "RunStatement.h"
#include "String.h"
#include "Array.h"
#include "Object.h"
#include "Memory.h"
#include "UTF8.h"
#include "Error.h"
#include <stdint.h>
#include <string.h>

extern ParseNode* Parser_parse_statement(Parser* self);
extern ParseNode* Parser_parse_if_statement(Parser* self);
extern ParseNode* Parser_parse_while_statement(Parser* self);
extern ParseNode* Parser_parse_for_statement(Parser* self);
extern ParseNode* Parser_parse_continue_statement(Parser* self);
extern ParseNode* Parser_parse_break_statement(Parser* self);
extern ParseNode* Parser_parse_return_statement(Parser* self);
extern ParseNode* Parser_parse_with_statement(Parser* self);
extern ParseNode* Parser_parse_fn_statement(Parser* self);
extern ParseNode* Parser_parse_logical_or_expression(Parser* self);
extern ParseNode* Parser_parse_logical_and_expression(Parser* self);
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
extern ParseNode* Parser_parse_array_literal(Parser* self);
extern ParseNode* Parser_parse_dict_literal(Parser* self);
extern ParseNode* Parser_parse_super_call(Parser* self);
extern bool String_is_one_of(String* str, const char** values);


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

	// Give access to the innermost block during parsing, so functions and
	// classes can be defined in the block during the parse phase.  That's so
	// forward references will work during the later emit/resolve phase.
	Block* prev_inner_block = self->inner_block;

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

		// We're lazily making the block object, in case there were nothing but
		// empty lines.
		if (block == NULL) {
			block = new_Block();
			self->inner_block = block;
			}

		ParseNode* statement = Parser_parse_statement(self);
		if (statement == NULL) {
			Error("Statement expected at line %d.", next_token.line_number);
			return NULL;
			}
		Block_append(block, statement);
		}

	self->inner_block = prev_inner_block;
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
	{ "continue", &Parser_parse_continue_statement },
	{ "break", &Parser_parse_break_statement },
	{ "return", &Parser_parse_return_statement },
	{ "with", &Parser_parse_with_statement },
	{ "fn", &Parser_parse_fn_statement },
	{ "class", &Parser_parse_class_statement },
	{ "$", &Parser_parse_run_statement },
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
	if (next_token.type == Identifier) {
		if (String_equals_c(next_token.token, "else")) {
			Lexer_next(self->lexer);
			next_token = Lexer_peek(self->lexer);
			if (next_token.type == Identifier && String_equals_c(next_token.token, "if"))
				statement->else_block = Parser_parse_if_statement(self);
			else {
				if (next_token.type != EOL)
					Error("Extra tokens at end of \"else\" in line %d.", next_token.line_number);
				Lexer_next(self->lexer);
				if (Lexer_peek(self->lexer).type == Indent) {
					Lexer_next(self->lexer);
					statement->else_block = Parser_parse_block(self);
					}
				}
			}
		else if (String_equals_c(next_token.token, "elif")) {
			// Don't consume the "elif", the recursive call will do that.
			statement->else_block = Parser_parse_if_statement(self);
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


ParseNode* Parser_parse_continue_statement(Parser* self)
{
	Lexer_next(self->lexer);
	return new_ContinueStatement();
}

ParseNode* Parser_parse_break_statement(Parser* self)
{
	Lexer_next(self->lexer);
	return new_BreakStatement();
}


ParseNode* Parser_parse_return_statement(Parser* self)
{
	Lexer_next(self->lexer);

	ReturnStatement* return_statement = new_ReturnStatement();

	ParseNode* value = Parser_parse_expression(self);
	if (value)
		return_statement->value = value;

	Token token = Lexer_next(self->lexer);
	if (token.type != EOL)
		Error("Extra characters at end of \"return\" statement on line %d.", token.line_number);

	return (ParseNode*) return_statement;
}


ParseNode* Parser_parse_with_statement(Parser* self)
{
	Lexer_next(self->lexer);

	// Parse.
	Token token = Lexer_next(self->lexer);
	if (token.type != Identifier)
		Error("Expected a name in \"with\" statement on line %d.", token.line_number);
	String* name = token.token;
	token = Lexer_next(self->lexer);
	if (token.type != Operator || !String_equals_c(token.token, "="))
		Error("Expected \"=\"  in \"with\" statement on line %d.", token.line_number);
	ParseNode* expr = Parser_parse_expression(self);
	if (expr == NULL)
		Error("Expected expression in \"with\" statement on line %d.", token.line_number);
	if (Lexer_next(self->lexer).type != EOL)
		Error("Extra characters after expression on line %d.", token.line_number);
	ParseNode* body = NULL;
	if (Lexer_peek(self->lexer).type == Indent) {
		Lexer_next(self->lexer);
		body = Parser_parse_block(self);
		}

	return (ParseNode*) new_WithStatement(name, expr, body);
}


FunctionStatement* Parser_parse_fn_statement_raw(Parser* self)
{
	// This could be in either a Block or a ClassStatement.

	// Name.
	String* name = Parser_parse_fn_name(self);

	FunctionStatement* function = new_FunctionStatement(name);

	// Arguments.
	function->arguments = Parser_parse_names_list(self, "argument");

	Token token = Lexer_next(self->lexer);
	if (token.type != EOL)
		Error("Extra characters at the end of a \"fn\" definition on line %d.", token.line_number);

	// Body.
	if (Lexer_peek(self->lexer).type == Indent) {
		Lexer_next(self->lexer);
		function->body = Parser_parse_block(self);
		}

	return function;
}

ParseNode* Parser_parse_fn_statement(Parser* self)
{
	// This one is only used in a Block.

	Lexer_next(self->lexer); 	// Consume the "fn".
	FunctionStatement* function = Parser_parse_fn_statement_raw(self);

	if (self->inner_block)
		Block_add_function(self->inner_block, function);

	return (ParseNode*) function;
}


ParseNode* Parser_parse_expression(Parser* self)
{
	static const char* modify_tokens[] = {
		"+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "|=", "&=", "^=",
		NULL
		};

	ParseNode* expr = Parser_parse_logical_or_expression(self);
	if (expr == NULL)
		return NULL;

	Token next_token = Lexer_peek(self->lexer);
	if (next_token.type == Operator) {
		// "="
		if (String_equals_c(next_token.token, "=")) {
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

		// "+=", etc.
		else if (String_is_one_of(next_token.token, modify_tokens)) {
			Lexer_next(self->lexer);
			if (!expr->emit_set)
				Error("Attempt to set something that isn't settable (line %d).", next_token.line_number);
			ParseNode* right = Parser_parse_expression(self);
			if (right == NULL)
				Error("Missing expression after \"=\" (line %d).", next_token.line_number);

			// Make the operation.
			String* op_name = new_String(next_token.token->str, next_token.token->size - 1);
			CallExpr* call = new_CallExpr_binop(expr, right, op_name);

			// Make the set.
			SetExpr* setter = new_SetExpr();
			setter->left = expr;
			setter->right = (ParseNode*) call;
			expr = (ParseNode*) setter;
			}
		}

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
	ParseNode* expr = Parser_parse_inclusive_or_expression(self);
	if (expr == NULL)
		return NULL;

	while (true) {
		Token next_token = Lexer_peek(self->lexer);
		if (next_token.type != Operator || !String_equals_c(next_token.token, "&&"))
			break;
		Lexer_next(self->lexer);

		ParseNode* expr2 = Parser_parse_inclusive_or_expression(self);
		if (expr2 == NULL)
			Error("Missing expression after \"&&\" (line %d).", next_token.line_number);
		expr = (ParseNode*) new_ShortCircuitExpr(expr, expr2, true);
		}

	return expr;
}


bool String_is_one_of(String* str, const char** values)
{
	for (const char** value = values; *value; ++value) {
		if (String_equals_c(str, *value))
			return true;
		}
	return false;
}


ParseNode* Parser_parse_binop(
	Parser* self,
	ParseNode* (*parse_tighter)(Parser* self),
	const char** tokens)
{
	ParseNode* expr = parse_tighter(self);
	if (expr == NULL)
		return NULL;

	while (true) {
		// Is the next token one of the ones we're looking for?
		Token op = Lexer_peek(self->lexer);
		if (op.type != Operator || !String_is_one_of(op.token, tokens))
			break;

		// Parse the argument.
		Lexer_next(self->lexer);
		ParseNode* expr2 = parse_tighter(self);
		if (expr2 == NULL)
			Error("Missing expression after \"%s\" in line %d.", String_c_str(op.token), op.line_number);

		// Make the binop.
		expr = (ParseNode*) new_CallExpr_binop(expr, expr2, op.token);
		}

	return expr;
}


ParseNode* Parser_parse_inclusive_or_expression(Parser* self)
{
	static const char* tokens[] = { "|", NULL };
	return Parser_parse_binop(self, Parser_parse_exclusive_or_expression, tokens);
}


ParseNode* Parser_parse_exclusive_or_expression(Parser* self)
{
	static const char* tokens[] = { "^", NULL };
	return Parser_parse_binop(self, Parser_parse_and_expression, tokens);
}


ParseNode* Parser_parse_and_expression(Parser* self)
{
	static const char* tokens[] = { "&", NULL };
	return Parser_parse_binop(self, Parser_parse_equality_expression, tokens);
}


ParseNode* Parser_parse_equality_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_relational_expression(self);
	if (expr == NULL)
		return NULL;

	while (true) {
		Token op = Lexer_peek(self->lexer);
		if (op.type != Operator || (!String_equals_c(op.token, "==") && !String_equals_c(op.token, "!=")))
			return expr;
		Lexer_next(self->lexer);
		ParseNode* expr2 = Parser_parse_relational_expression(self);
		if (expr2 == NULL)
			Error("Missing expression after \"%s\" in line %d.", String_c_str(op.token), op.line_number);

		// Special-case "== nil" and "!= nil".
		//*** TODO

		expr = (ParseNode*) new_CallExpr_binop(expr, expr2, op.token);
		}

	return expr;
}


ParseNode* Parser_parse_relational_expression(Parser* self)
{
	static const char* tokens[] = { "<", ">", "<=", ">=", NULL };
	return Parser_parse_binop(self, Parser_parse_shift_expression, tokens);
}


ParseNode* Parser_parse_shift_expression(Parser* self)
{
	static const char* tokens[] = { "<<", ">>", NULL };
	return Parser_parse_binop(self, Parser_parse_additive_expression, tokens);
}


ParseNode* Parser_parse_additive_expression(Parser* self)
{
	static const char* tokens[] = { "+", "-", NULL };
	return Parser_parse_binop(self, Parser_parse_multiplicative_expression, tokens);
}


ParseNode* Parser_parse_multiplicative_expression(Parser* self)
{
	static const char* tokens[] = { "*", "/", "%", NULL };
	return Parser_parse_binop(self, Parser_parse_unary_expression, tokens);
}


ParseNode* Parser_parse_unary_expression(Parser* self)
{
	static const char* tokens[] = { "~", "-", NULL };

	Token next_token = Lexer_peek(self->lexer);
	if (next_token.type == Operator) {
		if (String_equals_c(next_token.token, "!")) {
			Lexer_next(self->lexer);
			ParseNode* expr = Parser_parse_unary_expression(self);
			if (expr == NULL)
				Error("Expected expression after \"!\" on line %d.", next_token.line_number);
			return (ParseNode*) new_ShortCircuitNot(expr);
			}

		else if (String_is_one_of(next_token.token, tokens)) {
			Lexer_next(self->lexer);
			ParseNode* expr = Parser_parse_unary_expression(self);
			if (expr == NULL)
				Error("Expected expression after \"%s\" on line %d.", String_c_str(next_token.token), next_token.line_number);
			return (ParseNode*) new_CallExpr(expr, next_token.token);
			}
		}

	return Parser_parse_postfix_expression(self);
}


Array* Parser_parse_arguments(Parser* self)
{
	Token start_token = Lexer_next(self->lexer); 	// Consume "(".

	Array* args = new_Array();

	bool need_comma = false;
	while (true) {
		// Next ")" or ",".
		Token next_token = Lexer_peek(self->lexer);
		if (next_token.type == Operator && String_equals_c(next_token.token, ")")) {
			Lexer_next(self->lexer);
			break;
			}
		if (need_comma) {
			if (next_token.type == EndOfText)
				Error("Unterminated argument list starting at line %d.", start_token.line_number);
			if (next_token.type != Operator || !String_equals_c(next_token.token, ","))
				Error("Comma expected between arguments in line %d.", next_token.line_number);
			Lexer_next(self->lexer);
			need_comma = false;
			}

		ParseNode* arg = Parser_parse_expression(self);
		if (arg == NULL)
			Error("Expected expression in argument list in line %d.", next_token.line_number);
		Array_append(args, (Object*) arg);
		need_comma = true;
		}

	return args;
}


ParseNode* Parser_parse_dot_call(Parser* self, ParseNode* receiver)
{
	Lexer_next(self->lexer); 	// Consume the ".".

	// Get the name.
	Token token = Lexer_next(self->lexer);
	if (token.type != Identifier)
		Error("Expected a name after \".\" in line %d.", token.line_number);
	String* name = token.token;
	CallExpr* call = new_CallExpr(receiver, name);

	// Parse the arguments (if there are any).
	Token next_token = Lexer_peek(self->lexer);
	if (next_token.type == Operator && String_equals_c(next_token.token, "(")) {
		Array* args = Parser_parse_arguments(self);
		call->arguments = args;
		}
	call->got_args = true; 	// We know we handled them if there are any.

	return (ParseNode*) call;
}


ParseNode* Parser_parse_fn_call(Parser* self, ParseNode* fn)
{
	Array* arguments = Parser_parse_arguments(self);
	return (ParseNode*) new_FunctionCallExpr(fn, arguments);
}


ParseNode* Parser_parse_index_call(Parser* self, ParseNode* receiver)
{
	Lexer_next(self->lexer); 	// Consume the ".".

	CallExpr* call = new_CallExpr(receiver, new_c_static_String("[]"));

	// Get the argument.
	ParseNode* index_expr = Parser_parse_expression(self);
	CallExpr_add_argument(call, index_expr);

	// Finish.
	Token token = Lexer_next(self->lexer);
	if (token.type != Operator || !String_equals_c(token.token, "]"))
		Error("Expected \"]\" in line %d.", token.line_number);
	return (ParseNode*) call;
}


ParseNode* Parser_parse_postfix_expression(Parser* self)
{
	ParseNode* expr = Parser_parse_primary_expression(self);
	if (expr == NULL)
		return NULL;

	while (true) {
		Token next_token = Lexer_peek(self->lexer);
		if (next_token.type != Operator)
			break;

		// Method call.
		if (String_equals_c(next_token.token, "."))
			expr = Parser_parse_dot_call(self, expr);

		// Function call.
		else if (String_equals_c(next_token.token, "("))
			expr = Parser_parse_fn_call(self, expr);

		else if (String_equals_c(next_token.token, "["))
			expr = Parser_parse_index_call(self, expr);

		else
			break;
		}

	return expr;
}


ParseNode* Parser_parse_primary_expression(Parser* self)
{
	Token next_token = Lexer_peek(self->lexer);

	if (next_token.type == StringLiteral)
		return Parser_parse_string_literal(self);
	else if (next_token.type == RawStringLiteral) {
		Lexer_next(self->lexer);
		return (ParseNode*) new_StringLiteralExpr(next_token.token);
		}
	else if (next_token.type == IntLiteral) {
		Lexer_next(self->lexer);
		return (ParseNode*) new_IntLiteralExpr(next_token.token);
		}

	else if (next_token.type == Identifier) {
		Lexer_next(self->lexer);
		if (String_equals_c(next_token.token, "true"))
			return (ParseNode*) new_BooleanLiteral(true);
		else if (String_equals_c(next_token.token, "false"))
			return (ParseNode*) new_BooleanLiteral(false);
		else if (String_equals_c(next_token.token, "nil"))
			return new_NilLiteral();
		else if (String_equals_c(next_token.token, "self"))
			return (ParseNode*) new_SelfExpr();
		else if (String_equals_c(next_token.token, "super"))
			return Parser_parse_super_call(self);
		else if (String_equals_c(next_token.token, "$")) {
			Token next_token = Lexer_peek(self->lexer);
			if (next_token.type == Operator && String_equals_c(next_token.token, "("))
				return Parser_parse_capture(self);
			}
		return (ParseNode*) new_Variable(next_token.token, next_token.line_number);
		}

	else if (next_token.type == Operator) {
		if (String_equals_c(next_token.token, "["))
			return Parser_parse_array_literal(self);
		else if (String_equals_c(next_token.token, "{"))
			return Parser_parse_dict_literal(self);
		else if (String_equals_c(next_token.token, "(")) {
			int start_line_number = next_token.line_number;
			Lexer_next(self->lexer);
			ParseNode* expr = Parser_parse_expression(self);
			next_token = Lexer_next(self->lexer);
			if (next_token.type != Operator || !String_equals_c(next_token.token, ")"))
				Error("Missing \")\" on line %d.", start_line_number);
			return expr;
			}
		}

	return NULL;
}


static uint32_t parse_hex(const char** p_in_out, const char* end, int num_digits, int line_number)
{
	const char* p = *p_in_out;
	uint32_t value = 0;
	for (int i = 0; i < num_digits && p < end; ++i) {
		value <<= 4;
		uint8_t c = *p++;
		if (c >= '0' && c <= '9')
			value += c - '0';
		else if (c >= 'A' && c <= 'F')
			value += c - 'A' + 0x0A;
		else if (c >= 'a' && c <= 'f')
			value += c - 'a' + 0x0A;
		else
			Error("Bad \\x escape in line %d.", line_number);
		}
	*p_in_out = p;
	return value;
}

ParseNode* Parser_parse_string_literal(Parser* self)
{
	// Process interpolations and unescapes.

	Token token = Lexer_next(self->lexer);
	const char* p = token.token->str;
	const char* end = p + token.token->size;
	Array* segments = NULL;
	char* unescaped_segment = NULL;

	const char* segment_start = p;
	char* unescaped_out = NULL;
	while (true) {
		if (p >= end)
			break;

		else if (*p == '\\') {
			p += 1;
			if (unescaped_out == NULL) {
				if (unescaped_segment == NULL)
					unescaped_segment = alloc_mem(token.token->size);
				size_t size_so_far = p - segment_start - 1;
				memcpy(unescaped_segment, segment_start, size_so_far);
				unescaped_out = unescaped_segment + size_so_far;
				}
			char c = *p++;
			switch (c) {
				case 'n':
					*unescaped_out++ = '\n';
					break;
				case 't':
					*unescaped_out++ = '\t';
					break;
				case 'r':
					*unescaped_out++ = '\r';
					break;
				case 'e':
					*unescaped_out++ = 0x1B;
					break;
				case 'b':
					*unescaped_out++ = '\b';
					break;
				case 'a':
					*unescaped_out++ = '\a';
					break;
				case 'v':
					*unescaped_out++ = '\v';
					break;
				case 'f':
					*unescaped_out++ = '\f';
					break;
				case 'x':
					{
					const char* p_copy = p; 	// Let "p" stay in a register.
					uint32_t value = parse_hex(&p_copy, end, 2, token.line_number);
					p = p_copy;
					*unescaped_out++ = value;
					}
					break;
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
					{
					int value = c - '0';
					for (int i = 0; i < 2 && p < end; ++i) {
						value <<= 3;
						char c = *p++;
						if (c >= '0' && c <= '9')
							value += c - '0';
						else
							Error("Bad \\0 escape in line %d.", token.line_number);
						}
					}
					break;
				case 'u':
				case 'U':
					{
					const char* p_copy = p; 	// Let "p" stay in a register.
					uint32_t value = parse_hex(&p_copy, end, (c == 'u' ? 4 : 8), token.line_number);
					p = p_copy;
					if (value > 0x10FFFF)
						Error("Invalid unicode character (0x%Xd) on line %d.", value, token.line_number);
					unescaped_out += put_utf8(value, unescaped_out);
					}
					break;
				default:
					*unescaped_out++ = c;
					break;
				}
			}

		else if (*p == '{') {
			p += 1;
			if (p >= end)
				Error("Unterminated interpolated string in line %d.", token.line_number);
			else if (*p == '{') {
				// Escaping via "{{".
				p += 1;
				if (unescaped_out == NULL) {
					if (unescaped_segment == NULL)
						unescaped_segment = alloc_mem(token.token->size);
					size_t size_so_far = p - segment_start - 2;
					memcpy(unescaped_segment, segment_start, size_so_far);
					unescaped_out = unescaped_segment + size_so_far;
					}
				*unescaped_out++ = '{';
				}
			else {
				// Starting an interpolation.

				// Finish the current segment.
				if (segments == NULL)
					segments = new_Array();
				String* last_segment =
					(unescaped_out ?
					 new_String(unescaped_segment, unescaped_out - unescaped_segment) :
					 new_static_String(segment_start, p - segment_start - 1));
				if (last_segment->size > 0)
					Array_append(segments, (Object*) new_StringLiteralExpr(last_segment));
				unescaped_out = NULL;

				// Get the expression.
				const char* expression_start = p;
				int brace_level = 0;
				while (p < end) {
					char c = *p++;
					if (c == '{')
						brace_level += 1;
					else if (c == '}') {
						brace_level -= 1;
						if (brace_level <= 0)
							break;
						}
					}
				if (brace_level > 0)
					Error("Unterminated string interpolation in line %d.", token.line_number);

				// Parse the expression.
				Parser* parser = new_Parser(expression_start, p - expression_start - 1);
				Lexer_set_for_expression(parser->lexer);
				parser->lexer->line_number = token.line_number;
				ParseNode* expr = Parser_parse_expression(parser);
				if (expr)
					Array_append(segments, (Object*) expr);

				segment_start = p;
				}
			}

		else if (*p == '}' && unescaped_out) {
			p += 1;
			if (p < end && *p == '}')
				p += 1;
			*unescaped_out++ = '}';
			}

		else if (unescaped_out)
			*unescaped_out++ = *p++;
		else
			p += 1;
		}

	// Finish the last segment.
	String* last_segment =
		(unescaped_out ?
		 new_String(unescaped_segment, unescaped_out - unescaped_segment) :
		 new_static_String(segment_start, p - segment_start));

	if (segments == NULL)
		return (ParseNode*) new_StringLiteralExpr(last_segment);

	if (last_segment->size > 0)
		Array_append(segments, (Object*) new_StringLiteralExpr(last_segment));
	return (ParseNode*) new_InterpolatedStringLiteral(segments);
}


ParseNode* Parser_parse_array_literal(Parser* self)
{
	Lexer_next(self->lexer);
	ArrayLiteral* array_literal = new_ArrayLiteral();

	while (true) {
		Token next_token = Lexer_peek(self->lexer);
		if (next_token.type == Operator) {
			if (String_equals_c(next_token.token, "]")) {
				Lexer_next(self->lexer);
				break;
				}
			else if (String_equals_c(next_token.token, ",")) {
				Lexer_next(self->lexer);
				continue;
				}
			}

		ParseNode* item = Parser_parse_expression(self);
		if (item == NULL)
			Error("Expected expression in array literal in line %d.", next_token.line_number);
		ArrayLiteral_add_item(array_literal, item);
		}

	return (ParseNode*) array_literal;
}


ParseNode* Parser_parse_dict_literal(Parser* self)
{
	Lexer_next(self->lexer);
	DictLiteral* dict_literal = new_DictLiteral();

	while (true) {
		// Name.
		Token token = Lexer_next(self->lexer);
		if (token.type == Operator) {
			if (String_equals_c(token.token, "}"))
				break;
			else if (String_equals_c(token.token, ","))
				continue;
			else
				Error("Expected name in Dict literal in line %d.", token.line_number);
			}
		else if (token.type != Identifier)
			Error("Expected name in Dict literal in line %d.", token.line_number);
		String* name = token.token;

		// ":" or "=".
		token = Lexer_next(self->lexer);
		if (token.type != Operator || !(String_equals_c(token.token, ":") || String_equals_c(token.token, "=")))
			Error("Expected \":\" or \"=\" in Dict literal in line %d.", token.line_number);

		// Value.
		ParseNode* value = Parser_parse_expression(self);
		if (value == NULL)
			Error("Expected value expression in Dict literal in line %d.", token.line_number);

		// Add it.
		DictLiteral_add_item(dict_literal, name, value);
		}

	return (ParseNode*) dict_literal;
}


ParseNode* Parser_parse_super_call(Parser* self)
{
	// The "super" has already been consumed.

	// "."
	Token token = Lexer_next(self->lexer);
	if (token.type != Operator && !String_equals_c(token.token, "."))
		Error("Expected \".\" in \"super\" call on line %d.", token.line_number);

	// Name.
	String* name = Parser_parse_fn_name(self);

	SuperCallExpr* call = new_SuperCallExpr(name);

	// Arguments.
	token = Lexer_peek(self->lexer);
	if (token.type == Operator && String_equals_c(token.token, "(")) {
		Array* arguments = Parser_parse_arguments(self);
		call->arguments = arguments;
		}

	return (ParseNode*) call;
}


String* Parser_parse_fn_name(Parser* self)
{
	Token token = Lexer_next(self->lexer);
	String* name = token.token;
	bool can_be_set = true;
	if (token.type == Operator) {
		if (String_equals_c(token.token, "[")) {
			token = Lexer_next(self->lexer);
			if (token.type != Operator || !String_equals_c(token.token, "]"))
				Error("Expected \"[]\" as a function name, not just \"[\", on line %d.", token.line_number);
			name = new_c_static_String("[]");
			}
		else
			can_be_set = false;
		}
	else if (token.type != Identifier)
		Error("Expected a function name on line %d.", token.line_number);

	// Add "="?
	if (can_be_set) {
		token = Lexer_peek(self->lexer);
		if (token.type == Operator && String_equals_c(token.token, "=")) {
			name = String_add(name, token.token);
			Lexer_next(self->lexer);
			}
		}

	return name;
}


Array* Parser_parse_names_list(Parser* self, const char* type)
{
	Array* arguments = new_Array();

	Token token = Lexer_peek(self->lexer);
	if (token.type != Operator || !String_equals_c(token.token, "("))
		return arguments;

	Lexer_next(self->lexer); 	// Consume "(".
	while (true) {
		token = Lexer_next(self->lexer);
		if (token.type == Operator) {
			if (String_equals_c(token.token, ")"))
				break;
			else if (String_equals_c(token.token, ","))
				continue;
			}
		if (token.type != Identifier)
			Error("Expected %s name in line %d.", type, token.line_number);
		Array_append(arguments, (Object*) token.token);
		}

	return arguments;
}



