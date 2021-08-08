CC=gcc
SRC=./
BIN=./bin/
OBJ=./obj/
DEPENDENCIES_FILE=dependencies.mk
CFLAGS=-std=c89
LFLAGS=
INSTALL_PATH=/usr/local/bin/

DEVELOPMENT ?= 0

ifeq ($(DEVELOPMENT), 1)
	CFLAGS+=-g -Wall -O0
else
	CFLAGS+=-O3 -DNDEBUG
endif

PROGRAM=jcunit
PROGRAM_TEST_TOKENIZER=test-tokenizer

SAMPLE_FILE=./examples/base.test
SAMPLE_TEST_TOKENIZER=./examples/test-tokenizer.test

vpath %.c $(SRC)
vpath %.c $(HEADERS)

all: build run

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
OBJECTS+=list-iterator.o
OBJECTS+=compiler.o
OBJECTS+=source.o
OBJECTS+=options.o

ifeq ($(DEVELOPMENT), 1)
	OBJECTS+=print.o
endif

OBJECTS_TEST_TOKENIZER=test-tokenizer.o print.o tokenizer.o allocate.o string.o util.o

build: $(addprefix $(OBJ), $(OBJECTS)) | dependencies
	$(CC) $(LFLAGS) $^ -o $(BIN)$(PROGRAM)

test-tokenizer: $(addprefix $(OBJ), $(OBJECTS_TEST_TOKENIZER))
	$(CC) $(LFLAGS) $^ -o $(BIN)$(PROGRAM_TEST_TOKENIZER)
	$(BIN)$(PROGRAM_TEST_TOKENIZER) $(SAMPLE_TEST_TOKENIZER)

dependencies:
	@rm -rf $(DEPENDENCIES_FILE)
	@$(foreach file, $(OBJECTS), $(CC) -MT $(OBJ)$(file) -MM $(patsubst %.o, %.c, $(file)) >> $(DEPENDENCIES_FILE);)

run:
	$(BIN)$(PROGRAM) $(SAMPLE_FILE)

install: build
	cp $(BIN)$(PROGRAM) $(INSTALL_PATH)$(PROGRAM)

uninstall:
	rm -rfv $(INSTALL_PATH)$(PROGRAM)

clean:
	rm -rfv $(BIN)$(PROGRAM)
	rm -rfv $(OBJ)*
	rm -rfv $(BIN)$(PROGRAM_TEST_TOKENIZER)

$(OBJ)%.o: %.c
	$(CC) $(CFLAGS) -c $(SRC)$*.c -o $@

-include $(DEPENDENCIES_FILE)
