#pragma once

#include <stddef.h>

struct Lexer;
struct ParseNode;

typedef struct Parser {
	struct Lexer* lexer;
	} Parser;


extern Parser* new_Parser(const char* text, size_t size);
extern struct ParseNode* Parser_parse_block(Parser* self);



