
CXX = clang++

EXE := bf-cc
SRC != find . -name '*.cc'
OBJ := ${SRC:%.cc=%.o}
HDR != find . -name '*.h'

CXXFLAGS = --std=c++20 -pedantic
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -O2 -g

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
