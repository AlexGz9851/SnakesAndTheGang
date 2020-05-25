#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>

struct point {
    char x;
    char y;
};

struct snake{
    unsigned int direction;
    struct point *body;
    int length;
    char x, y;
    char growing;
};

void init();
void moveSnake(struct snake *snake, int direction);
void drawSnake(struct snake snake);
void growSnake(struct snake *snake);

int main(){
    init();
    struct snake s;
    s.length = 1;
    s.body = malloc(s.length * sizeof(struct point));
    s.body[0].x = COLS/2;
    s.body[0].y = LINES/2;
    s.direction = KEY_UP;
    drawSnake(s);
    int c = 0;
    while(1){
        drawSnake(s);
        mvaddch(10, 10, ACS_DIAMOND);
        mvaddch(0,0,' ');
        refresh();
        c = getch();
        if(c == 27){
            mvprintw(0,0, "Game ended");
            refresh();
            sleep(1);
            break;
        }
        if(c != KEY_UP && c != KEY_DOWN && c != KEY_LEFT && c != KEY_RIGHT){
            moveSnake(&s, s.direction);
        }
        else {
            moveSnake(&s, c);
        }
        if(s.body[0].x == 10 && s.body[0].y == 10){
            growSnake(&s);
        }
        // sleep(2);
        usleep(10000);
        clear();
    }
    endwin();
    return 0;
}

void growSnake(struct snake *snake){
    snake->growing = 1;
    struct point* newBody = malloc(sizeof(struct point) * snake->length+1);
    struct point* oldBody = snake->body;
    newBody[snake->length] = oldBody[snake->length - 1];
    newBody[snake->length];
    for(int i = 0; i < snake->length; i++){
        newBody[i] = oldBody[i];
    }
    snake->length++;
    snake->body = newBody;
    free(oldBody);
    snake->growing = 0;
}

void moveSnake(struct snake *snake, int direction){
    if(snake->growing)
        return;
    for(int i =snake->length-1; i>0; i--){
        snake->body[i] = snake->body[i-1];
    }
    char block = 0;
    switch(direction){    
        case KEY_UP: 
            if(snake->direction != KEY_DOWN)
                snake->body[0].y = (snake->body[0].y-1)%LINES;
            else
                block = 1;
            break;
        case KEY_RIGHT:
            if(snake->direction != KEY_LEFT)
                snake->body[0].x = (snake->body[0].x+1)%COLS;               
            else
                block = 1;
            break;
        case KEY_LEFT:
            if(snake->direction != KEY_RIGHT)
                snake->body[0].x = (snake->body[0].x-1)%COLS;
            else
                block = 1;
            break;
        case KEY_DOWN:
            if(snake->direction != KEY_UP)
                snake->body[0].y = (snake->body[0].y+1)%LINES;
            else
                block = 1;
            break;
    }
    mvprintw(0,0,"head position: %d, %d", snake->body[0].x, snake->body[0].y);
    if(snake->body[0].x < 0){
        snake->body[0].x = COLS;
    }
    if(snake->body[0].y < 0){
        snake->body[0].y = LINES;
    }
    if(!block){
        snake->direction = direction;
    }
    refresh();
}

void drawSnake(struct snake snake){
    for(int i =0; i<snake.length; i++){
        mvaddch(snake.body[i].y, snake.body[i].x, ACS_BLOCK);
    }
    refresh();
}

void init(){
    initscr(); 
    raw();
    keypad(stdscr, TRUE);
    noecho();
    nodelay(stdscr, TRUE);
    timeout(100);
}
