Challenge - Snakes
===================

Implement a multithreaded version of the video game [Snake](https://en.wikipedia.org/wiki/Snake_(video_game_genre)). This version will be a
Computer vs. Human game. (Refer to the readme for more information)

You can see the presentation in: [here](https://youtu.be/uX9RJp7bXXE)

How to build
---------------------------------------
- Run `make build`

OR

- Run `gcc main.c -o main -lpthread -lncurses`

How to run
------------

To build and run the game without any parameters just use the command: `make run`

The game can receive the number of snakes and diamonds (10 and 20 maximum respectively), you can set that by running the compiled file: directly `./main N M` where N is the number of snakes and M the number of diamonds