#include "ClassStatement.h"
#include "Parser.h"
#include "Lexer.h"
#include "MethodBuilder.h"
#include "Environment.h"
#include "ByteCode.h"
#include "String.h"
#include "Array.h"
#include "Dict.h"
#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include "Error.h"
#include <stdio.h>

// This file includes all the functions for compiling a class definition,
// instead of having them spread out over Parser.c, ParseNodes.c, and
// Environment.c.


typedef struct ClassFunctionContext {
	Environment environment;
	ClassStatement* class_statement;
	} ClassFunctionContext;

extern void ClassFunctionContext_init(ClassFunctionContext* self, ClassStatement* class_statement, Environment* parent);

typedef struct EnclosedClassContext {
	Environment environment;
	ClassStatement* class_statement;
	} EnclosedClassContext;

extern void EnclosedClassContext_init(EnclosedClassContext* self, ClassStatement* class_statement, Environment* parent);


ParseNode* Parser_parse_class_statement(Parser* self)
{
	Lexer_next(self->lexer); 	// Consume "class".

	// Make sure new functions and classes don't end up in the enclosing block.
	Block* inner_block = self->inner_block;
	self->inner_block = NULL;

	// Name.
	Token token = Lexer_next(self->lexer);
	if (token.type != Identifier)
		Error("Expected class name on line %d.", token.line_number);
	String* name = token.token;

	ClassStatement* class_statement = new_ClassStatement(name);

	// Superclass.
	token = Lexer_peek(self->lexer);
	if (token.type == Operator && String_equals_c(token.token, ":")) {
		Lexer_next(self->lexer);
		token = Lexer_next(self->lexer);
		if (token.type != Identifier)
			Error("Expected a class name as the superclass on line %d.", token.line_number);
		class_statement->superclass_name = token.token;
		}

	// Ivars.
	class_statement->ivars = Parser_parse_names_list(self, "instance variable");

	token = Lexer_next(self->lexer);
	if (token.type != EOL)
		Error("Extra characters after class definition on line %d.", token.line_number);

	// Functions and classes.
	token = Lexer_peek(self->lexer);
	if (token.type == Indent) {
		Lexer_next(self->lexer);
		while (true) {
			token = Lexer_peek(self->lexer);
			if (token.type == Unindent) {
				Lexer_next(self->lexer);
				break;
				}
			else if (token.type == EOL) {
				Lexer_next(self->lexer);
				continue;
				}

			// Ivars.
			else if (token.type == Operator && String_equals_c(token.token, "(")) {
				Array* arg_names = Parser_parse_names_list(self, "argument");
				token = Lexer_next(self->lexer);
				if (token.type != EOL)
					Error("Extra characters after ivars list on line %d.", token.line_number);
				if (class_statement->ivars == NULL)
					class_statement->ivars = arg_names;
				else {
					for (int i = 0; i < arg_names->size; ++i)
						Array_append(class_statement->ivars, Array_at(arg_names, i));
					}
				continue;
				}

			// "class"
			else if (token.type == Identifier && String_equals_c(token.token, "class")) {
				ClassStatement* enclosed_class = (ClassStatement*) Parser_parse_class_statement(self);
				if (class_statement->enclosed_classes == NULL)
					class_statement->enclosed_classes = new_Dict();
				Dict_set_at(
					class_statement->enclosed_classes,
					ClassStatement_get_name(enclosed_class),
					(Object*) enclosed_class);
				continue;
				}

			// Anything else is a function.

			// It might be preceded by "fn", or not.
			if (token.type == Identifier && String_equals_c(token.token, "fn")) {
				Lexer_next(self->lexer);
				token = Lexer_next(self->lexer);
				if (token.type != Identifier && token.type != Operator)
					Error("Bad function definition on line %d.", token.line_number);
				}

			FunctionStatement* function = Parser_parse_fn_statement_raw(self);
			if (function == NULL)
				Error("Expected function definition on line %d.", token.line_number);
			Dict_set_at(class_statement->functions, function->name, (Object*) function);
			}
		}

	if (inner_block)
		Block_add_class(inner_block, class_statement);

	self->inner_block = inner_block;
	return (ParseNode*) class_statement;
}


