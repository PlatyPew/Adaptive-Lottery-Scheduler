CC = gcc
PROGRAM = assignment

all:
	$(CC) -o $(PROGRAM) $(PROGRAM).c

clean:
	rm $(PROGRAM)
