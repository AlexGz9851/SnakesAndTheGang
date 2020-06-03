#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#define SLEEP_TIME 100000

struct point {
    int x;
    int y;
};

struct snake{
    unsigned int direction;
    struct point *body;
    int length;
    char growing;
    int  symbol;
}* snakes;

struct point* diamonds;
int totalSnakes = 6;
int totalDiamonds = 5;
char gameover = 0;


// Initialize ncurses
void initUI();
// creates the snakes in random locations
void initSnakes();
// sets diamonds in random locations
void initDiamonds();
// moves the snake to the desired direction (if it is no opposite to the current)
void moveSnake(struct snake *snake, int direction);
// draws the snake
void drawSnake(struct snake snake);
// grows by one the snake
void growSnake(struct snake *snake);
// thread that manages the ui drawing
void *manageUI(void *vargp);
// This looks for all points (diamonds and snakes body) and compares with the point to look if the place is already used by someone
char locationAvailable(int x, int y);
// sets the desired diamond in a random location
void placeDiamond(int diamond);
// Draws score
void drawScore();
//Check collision with diamonds and add new diamonds in case of collision
char collisionDiamond(struct snake *snake);
//Check collision with other snakes
char collisionSnake(struct snake *snake, int position);

void *moveEnemy(void *vargp);

int calculatetEnemyMove(struct snake *snake);

int main(int argc, char** argv){
    if(argc == 1){
        totalSnakes = 6;
        totalDiamonds = 5;
    }
    else if(argc == 3){
        totalSnakes = atoi(argv[1]);
        totalDiamonds = atoi(argv[2]);
        if(totalSnakes == 0 || totalDiamonds == 0){
            printf("Please input total number of snakes and diamonds: ./program 6 5\n");
            return -1;
        }
    }
    else {
        printf("Please input total number of snakes and diamonds: ./program 6 5\n");
        return -1;
    }
    snakes = malloc(sizeof(struct snake) * totalSnakes);
    diamonds = malloc(sizeof(struct point) * totalDiamonds);
    initUI();
    initSnakes();
    initDiamonds();


    pthread_t ui_thread;
    pthread_t * enemiesThread;
    enemiesThread = malloc(sizeof(pthread_t) * (totalSnakes-1));
    
    pthread_create(&ui_thread, NULL, manageUI, NULL); 
    //CREAR ENEMIGOS HILOS.
    for(int s=1;s<totalSnakes;s++){
        pthread_create(&enemiesThread[s-1], NULL, moveEnemy, (void*) (intptr_t) s);         
    }
    pthread_join(ui_thread, NULL); 
    
    endwin();
    free(snakes);
    free(diamonds);
    free(enemiesThread);
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
                snake->body[0].y = (snake->body[0].y-1)%(LINES-1);
            else
                block = 1;
            break;
        case KEY_RIGHT:
            if(snake->direction != KEY_LEFT)
                snake->body[0].x = (snake->body[0].x+1)% (COLS-1);               
            else
                block = 1;
            break;
        case KEY_LEFT:
            if(snake->direction != KEY_RIGHT)
                snake->body[0].x = (snake->body[0].x-1)% (COLS-1);
            else
                block = 1;
            break;
        case KEY_DOWN:
            if(snake->direction != KEY_UP)
                snake->body[0].y = (snake->body[0].y+1)%(LINES-1);
            else
                block = 1;
            break;
    }
    if(snake->body[0].x < 0){
        snake->body[0].x = (COLS-1);
    }
    if(snake->body[0].y < 1){
        snake->body[0].y = (LINES-1);
    }
    if(!block){
        snake->direction = direction;
    }
    refresh();
}

void drawSnake(struct snake snake){
    for(int i =0; i<snake.length; i++){
        mvaddch(snake.body[i].y, snake.body[i].x, snake.symbol);
    }
}

void initUI(){
    initscr(); 
    raw();
    keypad(stdscr, TRUE);
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    timeout(100);
}

void initSnakes(){
    int symbols[7] = {'*', '@', '$', ACS_DEGREE, ACS_BOARD, ACS_LANTERN, ACS_BLOCK};
    time_t t;
    srand((unsigned) time(&t));
    for(int i = 0; i<totalSnakes; i++){
        snakes[i].length = 1;
        snakes[i].body = malloc(snakes[0].length * sizeof(struct point));
        if(i == 0){
            snakes[i].body[0].x = COLS/2;
            snakes[i].body[0].y = LINES/2;
            snakes[i].direction = KEY_UP;
            snakes[i].symbol = ACS_BLOCK;
        }
        else {
            int x = 0, y = 0;
            do {
                x = 1 + rand() % (COLS-2);
                y = 1 + rand() % (LINES-2);
            } while (!locationAvailable(x, y));
            snakes[i].body[0].x = x;
            snakes[i].body[0].y =  y;
            snakes[i].direction = rand() % 4 + 402;
            snakes[i].symbol = symbols[rand()%7];
        }
    }
}

