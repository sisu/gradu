DIRS:=.

SRC:=$(wildcard $(addsuffix /*.cpp,$(DIRS)))
TSRC:=$(wildcard $(addsuffix /*Test.cpp,$(DIRS)))
SRC:=$(filter-out $(TSRC),$(SRC))

OBJ:=$(patsubst %.cpp,obj/%.o,$(SRC))
TOBJ:=$(patsubst %.cpp,obj/%.o,$(TSRC))
TBIN:=$(patsubst %.cpp,obj/%,$(TSRC))

ODIR:=obj
ODIRS:=$(addprefix $(ODIR)/, $(DIRS))
#BASEFLAGS:=-Wall -Wextra -std=c++0x -MMD
BASEFLAGS:=-Wall -Wextra -std=c++14 -MMD
DFLAGS:=-g
OFLAGS:=-O3 -DBOOST_DISABLE_ASSERTS -ffast-math
CXXFLAGS:=$(BASEFLAGS) $(DFLAGS)
#CXXFLAGS:=$(BASEFLAGS) $(OFLAGS)
TFLAGS:=-Wall -Wextra -std=c++14 -MMD -I. -g
CC=clang++

.PHONY: all clean $(BIN)
BIN:=minlink

all: $(ODIRS) $(BIN)

test: $(ODIRS) $(TBIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(CXXFLAGS)

$(OBJ): $(ODIR)/%.o: %.cpp
	$(CC) $< -c -o "$@" $(CXXFLAGS)

$(TOBJ): $(ODIR)/%.o: %.cpp
	$(CC) $< -c -o "$@" $(TFLAGS)

$(TBIN): %: %.o
	$(CC) $^ -o $@ $(TFLAGS) -lgtest -lgtest_main -lgmock -pthread
	"./$@"

$(ODIR)/./decompositionTest: $(ODIR)/./decomposition.o $(ODIR)/./obstacles.o

$(ODIR)/./pathTest: $(ODIR)/./decomposition.o $(ODIR)/./path.o $(ODIR)/./obstacles.o

clean:
	rm -rf "$(ODIR)"

$(ODIRS):
	mkdir -p "$@"

include $(wildcard $(ODIR)/*.d)
