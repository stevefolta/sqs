#pragma once

#include <stddef.h>
#include <stdbool.h>

struct Lexer;
struct ParseNode;
struct Block;
struct FunctionStatement;
struct Module;
struct String;
struct Array;


typedef struct Parser {
	struct Lexer* lexer;
	struct Block* inner_block;
	struct String* filename;
	} Parser;


extern Parser* new_Parser(const char* text, size_t size, struct String* filename);

extern struct ParseNode* Parser_parse_block(Parser* self, struct Module* module);
extern struct ParseNode* Parser_parse_fn_statement(Parser* self);
extern struct FunctionStatement* Parser_parse_fn_statement_raw(Parser* self);
extern struct ParseNode* Parser_parse_expression(Parser* self);
extern struct ParseNode* Parser_parse_string_literal(Parser* self);

extern struct String* Parser_parse_fn_name(Parser* self);
extern struct Array* Parser_parse_names_list(Parser* self, const char* type);

extern bool String_is_one_of(struct String* str, const char** values);

