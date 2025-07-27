cc = gcc
flags = -Wall -Werror -g -Wextra

all: test

test: test.o server.o
	$(cc) $(flags) -o test test.o server.o

test.o: test.c server.h
	$(cc) $(flags) -c test.c

server.o: server.c server.h
	$(cc) $(flags) -c server.c
