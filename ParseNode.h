#pragma once

#include <stdbool.h>

struct MethodBuilder;
struct Array;
struct Dict;
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
	struct Dict* locals;
	int locals_base;
	} Block;
extern Block* new_Block();
extern void Block_append(Block* self, ParseNode* statement);
extern ParseNode* Block_get_local(Block* self, struct String* name);
extern ParseNode* Block_autodeclare(Block* self, struct String* name);


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


typedef struct ExpressionStatement {
	ParseNode parse_node;
	ParseNode* expression;
	} ExpressionStatement;
extern ExpressionStatement* new_ExpressionStatement(ParseNode* expression);


typedef struct SetExpr {
	ParseNode parse_node;
	ParseNode* left;
	ParseNode* right;
	} SetExpr;
extern SetExpr* new_SetExpr();


typedef struct ShortCircuitExpr {
	ParseNode parse_node;
	ParseNode* expr1;
	ParseNode* expr2;
	bool is_and;
	} ShortCircuitExpr;
extern ShortCircuitExpr* new_ShortCircuitExpr(ParseNode* expr1, ParseNode* expr2, bool is_and);


typedef struct StringLiteralExpr {
	ParseNode parse_node;
	struct String* str;
	} StringLiteralExpr;
extern StringLiteralExpr* new_StringLiteralExpr(struct String* str);

typedef struct BooleanLiteral {
	ParseNode parse_node;
	bool value;
	} BooleanLiteral;
extern BooleanLiteral* new_BooleanLiteral(bool value);
extern ParseNode* new_NilLiteral();

typedef struct GlobalExpr {
	ParseNode parse_node;
	struct Object* object;
	} GlobalExpr;
extern GlobalExpr* new_GlobalExpr(struct Object* object);

typedef struct Variable {
	ParseNode parse_node;
	struct String* name;
	int line_number;
	ParseNode* resolved;
	} Variable;
extern Variable* new_Variable(struct String* name, int line_number);

typedef struct Local {
	ParseNode parse_node;
	Block* block;
	int block_index;
	} Local;
extern Local* new_Local(Block* block, int block_index);

typedef struct CallExpr {
	ParseNode parse_node;
	struct ParseNode* receiver;
	struct String* name;
	struct Array* arguments;
	} CallExpr;
extern CallExpr* new_CallExpr(ParseNode* receiver, struct String* name);
extern CallExpr* new_CallExpr_binop(ParseNode* receiver, ParseNode* arg, struct String* name);
extern void CallExpr_add_argument(CallExpr* self, ParseNode* arg);

typedef struct FunctionCallExpr {
	ParseNode parse_node;
	ParseNode* fn;
	struct Array* arguments;
	} FunctionCallExpr;
extern FunctionCallExpr* new_FunctionCallExpr(ParseNode* fn, struct Array* arguments);

