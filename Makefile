#Current make system
BIN=bin/
SRC=src/

all: $(BIN)cmd2 $(BIN)test2

clean:
	@rm -rf bin

$(BIN)test2:  $(BIN)test2.o
$(BIN)cmd2:  $(BIN)cmd2.o $(BIN)admin.o

include Makefile_inc
