#Current make system
BIN=bin/
SRC=src/

MAKEFLAGS:=-r
SHELL=bash

CC=c++
CFLAGS=-std=c++17 -g  -Wall -Wextra $(CXXFLAGS)

all: $(BIN)cmd2

$(BIN)cmd2:  $(BIN)cmd2.o $(BIN)admin.o
	@mkdir -p $(@D)
	$(CC) $(INC) $^ $(CFLAGS) -o $@ $(LIBS) -lstdc++

.SECONDEXPANSION:
$(BIN)%.o: $(SRC)%.C $(BIN)%.d $$(file <$(BIN)%.d)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MF $(BIN)$*.td -o $@ -c $<
	read obj src headers <$(BIN)$*.td; echo "$$headers" >$(BIN)$*.d
	touch -c $@

$(BIN)%.d : ;
%.h: ;

