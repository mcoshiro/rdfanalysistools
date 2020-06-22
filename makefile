WARNINGS := -Wall -Wextra -pedantic -Werror -Wshadow -Woverloaded-virtual -Wold-style-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-y2k -Winit-self -Winvalid-pch -Wlong-long -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith -Wredundant-decls -Wstack-protector -Wswitch-default -Wswitch-enum -Wundef -Wunused -Wvariadic-macros -Wwrite-strings -Wctor-dtor-privacy -Wnon-virtual-dtor -Wsign-promo -Wsign-compare -Wunsafe-loop-optimizations -Wfloat-equal -Wsign-conversion -Wunreachable-code
#isystem the root includes otherwise there will be a million errors
ROOTFLAGS := -isystem $(shell root-config --incdir) $(WARNINGS) $(shell root-config --cflags)

CORE_OBJECTS := $(addprefix bin/core/, $(addsuffix .o, $(notdir $(basename $(wildcard src/core/*.cpp)))))

all: $(CORE_OBJECTS)

#core objects
bin/core/%.o: src/core/%.cpp
	g++ $(ROOTFLAGS) -o $@ -c $<

clean:
	-rm bin/core/*
