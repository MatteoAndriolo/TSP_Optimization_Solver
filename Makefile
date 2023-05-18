
CC = gcc
CFLAGS = -Wall -Werror -pedantic -I./include 
#CFLAGS += -Wno-maybe-uninitialized  -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-uninitialized -Wno-stringop-overflow 
CFLAGS += -g
#CFLAGS += -O3
LDFLAGS = -lm
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TARGET = $(BIN_DIR)/main

dir_guard=@mkdir -p $(@D)

.PHONY: all clean production   # all and clean are not file, just targets.


#all: clean $(TARGET)
all: $(TARGET)
	mkdir plot


$(TARGET): $(OBJ_FILES)
	$(dir_guard)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) script.p example.log

production: CFLAGS+= -DPRODUCTION -O3
production: all