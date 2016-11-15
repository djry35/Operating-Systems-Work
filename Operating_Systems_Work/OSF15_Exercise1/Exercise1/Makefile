all: matlab

CFLAGS= -Wall -g -std=gnu99 
LIBS= -lreadline

matlab: main.o command.o matrix.o
	gcc main.o command.o matrix.o $(CFLAGS) -o matlab $(LIBS)

main.o: main.c command.h matrix.h
	gcc main.c $(CFLAGS)-c

command.o: command.c command.h
	gcc command.c $(CFLAGS)-c

matrix.o: matrix.c matrix.h
	gcc matrix.c $(CFLAGS)-c

clean:
	rm -f *.o matlab temp_mat
