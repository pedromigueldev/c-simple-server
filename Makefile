cc = gcc
flags = -Wall -Werror -g -Wextra -fsanitize=address
bin=server

.PHONY: all
all: $(bin) exec

$(bin): lib/serving.o lib/strpm.o main.c
	$(cc) $(flags) -o $@ $^

%.o: %.c %.h
	$(cc) $(flags) -c -o $@ $<

exec:
	./$(bin)
