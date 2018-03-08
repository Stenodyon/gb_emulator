BUILD_DIR=build/

SRC=$(wildcard GBEmu/*.cpp)
OBJ=$(addprefix $(BUILD_DIR),$(subst .cpp,.o,$(SRC)))

FLAGS=-std=c++17 -Wall -Wextra -I./lib/include
LIBS=-L./lib/lib/x64/ -lSDL2

BIN=GBEmu.exe

default: release

release: FLAGS+= -O2
release: $(BIN)

debug: FLAGS+= -g -D _DEBUG
debug: $(BIN).debug

clean:
	-rm -rf $(BUILD_DIR)
	-rm $(BIN) $(BIN).debug

.PHONY: release debug clean

$(BIN): $(OBJ)
	g++ $(FLAGS) $(OBJ) $(LIBS) -o $(BIN)

$(BIN).debug: $(OBJ)
	g++ $(FLAGS) $(OBJ) $(LIBS) -o $(BIN).debug

$(BUILD_DIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ -c $< $(FLAGS) -o $@
