#include "Run.h"
#include "Pipe.h"
#include "Object.h"
#include "Class.h"
#include "Array.h"
#include "String.h"
#include "Dict.h"
#include "ByteArray.h"
#include "ByteCode.h"
#include "Int.h"
#include "Boolean.h"
#include "Memory.h"
#include "Error.h"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

declare_string(capture_string, "capture");
declare_string(wait_string, "wait");
declare_string(stdin_pipe_string, "stdin-pipe");
declare_string(stdout_pipe_string, "stdout-pipe");
declare_string(stderr_pipe_string, "stderr-pipe");
declare_string(env_string, "env");

extern char** build_environ(Dict* env);


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
	Dict* env = NULL;
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		capture = Dict_option_turned_on(options, &capture_string);
		if (Dict_option_turned_off(options, &wait_string))
			wait = false;
		stdin_pipe = (Pipe*) Dict_at(options, &stdin_pipe_string);
		if (stdin_pipe && stdin_pipe->class_ != &Pipe_class)
			Error("run(): \"stdin-pipe\" must be a Pipe.");
		stdout_pipe = (Pipe*) Dict_at(options, &stdout_pipe_string);
		if (stdout_pipe && stdout_pipe->class_ != &Pipe_class)
			Error("run(): \"stdout-pipe\" must be a Pipe.");
		if (stdout_pipe && capture)
			Error("run(): Can't use \"capture\" and \"stdout-pipe\" options at the same time.");
		stderr_pipe = (Pipe*) Dict_at(options, &stderr_pipe_string);
		if (stderr_pipe && stderr_pipe->class_ != &Pipe_class)
			Error("run(): \"stderr-pipe\" must be a Pipe.");
		env = (Dict*) Dict_at(options, &env_string);
		if (env && env->class_ != &Dict_class)
			Error("run(): \"env\" must be a Dict.");
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

		// Environment.
		// We'd use execvpe(), but that's not part of POSIX.
		if (env) {
			extern char** environ;
			environ = build_environ(env);
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



char** build_environ(Dict* env)
{
	char** environ = (char**) alloc_mem((env->size + 1) * sizeof(char*));
	DictIterator* iterator = new_DictIterator(env);
	declare_static_string(equals_string, "=");

	char** next_env_entry = environ;
	while (true) {
		DictIteratorResult kv = DictIterator_next(iterator);
		if (kv.key == NULL)
			break;
		if (kv.value == NULL)
			continue;
		String* value = String_enforce(kv.value, "run(): \"env\" values must be strings.");
		String* entry = String_add(kv.key, String_add(&equals_string, value));
		*next_env_entry++ = (char*) String_c_str(entry);
		}
	// Null-terminate the list.
	*next_env_entry++ = NULL;

	return environ;
}



