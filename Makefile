
CXX = clang++

EXE := bf-cc
SRC != find . -name '*.cc'
HDR != find . -name '*.h'

all: $(EXE)

$(EXE): $(SRC) $(HDR)
	$(CXX) -O2 -Wall -Wextra --std=c++17 $(SRC) -o $(EXE)

clean:
	$(RM) $(EXE)

.PHONY: clear format