char locationAvailable(int x, int y){
    struct point* points;
    int size = totalDiamonds;
    for(int i = 0; i < totalSnakes; i++){
        size += snakes[i].length;
    }
    points = malloc(sizeof(struct point) * size);
    int pointer = 0;
    for(int i = 0; i < totalSnakes; i++){
        for(int j = 0; j < snakes[i].length; j++){
            points[pointer] = snakes[i].body[j];
            pointer++;
        }
    }
    for(int i = 0; i < totalDiamonds; i++){
        points[pointer] = diamonds[i];
    }
    for(int i = 0; i < size; i++){
        if(points[i].x == x && points[i].y == y){
            free(points);
            return 0;
        }
    }
    free(points);
    return 1;
}

void placeDiamond(int diamond){
    int x = 0, y = 0;
    do {
        x = 1 + rand() % (COLS - 2);
        y = 1 + rand() % (LINES - 2);
    } while (!locationAvailable(x, y));
    diamonds[diamond].x = x;
    diamonds[diamond].y = y;
}

void initDiamonds(){
    time_t t;
    srand((unsigned) time(&t));
    for(int i = 0; i < totalDiamonds; i++){
        placeDiamond(i);
    }
}

int moves[] = {KEY_UP, KEY_DOWN ,KEY_LEFT,KEY_RIGHT};
void *moveEnemy(void *vargp){
    int s= (intptr_t) vargp;
    int move=0;
    while(!gameover){
        move=calculatetEnemyMove(&snakes[s]);
        moveSnake(&snakes[s], move);
        if(collisionDiamond(&snakes[s])){
            growSnake(&snakes[s]);
        }
        if(collisionSnake(&snakes[s],s)){
            sleep(1);
            break;
        }
        usleep(SLEEP_TIME);
    }
    return 0;
}

int calculatetEnemyMove(struct snake *snake){
    //TODO HACER ESTA FUNCION. aqui va el calculo de IA, etc.
    return KEY_LEFT;
}

char collisionDiamond(struct snake *snake){
    char flag = 0; 
    for(int i = 0; i<totalDiamonds;i++){
        if(snake->body[0].x == diamonds[i].x && snake->body[0].y == diamonds[i].y){
            flag = 1;
        }
        if(flag && i!=(totalDiamonds-1)){
            diamonds[i]=diamonds[i+1];
        }
        if(flag && i==(totalDiamonds-1)){
            placeDiamond(i);
        }
    }
    return flag;
}

char collisionSnake(struct snake *snake, int position){
    char flag = 0;
    for(int i =0; i<totalSnakes;i++){
        if(i!= position){
            for(int j =0; j<snakes[i].length; j++){
                    if(snake->body[0].x == snakes[i].body[j].x && snake->body[0].y == snakes[i].body[j].y){
                        flag = 1;
                    }
            }
        }
    }

    if(flag){
        struct point* newBody = malloc(sizeof(struct point) * 0);
        snakes[position].body = newBody;
        snakes[position].length = 0;
        free(newBody);
    }
    return flag;
}

int enemySnakes(){
    int sum = 0;
    for(int i =1; i<totalSnakes;i++){
        if(snakes[i].length!=0){
            sum++;
        }
    }
    return sum;
}

void *manageUI(void *vargp){
    drawSnake(snakes[0]);
    int input = 0;
    while(!gameover){
        for(int i = 0; i < totalDiamonds; i++){
            mvaddch(diamonds[i].y, diamonds[i].x, ACS_DIAMOND);
        }
        for(int i = 0; i < totalSnakes; i++){
            drawSnake(snakes[i]);
        }   
        drawScore();
        refresh();
        input = getch();
        if(input == 27){
            gameover = 1;
            mvprintw(0,0, "Game ended");
            refresh();
            sleep(1);
            break;
        }
        if(input != KEY_UP && input != KEY_DOWN && input != KEY_LEFT && input != KEY_RIGHT){
            moveSnake(&snakes[0], snakes[0].direction);
        }
        else {
            moveSnake(&snakes[0], input);
        }
        if(collisionDiamond(&snakes[0])){
            growSnake(&snakes[0]);
        }
        if(collisionSnake(&snakes[0],0)){
            gameover = 1;
            mvprintw(0,0, "Game ended");
            refresh();
            sleep(1);
            break;
        }
        if(enemySnakes()==0){
            gameover = 1;
            mvprintw(0,0, "YOU WON!  ");
            //mvprintw(1,0,"Enemies missing: %d",enemySnakes()); 
            refresh();
            sleep(10);
            break;
        }
        clear();
    }
    return 0;
}

void drawScore(){
    int first = -1;
    int lengthFirst = -1;
    int user = 1;
    for(int i = 0; i < totalSnakes; i++){
        if(lengthFirst < snakes[i].length){
            lengthFirst = snakes[i].length;
            first = i;
        }
        if(snakes[0].length < snakes[i].length){
            user++;
        }
    }
    if(user == 1){
        mvprintw(0, 0, "First: YOU - length %d", lengthFirst);
        //mvprintw(1,0,"Enemies missing: %d",enemySnakes());   
    }
    else {
        mvprintw(0, 0, "First: %d - length %d\t %dth: YOU - length %d", first, lengthFirst, user, snakes[0].length);
        //mvprintw(1,0,"Enemies missing: %d",enemySnakes());      
    }
}