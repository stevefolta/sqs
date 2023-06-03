#include "Run.h"
#include "Pipe.h"
#include "Object.h"
#include "Class.h"
#include "Array.h"
#include "String.h"
#include "Dict.h"
#include "ByteArray.h"
#include "Int.h"
#include "Boolean.h"
#include "Memory.h"
#include "Error.h"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>


typedef struct RunResult {
	Class* class_;
	int return_code;
	Object* captured_output;
	} RunResult;
Class RunResult_class;
static RunResult* new_RunResult(int return_code, Object* captured_output);


Object* Run(Object* self, Object** args)
{
	// The command: Array or String?
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

	// Options.
	bool capture = false;
	declare_static_string(capture_option, "capture");
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		capture = Dict_option_turned_on(options, &capture_option);
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

	// Set up to capture.
	Pipe* stdout_pipe = NULL;
	if (capture)
		stdout_pipe = new_Pipe();

	// Fork.
	pid_t pid = fork();
	if (pid < 0)
		Error("run(): fork() failed (%s).", strerror(errno));
	else if (pid == 0) {
		// We're now in the child.

		// If piping, set that up.
		if (stdout_pipe) {
			close(stdout_pipe->read_fd); 	// Close the read end.
			dup2(stdout_pipe->write_fd, STDOUT_FILENO);
			}

		// Run.
		execvp(argv[0], argv);
		Error("run(): Failed to start the program (%s).", strerror(errno));
		}
	else {
		// This is still the parent process.

		// Piping and capturing output.
		Object* captured_output = NULL;
		if (stdout_pipe)
			close(stdout_pipe->write_fd);
		if (capture) {
			captured_output = Pipe_capture(stdout_pipe, true, 0);
			Pipe_close(stdout_pipe);
			}

		// Wait for child to exit.
		int status = 0;
		waitpid(pid, &status, 0);
		return (Object*) new_RunResult(WEXITSTATUS(status), captured_output);
		}

	return NULL;
}


static RunResult* new_RunResult(int return_code, Object* captured_output)
{
	RunResult* self = alloc_obj(RunResult);
	self->class_ = &RunResult_class;
	self->return_code = return_code;
	self->captured_output = captured_output;
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

Object* RunResult_output(Object* super, Object** args)
{
	return ((RunResult*) super)->captured_output;
}

void Run_init()
{
	init_static_class(RunResult);
	static const BuiltinMethodSpec run_result_methods[] = {
		{ "return-code", 0, RunResult_return_code },
		{ "ok", 0, RunResult_ok },
		{ "output", 0, RunResult_output },
		{ NULL },
		};
	Class_add_builtin_methods(&RunResult_class, run_result_methods);
}