int ClassStatement_emit(ParseNode* super, MethodBuilder* method)
{
	ClassStatement* self = (ClassStatement*) super;

	if (self->is_built) {
		// This was already built because it was someone's superclass.
		return 0;
		}
	self->is_building = true;

	// Superclass.
	if (self->superclass_name) {
		Class* superclass = Environment_find_class_for_superclass(method->environment, self->superclass_name, method);
		if (superclass == NULL)
			Error("Couldn't find a superclass named \"%s\" for \"%s\".", String_c_str(self->superclass_name), String_c_str(self->built_class->name));
		self->built_class->superclass = superclass;
		}

	// Calculate "num_ivars".
	int num_ivars = (self->ivars ? self->ivars->size : 0);
	Class* ancestor = self->built_class->superclass;
	while (ancestor) {
		num_ivars += ancestor->num_ivars;
		ancestor = ancestor->superclass;
		}
	self->built_class->num_ivars = num_ivars;
	self->built_class->slot_names = self->ivars;

	// Compile functions.
	// Set up environment.
	ClassFunctionContext context;
	ClassFunctionContext_init(&context, self, method->environment);
	method->environment = (Environment*) &context;
	// Compile all functions.
	DictIterator* it = new_DictIterator(self->functions);
	while (true) {
		DictIteratorResult kv = DictIterator_next(it);
		if (kv.key == NULL)
			break;
		FunctionStatement* function = (FunctionStatement*) kv.value;
		Object* compiled_method = FunctionStatement_compile(function, method->environment);
		if (dump_requested) {
			dump_bytecode((struct Method*) compiled_method, self->built_class->name, kv.key);
			printf("\n");
			}
		if (self->built_class->methods == NULL)
			self->built_class->methods = new_Dict();
		Dict_set_at(self->built_class->methods, function->name, compiled_method);
		}
	// Clean up environment.
	method->environment = context.environment.parent;

	// Compile enclosed classes.
	if (self->enclosed_classes) {
		EnclosedClassContext enclosed_class_context;
		EnclosedClassContext_init(&enclosed_class_context, self, method->environment);
		method->environment = &enclosed_class_context.environment;

		DictIterator* it = new_DictIterator(self->enclosed_classes);
		while (true) {
			DictIteratorResult kv = DictIterator_next(it);
			if (kv.key == NULL)
				break;
			ClassStatement* enclosed_class = (ClassStatement*) kv.value;
			ClassStatement_emit((ParseNode*) enclosed_class, method);
			}

		method->environment = enclosed_class_context.environment.parent;
		}

	self->is_building = false;
	self->is_built = true;
	return 0;
}

ClassStatement* new_ClassStatement(String* name)
{
	ClassStatement* self = alloc_obj(ClassStatement);
	self->parse_node.type = PN_ClassStatement;
	self->parse_node.emit = ClassStatement_emit;
	self->built_class = new_Class(name);
	self->functions = new_Dict();
	return self;
}


String* ClassStatement_get_name(ClassStatement* self)
{
	return self->built_class->name;
}


ParseNode* ClassStatement_make_reference(ClassStatement* self)
{
	return (ParseNode*) new_GlobalExpr((Object*) self->built_class);
}

ClassStatement* ClassStatement_get_enclosed_class(ClassStatement* self, String* name)
{
	if (self->enclosed_classes)
		return (ClassStatement*) Dict_at(self->enclosed_classes, name);
	return NULL;
}


typedef struct IvarExpr {
	ParseNode parse_node;
	int ivar_index;
	} IvarExpr;

int IvarExpr_emit(ParseNode* super, MethodBuilder* method)
{
	IvarExpr* self = (IvarExpr*) super;

	int result_loc = MethodBuilder_reserve_locals(method, 1);

	MethodBuilder_add_bytecode(method, BC_GET_IVAR);
	MethodBuilder_add_bytecode(method, self->ivar_index);
	MethodBuilder_add_bytecode(method, result_loc);

	return result_loc;
}

int IvarExpr_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* method)
{
	IvarExpr* self = (IvarExpr*) super;

	int value_loc = value->emit(value, method);

	MethodBuilder_add_bytecode(method, BC_SET_IVAR);
	MethodBuilder_add_bytecode(method, self->ivar_index);
	MethodBuilder_add_bytecode(method, value_loc);

	return value_loc;
}

IvarExpr* new_IvarExpr(int ivar_index)
{
	IvarExpr* self = alloc_obj(IvarExpr);
	self->parse_node.emit = IvarExpr_emit;
	self->parse_node.emit_set = IvarExpr_emit_set;
	self->ivar_index = ivar_index;
	return self;
}


