CPLEXDIR=/opt/ibm/ILOG/CPLEX_Studio2211
CCDIR=/home/matteo/Documenti/OperationResearchLaboratory/concorde_build


CC = gcc
CFLAGS += -Wall -Werror -pedantic -I./include
CFLAGS += -I${CPLEXDIR}/cplex/include/ilcplex -I${CPLEXDIR}/concert/includes -I${CCDIR}
#CFLAGS += -g


LDFLAGS = -lm -L${CPLEXDIR}/cplex/lib/x86-64_linux/static_pic -L${CPLEXDIR}/concert/lib/x86-64_linux/static_pic -L${CCDIR} -lilocplex -lcplex -lconcert -lpthread -ldl -lconcorde

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TARGET = $(BIN_DIR)/main

dir_guard=@mkdir -p $(@D)

.PHONY: all clean production preprofiling profiling   # all and clean are not file, just targets.

all: clean $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(dir_guard)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) script.p example.log

production: CFLAGS += -DPRODUCTION -O3
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

