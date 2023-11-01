CC = gcc
CFLAGS = -Wall -Werror -pedantic -I./include -g
LDFLAGS = -lm
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
COVERAGE_DIR = ./coverage  # Directory for coverage data

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TARGET = $(BIN_DIR)/main

dir_guard=@mkdir -p $(@D)

.PHONY: all clean production coverage asan

#all: CFLAGS += -fprofile-arcs -ftest-coverage
all: $(TARGET)
	mkdir -p plot

$(TARGET): $(OBJ_FILES)
	$(dir_guard)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) script.p example.log

production: CFLAGS += -DPRODUCTION -O3
production: all

coverage: CFLAGS += --coverage # Add coverage flags to CFLAGS
coverage: clean $(TARGET)
	mkdir -p $(COVERAGE_DIR)
	lcov --capture --directory $(OBJ_DIR) --output-file ./coverage/coverage.info
	genhtml $(COVERAGE_DIR)/coverage.info --output-directory ./coverage/coverage_report
	#lcov --capture --directory $(OBJ_DIR) --output-file $(COVERAGE_DIR)/coverage.info
	#genhtml $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/coverage_report

asan: CFLAGS += $(ASAN_FLAGS)
asan: LDFLAGS += $(ASAN_FLAGS)
asan: all
