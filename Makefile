build: main.c
	gcc main.c -o main -lpthread -lncurses -g -ggdb

run: build
	./main