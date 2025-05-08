
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC_DIR = src
BIN = sort_exp

SRCS = $(wildcard $(SRC_DIR)/*.cpp)

all: $(BIN)

$(BIN): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
clean:
	rm -f $(BIN)

.PHONY: all clean