ParseNode* ClassFunctionContext_find(Environment* super, String* name)
{
	ClassFunctionContext* self = (ClassFunctionContext*) super;
	ClassStatement* class_statement = self->class_statement;

	// Ivars.
	Array* ivars = class_statement->ivars;
	if (ivars) {
		for (int i = 0; i < ivars->size; ++i) {
			if (String_equals(name, (String*) Array_at(ivars, i))) {
				int added_ivars_base = class_statement->built_class->num_ivars - class_statement->ivars->size;
				return (ParseNode*) new_IvarExpr(i + added_ivars_base);
				}
			}
		}
	// Superclass ivars.
	Class* cur_class = class_statement->built_class->superclass;
	while (cur_class) {
		if (cur_class->slot_names) {
			for (int i = 0; i < cur_class->slot_names->size; ++i) {
				if (String_equals((String*) Array_at(cur_class->slot_names, i), name)) {
					int added_ivars_base = cur_class->num_ivars - cur_class->slot_names->size;
					return (ParseNode*) new_IvarExpr(i + added_ivars_base);
					}
				}
			}
		cur_class = cur_class->superclass;
		}

	// Self calls.
	FunctionStatement* function = (FunctionStatement*) Dict_at(class_statement->functions, name);
	if (function)
		return (ParseNode*) new_CallExpr((ParseNode*) new_SelfExpr(), name);
	// Self calls of super functions.
	cur_class = class_statement->built_class->superclass;
	while (cur_class) {
		if (cur_class->methods && Dict_at(cur_class->methods, name)) {
			return (ParseNode*) new_CallExpr((ParseNode*) new_SelfExpr(), name);
			}
		cur_class = cur_class->superclass;
		}

	// Enclosed classes.
	ClassStatement* enclosed_class = ClassStatement_get_enclosed_class(class_statement, name);
	if (enclosed_class)
		return (ParseNode*) new_GlobalExpr((Object*) enclosed_class->built_class);

	if (self->environment.parent)
		return self->environment.parent->find(self->environment.parent, name);

	return NULL;
}

Class* ClassFunctionContext_get_class_for_superclass(struct Environment* super, String* name, MethodBuilder* method)
{
	ClassFunctionContext* self = (ClassFunctionContext*) super;
	ClassStatement* class_statement = self->class_statement;

	ClassStatement* enclosed_class = ClassStatement_get_enclosed_class(class_statement, name);
	if (enclosed_class)
		return enclosed_class->built_class;

	return NULL;
}

void ClassFunctionContext_init(struct ClassFunctionContext* self, ClassStatement* class_statement, Environment* parent)
{
	self->environment.parent = parent;
	self->environment.find = ClassFunctionContext_find;
	self->environment.get_class_for_superclass = ClassFunctionContext_get_class_for_superclass;
	self->class_statement = class_statement;
}


ParseNode* EnclosedClassContext_find(Environment* super, String* name)
{
	EnclosedClassContext* self = (EnclosedClassContext*) super;
	ClassStatement* class_statement = self->class_statement;

	// Enclosed classes.
	ClassStatement* enclosed_class = ClassStatement_get_enclosed_class(class_statement, name);
	if (enclosed_class)
		return (ParseNode*) new_GlobalExpr((Object*) enclosed_class->built_class);

	if (self->environment.parent)
		return self->environment.parent->find(self->environment.parent, name);

	return NULL;
}

Class* EnclosedClassContext_get_class_for_superclass(Environment* super, String* name, MethodBuilder* method)
{
	EnclosedClassContext* self = (EnclosedClassContext*) super;
	ClassStatement* class_statement = self->class_statement;

	ClassStatement* enclosed_class = ClassStatement_get_enclosed_class(class_statement, name);
	if (enclosed_class) {
		// If it's building, that's a superclass mutual recursion.
		if (enclosed_class->is_building) {
			Error(
				"Two classes can't be each other's superclass!  (One of them is \"%s\".)",
				String_c_str(name));
			}
		// If it's not built yet, we need to build it now, so we know how many
		// ivars it has.
		if (!enclosed_class->is_built)
			ClassStatement_emit((ParseNode*) enclosed_class, method);

		return enclosed_class->built_class;
		}

	return NULL;
}

void EnclosedClassContext_init(EnclosedClassContext* self, ClassStatement* class_statement, Environment* parent)
{
	self->environment.parent = parent;
	self->environment.find = EnclosedClassContext_find;
	self->environment.get_class_for_superclass = EnclosedClassContext_get_class_for_superclass;
	self->class_statement = class_statement;
}


