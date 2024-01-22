
CXX = clang++

EXE := bf-cc
SRC != find . -name '*.cc'
HDR != find . -name '*.h'

CXXFLAGS = --std=c++20 -pedantic
CXXFLAGS += -Wall -Wextra

all: $(EXE)

$(EXE): $(SRC) $(HDR)
	$(CXX) -O2 -g $(CXXFLAGS) $(SRC) -o $(EXE)

clean:
	$(RM) $(EXE)

format: fmt
fmt:
	clang-format -i $(SRC) $(HDR)

.PHONY: clear format fmt
