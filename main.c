#include "Lexer.h"
#include "Parser.h"
#include "ParseNode.h"
#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "Module.h"
#include "Init.h"
#include "ByteCode.h"
#include "Object.h"
#include "String.h"
#include "Array.h"
#include "Int.h"
#include "File.h"
#include "Memory.h"
#include "Error.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


static void lexer_test(const char* file_path)
{
	// Read the test file.
	String* contents = file_contents(file_path);
	if (contents == NULL) {
		fprintf(stderr, "Couldn't open \"%s\" (%s).", file_path, strerror(errno));
		return;
		}

	Lexer* lexer = new_Lexer(contents->str, contents->size);
	while (true) {
		Token token = Lexer_next(lexer);
		if (token.type == EndOfText)
			break;
		switch (token.type) {
			case Identifier:
			case IntLiteral:
			case FloatLiteral:
			case Operator:
				fwrite(token.token->str, token.token->size, 1, stdout);
				printf("\n");
				break;
			case RawStringLiteral:
				printf("r");
			case StringLiteral:
				printf("\"");
				fwrite(token.token->str, token.token->size, 1, stdout);
				printf("\"\n");
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
	// Read the file.
	String* contents = file_contents(file_path);
	if (contents == NULL) {
		fprintf(stderr, "Couldn't open \"%s\" (%s).", file_path, strerror(errno));
		return NULL;
		}

	Parser* parser = new_Parser(contents->str, contents->size);
	ParseNode* ast = Parser_parse_block(parser, NULL);
	MethodBuilder* method_builder = new_MethodBuilder(new_Array(), NULL);
	ast->emit(ast, method_builder);
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
	bool test_lexer = false;
	int first_arg = 1;
	while (first_arg < argc) {
		const char* arg = argv[first_arg];
		if (arg[0] == '-') {
			if (arg[1] == 'd')
				dump_requested = true;
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

	// Set up argv.
	Array* argv_array = new_Array();
	for (int i = first_arg; i < argc; ++i)
		Array_append(argv_array, (Object*) new_c_static_String(argv[i]));
	GlobalEnvironment_add_c("argv", (Object*) argv_array);

	// Compile and run the script.
	Method* method = compile_script(argv[first_arg]);
	if (method) {
		if (dump_requested)
			dump_bytecode(method, NULL, new_c_static_String("main"));
		Object* result = call_method(method, NULL);
		if (result && result->class_ == &Int_class)
			return Int_value(result);
		}

	return EXIT_SUCCESS;
}

