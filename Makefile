
CXX = clang++

EXE := bf-cc
EXE_TEST := bf-cc-test
SRC != find src -name '*.cc' -a -not -name 'bf.cc'
SRC_MAIN != find src -name 'bf.cc'
SRC_TEST != find test -name '*.cc'
OBJ := ${SRC:%.cc=%.o}
OBJ_MAIN := ${SRC_MAIN:%.cc=%.o}
OBJ_TEST := ${SRC_TEST:%.cc=%.o}
HDR != find src -name '*.h'

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

CPPFLAGS += -Isrc/

all: $(EXE)

test: $(EXE_TEST)

$(EXE): $(OBJ) $(OBJ_MAIN)
	$(CXX) $(CXXFLAGS) $(OBJ) $(OBJ_MAIN) -o $(EXE)

$(EXE_TEST): $(OBJ) $(OBJ_TEST)
	$(CXX) $(CXXFLAGS) $(OBJ) $(OBJ_TEST) -o $(EXE_TEST) -lgtest

$(OBJ): $(HDR)
$(OBJ_MAIN): $(HDR)
$(OBJ_TEST): $(HDR)

.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

check: $(EXE_TEST)
	./$(EXE_TEST) --gtest_shuffle --gtest_color=no --gtest_brief=1

checkfull: $(EXE_TEST)
	./$(EXE_TEST)

clean:
	$(RM) $(OBJ)
	$(RM) $(OBJ_MAIN)
	$(RM) $(OBJ_TEST)
	$(RM) $(EXE)
	$(RM) $(EXE_TEST)

format: fmt
fmt:
	clang-format -i $(SRC) $(SRC_MAIN) $(SRC_TEST) $(HDR)

.PHONY: all test check checkfull clean format fmt
