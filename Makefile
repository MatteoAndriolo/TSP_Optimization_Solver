CPLEXDIR=/opt/ibm/ILOG/CPLEX_Studio2211
#CCDIR=/home/matteo/Documenti/OperationResearchLaboratory/concorde_build
CCDIR=/opt/concorde 

CC = gcc
CFLAGS = -Wall -Werror -pedantic -I./include
CFLAGS += -I${CPLEXDIR}/cplex/include/ilcplex -I${CPLEXDIR}/concert/includes -I${CCDIR}
#CFLAGS += -g

LDFLAGS = -lm -L${CPLEXDIR}/cplex/lib/x86-64_linux/static_pic -L${CPLEXDIR}/concert/lib/x86-64_linux/static_pic -L${CCDIR} -lilocplex -lcplex -lconcert -lpthread -ldl -lconcorde

CFLAGS = -Wall -Werror -pedantic -I./include -g
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
COVERAGE_DIR = ./coverage  # Directory for coverage data

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TARGET = $(BIN_DIR)/main

dir_guard=@mkdir -p $(@D)

.PHONY: all clean production preprofiling profiling coverage

#all: CFLAGS += -fprofile-arcs -ftest-coverage
#all: clean $(TARGET)
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

production: CFLAGS  += -DPRODUCTION -O3
production: OPT := -O3
production: all

preprofiling : CFLAGS += -pg
preprofiling : LDFLAGS += -pg
preprofiling: OPT := -O0
preprofiling: all

profiling:
	gprof $(TARGET) > profiling/output.txt
	gprof ./bin/main | gprof2dot | dot -Tpng -o profiling/output.png
#gprof2dot -f pstats output.txt | dot -Tpng -o plot/profiling.png



coverage: CFLAGS += --coverage # Add coverage flags to CFLAGS
coverage: clean $(TARGET)
	mkdir -p $(COVERAGE_DIR)
	lcov --capture --directory $(OBJ_DIR) --output-file ./coverage/coverage.info
	genhtml $(COVERAGE_DIR)/coverage.info --output-directory ./coverage/coverage_report
	#lcov --capture --directory $(OBJ_DIR) --output-file $(COVERAGE_DIR)/coverage.info
	#genhtml $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/coverage_report
