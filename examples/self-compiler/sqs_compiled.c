#include "sqs_compiled.h"
#include "LinesIterator.h"
#include "Error.h"
#include <string.h>


UpvalFrame* cur_upval_frame_ = NULL;
Array argv_obj = { &Array_class, 0, 0, NULL };


Object* call_(const char* name, Object* receiver, int num_args, Object** args)
{
	// Find the method.
	String name_str;
	String_init_static_c(&name_str, name);
	Object* method = Object_find_method(receiver, &name_str);
	if (method == NULL || method->class_ != &BuiltinMethod_class) {
		Class* receiver_class = (receiver ? receiver->class_ : &Nil_class);
		Error("Unhandled method call: \"%s\" on %s.", name, String_c_str(receiver_class->name));
		}

	// Call it.
	BuiltinMethod* builtin_method = (BuiltinMethod*) method;
	int method_num_args = builtin_method->num_args;
	if (num_args < method_num_args) {
		Object* more_args[method_num_args];
		memset(more_args, 0, sizeof(more_args));
		int i = 0;
		for (; i < num_args; ++i)
			more_args[i] = args[i];
		for (; i < method_num_args; ++i)
			more_args[i] = NULL;
		return builtin_method->fn(receiver, more_args);
		}
	else
		return builtin_method->fn(receiver, args);
}


Object* super_call_(const char* name, Class* child_class, Object* receiver, int num_args, Object** args)
{
	// Find the method.
	String name_str;
	String_init_static_c(&name_str, name);
	Object* method = Class_find_super_method(child_class, &name_str);
	if (method == NULL || method->class_ != &BuiltinMethod_class) {
		Class* receiver_class = (receiver ? receiver->class_ : &Nil_class);
		Error("Unhandled method call: \"%s\" on %s.", name, String_c_str(receiver_class->name));
		}

	// Call it.
	BuiltinMethod* builtin_method = (BuiltinMethod*) method;
	int method_num_args = builtin_method->num_args;
	if (num_args < method_num_args) {
		Object* more_args[method_num_args];
		memset(more_args, 0, sizeof(more_args));
		int i = 0;
		for (; i < num_args; ++i)
			more_args[i] = args[i];
		for (; i < method_num_args; ++i)
			more_args[i] = NULL;
		return builtin_method->fn(receiver, more_args);
		}
	else
		return builtin_method->fn(receiver, args);
}


Object** get_upvalue_(int capture_id, int index)
{
	for (UpvalFrame* frame = cur_upval_frame_; frame; frame = frame->up) {
		if (frame->capture_id == capture_id)
			return frame->captures[index];
		}
	Error("Internal error: bad capture ID (%d)", capture_id);
}

Object* call_object(Object* receiver, String* name, Array* args)
{
	// Find the method.
	Object* method = Object_find_method(receiver, name);
	if (method == NULL || method->class_ != &BuiltinMethod_class) {
		Class* receiver_class = (receiver ? receiver->class_ : &Nil_class);
		Error("Unhandled method call: \"%s\" on %s.", name, String_c_str(receiver_class->name));
		}

	// Call it.
	BuiltinMethod* builtin_method = (BuiltinMethod*) method;
	int num_args = (args ? args->size : 0);
	int method_num_args = builtin_method->num_args;
	if (num_args < method_num_args) {
		Object* more_args[method_num_args];
		memset(more_args, 0, sizeof(more_args));
		int i = 0;
		for (; i < num_args; ++i)
			more_args[i] = args->items[i];
		for (; i < method_num_args; ++i)
			more_args[i] = NULL;
		return builtin_method->fn(receiver, more_args);
		}
	else
		return builtin_method->fn(receiver, (args ? args->items : NULL));
}


static void init_all()
{
	Class_init_class();
	Object_init_class();
	String_init_class();
	Int_init_class();
	Float_init_class();
	Boolean_init_class();
	Array_init_class();
	ByteArray_init_class();
	Dict_init_class();
	BuiltinMethod_init_class();
	Nil_init_class();
	File_init_class();
	Pipe_init_class();
	LinesIterator_init_class();
	Regex_init_class();
	Path_init_class();
	Env_init_class();
	Run_init();
}


int main(int argc, char* argv[])
{
	init_all();
	extern void sqs_init_classes();
	sqs_init_classes();

	for (int i = 0; i < argc; ++i)
		Array_append(&argv_obj, (Object*) new_c_static_String(argv[i]));
	extern Object* sqs_main(Object* self, Object** args);
	Object* result = sqs_main(NULL, NULL);
	if (result)
		return Int_enforce(result, "main result");
	return 0;
}


