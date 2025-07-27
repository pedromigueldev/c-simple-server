cc = gcc
flags = -Wall -Werror -g -Wextra

all: serving

serving: serving.o server.o stringpm.o
	$(cc) $(flags) -o serving serving.o server.o stringpm.o

# main program
serving.o: serving.c server.h stringpm.h
	$(cc) $(flags) -c serving.c

server.o: server.c server.h
	$(cc) $(flags) -c server.c

stringpm.o: stringpm.c stringpm.h
	$(cc) $(flags) -c stringpm.c
