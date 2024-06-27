SQSC_SOURCES += sqsc Parser ParseNode Context Block Statements ClassStatement FunctionStatement
SQSC_SOURCES += Expressions Calls Modules Builtins Codegen CCodegen QBECodegen
SQSC_SOURCES += Lexer
SQSC_SOURCES += sqs_compiled.h sqs_compiled.c

SQS_SOURCE ?= ../..

.PHONY: clean
clean:
	rm -rf build

.PHONY: test
test:
	@./sqsc $(SQS_SOURCE)/all-tests

.PHONY: self
self:
	@./sqsc ./sqsc

.PHONY: edit-all
edit-all:
	$(EDITOR) $(SQSC_SOURCES)

