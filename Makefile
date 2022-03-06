CC=gcc
SRC=./
BIN=./bin/
OBJ=./obj/
DEPENDENCIES_FILE=dependencies.mk
CFLAGS=-std=c89
LFLAGS=
INSTALL_PATH=/usr/local/bin/
TESTERS_PATH=testers/
BIN_TESTERS=$(BIN)$(TESTERS_PATH)
OBJ_TESTERS=$(OBJ)$(TESTERS_PATH)

DEVELOPMENT ?= 0

ifeq ($(DEVELOPMENT), 1)
	CFLAGS+=-g -Wall -O0
else
	CFLAGS+=-O3 -DNDEBUG
endif

TESTERS=tokenizer-tester tokenizer-special-mode-tester parser-tester compiler-tester

PROGRAM=jcunit

SAMPLE_FILE=./examples/base.test

vpath %.c $(SRC)
vpath %.c $(HEADERS)

all: build

OBJECTS+=main.o
OBJECTS+=tokenizer.o
OBJECTS+=allocate.o
OBJECTS+=string.o
OBJECTS+=list.o
OBJECTS+=ast.o
OBJECTS+=parse.o
OBJECTS+=assembler.o
OBJECTS+=runner.o
OBJECTS+=finder.o
OBJECTS+=child-process.o
OBJECTS+=show-result.o
OBJECTS+=util.o
OBJECTS+=fs.o
OBJECTS+=version.o
OBJECTS+=test-suite-iterator.o
OBJECTS+=compiler.o
OBJECTS+=source.o
OBJECTS+=options.o
OBJECTS+=application.o

ifeq ($(DEVELOPMENT), 1)
	OBJECTS+=print.o
endif

OBJECTS_TOKENIZER_TESTER=$(TESTERS_PATH)tokenizer-tester.o print.o tokenizer.o allocate.o string.o util.o options.o
OBJECTS_TOKENIZER_SPECIAL_MODE_TESTER=$(TESTERS_PATH)tokenizer-special-mode-tester.o print.o tokenizer.o allocate.o string.o util.o options.o
OBJECTS_PARSER_TESTER=$(TESTERS_PATH)parser-tester.o print.o tokenizer.o allocate.o string.o util.o options.o parse.o ast.o list.o
OBJECTS_COMPILER_TESTER=$(TESTERS_PATH)compiler-tester.o print.o tokenizer.o allocate.o string.o util.o options.o parse.o ast.o list.o compiler.o assembler.o finder.o

build: $(addprefix $(OBJ), $(OBJECTS)) | dependencies
	$(CC) $(LFLAGS) $^ -o $(BIN)$(PROGRAM)

tokenizer-tester: $(addprefix $(OBJ), $(OBJECTS_TOKENIZER_TESTER))
	$(CC) $(LFLAGS) $^ -o $(BIN_TESTERS)tokenizer-tester

tokenizer-special-mode-tester: $(addprefix $(OBJ), $(OBJECTS_TOKENIZER_SPECIAL_MODE_TESTER))
	$(CC) $(LFLAGS) $^ -o $(BIN_TESTERS)tokenizer-special-mode-tester

parser-tester: $(addprefix $(OBJ), $(OBJECTS_PARSER_TESTER))
	$(CC) $(LFLAGS) $^ -o $(BIN_TESTERS)parser-tester

compiler-tester: $(addprefix $(OBJ), $(OBJECTS_COMPILER_TESTER))
	$(CC) $(LFLAGS) $^ -o $(BIN_TESTERS)compiler-tester

dependencies:
	@rm -rf $(DEPENDENCIES_FILE)
	@$(foreach file, $(OBJECTS), $(CC) -MT $(OBJ)$(file) -MM $(patsubst %.o, %.c, $(file)) >> $(DEPENDENCIES_FILE);)

testers: $(TESTERS)

install: build
	cp $(BIN)$(PROGRAM) $(INSTALL_PATH)$(PROGRAM)

uninstall:
	rm -rfv $(INSTALL_PATH)$(PROGRAM)

clean:
	rm -rfv $(BIN)$(PROGRAM)
	rm -rfv $(OBJ)*.o
	rm -rfv $(OBJ_TESTERS)*.o
	rm -rfv $(BIN_TESTERS)*

$(OBJ)%.o: %.c
	$(CC) $(CFLAGS) -c $(SRC)$*.c -o $@

-include $(DEPENDENCIES_FILE)
