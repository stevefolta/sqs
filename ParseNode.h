#pragma once

struct ByteArray;
struct Array;

typedef struct ParseNode {
	int (*emit_method)(struct ParseNode* self, struct ByteArray* bytecode);
		// Returns register containing the result (if an expression).
	} ParseNode;


typedef struct Block {
	ParseNode parse_node;
	struct Array* statements;
	} Block;
extern Block* new_Block();
extern void Block_append(Block* self, ParseNode* statement);


typedef struct IfStatement {
	ParseNode parse_node;
	ParseNode* condition;
	ParseNode* if_block;
	ParseNode* else_block;
	} IfStatement;
extern IfStatement* new_IfStatement();



