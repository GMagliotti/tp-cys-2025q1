CC = gcc
CFLAGS = -Wall -pedantic
MEMORY_DEBUG_FLAGS = -fsanitize=address

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

TARGET = shamigo
DEBUG_TARGET = shamigo_debug

.PHONY: all clean MEMORY_DEBUG

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

MEMORY_DEBUG: CFLAGS += $(MEMORY_DEBUG_FLAGS)
MEMORY_DEBUG: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJ_DIR) *.o $(TARGET) $(DEBUG_TARGET)
