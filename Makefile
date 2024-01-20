
CXX = clang++
EXE := bf-cc
SRC := bf.cc interp.cc opt_fusion_op.cc opt_peep.cc opt_comment_loop.cc optimize.cc util.cc

all: $(EXE)

$(EXE): $(SRC)
	$(CXX) -O2 -Wall -Wextra --std=c++17 $(SRC) -o $(EXE)

clean:
	$(RM) $(EXE)

format:
	clang-format bf.cc

.PHONY: clear format
