# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Iorder_book/include -Wall -Wextra -O2

# Directories
SRC_DIR = order_book/src
OBJ_DIR = build
BIN = orderbook

# Find all .cpp source files
SRCS = $(filter-out $(SRC_DIR)/test_main.cpp $(SRC_DIR)/performance_benchmark.cpp, $(wildcard $(SRC_DIR)/*.cpp))

# Convert source files to object files in build/
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(BIN)

# Linking the final binary
$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN)

# Run the binary
run: all
	./$(BIN)

test: $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $(OBJ_DIR)/test_main.o
	$(CXX) $(CXXFLAGS) -o test_runner $^

$(OBJ_DIR)/test_main.o: $(SRC_DIR)/test_main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

benchmark: $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $(OBJ_DIR)/performance_benchmark.o
	$(CXX) $(CXXFLAGS) -o performance_benchmark $^

$(OBJ_DIR)/performance_benchmark.o: $(SRC_DIR)/performance_benchmark.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
