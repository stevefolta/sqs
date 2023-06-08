#include "Lexer.h"
#include "Memory.h"
#include "Error.h"
#include <stdlib.h>

#define indent_stack_max 64


extern void Lexer_init(Lexer* self, const char* text, size_t size);
extern bool Lexer_skip_comment(struct Lexer* self);


Lexer* new_Lexer(const char* text, size_t size)
{
	Lexer* lexer = (Lexer*) alloc_mem(sizeof(Lexer));
	Lexer_init(lexer, text, size);
	return lexer;
}

void Lexer_init(Lexer* self, const char* text, size_t size)
{
	self->p = text;
	self->end = text + size;
	self->at_line_start = true;
	self->paren_level = 0;
	self->line_number = 1;
	self->indent_stack = (size_t*) alloc_mem(indent_stack_max * sizeof(size_t));
	self->indent_stack_size = 0;
	self->unindent_to = -1;
	self->have_peeked_token = false;
}


Token Lexer_peek(Lexer* self)
{
	if (!self->have_peeked_token) {
		self->peeked_token = Lexer_next_token(self);
		self->have_peeked_token = true;
		}
	return self->peeked_token;
}


Token Lexer_next(Lexer* self)
{
	if (self->have_peeked_token) {
		self->have_peeked_token = false;
		return self->peeked_token;
		}
	return Lexer_next_token(self);
}


static bool is_identifier_character(char c)
{
	return
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		(c >= '0' && c <= '9') ||
		c == '-' ||
		c == '_' ||
		(c & 0x80) != 0;
}


