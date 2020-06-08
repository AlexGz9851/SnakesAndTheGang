build: main.c
	gcc main.c -o main -lpthread -lncurses -g -ggdb

run: build
	./main

val: build
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./main 10 5