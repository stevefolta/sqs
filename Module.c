#include "Module.h"
#include "Parser.h"
#include "Lexer.h"
#include "ParseNode.h"
#include "ClassStatement.h"
#include "MethodBuilder.h"
#include "Method.h"
#include "Environment.h"
#include "String.h"
#include "Dict.h"
#include "Array.h"
#include "Object.h"
#include "Memory.h"
#include "ByteCode.h"
#include "File.h"
#include "Error.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static Dict* modules = NULL;


ParseNode* Parser_parse_export(Parser* self)
{
	Lexer_next(self->lexer); 	// Consume "export".
	Token next_token = Lexer_peek(self->lexer);

	Module* module = self->inner_block->module;
	if (module == NULL)
		Error("\"export\" statement is not part of a module %s.", where(next_token.line_number, self->filename));

	if (next_token.type == Identifier) {
		if (String_equals_c(next_token.token, "class")) {
			ClassStatement* class_statement = (ClassStatement*) Parser_parse_class_statement(self);
			Dict_set_at(module->exported_classes, ClassStatement_get_name(class_statement), (Object*) class_statement);
			return (ParseNode*) class_statement;
			}
		else if (String_equals_c(next_token.token, "fn")) {
			FunctionStatement* fn_statement = (FunctionStatement*) Parser_parse_fn_statement(self);
			Dict_set_at(module->exported_functions, fn_statement->name, (Object*) fn_statement);
			return (ParseNode*) fn_statement;
			}
		}

	Error("Expected \"class\" or \"fn\" after \"export\" %s.", where(next_token.line_number, self->filename));
	return NULL;
}


typedef struct ImportStatement {
	ParseNode parse_node;
	Array* imported_modules;
	} ImportStatement;

int ImportStatement_emit(ParseNode* super, MethodBuilder* builder)
{
	ImportStatement* self = (ImportStatement*) super;

	// This is a good time to build the imported modules, if they haven't been
	// built yet.  This is to ensure that they get built (and more importantly,
	// had their names resolved) before the current block can try to use any of
	// the imported classes as a superclass.
	for (int which_module = 0; which_module < self->imported_modules->size; ++which_module) {
		Module* module = (Module*) self->imported_modules->items[which_module];
		Module_build(module);
		}

	return 0;
}

ImportStatement* new_ImportStatement()
{
	ImportStatement* self = alloc_obj(ImportStatement);
	self->parse_node.emit = ImportStatement_emit;
	self->imported_modules = new_Array();
	return self;
}


ParseNode* Parser_parse_import(Parser* self)
{
	Lexer_next(self->lexer); 	// Consume "import".

	ImportStatement* import_statement = new_ImportStatement();

	while (true) {
		Token token = Lexer_next(self->lexer);
		if (token.type == EOL)
			break;
		if (token.type == Operator && String_equals_c(token.token, ","))
			continue;
		if (token.type != Identifier)
			Error("Expected a module name in \"import\" statement %s.", where(token.line_number, self->filename));

		// Load the module.
		// This will parse it, if needed, which will make all its exports known.
		// It won't be emitted yet, though.
		Module* module = Module_get_module(token.token);
		if (module == NULL)
			Error("Couldn't load module named \"%s\" %s.", String_c_str(token.token), where(token.line_number, self->filename));

		// Add it to the Block's imported modules.
		if (self->inner_block->imported_modules == NULL)
			self->inner_block->imported_modules = new_Array();
		Array_append(self->inner_block->imported_modules, (Object*) module);

		Array_append(import_statement->imported_modules, (Object*) module);
		}

	return (ParseNode*) import_statement;
}


static Module* new_Module()
{
	Module* module = alloc_obj(Module);
	module->exported_classes = new_Dict();
	module->exported_functions = new_Dict();
	return module;
}


const char* follow_link(const char* path)
{
	struct stat stat_buf;
	if (lstat(path, &stat_buf) < 0)
		return NULL;
	// readlink() is tricky!  We have to ask for *more* bytes than we need,
	// otherwise we can't detect truncation.
	ssize_t string_size = (stat_buf.st_size > 0 ? stat_buf.st_size + 2 : PATH_MAX);
	char* new_path = alloc_mem_no_pointers(string_size);
	ssize_t num_bytes = readlink(path, new_path, string_size - 1);
	if (num_bytes < 0 || num_bytes == string_size - 1)
		return NULL;
	return new_path;
}


Module* Module_load_from_file(String* name, const char* file_path);

