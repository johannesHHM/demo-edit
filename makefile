SRC_DIR := src
BIN_DIR := build
OBJ_DIR := $(BIN_DIR)/obj

EXE := $(BIN_DIR)/dedit
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall
LDFLAGS  := -Llib
LDLIBS   := -lm

.PHONY: all clean run format install

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: $(EXE)
	$(EXE)

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

format:
	clang-format -i --style=Microsoft src/*.c
	clang-format -i --style=Microsoft inc/*.h

install: $(EXE)
	cp $(EXE) ~/.local/bin


-include $(OBJ:.o=.d)
