
CXX = clang++

EXE := bf-cc
SRC != find . -name '*.cc'
OBJ := ${SRC:%.cc=%.o}
HDR != find . -name '*.h'

CXXFLAGS = --std=c++20 -pedantic
CXXFLAGS += -Wall -Wextra \
	    -Wformat -Wreturn-type -Wstrict-aliasing \
	    -Wcast-qual -Wcast-align -Wconversion \
	    -Wwrite-strings -Wsign-conversion \
	    -Wshadow \
	    -pedantic \
	    -fno-exceptions \
	    -fno-rtti \
	    -fPIE -fstack-clash-protection -fstack-protector-strong \
#	    -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract \
#	    -fsanitize=leak \
#	    -fsanitize=undefined \
#	    -fno-sanitize-recover=all
CXXFLAGS += -O2
#CXXFLAGS += -ggdb3 -pg

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(EXE)

$(OBJ): $(HDR)

.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)
	$(RM) $(EXE)

format: fmt
fmt:
	clang-format -i $(SRC) $(HDR)

.PHONY: clear format fmt
