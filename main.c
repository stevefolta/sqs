#include "Lexer.h"
#include "Parser.h"
#include "ParseNode.h"
#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "String.h"
#include "Memory.h"
#include "Init.h"
#include "ByteCode.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


static void lexer_test(const char* file_path)
{
	// Read the test file.
	FILE* file = fopen(file_path, "r");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open \"%s\" (%s).", file_path, strerror(errno));
		return;
		}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	char* text = (char*) alloc_mem(size);
	size_t bytes_read = fread(text, 1, size, file);
	fclose(file);

	Lexer* lexer = new_Lexer(text, bytes_read);
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

static void compile_test(const char* file_path)
{
	// Read the test file.
	FILE* file = fopen(file_path, "r");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open \"%s\" (%s).", file_path, strerror(errno));
		return;
		}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	char* text = (char*) alloc_mem(size);
	size_t bytes_read = fread(text, 1, size, file);
	fclose(file);

	Parser* parser = new_Parser(text, bytes_read);
	ParseNode* ast = Parser_parse_block(parser);
	MethodBuilder* method_builder = new_MethodBuilder(0);
	ast->emit(ast, method_builder);
	MethodBuilder_add_bytecode(method_builder, BC_TERMINATE);
	MethodBuilder_finish(method_builder);
	Method_dump(method_builder->method);
	interpret_bytecode(method_builder->method);
}


int main(int argc, char* argv[])
{
	// Set up.
	init_all();

	if (argc < 2)
		return 1;
	compile_test(argv[1]);

	return 0;
}

