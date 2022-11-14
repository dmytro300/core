#Current make system
BIN=bin/
SRC=src/

MAKEFLAGS:=-r
SHELL=bash

CC=c++
CFLAGS=-std=c++2a -g  -Wall -Wextra $(CXXFLAGS) -Wno-nonportable-include-path
LDFLAGS=-Wl,-demangle

all: $(BIN)cmd2 $(BIN)test2

clean:
	@rm -rf bin

$(BIN)%:  $(BIN)%.o
	@mkdir -p $(@D)
	$(CC) $(INC) $^ $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS) -lstdc++ 

.PRECIOUS:$(SRC)%.h $(BIN)%.d

.SECONDEXPANSION:

#$(BIN)%.o: $(SRC)%.C $(BIN)%.d $$(file < $(BIN)%.d)
$(BIN)%.o: $(SRC)%.C $(BIN)%.d
	@mkdir -p $(@D)
	@printf "%s\n" $^
	$(CC) $(CFLAGS) -MMD -MF $(BIN)$*.td -o $@ -c $<
	read obj src headers <$(BIN)$*.td; echo "$$headers" >$(BIN)$*.d
	touch -c $@

$(BIN)test2:  $(BIN)test2.o
$(BIN)cmd2:  $(BIN)cmd2.o $(BIN)admin.o

$(BIN)%.d : ;
%.h: ;

