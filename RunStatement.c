#include "RunStatement.h"
#include "ParseNode.h"
#include "Parser.h"
#include "Lexer.h"
#include "MethodBuilder.h"
#include "Environment.h"
#include "ByteCode.h"
#include "Array.h"
#include "Object.h"
#include "Pipe.h"
#include "Memory.h"
#include "Error.h"


typedef struct RunCommand {
	ParseNode parse_node;
	Array* arguments;
	int in_pipe_loc, out_pipe_loc;
	} RunCommand;
extern RunCommand* new_RunCommand(Array* arguments);

typedef struct RunPipeline {
	ParseNode parse_node;
	Array* commands;
	} RunPipeline;
extern RunPipeline* new_RunPipeline();


RunCommand* Parser_parse_run_command(Parser* self)
{
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
			static const char* terminating_operators[] = { "&&", "||", "|", NULL };

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

			else if (String_is_one_of(token.token, terminating_operators)) {
				// These end a command.
				break;
				}

			else {
				// Any other operator: just another argument.
				Lexer_next(self->lexer);
				Array_append(arguments, (Object*) new_StringLiteralExpr(token.token));
				}
			}
		}

	return arguments->size == 0 ? NULL : new_RunCommand(arguments);
}

ParseNode* Parser_parse_run_pipeline(Parser* self)
{
	RunCommand* command = Parser_parse_run_command(self);
	if (command == NULL)
		return NULL;

	RunPipeline* pipeline = NULL;
	while (true) {
		Token token = Lexer_peek(self->lexer);
		if (token.type != Operator || !String_equals_c(token.token, "|"))
			break;
		Lexer_next(self->lexer);

		if (pipeline == NULL) {
			pipeline = new_RunPipeline();
			Array_append(pipeline->commands, (Object*) command);
			}
		command = Parser_parse_run_command(self);
		if (command == NULL)
			Error("Missing command after \"|\" on line %d.", token.line_number);
		Array_append(pipeline->commands, (Object*) command);
		}

	return pipeline ? (ParseNode*) pipeline : (ParseNode*) command;
}

ParseNode* Parser_parse_run_statement(Parser* self)
{
	int line_number = Lexer_next(self->lexer).line_number;
	declare_static_string(ok_string, "ok");

	ParseNode* statement = Parser_parse_run_pipeline(self);
	if (statement == NULL)
		Error("Empty \"$\" statement on line %d.", line_number);

	while (true) {
		Token token = Lexer_peek(self->lexer);
		if (token.type != Operator)
			break;

		if (String_equals_c(token.token, "&&") || String_equals_c(token.token, "||")) {
			int line_number = Lexer_next(self->lexer).line_number;
			ParseNode* command_2 = Parser_parse_run_pipeline(self);
			if (command_2 == NULL)
				Error("Empty command after \"&&\"  on line %d.", line_number);
			if (statement->type == PN_RunCommand || statement->type == PN_RunPipeline)
				statement = (ParseNode*) new_CallExpr(statement, &ok_string);
			statement =
				(ParseNode*) new_ShortCircuitExpr(
					statement,
					(ParseNode*) new_CallExpr(command_2, &ok_string),
					token.token->str[0] == '&');
			}

		else
			break;
		}

	Token token = Lexer_next(self->lexer);
	if (token.type != EOL)
		Error("Extra characters at end of line %d.", token.line_number);

	return statement;
}



int RunCommand_emit(ParseNode* super, MethodBuilder* method)
{
	RunCommand* self = (RunCommand*) super;

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
	if (self->in_pipe_loc || self->out_pipe_loc) {
		int options_loc = args_start + 2;
		MethodBuilder_add_bytecode(method, BC_NEW_DICT);
		MethodBuilder_add_bytecode(method, options_loc);
		if (self->in_pipe_loc) {
			declare_static_string(stdin_pipe_string, "stdin-pipe");
			int key_loc = -MethodBuilder_add_literal(method, (Object*) &stdin_pipe_string) - 1;
			MethodBuilder_add_bytecode(method, BC_DICT_ADD);
			MethodBuilder_add_bytecode(method, options_loc);
			MethodBuilder_add_bytecode(method, key_loc);
			MethodBuilder_add_bytecode(method, self->in_pipe_loc);
			}
		if (self->out_pipe_loc) {
			declare_static_string(stdout_pipe_string, "stdout-pipe");
			int key_loc = -MethodBuilder_add_literal(method, (Object*) &stdout_pipe_string) - 1;
			MethodBuilder_add_bytecode(method, BC_DICT_ADD);
			MethodBuilder_add_bytecode(method, options_loc);
			MethodBuilder_add_bytecode(method, key_loc);
			MethodBuilder_add_bytecode(method, self->out_pipe_loc);
			}
		}
	else {
		MethodBuilder_add_bytecode(method, BC_NIL);
		MethodBuilder_add_bytecode(method, args_start + 2);
		}

	// Emit the function call to run().
	MethodBuilder_add_bytecode(method, BC_FN_CALL);
	MethodBuilder_add_bytecode(method, run_fn_loc);
	MethodBuilder_add_bytecode(method, 2);
	MethodBuilder_add_bytecode(method, args_start);

	method->cur_num_variables = orig_locals + 1;
	return orig_locals;
}

