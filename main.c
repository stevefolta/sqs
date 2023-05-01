#include "Lexer.h"
#include "Boolean.h"
#include "String.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char* test_text = "\
hello there 	# comment\n\
\n\
# comment\n\
+ 3\n\
if foo == bar\n\
	baz = 12\n\
	for quux in quixm\n\
		print(quux)\n\
\n\
foo = [\n\
	bar\n\
	baz\n\
	quux\n\
	]\n\
";

int main(int argc, char* argv[])
{
	true_obj.class_ = NULL; 	// TODO
	false_obj.class_ = NULL; 	// TODO

	Lexer* lexer = new_Lexer(test_text, strlen(test_text));
	while (true) {
		Token token = lexer->next_token(lexer);
		if (token.type == EndOfText)
			break;
		switch (token.type) {
			case Identifier:
			case Number:
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
	return 0;
}

