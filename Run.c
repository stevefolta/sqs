#include "Run.h"
#include "Pipe.h"
#include "File.h"
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
declare_string(stdin_string, "stdin");
declare_string(stdout_string, "stdout");
declare_string(stderr_string, "stderr");
declare_string(env_string, "env");

extern char** build_environ(Dict* env);


typedef struct RunResult {
	Class* class_;
	pid_t pid;
	bool done;
	int return_code;
	Pipe* capture_pipe;
	Object* captured_output;
	} RunResult;
Class RunResult_class;
static RunResult* new_RunResult(pid_t pid, Pipe* capture_pipe);
Object* RunResult_wait(Object* super, Object** args);
void RunResult_capture(RunResult* self);


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
	int stdin_fd = -1, stdout_fd = -1, stderr_fd = -1;
	Pipe* stdin_pipe = NULL;
	Pipe* stdout_pipe = NULL;
	Pipe* stderr_pipe = NULL;
	Dict* env = NULL;
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		capture = Dict_option_turned_on(options, &capture_string);
		if (Dict_option_turned_off(options, &wait_string))
			wait = false;
		Object* option = Dict_at(options, &stdin_string);
		if (option) {
			if (option->class_ == &Pipe_class) {
				stdin_pipe = (Pipe*) option;
				stdin_fd = stdin_pipe->read_fd;
				}
			else if (option->class_ == &File_class)
				stdin_fd = File_fd((struct File*) option);
			else
				Error("run(): \"stdin\" must be a Pipe or a File.");
			}
		option = Dict_at(options, &stdout_string);
		if (option) {
			if (capture)
				Error("run(): Can't use \"capture\" and \"stdout\" options at the same time.");
			if (option->class_ == &Pipe_class) {
				stdout_pipe = (Pipe*) option;
				stdout_fd = stdout_pipe->write_fd;
				}
			else if (option->class_ == &File_class) {
				stdout_fd = File_fd((struct File*) option);
				File_flush(option, NULL);
				}
			else
				Error("run(): \"stdout\" must be a Pipe or a File.");
			}
		option = Dict_at(options, &stderr_string);
		if (option) {
			if (option->class_ == &Pipe_class) {
				stderr_pipe = (Pipe*) option;
				stderr_fd = stderr_pipe->write_fd;
				}
			else if (option->class_ == &File_class) {
				stderr_fd = File_fd((struct File*) option);
				File_flush(option, NULL);
				}
			else
				Error("run(): \"stderr\" must be a Pipe or a File.");
			}
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
	if (capture) {
		stdout_pipe = new_Pipe();
		stdout_fd = stdout_pipe->write_fd;
		}

	// Fork.
	pid_t pid = fork();
	if (pid < 0)
		Error("run(): fork() failed (%s).", strerror(errno));
	else if (pid == 0) {
		// We're now in the child.

		// If piping or redirecting, set that up.
		if (stdin_fd >= 0) {
			dup2(stdin_fd, STDIN_FILENO);
			if (stdin_pipe) {
				close(stdin_pipe->write_fd);
				stdin_pipe->write_fd = -1;
				}
			}
		if (stdout_fd >= 0) {
			dup2(stdout_fd, STDOUT_FILENO);
			if (stdout_pipe) {
				close(stdout_pipe->read_fd); 	// Close the read end.
				stdout_pipe->read_fd = -1;
				}
			}
		if (stderr_fd >= 0) {
			dup2(stderr_fd, STDERR_FILENO);
			if (stderr_pipe) {
				close(stderr_pipe->read_fd);
				stderr_pipe->read_fd = -1;
				}
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

		// Wait for child to exit.
		RunResult* run_result = new_RunResult(pid, stdout_pipe);
		if (wait) {
			if (capture)
				RunResult_capture(run_result);
			RunResult_wait((Object*) run_result, NULL);
			}
		return (Object*) run_result;
		}

	return NULL;
}


static RunResult* new_RunResult(pid_t pid, Pipe* capture_pipe)
{
	RunResult* self = alloc_obj(RunResult);
	self->class_ = &RunResult_class;
	self->pid = pid;
	self->capture_pipe = capture_pipe;
	return self;
}


Object* RunResult_return_code(Object* super, Object** args)
{
	RunResult_wait(super, NULL);
	return (Object*) new_Int(((RunResult*) super)->return_code);
}

Object* RunResult_ok(Object* super, Object** args)
{
	RunResult_wait(super, NULL);
	return make_bool(((RunResult*) super)->return_code == 0);
}

Object* RunResult_output(Object* super, Object** args)
{
	RunResult_capture((RunResult*) super);
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

void RunResult_capture(RunResult* self)
{
	if (self->capture_pipe == NULL || self->captured_output)
		return;

	self->captured_output = Pipe_capture(self->capture_pipe, true, 0);
	Pipe_close(self->capture_pipe);
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



