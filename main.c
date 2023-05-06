#include "Lexer.h"
#include "Parser.h"
#include "ParseNode.h"
#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "Init.h"
#include "ByteCode.h"
#include "String.h"
#include "Array.h"
#include "Memory.h"
#include "Error.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


static String* file_contents(const char* file_path)
{
	FILE* file = fopen(file_path, "r");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open \"%s\" (%s).", file_path, strerror(errno));
		return NULL;
		}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	char* text = (char*) alloc_mem(size);
	size_t bytes_read = fread(text, 1, size, file);
	fclose(file);
	return new_static_String(text, bytes_read);
}

static void lexer_test(const char* file_path)
{
	// Read the test file.
	String* contents = file_contents(file_path);
	if (contents == NULL)
		return;

	Lexer* lexer = new_Lexer(contents->str, contents->size);
	while (true) {
		Token token = Lexer_next(lexer);
		if (token.type == EndOfText)
			break;
		switch (token.type) {
			case Identifier:
			case IntLiteral:
			case FloatLiteral:
			case StringLiteral:
			case Operator:
				fwrite(token.token->str, token.token->size, 1, stdout);
				printf("\n");
				break;
			case EOL:
				printf("-eol-\n");
				break;
			case Indent:
				printf("-indent-\n");
				break;
			case Unindent:
				printf("-unindent-\n");
			case EndOfText:
				break;
			}
		}
}

static Method* compile_script(const char* file_path)
{
	// Read the test file.
	String* contents = file_contents(file_path);
	if (contents == NULL)
		return NULL;

	Parser* parser = new_Parser(contents->str, contents->size);
	ParseNode* ast = Parser_parse_block(parser);
	MethodBuilder* method_builder = new_MethodBuilder(new_Array(), NULL);
	ast->emit(ast, method_builder);
	MethodBuilder_add_bytecode(method_builder, BC_TERMINATE);
	MethodBuilder_finish(method_builder);
	return method_builder->method;
}


int main(int argc, char* argv[])
{
	// Set up.
	init_all();

	if (argc < 2)
		return 1;

	// Parse the initial arguments.
	bool dump = false;
	bool test_lexer = false;
	int first_arg = 1;
	while (first_arg < argc) {
		const char* arg = argv[first_arg];
		if (arg[0] == '-') {
			if (arg[1] == 'd')
				dump = true;
			else if (arg[1] == 'l')
				test_lexer = true;
			else
				Error("Unknown argument: %s", arg);
			first_arg += 1;
			}
		else
			break;
		}

	if (test_lexer) {
		lexer_test(argv[first_arg]);
		return 0;
		}

	// Compile and run the script.
	Method* method = compile_script(argv[first_arg]);
	if (method) {
		if (dump)
			Method_dump(method);
		interpret_bytecode(method);
		}

	return 0;
}

