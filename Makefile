build: main.c
	gcc main.c -o main -lpthread -lncurses -g -ggdb 

buildng: nonGUI.c
	gcc nonGUI.c -o ng -lpthread -lncurses -g -ggdb 

run: build
	./main

ng: buildng
	./ng

val: build
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./main 10 5

valng: buildng
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out2.txt ./test