#include "RunStatement.h"
#include "ParseNode.h"
#include "Parser.h"
#include "Lexer.h"
#include "MethodBuilder.h"
#include "Environment.h"
#include "ByteCode.h"
#include "Array.h"
#include "Object.h"
#include "Memory.h"
#include "Error.h"


typedef struct RunStatement {
	ParseNode parse_node;
	Array* arguments;
	} RunStatement;
extern RunStatement* new_RunStatement(Array* arguments);


ParseNode* Parser_parse_run_statement(Parser* self)
{
	int line_number = Lexer_next(self->lexer).line_number;

	Array* arguments = new_Array();
	while (true) {
		Token token = Lexer_peek(self->lexer);
		if (token.type == EOL)
			break;
		if (token.type == StringLiteral)
			Array_append(arguments, (Object*) Parser_parse_string_literal(self));
		else if (token.type == Identifier || token.type == RawStringLiteral || token.type == IntLiteral) {
			Lexer_next(self->lexer);
			Array_append(arguments, (Object*) new_StringLiteralExpr(token.token));
			}

		else if (token.type == Operator) {
			// Coalesce "-" or "+" with the next identifier, "-", or "+".
			if (String_equals_c(token.token, "-") || String_equals_c(token.token, "+")) {
				String* arg = token.token;
				Lexer_next(self->lexer);
				while (true) {
					token = Lexer_peek(self->lexer);
					if (token.type == Identifier || token.type == IntLiteral || token.type == FloatLiteral) {
						arg = String_add(arg, token.token);
						Lexer_next(self->lexer);
						break;
						}
					else if (token.type == Operator) {
						if (String_equals_c(token.token, "-") || String_equals_c(token.token, "+")) {
							arg = String_add(arg, token.token);
							Lexer_next(self->lexer);
							}
						else
							break;
						}
					else
						break;
					}
				Array_append(arguments, (Object*) new_StringLiteralExpr(arg));
				}

			// Expression inside "{}" or "()".
			else if (String_equals_c(token.token, "{") || String_equals_c(token.token, "(")) {
				char start_char = token.token->str[0];
				Lexer_next(self->lexer);
				Array_append(arguments, (Object*) Parser_parse_expression(self));
				const char* end_char = (start_char == '{' ? "}" : ")");
				token = Lexer_next(self->lexer);
				if (token.type != Operator || !String_equals_c(token.token, end_char))
					Error("Missing \"%s\" on line %d.", end_char, token.line_number);
				}

			else {
				// Any other operator: just another argument.
				Lexer_next(self->lexer);
				Array_append(arguments, (Object*) new_StringLiteralExpr(token.token));
				}
			}
		}

	if (arguments->size == 0)
		Error("Empty \"$\" statement on line %d.", line_number);
	Token token = Lexer_next(self->lexer);
	if (token.type != EOL)
		Error("Extra characters at end of line %d.", token.line_number);

	return (ParseNode*) new_RunStatement(arguments);
}



int RunStatement_emit(ParseNode* super, MethodBuilder* method)
{
	RunStatement* self = (RunStatement*) super;

	// Allocate stack space for the new frame.
	int orig_locals =
		MethodBuilder_reserve_locals(
			method,
			frame_saved_area_size + 1 /* receiver's "self" */ + 2 /* arguments: "arguments" and "options" */);
	int args_start = orig_locals + frame_saved_area_size;

	// Get the "run" function.
	declare_static_string(run_string, "run");
	ParseNode* run_fn = global_environment.environment.find((Environment*) &global_environment, &run_string);
	int run_fn_loc = run_fn->emit(run_fn, method);

	// Create the "arguments" array.
	int arguments_loc = args_start + 1;
	MethodBuilder_add_bytecode(method, BC_NEW_ARRAY);
	MethodBuilder_add_bytecode(method, arguments_loc);
	int saved_locals = method->cur_num_variables;
	for (int i = 0; i < self->arguments->size; ++i) {
		ParseNode* arg = (ParseNode*) Array_at(self->arguments, i);
		int arg_loc = arg->emit(arg, method);
		MethodBuilder_add_bytecode(method, BC_ARRAY_APPEND_STRINGS);
		MethodBuilder_add_bytecode(method, arguments_loc);
		MethodBuilder_add_bytecode(method, arg_loc);
		method->cur_num_variables = saved_locals;
		}

	// Emit options.
	MethodBuilder_add_bytecode(method, BC_NIL);
	MethodBuilder_add_bytecode(method, args_start + 2);

	// Emit the function call to run().
	MethodBuilder_add_bytecode(method, BC_FN_CALL);
	MethodBuilder_add_bytecode(method, run_fn_loc);
	MethodBuilder_add_bytecode(method, 2);
	MethodBuilder_add_bytecode(method, args_start);

	method->cur_num_variables = orig_locals;
	return orig_locals;
}

void RunStatement_resolve_names(ParseNode* super, MethodBuilder* method)
{
	RunStatement* self = (RunStatement*) super;
	for (int i = 0; i < self->arguments->size; ++i) {
		ParseNode* arg = (ParseNode*) Array_at(self->arguments, i);
		if (arg->resolve_names)
			arg->resolve_names(arg, method);
		}
}

RunStatement* new_RunStatement(Array* arguments)
{
	RunStatement* run_statement = alloc_obj(RunStatement);
	run_statement->parse_node.emit = RunStatement_emit;
	run_statement->parse_node.resolve_names = RunStatement_resolve_names;
	run_statement->arguments = arguments;
	return run_statement;
}



