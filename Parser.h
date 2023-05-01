#pragma once

#include <stddef.h>

struct Lexer;

typedef struct Parser {
	struct Lexer* lexer;
	} Parser;


extern Parser* new_Parser(const char* text, size_t size);



