#!/usr/bin/env vim

CC	:= g++
V	:= @
RM	+= -r
#LIB += -lpthread
USER_FLAGS+= -Wall -O2 -I .
OBJ := ./obj/
#EXCLUDE := index

targets := $(wildcard main/*.cpp) $(wildcard main/*.c)
objects := $(wildcard */*.cpp) $(wildcard */*.c)
objects := $(filter-out $(targets), $(objects))
objects := $(filter-out $(EXCLUDE)/%, $(objects))
objects := $(patsubst %.cpp,%.o,$(objects))
dirctry := $(sort $(dir $(objects)))
dirctry := $(patsubst %/,%,$(dirctry))
dirctry := $(filter-out $(EXCLUDE),$(dirctry))
objects := $(notdir $(objects))
objects := $(addprefix $(OBJ),$(objects))
targets := $(basename $(notdir $(targets)))
targets := $(addprefix $(OBJ),$(targets))

define make-target
$(OBJ)$1: main/$1.cpp $(objects)
	@echo + cc $$<
	$(V)$(CC) $(USER_FLAGS) -o $$@ $$^ $(LIB)
endef

define make-intermediate 
$(OBJ)%.o: $1/%.cpp
	@echo + cc $$<
	$(V)$(CC) -c $(USER_FLAGS) -o $$@ $$^
endef


all:always $(targets)

$(foreach btar,$(targets),$(eval $(call make-target,$(notdir $(btar)))))
$(foreach bdir,$(dirctry),$(eval $(call make-intermediate,$(bdir))))

run:
	@./obj/run
test:
	@./obj/test

.PHONY:clean always reset test
always:
	$(V)mkdir -p $(OBJ)
clean:
	$(V)$(RM) $(OBJ) data/*
reset:
	$(V)$(RM) data/*
