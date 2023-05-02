#pragma once

struct MethodBuilder;
struct Array;
struct String;

typedef struct ParseNode {
	int (*emit)(struct ParseNode* self, struct MethodBuilder* method);
		// Returns register containing the result (if an expression).
	int (*emit_set)(struct ParseNode* self, struct ParseNode* value, struct MethodBuilder* method);
		// Emits setting an lvalue.  Won't exist for non-lvalue nodes.
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


typedef struct SetExpr {
	ParseNode parse_node;
	ParseNode* left;
	ParseNode* right;
	} SetExpr;
extern SetExpr* new_SetExpr();


typedef struct ShortCircuitOrExpr {
	ParseNode parse_node;
	ParseNode* expr1;
	ParseNode* expr2;
	} ShortCircuitOrExpr;
extern ShortCircuitOrExpr* new_ShortCircutOrExpr(ParseNode* expr1, ParseNode* expr2);

typedef struct ShortCircuitAndExpr {
	ParseNode parse_node;
	ParseNode* expr1;
	ParseNode* expr2;
	} ShortCircuitAndExpr;
extern ShortCircuitAndExpr* new_ShortCircutAndExpr(ParseNode* expr1, ParseNode* expr2);


typedef struct StringLiteralExpr {
	ParseNode parse_node;
	struct String* str;
	} StringLiteralExpr;
extern StringLiteralExpr* new_StringLiteralExpr(struct String* str);

typedef struct GlobalExpr {
	ParseNode parse_node;
	struct String* name;
	} GlobalExpr;
extern GlobalExpr* new_GlobalExpr(struct String* name);

