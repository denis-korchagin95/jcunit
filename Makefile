CC=gcc
SRC=./
BIN=./bin/
OBJ=./obj/
DEPENDENCIES_FILE=dependencies.mk
CFLAGS=-g -Wall -std=c89
LFLAGS=
INSTALL_PATH=/usr/local/bin/

PROGRAM=jcunit
PROGRAM_TEST_TOKENIZER=test-tokenizer

SAMPLE_FILE=./examples/base.test
SAMPLE_TEST_TOKENIZER=./examples/test-tokenizer.test

vpath %.c $(SRC)
vpath %.c $(HEADERS)

all: build run

OBJECTS=main.o tokenizer.o allocate.o string.o print.o\
		list.o ast.o parse.o assembler.o runner.o finder.o child_process.o\
		show_result.o util.o fs.o
OBJECTS_TEST_TOKENIZER=test-tokenizer.o print.o tokenizer.o allocate.o string.o util.o

build: $(addprefix $(OBJ), $(OBJECTS)) | dependencies
	@$(CC) $(LFLAGS) $^ -o $(BIN)$(PROGRAM)

test-tokenizer: $(addprefix $(OBJ), $(OBJECTS_TEST_TOKENIZER))
	@$(CC) $(LFLAGS) $^ -o $(BIN)$(PROGRAM_TEST_TOKENIZER)
	$(BIN)$(PROGRAM_TEST_TOKENIZER) $(SAMPLE_TEST_TOKENIZER)

dependencies:
	@rm -rf $(DEPENDENCIES_FILE)
	@$(foreach file, $(OBJECTS), $(CC) -MT $(OBJ)$(file) -MM $(patsubst %.o, %.c, $(file)) >> $(DEPENDENCIES_FILE);)

run:
	$(BIN)$(PROGRAM) $(SAMPLE_FILE)

install:
	cp $(BIN)$(PROGRAM) $(INSTALL_PATH)$(PROGRAM)

uninstall:
	rm -rfv $(INSTALL_PATH)$(PROGRAM)

clean:
	@rm -rfv $(BIN)$(PROGRAM)
	@rm -rfv $(OBJ)*
	@rm -rfv $(BIN)$(PROGRAM_TEST_TOKENIZER)

$(OBJ)%.o: %.c
	$(CC) $(CFLAGS) -c $(SRC)$*.c -o $@

-include $(DEPENDENCIES_FILE)
