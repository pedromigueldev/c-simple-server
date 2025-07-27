cc = gcc
flags = -Wall -Werror -g -Wextra

all: serving

serving: serving.o server.o
	$(cc) $(flags) -o serving serving.o server.o

serving.o: serving.c server.h
	$(cc) $(flags) -c serving.c

server.o: server.c server.h
	$(cc) $(flags) -c server.c
