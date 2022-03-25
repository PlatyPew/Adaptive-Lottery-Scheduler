CC = gcc
CFLAGS = -Wall
PROGRAM = assignment

all:
	$(CC) $(CFLAGS) -o $(PROGRAM) $(PROGRAM).c

clean:
	rm $(PROGRAM)
