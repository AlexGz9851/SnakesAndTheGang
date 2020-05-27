build: main.c
	gcc main.c -o main -lpthread -lncurses

run: build
	./main