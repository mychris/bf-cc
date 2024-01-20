
CXX = clang++
EXE := bf-cc

all: $(EXE)

$(EXE): bf.cc
	$(CXX) -g -Wall -Wextra --std=c++17 bf.cc -o bf

format:
	clang-format bf.cc
