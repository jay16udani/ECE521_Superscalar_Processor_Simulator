CC = gcc
OPT = -g  
WARN = -Wall
CFLAGS = $(OPT) $(WARN) 

# List all your .cc files here (source files, excluding header files)
SIM_SRC = main.c 

# List corresponding compiled object files here (.o files)
SIM_OBJ = main.o 



 
#################################

# default rule

all: sim_ds
	@echo "my work is done here..."


# rule for making sim_cache
sim_ds: $(SIM_OBJ)
	$(CC) -o sim_ds $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM_DS -----------"


# generic rule for converting any .cc file to any .o file
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cc


# type "make clean" to remove all .o files plus the sim_ds binary
clean:
	rm -f *.o sim_ds


# type "make clobber" to remove all .o files (leaves sim_ds binary)
clobber:
	rm -f *.o
