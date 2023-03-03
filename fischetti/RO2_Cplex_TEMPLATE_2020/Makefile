OBJS = main.o vrp.o mip_utilities.o chrono.o            
HEADERS = vrp.h 
EXE = vrp
all: $(EXE) 
setting = -1   
OS := $(shell uname)

# ---------------------------------------------------------------------
# Compiler selection and libraries for Cplex -- windows
# ---------------------------------------------------------------------
#
#CC = c:/usr/mingw/bin/gcc -mno-cygwin
#LIBS = -Lc:/usr/cplex -lcplex
#INC = -Ic:/usr/cplex
#
# ---------------------------------------------------------------------
# Compiler selection and libraries for Cplex 
# ---------------------------------------------------------------------
# 

ifeq ($(OS),Darwin)
	setting = 0
	CPLEX_HOME = /Applications/CPLEX_Studio128/cplex
	CC = clang -Qunused-arguments
	AR = ar rc
	LIBS = -Wl,-no_compact_unwind -L${CPLEX_HOME}/lib/x86-64_osx/static_pic -L. -lcplex -lm -lpthread 
	INC = -I. -I${CPLEX_HOME}/include/ilcplex
endif

ifeq ($(OS),Linux)
	setting = 1
	CPLEX_HOME = /nfsd/rop/sw/ibm/cos128/cplex
	CC = gcc
	AR = ar rc
	LIBS = -L${CPLEX_HOME}/lib/x86-64_linux/static_pic -L. -lcplex -lm -lpthread -ldl
	INC = -I. -I${CPLEX_HOME}/include/ilcplex
endif


# ---------------------------------------------------------------------
# Rules
# ---------------------------------------------------------------------
CFLAGS = -Wall -O3
##CFLAGS = -Wall -g -O0 
RM = rm -rf

.SUFFIXES:
.SUFFIXES: .o .c .cpp
.c.o :
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
.cpp.o :
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(EXE): $(OBJS) $(LIBUTILS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIBS)

$(OBJS) : $(HEADERS)

$(LIBUTILS): $(OBJS_LIBUTILS)
	$(AR) $(LIBUTILS) $(OBJS_LIBUTILS)

$(LIBUTILS) : $(HEADERS_LIBUTILS)

clean:
	$(RM) $(OBJS)
	$(RM) $(OBJS_LIBUTILS)
	$(RM) $(LIBUTILS)
	$(RM) $(EXE) 
	
again:                                                               
	make clean
	make    
	
wow:
	@echo "                                      W O W W W W WWWWWWWWWWWWWWWWWWW"

who:
	@echo "you are user $(USER) with uname `uname` (OS = $(OS)) and you working with compiler setting $(setting)" 

