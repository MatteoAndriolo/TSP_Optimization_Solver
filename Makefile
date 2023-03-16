
CC = gcc
CFLAGS = -Wall -Werror -pedantic -I./include
LDFLAGS = -lm
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TARGET = $(BIN_DIR)/main

.PHONY: all clean production   # all and clean are not file, just targets.

all: clean $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) script.p example.log

production: CFLAGS+= -DPRODUCTION
production: all