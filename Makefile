default: all

all: clean shell

shell:
	gcc -o shell shell.c history.c utils.c -Wall -Werror -g

clean:
	rm -rf shell