void RunCommand_resolve_names(ParseNode* super, MethodBuilder* method)
{
	RunCommand* self = (RunCommand*) super;
	for (int i = 0; i < self->arguments->size; ++i) {
		ParseNode* arg = (ParseNode*) Array_at(self->arguments, i);
		if (arg->resolve_names)
			arg->resolve_names(arg, method);
		}
}

RunCommand* new_RunCommand(Array* arguments)
{
	RunCommand* run_statement = alloc_obj(RunCommand);
	run_statement->parse_node.type = PN_RunCommand;
	run_statement->parse_node.emit = RunCommand_emit;
	run_statement->parse_node.resolve_names = RunCommand_resolve_names;
	run_statement->arguments = arguments;
	return run_statement;
}


int RunPipeline_emit(ParseNode* super, MethodBuilder* method)
{
	RunPipeline* self = (RunPipeline*) super;

	int result_loc = MethodBuilder_reserve_locals(method, 1);

	// Get the Pipe class.
	ParseNode* pipe_global = (ParseNode*) new_GlobalExpr((Object*) &Pipe_class);
	int pipe_class_loc = pipe_global->emit(pipe_global, method);

	// Make the pipes.
	int pipes_locs = method->cur_num_variables;
	for (int i = 0; i < self->commands->size - 1; ++i) {
		// Emit call to Pipe().
		int result_loc =
			MethodBuilder_reserve_locals(
				method,
				frame_saved_area_size + 1 /* receiver's "self" (Pipe) */);
		int args_start = result_loc + frame_saved_area_size;
		MethodBuilder_add_bytecode(method, BC_FN_CALL);
		MethodBuilder_add_bytecode(method, pipe_class_loc);
		MethodBuilder_add_bytecode(method, 0);
		MethodBuilder_add_bytecode(method, args_start);

		// Leave the result where it is.
		method->cur_num_variables = result_loc + 1;
		}

	// Emit the RunCommands.
	int save_locals = method->cur_num_variables;
	for (int i = 0; i < self->commands->size; ++i) {
		RunCommand* command = (RunCommand*) Array_at(self->commands, i);
		if (i != 0)
			command->in_pipe_loc = pipes_locs + i - 1;
		bool is_last_command = (i == self->commands->size - 1);
		if (!is_last_command)
			command->out_pipe_loc = pipes_locs + i;

		int command_result_loc = command->parse_node.emit(&command->parse_node, method);

		if (is_last_command) {
			MethodBuilder_add_bytecode(method, BC_SET_LOCAL);
			MethodBuilder_add_bytecode(method, command_result_loc);
			MethodBuilder_add_bytecode(method, result_loc);
			}
		method->cur_num_variables = save_locals;
		}

	// Return the result of the last RunCommand.
	method->cur_num_variables = result_loc + 1;
	return result_loc;
}

void RunPipeline_resolve_names(ParseNode* super, MethodBuilder* method)
{
	RunPipeline* self = (RunPipeline*) super;
	for (int i = 0; i < self->commands->size; ++i) {
		ParseNode* command = (ParseNode*) Array_at(self->commands, i);
		command->resolve_names(command, method);
		}
}

RunPipeline* new_RunPipeline()
{
	RunPipeline* self = alloc_obj(RunPipeline);
	self->parse_node.emit = RunPipeline_emit;
	self->parse_node.resolve_names = RunPipeline_resolve_names;
	self->commands = new_Array();
	return self;
}



