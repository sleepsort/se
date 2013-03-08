CC	:= g++
V	:= @
RM	+= -r
#LIB += -lpthread
USER_FLAGS+= -Wall -O2 -I .
OBJ := ./obj/

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
$(OBJ)index: main/index.cpp $(objects)
	$(make-target)
$(OBJ)search: main/search.cpp $(objects)
	$(make-target)
$(OBJ)reader.o: ./index/reader.cpp
	$(make-intermediate)
$(OBJ)writer.o: ./index/writer.cpp
	$(make-intermediate)
$(OBJ)searcher.o: ./index/searcher.cpp
	$(make-intermediate)
$(OBJ)parser.o: ./query/parser.cpp
	$(make-intermediate)
$(OBJ)query.o: ./query/query.cpp
	$(make-intermediate)
$(OBJ)util.o: ./util/util.cpp
	$(make-intermediate)
sch:
	@./obj/search data/index
idx:
	@./obj/index data/shakespeare data/index


.PHONY:clean always test
always:
	$(V)mkdir -p $(OBJ)
clean:
	$(V)$(RM) obj 2>/dev/null
reset:
	$(V)$(RM) data/index 2>/dev/null
