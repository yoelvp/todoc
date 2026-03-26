main: src/main.c
	gcc -Wall -Wextra -g src/main.c -o debug/main

run: main
		./debug/main
