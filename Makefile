CC = gcc
CFLAGS = -Wall -pedantic
MEMORY_DEBUG_FLAGS = -fsanitize=address

SRC = main.c bmp.c file_utils.c sss.c sss_algos.c sss_helpers.c lsb_decoder.c lsb_encoder.c permutation_table.c
OBJ = $(SRC:.c=.o)

TARGET = program
DEBUG_TARGET = program_debug

.PHONY: all clean MEMORY_DEBUG

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

MEMORY_DEBUG: CFLAGS += $(MEMORY_DEBUG_FLAGS)
MEMORY_DEBUG: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET) $(DEBUG_TARGET)