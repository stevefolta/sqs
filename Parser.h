#pragma once

#include <stddef.h>

struct Lexer;
struct ParseNode;
struct Block;

typedef struct Parser {
	struct Lexer* lexer;
	struct Block* inner_block;
	} Parser;


extern Parser* new_Parser(const char* text, size_t size);
extern struct ParseNode* Parser_parse_block(Parser* self);



