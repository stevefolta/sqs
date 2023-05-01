#pragma once

struct MethodBuilder;
struct Array;
struct String;

typedef struct ParseNode {
	int (*emit_method)(struct ParseNode* self, struct MethodBuilder* method);
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


typedef struct WhileStatement {
	ParseNode parse_node;
	ParseNode* condition;
	ParseNode* body;
	} WhileStatement;
extern WhileStatement* new_WhileStatement();


typedef struct ForStatement {
	ParseNode parse_node;
	struct String* variable_name;
	ParseNode* collection;
	ParseNode* body;
	} ForStatement;
extern ForStatement* new_ForStatement();



