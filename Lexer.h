#pragma once

#include "String.h"
#include "Memory.h"
#include <stdlib.h>
#include <stdbool.h>


typedef struct Token {
	enum {
		EndOfText, EOL, Identifier, IntLiteral, FloatLiteral, StringLiteral, Operator, Indent, Unindent,
		} type;
	String* token;
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
	} Lexer;

extern Lexer* new_Lexer(const char* text, size_t size);
extern Token Lexer_next_token(struct Lexer* self);

