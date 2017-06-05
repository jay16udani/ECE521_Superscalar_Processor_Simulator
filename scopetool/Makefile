CC = g++
OPT = -O3
#OPT = -g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) #$(INC) $(LIB)

SRC = main.cc
OBJ = main.o

#################################

all: scope

scope: $(OBJ)
	$(CC) -o scope $(CFLAGS) $(OBJ)
	@echo "-----------DONE WITH SCOPE-----------"
 
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cc


clean:
	rm -f *.o scope core Makefile.bak

clobber:
	rm -f *.o
