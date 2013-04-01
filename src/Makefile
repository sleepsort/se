#!/usr/bin/env vim
# note:
# 
# $@: target
# $<: first dependency
# $^: all dependencies
# $?: dependencies newer than target
# $+: similar with $^, but reserving duplicate
#
# http://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
#

ARCH := $(shell getconf LONG_BIT)
CC	:= g++
V	:= @
RM	+= -r
LIB += ../lib/lib$(ARCH)pugixml.a ../lib/lib$(ARCH)porter.a
#LIB += ../lib/lib32pugixml.a ../lib/lib32porter.a
#LIB += ../lib/lib64pugixml.a ../lib/lib64porter.a
USER_FLAGS += -Wall -O2 -I .
OBJ := ../obj/

# targets: executable with main method
# objects: .o files, (files under 'template' are omited)
# dirctry: folders with include files (also omit 'template')
targets := $(wildcard main/*.cpp) $(wildcard main/*.c)
objects := $(wildcard */*.cpp) $(wildcard */*.c)
objects := $(filter-out $(targets), $(objects))
objects := $(patsubst %.cpp,%.o,$(objects))
dirctry := $(sort $(dir $(objects)))
dirctry := $(patsubst %/,%,$(dirctry))
dirctry := $(filter-out template, $(dirctry))
objects := $(filter-out template/%,$(objects))
objects := $(notdir $(objects))
objects := $(addprefix $(OBJ),$(objects))
targets := $(basename $(notdir $(targets)))
targets := $(addprefix $(OBJ),$(targets))

define make-target
$(OBJ)$1: main/$1.cpp $(objects) template/btree.h template/btree.cpp
	@echo + cc $$<
	$(V)$(CC) $(USER_FLAGS) -o $$@ main/$1.cpp $(objects) $(LIB)
endef

define make-intermediate 
$(OBJ)%.o: $1/%.cpp $1/%.h
	@echo + cc $$<
	$(V)$(CC) -c $(USER_FLAGS) -o $$@ $$<
endef


all:always $(targets)

$(foreach btar,$(targets),$(eval $(call make-target,$(notdir $(btar)))))
$(foreach bdir,$(dirctry),$(eval $(call make-intermediate,$(bdir))))

.PHONY:clean always reset test
always:
	$(V)mkdir -p $(OBJ)
clean:
	$(V)$(RM) $(OBJ)