Module* Module_get_module(String* name)
{
	// Did we already load it?
	if (modules == NULL)
		modules = new_Dict();
	Module* module = (Module*) Dict_at(modules, name);
	if (module)
		return module;

	// Load it.
	// Get the path to the original script.
	declare_static_string(argv_string, "argv");
	GlobalExpr* argv_expr =
		(GlobalExpr*) global_environment.environment.find(&global_environment.environment, &argv_string);
	if (argv_expr == NULL)
		return NULL;
	Array* argv = (Array*) argv_expr->object;
	const char* argv_0 = String_c_str((String*) Array_at(argv, 0));
	const char* followed_argv_0 = follow_link(argv_0);
	if (followed_argv_0)
		argv_0 = followed_argv_0;
	const char* path = NULL;
	char* end = strrchr(argv_0, '/');
	if (end == NULL)
		path = String_c_str(name);
	else {
		Array* path_array = new_Array();
		Array_append(path_array, (Object*) new_String(argv_0, end - argv_0));
		declare_static_string(slash_string, "/");
		Array_append(path_array, (Object*) &slash_string);
		Array_append(path_array, (Object*) name);
		path = String_c_str(Array_join(path_array, NULL));
		}

	// Load it.
	module = Module_load_from_file(name, path);
	return module;
}


Module* Module_load_from_file(String* name, const char* file_path)
{
	// Read the file.
	String* contents = file_contents(file_path);
	if (contents == NULL) {
		// Try it with a ".sqs" extension.
		declare_static_string(extension_string, ".sqs");
		String* new_path = String_add(make_string(file_path), &extension_string);
		contents = file_contents(String_c_str(new_path));
		}
	if (contents == NULL)
		Error("Couldn't open \"%s\" (%s).", file_path, strerror(errno));

	// Get the module into "modules" before parsing it, in case there is mutual
	// recursion.
	Module* module = new_Module();
	Dict_set_at(modules, name, (Object*) module);

	Parser* parser = new_Parser(contents->str, contents->size, name);
	ParseNode* ast = Parser_parse_block(parser, module);
	if (ast == NULL)
		return NULL;
	module->block = (Block*) ast;
	return module;
}


struct FunctionStatement* Module_exported_function(Module* self, String* name)
{
	return (FunctionStatement*) Dict_at(self->exported_functions, name);
}

struct ClassStatement* Module_exported_class(Module* self, String* name)
{
	return (ClassStatement*) Dict_at(self->exported_classes, name);
}


void Module_create_module_locals(Module* self, int num_locals)
{
	self->locals = (Object**) alloc_mem(num_locals * sizeof(Object*));
}


void Module_build(Module* module)
{
	if (module->method || module->is_building)
		return;
	module->is_building = true;

	MethodBuilder* method_builder = new_MethodBuilder(new_Array(), NULL);
	MethodBuilder_add_literal(method_builder, (Object*) new_c_static_String("-module-"));
	module->block->parse_node.emit(&module->block->parse_node, method_builder);
	MethodBuilder_finish(method_builder);
	module->method = method_builder->method;
	call_method(module->method, NULL);
	module->is_building = false;
}


int ModuleLocal_emit(ParseNode* super, MethodBuilder* builder)
{
	Local* self = (Local*) super;

	int loc = MethodBuilder_reserve_locals(builder, 1);
	int frame_loc = MethodBuilder_emit_literal(builder, (Object*) self->block->module->locals);

	MethodBuilder_add_bytecode(builder, BC_GET_MODULE_LOCAL);
	MethodBuilder_add_bytecode(builder, frame_loc);
	MethodBuilder_add_bytecode(builder, self->block_index);
	MethodBuilder_add_bytecode(builder, loc);

	builder->cur_num_variables = loc + 1;
	return loc;
}

int ModuleLocal_emit_set(ParseNode* super, ParseNode* value, MethodBuilder* builder)
{
	Local* self = (Local*) super;

	int value_loc = value->emit(value, builder);
	int orig_locals = builder->cur_num_variables;
	int frame_loc = MethodBuilder_emit_literal(builder, (Object*) self->block->module->locals);

	MethodBuilder_add_bytecode(builder, BC_SET_MODULE_LOCAL);
	MethodBuilder_add_bytecode(builder, frame_loc);
	MethodBuilder_add_bytecode(builder, self->block_index);
	MethodBuilder_add_bytecode(builder, value_loc);

	builder->cur_num_variables = orig_locals;
	return value_loc;
}


void Module_make_module_local(Local* local)
{
	local->parse_node.emit = ModuleLocal_emit;
	local->parse_node.emit_set = ModuleLocal_emit_set;
}

ParseNode* ModuleContext_find(Environment* self, String* name)
{
	ParseNode* result = BlockContext_find(self, name);
	if (result && result->type == PN_Local)
		Module_make_module_local((Local*) result);
	return result;
}

ParseNode* ModuleContext_find_autodeclaring(Environment* self, String* name)
{
	ParseNode* result = BlockContext_find_autodeclaring(self, name);
	if (result && result->type == PN_Local)
		Module_make_module_local((Local*) result);
	return result;
}

void BlockContext_make_module_context(BlockContext* context)
{
	context->environment.find = ModuleContext_find;
	context->environment.find_autodeclaring = ModuleContext_find_autodeclaring;
}

void BlockUpvalueContext_make_module_context(BlockUpvalueContext* context)
{
	context->environment.find = ModuleContext_find;
	context->environment.find_autodeclaring = ModuleContext_find;
}



