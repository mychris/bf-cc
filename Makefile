
CXX = clang++
EXE := bf-cc

all: $(EXE)

$(EXE): bf.cc opt_fusion_op.cc
	$(CXX) -g -Wall -Wextra --std=c++17 bf.cc opt_fusion_op.cc -o bf

format:
	clang-format bf.cc
