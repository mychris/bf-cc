
CXX = clang++

EXE := bf-cc
SRC != find . -name '*.cc'
HDR != find . -name '*.h'

all: $(EXE)

$(EXE): $(SRC) $(HDR)
	$(CXX) -O0 -g -Wall -Wextra --std=c++17 $(SRC) -o $(EXE)

clean:
	$(RM) $(EXE)

format: fmt
fmt:
	clang-format -i $(SRC) $(HDR)

.PHONY: clear format fmt
