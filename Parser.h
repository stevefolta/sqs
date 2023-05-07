#pragma once

#include <stddef.h>

struct Lexer;
struct ParseNode;
struct Block;
struct FunctionStatement;
struct String;
struct Array;


typedef struct Parser {
	struct Lexer* lexer;
	struct Block* inner_block;
	} Parser;


extern Parser* new_Parser(const char* text, size_t size);

extern struct ParseNode* Parser_parse_block(Parser* self);
extern struct FunctionStatement* Parser_parse_fn_statement_raw(Parser* self);

extern struct String* Parser_parse_fn_name(Parser* self);
extern struct Array* Parser_parse_names_list(Parser* self, const char* type);



