build: main.c
	gcc main.c -o main.o -lpthread -lncurses -g -ggdb

run: build
	./main.o