#!/usr/bin/env vim

CC	:= g++
V	:= @
RM	+= -r
#LIB += -lpthread
USER_FLAGS+= -Wall -O2 -I .
OBJ := ../obj/

define make-intermediate 
	@echo + cc $<
	$(V)$(CC) -c $(USER_FLAGS) -o $@ $^
endef
define make-target
	@echo + cc $<
	$(V)$(CC) $(USER_FLAGS) -o $@ $^ $(LIB)
	$(V)#mv $@ $(OBJ)
endef

targets := $(wildcard main/*.cpp) $(wildcard main/*.c)
objects := $(wildcard */*.cpp) $(wildcard */*.c)
objects := $(filter-out $(targets), $(objects))
objects := $(patsubst %.cpp,%.o,$(objects))
objects := $(notdir $(objects))
objects := $(addprefix $(OBJ),$(objects))
targets := $(basename $(notdir $(targets)))
targets := $(addprefix $(OBJ),$(targets))


all:always $(targets)
$(OBJ)test: main/test.cpp $(objects)
	$(make-target)
$(OBJ)stem: main/stem.cpp $(objects)
	$(make-target)
$(OBJ)index: main/index.cpp $(objects)
	$(make-target)
$(OBJ)search: main/search.cpp $(objects)
	$(make-target)
$(OBJ)reader.o: ./index/reader.cpp
	$(make-intermediate)
$(OBJ)writer.o: ./index/writer.cpp
	$(make-intermediate)
$(OBJ)postings.o: ./index/postings.cpp
	$(make-intermediate)
$(OBJ)searcher.o: ./search/searcher.cpp
	$(make-intermediate)
$(OBJ)suggester.o: ./search/suggester.cpp
	$(make-intermediate)
$(OBJ)parser.o: ./query/parser.cpp
	$(make-intermediate)
$(OBJ)query.o: ./query/query.cpp
	$(make-intermediate)
$(OBJ)util.o: ./util/util.cpp
	$(make-intermediate)
$(OBJ)porter.o: ./util/porter.cpp
	$(make-intermediate)

.PHONY:clean always reset test
always:
	$(V)mkdir -p $(OBJ)
