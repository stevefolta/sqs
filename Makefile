PROGRAM := sqs
OBJECTS_DIR := objects
CFLAGS += -Wall

-include Makefile.local

SOURCES := main.c
SOURCES += Lexer.c Parser.c ParseNode.c Environment.c
SOURCES += ClassStatement.c Upvalues.c RunStatement.c Module.c
SOURCES += Method.c MethodBuilder.c ByteCode.c
SOURCES += BuiltinMethod.c
SOURCES += Class.c Object.c Init.c
SOURCES += String.c Boolean.c Int.c Float.c Array.c Dict.c ByteArray.c Nil.c
SOURCES += File.c LinesIterator.c Regex.c
SOURCES += Print.c Run.c Pipe.c Glob.c Path.c Env.c MiscFunctions.c Fail.c
SOURCES += Error.c UTF8.c
LIBRARIES = gc

OBJECTS = $(foreach source,$(SOURCES),$(OBJECTS_DIR)/$(source:.c=.o))
OBJECTS_SUBDIRS = $(foreach dir,$(SUBDIRS),$(OBJECTS_DIR)/$(dir))

ifndef VERBOSE_MAKE
	QUIET := @
endif

all: $(PROGRAM)

CPP := g++
CFLAGS += -MMD
CFLAGS += -g
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-result
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))
LINK_FLAGS += $(foreach lib,$(LIBRARIES),-l$(lib))

$(OBJECTS_DIR)/%.o: %.c
	@echo Compiling $<...
	$(QUIET) $(CC) -c $< -g $(CFLAGS) -o $@

$(OBJECTS): | $(OBJECTS_DIR)

$(PROGRAM): $(OBJECTS)
	@echo "Linking $@..."
	$(QUIET) $(CC) $(filter-out $(OBJECTS_DIR),$^) -g $(LINK_FLAGS) -o $@

$(OBJECTS_DIR):
	@echo "Making $@..."
	$(QUIET) mkdir -p $(OBJECTS_DIR) $(OBJECTS_SUBDIRS)

-include $(OBJECTS_DIR)/*.d


.PHONY: runnit
runnit: $(PROGRAM)
	@./$(PROGRAM) $(RUN_ARGS)

.PHONY: clean
clean:
	rm -rf $(OBJECTS_DIR)

.PHONY: tags
tags:
	ctags -R .

.PHONY: edit-all
edit-all:
	@ $(EDITOR) $(filter-out main.h,$(foreach source,$(SOURCES),$(source:.c=.h) $(source)))


