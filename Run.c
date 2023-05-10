#include "Run.h"
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

static Object* capture_output(int fd, bool as_string, size_t size_limit);


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
	int capture_fds[2] = { -1, -1 };
	if (capture) {
		if (pipe(capture_fds) != 0)
			Error("run(): Couldn't open a pipe to capture output (%s).", strerror(errno));
		}

	// Fork.
	pid_t pid = fork();
	if (pid < 0)
		Error("run(): fork() failed (%s).", strerror(errno));
	else if (pid == 0) {
		// We're now in the child.

		// If capturing, set that up.
		if (capture) {
			close(capture_fds[0]); 	// Close the read end.
			dup2(capture_fds[1], STDOUT_FILENO);
			}

		// Run.
		execvp(argv[0], argv);
		Error("run(): Failed to start the program (%s).", strerror(errno));
		}
	else {
		// This is still the parent process.

		// Capture output.
		Object* captured_output = NULL;
		if (capture) {
			close(capture_fds[1]); 	// Close the write end.
			captured_output = capture_output(capture_fds[0], true, 0);
			close(capture_fds[0]);
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


static Object* capture_output(int fd, bool as_string, size_t size_limit)
{
	ByteArray* bytes = new_ByteArray();

	int buf_size = 1024;
	uint8_t* buffer = alloc_mem_no_pointers(buf_size);
	while (true) {
		ssize_t bytes_read = read(fd, buffer, buf_size);
		if (bytes_read < 0)
			Error("Error while capturing run() output (%s).", strerror(errno));
		else if (bytes_read == 0) {
			// EOF.
			break;
			}
		ByteArray_append_bytes(bytes, buffer, bytes_read);
		if (size_limit != 0 && bytes->size >= size_limit)
			Error("Capturing too much output in run().");
		}

	return (as_string ? (Object*) ByteArray_as_string(bytes) : (Object*) bytes);
}



