ifneq (,)
This makefile requires GNU Make.
endif

SHELL=bash

MAKEFLAGS:=-r
.SUFFIXES:

all_cxxflags:=-std=c++17 -g -Wall -Wextra $(CXXFLAGS)
basedir:=..
vpath %.C $(basedir)/src

stem:=cmd2
targets:=$(addsuffix .tsk, $(stem))
obj:=$(addsuffix .o, $(stem))
tdfiles:=$(addsuffix .td, $(stem))
dfiles:=$(addsuffix .d, $(stem))
.SECONDARY: $(obj)

all: $(targets)
$(targets): %.tsk: %.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lstdc++

.SECONDEXPANSION:
%.o: %.C %.d $$(file <%.d)
	$(CXX) $(CPPFLAGS) $(all_cxxflags) -MMD -MF $*.td -o $@ -c $<
	read obj src headers <$*.td; echo "$$headers" >$*.d
	touch -c $@

$(dfiles): ;
%.h: ;

clean:
	-rm -f $(targets) $(obj) $(tdfiles) $(dfiles)

print-%: force
	$(info $*=$($*))

force: ;

.PHONY: all clean force

$(basedir)/Makefile::;
