#pragma once

#include <stdbool.h>

struct MethodBuilder;
struct Array;
struct Dict;
struct String;
struct FunctionStatement;
struct ClassStatement;
struct UpvalueFunction;
struct Environment;


// Types.
enum {
	// We only care about a few of the types; most ParseNodes will just keep
	// their type as PN_Undefined.
	PN_Undefined,
	PN_IfStatement,
	PN_CallExpr,
	PN_Variable,
	};

typedef struct ParseNode {
	int type;
	int (*emit)(struct ParseNode* self, struct MethodBuilder* method);
		// Returns register containing the result (if an expression).
	int (*emit_set)(struct ParseNode* self, struct ParseNode* value, struct MethodBuilder* method);
		// Emits setting an lvalue.  Won't exist for non-lvalue nodes.
	void (*resolve_names)(struct ParseNode* self, struct MethodBuilder* method);
	void (*resolve_names_autodeclaring)(struct ParseNode* self, struct MethodBuilder* method);
	} ParseNode;


typedef struct Block {
	ParseNode parse_node;
	struct Array* statements;
	struct Dict* locals;
	int locals_base;
	struct Dict* functions;
	struct Dict* classes;
	} Block;
extern Block* new_Block();
extern void Block_append(Block* self, ParseNode* statement);
extern ParseNode* Block_get_local(Block* self, struct String* name);
extern struct FunctionStatement* Block_get_function(Block* self, struct String* name);
extern struct ClassStatement* Block_get_class(Block* self, struct String* name);
extern ParseNode* Block_autodeclare(Block* self, struct String* name);
extern void Block_add_function(Block* self, struct FunctionStatement* function);
extern void Block_add_class(Block* self, struct ClassStatement* class_statement);


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


extern ParseNode* new_ContinueStatement();
extern ParseNode* new_BreakStatement();


typedef struct ReturnStatement {
	ParseNode parse_node;
	ParseNode* value;
	} ReturnStatement;
extern ReturnStatement* new_ReturnStatement();


typedef struct WithStatement {
	ParseNode parse_node;
	struct String* name;
	ParseNode* value;
	ParseNode* body;
	} WithStatement;
extern WithStatement* new_WithStatement(struct String* name, ParseNode* value, ParseNode* body);


typedef struct FunctionStatement {
	ParseNode parse_node;
	struct String* name;
	struct Array* arguments;
	ParseNode* body;
	struct Object* compiled_method;
	struct UpvalueFunction* pending_references;
	} FunctionStatement;
extern FunctionStatement* new_FunctionStatement(struct String* name);
extern struct Object* FunctionStatement_compile(FunctionStatement* self, struct Environment* environment);
extern void FunctionStatement_add_reference(FunctionStatement* self, struct UpvalueFunction* reference);

typedef struct UpvalueFunction {
	ParseNode parse_node;
	FunctionStatement* function;
	int literal;
	struct MethodBuilder* method_builder;
	struct UpvalueFunction* next_pending_reference;
	} UpvalueFunction;
extern UpvalueFunction* new_UpvalueFunction(FunctionStatement* function);
extern void UpvalueFunction_patch(UpvalueFunction* self, struct Object* compiled_function);


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


typedef struct ShortCircuitNot {
	ParseNode parse_node;
	ParseNode* expr;
	} ShortCircuitNot;
extern ShortCircuitNot* new_ShortCircuitNot(ParseNode* expr);


typedef struct StringLiteralExpr {
	ParseNode parse_node;
	struct String* str;
	} StringLiteralExpr;
extern StringLiteralExpr* new_StringLiteralExpr(struct String* str);

typedef struct InterpolatedStringLiteral {
	ParseNode parse_node;
	struct Array* components;
	} InterpolatedStringLiteral;
extern InterpolatedStringLiteral* new_InterpolatedStringLiteral(struct Array* components);

typedef struct IntLiteralExpr {
	ParseNode parse_node;
	int value;
	} IntLiteralExpr;
extern IntLiteralExpr* new_IntLiteralExpr(struct String* value_str);

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

typedef struct SelfExpr {
	ParseNode parse_node;
	} SelfExpr;
extern SelfExpr* new_SelfExpr();

typedef struct RawLoc {
	ParseNode parse_node;
	int loc;
	} RawLoc;
extern RawLoc* new_RawLoc(int loc);

typedef struct ArrayLiteral {
	ParseNode parse_node;
	struct Array* items;
	} ArrayLiteral;
extern ArrayLiteral* new_ArrayLiteral();
extern void ArrayLiteral_add_item(ArrayLiteral* self, ParseNode* item);

typedef struct DictLiteral {
	ParseNode parse_node;
	struct Dict* items;
	} DictLiteral;
extern DictLiteral* new_DictLiteral();
extern void DictLiteral_add_item(DictLiteral* self, struct String* key, ParseNode* value);


typedef struct CallExpr {
	ParseNode parse_node;
	struct ParseNode* receiver;
	struct String* name;
	struct Array* arguments;
	bool got_args;
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

