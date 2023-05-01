#pragma once

#include "String.h"
#include <stdbool.h>


typedef struct Token {
	enum {
		EndOfText, EOL, Identifier, IntLiteral, FloatLiteral, StringLiteral, Operator, Indent, Unindent,
		} type;
	String* token;
	size_t line_number;
	} Token;

typedef struct Lexer {
	const char* p;
	const char* end;
	bool at_line_start;
	size_t paren_level;
	size_t line_number;
	size_t* indent_stack;
	size_t indent_stack_size;
	int unindent_to; 	// -1: no unindents pending
	Token peeked_token;
	bool have_peeked_token;
	} Lexer;

extern Lexer* new_Lexer(const char* text, size_t size);
extern Token Lexer_peek(Lexer* self);
extern Token Lexer_next(Lexer* self);
extern Token Lexer_next_token(Lexer* self);

