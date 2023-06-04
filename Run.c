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
	pid_t pid;
	bool done;
	int return_code;
	Object* captured_output;
	} RunResult;
Class RunResult_class;
static RunResult* new_RunResult(pid_t pid, Object* captured_output);
Object* RunResult_wait(Object* super, Object** args);


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
	bool wait = true;
	Pipe* stdin_pipe = NULL;
	Pipe* stdout_pipe = NULL;
	Pipe* stderr_pipe = NULL;
	declare_static_string(capture_option, "capture");
	declare_static_string(wait_option, "wait");
	declare_static_string(stdin_pipe_option, "stdin-pipe");
	declare_static_string(stdout_pipe_option, "stdout-pipe");
	declare_static_string(stderr_pipe_option, "stderr-pipe");
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		capture = Dict_option_turned_on(options, &capture_option);
		if (Dict_option_turned_off(options, &wait_option))
			wait = false;
		stdin_pipe = (Pipe*) Dict_at(options, &stdin_pipe_option);
		if (stdin_pipe && stdin_pipe->class_ != &Pipe_class)
			Error("run(): \"stdin-pipe\" must be a Pipe.");
		stdout_pipe = (Pipe*) Dict_at(options, &stdout_pipe_option);
		if (stdout_pipe && stdout_pipe->class_ != &Pipe_class)
			Error("run(): \"stdout-pipe\" must be a Pipe.");
		if (stdout_pipe && capture)
			Error("run(): Can't use \"capture\" and \"stdout-pipe\" options at the same time.");
		stderr_pipe = (Pipe*) Dict_at(options, &stderr_pipe_option);
		if (stderr_pipe && stderr_pipe->class_ != &Pipe_class)
			Error("run(): \"stderr-pipe\" must be a Pipe.");
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
	if (capture)
		stdout_pipe = new_Pipe();

	// Fork.
	pid_t pid = fork();
	if (pid < 0)
		Error("run(): fork() failed (%s).", strerror(errno));
	else if (pid == 0) {
		// We're now in the child.

		// If piping, set that up.
		if (stdin_pipe) {
			close(stdin_pipe->write_fd);
			stdin_pipe->write_fd = -1;
			dup2(stdin_pipe->read_fd, STDIN_FILENO);
			}
		if (stdout_pipe) {
			close(stdout_pipe->read_fd); 	// Close the read end.
			stdout_pipe->read_fd = -1;
			dup2(stdout_pipe->write_fd, STDOUT_FILENO);
			}
		if (stderr_pipe) {
			close(stderr_pipe->read_fd);
			stderr_pipe->read_fd = -1;
			dup2(stderr_pipe->write_fd, STDERR_FILENO);
			}

		// Run.
		execvp(argv[0], argv);
		Error("run(): Failed to start the program (%s).", strerror(errno));
		}
	else {
		// This is still the parent process.

		// Piping and capturing output.
		Object* captured_output = NULL;
		if (stdin_pipe) {
			close(stdin_pipe->read_fd);
			stdin_pipe->read_fd = -1;
			}
		if (stdout_pipe) {
			close(stdout_pipe->write_fd);
			stdout_pipe->write_fd = -1;
			}
		if (stderr_pipe) {
			close(stderr_pipe->write_fd);
			stderr_pipe->write_fd = -1;
			}
		if (capture) {
			captured_output = Pipe_capture(stdout_pipe, true, 0);
			Pipe_close(stdout_pipe);
			}

		// Wait for child to exit.
		Object* run_result = (Object*) new_RunResult(pid, captured_output);
		if (wait)
			RunResult_wait((Object*) run_result, NULL);
		return run_result;
		}

	return NULL;
}


static RunResult* new_RunResult(pid_t pid, Object* captured_output)
{
	RunResult* self = alloc_obj(RunResult);
	self->class_ = &RunResult_class;
	self->pid = pid;
	self->captured_output = captured_output;
	return self;
}


Object* RunResult_return_code(Object* super, Object** args)
{
	return (Object*) new_Int(((RunResult*) super)->return_code);
}

Object* RunResult_ok(Object* super, Object** args)
{
	RunResult_wait(super, NULL);
	return make_bool(((RunResult*) super)->return_code == 0);
}

Object* RunResult_output(Object* super, Object** args)
{
	RunResult_wait(super, NULL);
	return ((RunResult*) super)->captured_output;
}

Object* RunResult_wait(Object* super, Object** args)
{
	RunResult* self = (RunResult*) super;
	if (!self->done) {
		int status = 0;
		waitpid(self->pid, &status, 0);
		self->return_code = WEXITSTATUS(status);
		self->done = true;
		}
	return (Object*) self;
}


void Run_init()
{
	init_static_class(RunResult);
	static const BuiltinMethodSpec run_result_methods[] = {
		{ "return-code", 0, RunResult_return_code },
		{ "ok", 0, RunResult_ok },
		{ "output", 0, RunResult_output },
		{ "wait", 0, RunResult_wait },
		{ NULL },
		};
	Class_add_builtin_methods(&RunResult_class, run_result_methods);
}



