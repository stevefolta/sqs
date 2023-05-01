PROGRAM := qqs
OBJECTS_DIR := objects
CFLAGS += -Wall

-include Makefile.local

SOURCES := main.c
SOURCES += Lexer.c Parser.c ParseNode.c Method.c MethodBuilder.c
SOURCES += String.c Boolean.c Array.c Dict.c ByteArray.c
SOURCES += Error.c
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
CFLAGS += $(foreach switch,$(SWITCHES),-D$(switch))
LINK_FLAGS += $(foreach lib,$(LIBRARIES),-l$(lib))

$(OBJECTS_DIR)/%.o: %.c
	@echo Compiling $<...
	$(QUIET) $(CC) -c $< -g $(CFLAGS) -o $@

$(OBJECTS): | $(OBJECTS_DIR)

$(PROGRAM): $(OBJECTS)
	@echo "Linking $@..."
	$(QUIET) $(CPP) $(filter-out $(OBJECTS_DIR),$^) -g $(LINK_FLAGS) -o $@
	@echo "---------------------------------------------"
	@echo

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
	$(EDITOR) $(foreach source,$(SOURCES),$(source:.c=.h) $(source))


