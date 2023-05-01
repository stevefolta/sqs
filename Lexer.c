#include "Lexer.h"
#include "Error.h"
#include <stdbool.h>

#define indent_stack_max 64


extern void Lexer_init(Lexer* self, const char* text, size_t size);
extern Token Lexer_next_token(struct Lexer* self);
extern bool Lexer_skip_comment(struct Lexer* self);


Lexer* new_Lexer(const char* text, size_t size)
{
	Lexer* lexer = (Lexer*) alloc_mem(sizeof(Lexer));
	lexer->init = Lexer_init;
	lexer->init(lexer, text, size);
	return lexer;
}

void Lexer_init(Lexer* self, const char* text, size_t size)
{
	self->next_token = Lexer_next_token;
	self->skip_comment = Lexer_skip_comment;

	self->p = text;
	self->end = text + size;
	self->at_line_start = true;
	self->paren_level = 0;
	self->line_number = 0;
	self->indent_stack = (size_t*) alloc_mem(indent_stack_max * sizeof(size_t));
	self->indent_stack_size = 0;
	self->unindent_to = -1;
}


Token Lexer_next_token(struct Lexer* self)
{
	Token result = { EndOfText, NULL };

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

		// Get the indentation.
		size_t indentation = 0;
		while (self->p < self->end) {
			char c = *self->p;
			if (c != '\t' && c != ' ')
				break;
			self->p += 1;
			indentation += 1;
			}

		// Comment?
		if (self->skip_comment(self)) {
			result.type = EOL;
			return result;
			}
		// EOL?
		if (*self->p == '\n') {
			self->p += 1;
			result.type = EOL;
			self->line_number += 1;
			self->at_line_start = true;
			return result;
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
		}
	else {
		while (true) {
			// Skip whitespace since the previous token.
			char c = *self->p;
			if (c != ' ' && c != '\t' && c != '\r' && !(c == '\n' && self->paren_level > 0))
				break;
			self->p += 1;
			if (self->p >= self->end)
				return result;
			}

		// Comment?
		if (self->skip_comment(self)) {
			if (self->paren_level == 0) {
				result.type = EOL;
				return result;
				}
			}
		}

	const char* token_start = self->p;
	char c = *self->p++;
	switch (c) {
		case '\n':
			self->line_number += 1;
			self->at_line_start = true;
			result.type = EOL;
			break;

		case '(':
		case '[':
		case '{':
			result.type = Operator;
			result.token = new_String(token_start, self->p - token_start);
			self->paren_level += 1;
			break;
		case ')':
		case ']':
		case '}':
			result.type = Operator;
			result.token = new_String(token_start, self->p - token_start);
			self->paren_level -= 1;
			break;

		case '=':
		case '!':
			// Possibly followed by '='.
			if (self->p < self->end) {
				c = *self->p;
				if (c == '=')
					self->p += 1;
				}
			result.type = Operator;
			result.token = new_String(token_start, self->p - token_start);
			break;

		default:
			// Identifier.
			while (self->p < self->end) {
				c = *self->p;
				if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || (c & 0x80) != 0) {
					// Good identifier character, keep going.
					self->p += 1;
					}
				else
					break;
				}
			result.type = Identifier;
			result.token = new_String(token_start, self->p - token_start);
			break;
		}

	return result;
}


bool Lexer_skip_comment(struct Lexer* self)
{
	if (self->p < self->end && *self->p == '#') {
		while (true) {
			self->p += 1;
			if (self->p >= self->end)
				return true;
			if (*self->p++ == '\n')
				break;
			}
		self->line_number += 1;
		self->at_line_start = true;
		return true;
		}

	return false;
}



