#include "Run.h"
#include "Object.h"
#include "Class.h"
#include "Array.h"
#include "String.h"
#include "Int.h"
#include "Boolean.h"
#include "Memory.h"
#include "Error.h"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>


typedef struct RunResult {
	Class* class_;
	int return_code;
	} RunResult;
Class RunResult_class;
extern RunResult* new_RunResult(int return_code);


Object* Run(Object* self, Object** args)
{
	Array* args_array = (Array*) args[0];
	if (args_array == NULL || args_array->class_ != &Array_class) {
		if (args_array != NULL && args_array->class_ == &String_class) {
			// Following the example of the system(3) man page.
			args_array = new_Array();
			Array_append(args_array, (Object*) new_c_static_String("/bin/sh"));
			Array_append(args_array, (Object*) new_c_static_String("-c"));
			Array_append(args_array, args[0]);
			}
		else
			Error("run() needs an Array or String as its first argument.");
		}

	// Make the argv.
	char* argv[args_array->size + 1];
	for (int i = 0; i < args_array->size; ++i) {
		String* arg = (String*) Array_at(args_array, i);
		if (arg->class_ != &String_class)
			Error("run(): All program arguments must be strings.");
		argv[i] = (char*) String_c_str(arg);
		}
	argv[args_array->size] = NULL;

	// Fork.
	pid_t pid = fork();
	if (pid < 0)
		Error("run(): fork() failed (%s).", strerror(errno));
	else if (pid == 0) {
		// We're now in the child.
		execvp(argv[0], argv);
		Error("run(): Failed to start the program (%s).", strerror(errno));
		}
	else {
		// This is still the parent process.  Wait for child to exit.
		int status = 0;
		waitpid(pid, &status, 0);
		return (Object*) new_RunResult(WEXITSTATUS(status));
		}

	return NULL;
}


RunResult* new_RunResult(int return_code)
{
	RunResult* self = alloc_obj(RunResult);
	self->class_ = &RunResult_class;
	self->return_code = return_code;
	return self;
}


Object* RunResult_return_code(Object* super, Object** args)
{
	return (Object*) new_Int(((RunResult*) super)->return_code);
}

Object* RunResult_ok(Object* super, Object** args)
{
	return make_bool(((RunResult*) super)->return_code == 0);
}

void Run_init()
{
	init_static_class(RunResult);
	static const BuiltinMethodSpec run_result_methods[] = {
		{ "return-code", 0, RunResult_return_code },
		{ "ok", 0, RunResult_ok },
		{ NULL },
		};
	Class_add_builtin_methods(&RunResult_class, run_result_methods);
}



