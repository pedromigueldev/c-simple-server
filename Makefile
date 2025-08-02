cc = gcc
flags = -Wall -Werror -g -Wextra -fsanitize=address

all: serving exec

serving: main.c lib/server.c lib/stringpm.c
	$(cc) $(flags) -o serving main.c

exec:
	./serving
