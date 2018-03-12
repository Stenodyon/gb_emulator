BUILD_DIR=build/

SRC=$(wildcard GBEmu/*.cpp)
OBJ=$(addprefix $(BUILD_DIR),$(subst .cpp,.o,$(SRC)))

FLAGS=-std=c++17 -Wall -Wextra -I./lib/include
LIBS=-L./lib/lib/x64/ -lSDL2

BIN=GBEmu.exe
BIN_DEBUG=GBEmu.debug.exe
BIN_TEST=GBEmu.test.exe

default: release

release: FLAGS+= -O2
release: $(BIN)

debug: FLAGS+= -g -D _DEBUG
debug: $(BIN_DEBUG)

test: FLAGS+= -D _TESTING
test: $(BIN_TEST)
	$(BIN_TEST)

clean:
	-rm -rf $(BUILD_DIR)
	-rm $(BIN) $(BIN_DEBUG) $(BIN_TEST)

.PHONY: release debug test clean

$(BIN): $(OBJ)
	g++ $(FLAGS) $(OBJ) $(LIBS) -o $(BIN)

$(BIN_DEBUG): $(OBJ)
	g++ $(FLAGS) $(OBJ) $(LIBS) -o $(BIN_DEBUG)

$(BIN_TEST): $(OBJ)
	g++ $(FLAGS) $(OBJ) $(LIBS) -o $(BIN_TEST)

$(BUILD_DIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ -c $< $(FLAGS) -o $@