Token Lexer_next_token(struct Lexer* self)
{
	Token result = { EndOfText, NULL, self->line_number };

	// End of text.
	if (self->p >= self->end) {
		if (self->indent_stack_size > 0) {
			self->unindent_to = 0;
			}
		else
			return result;
		}

	if (self->unindent_to >= 0) {
		if (self->indent_stack_size == 0 || self->unindent_to == self->indent_stack[self->indent_stack_size - 1]) {
			// We're done unindenting.
			self->unindent_to = -1;
			}
		else {
			// Unindent one more level.
			self->indent_stack_size -= 1;
			result.type = Unindent;
			return result;
			}
		}

	if (self->at_line_start && self->paren_level == 0) {
		self->at_line_start = false;

		// Consume blank lines before looking at indentation.
		while (true) {
			if (self->p >= self->end)
				return result;
			if (*self->p != '\n')
				break;
			self->p += 1;
			self->line_number += 1;
			}

		// Get the indentation.
		size_t indentation = 0;
		while (self->p < self->end) {
			char c = *self->p;
			if (c != '\t' && c != ' ')
				break;
			self->p += 1;
			indentation += 1;
			}

		// Figure out the indentation change.
		if ((indentation > 0 && self->indent_stack_size == 0) || indentation > self->indent_stack[self->indent_stack_size - 1]) {
			self->indent_stack[self->indent_stack_size] = indentation;
			self->indent_stack_size += 1;
			if (self->indent_stack_size > indent_stack_max)
				Error("Too much indentation.");
			result.type = Indent;
			return result;
			}
		else if (indentation < self->indent_stack[self->indent_stack_size - 1]) {
			self->unindent_to = indentation;
			self->indent_stack_size -= 1;
			result.type = Unindent;
			return result;
			}

		// Comment?
		if (Lexer_skip_comment(self)) {
			result.type = EOL;
			return result;
			}
		// EOL?
		if (*self->p == '\n') {
			self->p += 1;
			self->line_number += 1;
			self->at_line_start = true;
			result.type = EOL;
			return result;
			}
		}
	else {
		while (true) {
			// Skip whitespace since the previous token.
			char c = *self->p;
			if (c == '\n') {
				if (self->paren_level > 0)
					self->line_number += 1;
				else
					break;
				}
			else if (c != ' ' && c != '\t' && c != '\r')
				break;
			self->p += 1;
			if (self->p >= self->end)
				return result;
			}

		// Comment?
		if (Lexer_skip_comment(self)) {
			if (self->paren_level == 0) {
				result.type = EOL;
				return result;
				}
			}
		}

	const char* token_start = self->p;
	char c = *self->p++;
	bool string_is_raw = false;
	switch (c) {
		case '\n':
			self->line_number += 1;
			self->at_line_start = true;
			result.type = EOL;
			return result;
			break;

		case '(':
		case '[':
		case '{':
			result.type = Operator;
			self->paren_level += 1;
			break;
		case ')':
		case ']':
		case '}':
			result.type = Operator;
			self->paren_level -= 1;
			break;

		case '.':
		case ',':
		case ':':
		case '~':
			result.type = Operator;
			break;

		case '=':
		case '!':
		case '+':
		case '*':
		case '/':
		case '%':
		case '^':
			// Possibly followed by '='.
			if (self->p < self->end) {
				c = *self->p;
				if (c == '=')
					self->p += 1;
				}
			result.type = Operator;
			break;

		case '-':
			// Possibly followed by '=', or the start of a number.
			if (self->p < self->end) {
				c = *self->p;
				if (c == '=')
					self->p += 1;
				else if (c >= '0' && c <= '9')
					goto number;
				}
			result.type = Operator;
			break;

		case '<':
		case '>':
			// Could be doubled, then possibly followed by '='.
			if (self->p < self->end && *self->p == c) {
				self->p += 1;
				}
			if (self->p < self->end && *self->p == '=')
				self->p += 1;
			result.type = Operator;
			break;

		case '&':
		case '|':
			// Could be doubled, or followed by '='.
			if (self->p < self->end) {
				char next_c = *self->p;
				if (next_c == c || next_c == '=')
					self->p += 1;
				}
			result.type = Operator;
			break;

		case '$':
			result.type = Identifier;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		number:
			{
			bool is_float = false;
			if (c == '0' && self->p < self->end && *self->p == 'x') {
				self->p += 1;
				while (self->p < self->end) {
					c = *self->p;
					if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
						// Keep going.
						}
					else
						break;
					self->p += 1;
					}
				}
			else {
				while (self->p < self->end) {
					c = *self->p;
					if (c == '.') {
						if (is_float)
							break;
						is_float = true;
						}
					else if (c == 'e' || c == 'E') {
						is_float = true;
						self->p += 1;
						if (self->p < self->end) {
							c = *self->p;
							if (c == '+' || c == '-')
								self->p += 1;
							}
						}
					else if (c >= '0' && c <= '9') {
						// Keep going.
						}
					else
						break;
					self->p += 1;
					}
				}
			result.type = (is_float ? FloatLiteral : IntLiteral);
			}
			break;

		case '"':
		case '\'':
		case '`':
		string_literal:
			{
			char delimiter = c;
			size_t start_line = self->line_number;
			token_start += 1;
			while (self->p < self->end) {
				c = *self->p;
				if (c == delimiter) {
					result.type = (string_is_raw ? RawStringLiteral : StringLiteral);
					result.token = new_String(token_start, self->p - token_start);
					self->p += 1;
					return result;
					}
				self->p += 1;
				if (c == '\\')
					self->p += 1;
				else if (c == '\n')
					self->line_number += 1;
				}
			Error("Unterminated string starting at line %d.", start_line);
			}
			break;

		case 'r':
			if (self->p < self->end) {
				char next_c = *self->p;
				if (next_c == '"' || next_c == '\'' || next_c == '`') {
					string_is_raw = true;
					token_start = self->p;
					c = *self->p++;
					goto string_literal;
					}
				}
			goto identifier;
			break;

		default:
		identifier:
			// Identifier.
			if (!is_identifier_character(c))
				Error("Unknown character");
			while (self->p < self->end) {
				c = *self->p;
				if (is_identifier_character(c)) {
					// Good identifier character, keep going.
					self->p += 1;
					}
				else
					break;
				}
			result.type = Identifier;
			break;
		}

	result.token = new_String(token_start, self->p - token_start);
	return result;
}


bool Lexer_skip_comment(struct Lexer* self)
{
	if (self->p < self->end && *self->p == '#') {
		while (true) {
			self->p += 1;
			if (self->p >= self->end)
				return true;
			if (*self->p == '\n')
				break;
			}
		self->p += 1;
		self->line_number += 1;
		self->at_line_start = true;
		return true;
		}

	return false;
}


void Lexer_set_for_expression(Lexer* self)
{
	self->at_line_start = false;
}



