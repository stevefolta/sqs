SQSC_SOURCES += sqsc Parser ParseNode Context Block Statements ClassStatement FunctionStatement
SQSC_SOURCES += Expressions Calls Modules Codegen Builtins
SQSC_SOURCES += Lexer
SQSC_SOURCES += sqs_compiled.h sqs_compiled.c

.PHONY: test
test:
	@./sqsc $(SQS_SOURCE)/all-tests

.PHONY: self
self:
	@./sqsc ./sqsc

.PHONY: edit-all
edit-all:
	$(EDITOR) $(SQSC_SOURCES)

