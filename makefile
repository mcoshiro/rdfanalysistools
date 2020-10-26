WARNINGS := -Wall -Wextra -pedantic -Werror -Wshadow -Woverloaded-virtual -Wold-style-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-y2k -Winit-self -Winvalid-pch -Wlong-long -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith -Wredundant-decls -Wstack-protector -Wswitch-default -Wswitch-enum -Wundef -Wunused -Wvariadic-macros -Wwrite-strings -Wctor-dtor-privacy -Wnon-virtual-dtor -Wsign-promo -Wsign-compare -Wunsafe-loop-optimizations -Wfloat-equal -Wsign-conversion -Wunreachable-code
#isystem the root includes otherwise there will be a million errors
COMPFLAGS := -isystem $(shell root-config --incdir) $(WARNINGS) $(shell root-config --cflags) -I inc
LINKFLAGS := $(shell root-config --ld) $(shell root-config --ldflags) $(shell root-config --libs)

CORE_OBJECTS := $(addprefix bin/core/, $(addsuffix .o, $(notdir $(basename $(wildcard src/core/*.cpp)))))
CORE_EXE_OBJECTS := $(addprefix bin/core/, $(addsuffix .o, $(notdir $(basename $(wildcard src/core/*.cxx)))))
CORE_EXECUTABLES := $(addprefix bin/core/, $(addsuffix .exe, $(notdir $(basename $(wildcard src/core/*.cxx)))))
TTZ_OBJECTS := $(addprefix bin/ttz/, $(addsuffix .o, $(notdir $(basename $(wildcard src/ttz/*.cpp)))))
TTZ_EXE_OBJECTS := $(addprefix bin/ttz/, $(addsuffix .o, $(notdir $(basename $(wildcard src/ttz/*.cxx)))))
TTZ_EXECUTABLES := $(addprefix bin/ttz/, $(addsuffix .exe, $(notdir $(basename $(wildcard src/ttz/*.cxx)))))
HHMET_OBJECTS := $(addprefix bin/higgsino/, $(addsuffix .o, $(notdir $(basename $(wildcard src/higgsino/*.cpp)))))
HHMET_EXE_OBJECTS := $(addprefix bin/higgsino/, $(addsuffix .o, $(notdir $(basename $(wildcard src/higgsino/*.cxx)))))
HHMET_EXECUTABLES := $(addprefix bin/higgsino/, $(addsuffix .exe, $(notdir $(basename $(wildcard src/higgsino/*.cxx)))))
MISC_OBJECTS := $(addprefix bin/misc/, $(addsuffix .o, $(notdir $(basename $(wildcard src/misc/*.cpp)))))
MISC_EXE_OBJECTS := $(addprefix bin/misc/, $(addsuffix .o, $(notdir $(basename $(wildcard src/misc/*.cxx)))))
MISC_EXECUTABLES := $(addprefix bin/misc/, $(addsuffix .exe, $(notdir $(basename $(wildcard src/misc/*.cxx)))))

all: $(CORE_OBJECTS) $(CORE_EXE_OBJECTS) $(CORE_EXECUTABLES) $(TTZ_OBJECTS) $(TTZ_EXE_OBJECTS) $(TTZ_EXECUTABLES) $(HHMET_OBJECTS) $(HHMET_EXE_OBJECTS) $(HHMET_EXECUTABLES) $(MISC_OBJECTS) $(MISC_EXE_OBJECTS) $(MISC_EXECUTABLES)

bin/core/%.o: src/core/%.cpp
	g++ $(COMPFLAGS) -o $@ -c $<

bin/core/%.o: src/core/%.cxx
	g++ $(COMPFLAGS) -o $@ -c $<

bin/core/%.exe: bin/core/%.o $(CORE_OBJECTS) 
	$(LINKFLAGS) -o $@ $^

bin/ttz/%.o: src/ttz/%.cpp
	g++ $(COMPFLAGS) -o $@ -c $<

bin/ttz/%.o: src/ttz/%.cxx
	g++ $(COMPFLAGS) -o $@ -c $<

bin/ttz/%.exe: bin/ttz/%.o $(CORE_OBJECTS) $(TTZ_OBJECTS)
	$(LINKFLAGS) -o $@ $^

bin/higgsino/%.o: src/higgsino/%.cpp
	g++ $(COMPFLAGS) -o $@ -c $<

bin/higgsino/%.o: src/higgsino/%.cxx
	g++ $(COMPFLAGS) -o $@ -c $<

bin/higgsino/%.exe: bin/higgsino/%.o $(CORE_OBJECTS) $(HHMET_OBJECTS)
	$(LINKFLAGS) -o $@ $^

bin/misc/%.o: src/misc/%.cpp
	g++ $(COMPFLAGS) -o $@ -c $<

bin/misc/%.o: src/misc/%.cxx
	g++ $(COMPFLAGS) -o $@ -c $<

bin/misc/%.exe: bin/misc/%.o $(CORE_OBJECTS) $(MISC_OBJECTS)
	$(LINKFLAGS) -o $@ $^

clean:
	-rm bin/core/*.o
	-rm bin/core/*.exe
	-rm bin/ttz/*.o
	-rm bin/ttz/*.exe
	-rm bin/higgsino/*.o
	-rm bin/higgsino/*.exe
	-rm bin/misc/*.o
	-rm bin/misc/*.exe
