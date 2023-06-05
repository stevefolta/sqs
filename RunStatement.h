#pragma once

struct ParseNode;
struct Parser;


extern struct ParseNode* Parser_parse_run_statement(struct Parser* self);
extern struct ParseNode* Parser_parse_capture(struct Parser* self);


