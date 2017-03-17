# usage: if this file is named "Makefile", then the commands are:
#	"make" will build the specified executable (PROG)
#	"make clean" will delete all of the .o and .exe files
#
# if this file is named something else, then use the -f option for make:
#	"make -f makefilename clean"

# set the variables below depending on compiler and options

CC = g++
LD = g++

# add -ggdb to the CFLAGS for debugging (or -ggdbgdb if you use gdb). 
# -pedantic-errors attempts to enforce Standard
# -Wall asks for certain warnings of possible errors
# -c is required to specify compile-only (no linking)

CFLAGS = -std=c++11 -pedantic-errors -Wall -c -ggdb

#no load flags defined, but -l would be used to include a special library
LFLAGS = 

OBJS = Room.o Person.o Meeting.o Utility.o meeting_room.o 
PROG = proj3exe

default: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o $(PROG) -ggdb

test:
	$(LD) $(LFLAGS) meeting_room.o -o test -ggdb

Room.o: Room.cpp Room.h  Meeting.h Utility.h
	$(CC) $(CFLAGS) Room.cpp

Meeting.o: Meeting.cpp Meeting.h  Person.h Utility.h 
	$(CC) $(CFLAGS) Meeting.cpp

Person.o: Person.cpp Person.h Utility.h 
	$(CC) $(CFLAGS) Person.cpp


Utility.o: Utility.cpp Utility.h
	$(CC) $(CFLAGS) Utility.cpp

meeting_room.o: meeting_room.cpp Room.h Meeting.h Person.h Utility.h
	$(CC) $(CFLAGS) meeting_room.cpp


clean:
	rm -f *.o ./test
real_clean:
	rm -rf *.o $(PROG)
