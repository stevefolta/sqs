#include "Lexer.h"
#include "Boolean.h"
#include "String.h"
#include "Memory.h"
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

int main(int argc, char* argv[])
{
	true_obj.class_ = NULL; 	// TODO
	false_obj.class_ = NULL; 	// TODO

	if (argc < 2)
		return 1;
	lexer_test(argv[1]);

	return 0;
}

