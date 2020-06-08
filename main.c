#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#define ABS(x) (((x) < 0) ? (-(x)) : (x) )
#define SLEEP_TIME 150000
#define ENEMY_RANDOMNESS 30
//randomness from 0-100
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
    char alive;
}* snakes = NULL;

int moves[] = {KEY_UP, KEY_DOWN ,KEY_LEFT,KEY_RIGHT};
struct point* diamonds = NULL;
int totalSnakes = 5;
int totalDiamonds = 5;
char gameover = 0;
char error = 0;
pthread_mutex_t lock;


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
char collisionSnake(int position);

void *moveEnemy(void *vargp);

int calculatetEnemyMove(struct snake *snake);

int getSqDistance(int x1,int y1 , int x2, int y2);

int main(int argc, char** argv){
    if(argc == 1){
        totalSnakes = 10;
        totalDiamonds = totalSnakes * 2;
    }
    else if(argc == 3){
        totalSnakes = atoi(argv[1]);
        totalDiamonds = atoi(argv[2]);
        if(totalSnakes == 0 || totalDiamonds == 0){
            printf("Please input total number of snakes and diamonds: ./program 6 5\n");
            return -1;
        }
        if(totalSnakes > 10){
            printf("There can't be more than 10 snakes\n");
            totalSnakes = 10;
        }
        if(totalDiamonds > 20){
            printf("There can't be more than 20 diamonds\n");
            totalDiamonds = 10;
        }
    }
    else {
        printf("Please input total number of snakes and diamonds: ./program 6 5\n");
        return -1;
    }
    if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    }

    snakes = calloc(totalSnakes, sizeof(struct snake));
    if(snakes == NULL){
        printf("Error while assigning memory\n"); 
        return -1;
    }
    diamonds = calloc(totalDiamonds, sizeof(struct point));
    if(diamonds == NULL){
        free(snakes);
        printf("Error while assigning memory\n"); 
        return -1;
    }
    initUI();
    initSnakes();
    initDiamonds();
    if(error){
        free(snakes);
        free(diamonds);
        return -1;
    }


    pthread_t ui_thread;
    pthread_t * enemiesThread;
    enemiesThread = calloc(totalSnakes-1, sizeof(pthread_t));
    if(enemiesThread == NULL){
        endwin();
        free(snakes);
        free(diamonds); 
        printf("Error while assigning memory\n"); 
        return -1;
    }
    
    pthread_create(&ui_thread, NULL, manageUI, NULL); 
    //CREAR ENEMIGOS HILOS.
    for(int s=1;s<totalSnakes;s++){
        pthread_create(&enemiesThread[s-1], NULL, moveEnemy, (void*) (intptr_t) s);         
    }
    pthread_join(ui_thread, NULL); 


    sleep(5);
    
    endwin();
    free(snakes);
    free(diamonds);
    free(enemiesThread);
    pthread_mutex_destroy(&lock); 
    return 0;
}

void growSnake(struct snake *snake){
    snake->growing = 1;
    struct point* newBody = calloc(snake->length+1, sizeof(struct point));
    struct point* oldBody = snake->body;
    newBody[snake->length] = oldBody[snake->length - 1];
    for(int i = 0; i < snake->length; i++){
        newBody[i] = oldBody[i];
    }
    snake->length++;
    snake->body = newBody;
    free(oldBody);
    snake->growing = 0;
}

void moveSnake(struct snake *snake, int direction){
    if(snake->growing || !snake->alive)
        return;
    for(int i =snake->length-1; i>0; i--){
        snake->body[i] = snake->body[i-1];
    }
    char block = 0;
    switch(direction){    
        case KEY_UP: 
            if(snake->direction != KEY_DOWN)
                snake->body[0].y = (snake->body[0].y-1)%(LINES-1);
            else{
                block = 1;
            }
            break;
        case KEY_RIGHT:
            if(snake->direction != KEY_LEFT)
                snake->body[0].x = (snake->body[0].x+1)% (COLS-1);               
            else{
                block = 1;
            }
            break;
        case KEY_LEFT:
            if(snake->direction != KEY_RIGHT)
                snake->body[0].x = (snake->body[0].x-1)% (COLS-1);
            else{
                block = 1;
            }
            break;
        case KEY_DOWN:
            if(snake->direction != KEY_UP)
                snake->body[0].y = (snake->body[0].y+1)%(LINES-1);
            else{
                block = 1;
            }
            break;
    }
    if(snake->body[0].x < 0){
        snake->body[0].x = (COLS-1);
    }
    if(snake->body[0].y < 1){
        snake->body[0].y = (LINES-1);
    }
    if(block){
        if(snake->direction == KEY_DOWN)
            snake->body[0].y = (snake->body[0].y+1)%(LINES-1);
        else if(snake->direction == KEY_UP)
            snake->body[0].y = (snake->body[0].y-1)%(LINES-1);
        else if(snake->direction == KEY_LEFT)
            snake->body[0].x = (snake->body[0].x-1)% (COLS-1);
        else if(snake->direction == KEY_RIGHT)
            snake->body[0].x = (snake->body[0].x+1)% (COLS-1);
    }
    if(!block){
        snake->direction = direction;
    }
    refresh();
}

void drawSnake(struct snake snake){
    for(int i =0; i<snake.length; i++){
        if(snake.body == NULL){
            return;
        }
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
    timeout(SLEEP_TIME/1000);
}

void initSnakes(){
    int symbols[7] = {ACS_BLOCK,'*', '@', '$', ACS_DEGREE, ACS_BOARD, ACS_LANTERN};
    time_t t;
    srand((unsigned) time(&t));
    for(int i = 0; i<totalSnakes; i++){
        if(i == 0){
            snakes[i].length = 1;
            snakes[i].body = calloc(snakes[0].length, sizeof(struct point));
            snakes[i].body[0].x = COLS/2;
            snakes[i].body[0].y = LINES/2;
            snakes[i].direction = KEY_UP;
            snakes[i].symbol = symbols[0];
        }
        else {
            snakes[i].length = rand()%3 +1;
            snakes[i].body = calloc(snakes[i].length, sizeof(struct point));
            int x = 0, y = 0;
            do {
                x = 1 + rand() % (COLS-2);
                y = 1 + rand() % (LINES-2);
            } while (!locationAvailable(x, y));
            if(error){
                return;
            }
            for(int j = 0; j < snakes[i].length; j++){
                snakes[i].body[j].x = x;
                snakes[i].body[j].y = y;
            }
            snakes[i].direction = moves[rand() % 4 ];
            int indx =i%6;
            indx = (indx<0) ? 6 : (indx+1);
            snakes[i].symbol = symbols[indx];
        }
        snakes[i].alive=1;
    }
}

char locationAvailable(int x, int y){
    struct point* points = NULL;
    int size = totalDiamonds;
    for(int i = 0; i < totalSnakes; i++){
        size += snakes[i].length;
    }
    points = calloc(size, sizeof(struct point));
    if(points == NULL){
        error = 1;
        return 1;
    }
    int pointer = 0;
    for(int i = 0; i < totalSnakes; i++){
        if(snakes[i].body==NULL) continue;
        for(int j = 0; j < snakes[i].length; j++){
            points[pointer] = snakes[i].body[j];
            pointer++;
        }
    }
    for(int i = 0; i < totalDiamonds; i++){
        points[pointer++] = diamonds[i];
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

void *moveEnemy(void *vargp){
    int s= (intptr_t) vargp;
    int move=0;
    while(!gameover && snakes[s].alive){
        move=calculatetEnemyMove(&snakes[s]);
        moveSnake(&snakes[s], move);
        if(collisionDiamond(&snakes[s])){
            growSnake(&snakes[s]);
        }
        if(snakes[s].length==0){
            break;
        }
        if(collisionSnake(s)){
            break;
        }
        usleep(SLEEP_TIME);
    }
    return 0;
}

int calculatetEnemyMove(struct snake *enemy){
    int len  = snakes[0].length;
    int closestPositon, dummy, xTarget, yTarget;
    closestPositon = getSqDistance(snakes[0].body[0].x, snakes[0].body[0].y, enemy->body[0].x, enemy->body[0].y );
    xTarget=snakes[0].body[0].x;
    yTarget=snakes[0].body[0].y;

    for(int b=1;b<len;b++){
        dummy = getSqDistance(snakes[0].body[b].x, snakes[0].body[b].y, enemy->body[0].x, enemy->body[0].y );
        if(dummy< closestPositon){
            closestPositon = dummy;
            xTarget=snakes[0].body[b].x;
            yTarget=snakes[0].body[b].y;
        }
    }
    int xdistance = xTarget - enemy->body[0].x;
    if(xdistance <-COLS/2) xdistance +=COLS;
    if(xdistance > COLS/2) xdistance -=COLS;

    int ydistance = yTarget - enemy->body[0].y;
    if(ydistance <-LINES/2) ydistance +=LINES;
    if(ydistance > LINES/2) ydistance -=LINES;
    int move, rnd;
    if(ydistance==0){
        if(xdistance<0) {
            move = KEY_LEFT;
        }else {
            move = KEY_RIGHT;
        }
    }else if(xdistance == 0){
        if(ydistance<0) {
            move = KEY_UP;
        }else {
            move = KEY_DOWN;
        }
    }
    else{
        if(ABS(ydistance) < ABS(xdistance)){
            if(ydistance<0) {
                move = KEY_UP;
            }else {
                move = KEY_DOWN;
            }
        }else{
            if(xdistance<0) {
                move = KEY_LEFT;
            }else {
                move = KEY_RIGHT;
            }
        }
    }
    //The posible movements of the snake is basically perfect.
    //adding randomness to the movements.
    rnd = rand() % 100;
    if(rnd < ENEMY_RANDOMNESS) return moves[rand()%4];
    return move;
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

char collisionSnake(int position){
    struct snake snake = snakes[position];
    if(pthread_mutex_lock(&lock)!=0){
        //mvprintw(0,0, "Collision failure");
    } 
    char flag = 0;
    for(int i = 0; i<totalSnakes;i++){
        if(i==position || !snakes[i].alive) continue;
        for(int j =0; j<snakes[i].length; j++){
            if(snake.body[0].x == snakes[i].body[j].x && snake.body[0].y == snakes[i].body[j].y){
                if(j==0){
                    snakes[i].alive = 0;
                }
                flag = 1;
                break;
            }
        }
        if(flag){
            break;
        }
    }
    if(flag){
        snakes[position].alive = 0;
        snakes[position].length = 0;
    }
    pthread_mutex_unlock(&lock); 
    return flag;
}

int enemySnakes(){
    int sum = 0;
    for(int i =1; i<totalSnakes;i++){
        if(snakes[i].alive){
            sum++;
        }
    }
    return sum;
}

int getSqDistance(int x1,int y1 , int x2, int y2){ return (x1-x2) * (x1-x2)  + (y1-y2) * (y1-y2);   }



void *manageUI(void *vargp){
    drawSnake(snakes[0]);
    int input = 0;
    while(!gameover){
        for(int i = 0; i < totalDiamonds; i++){
            mvaddch(diamonds[i].y, diamonds[i].x, ACS_DIAMOND);
        }
        for(int i = 0; i < totalSnakes; i++){
            if(snakes[i].alive)
                drawSnake(snakes[i]);
        }   
        drawScore();
        refresh();
        input = getch();
        if(input == 27){
            gameover = 1;
            mvprintw(0,COLS/2-5, "GAME ENDED");
            refresh();
            break;
        }
        if(input != KEY_UP && input != KEY_DOWN && input != KEY_LEFT && input != KEY_RIGHT){
            moveSnake(&snakes[0], snakes[0].direction);
        }
        else if((snakes[0].direction == KEY_UP && input == KEY_DOWN) || (snakes[0].direction == KEY_RIGHT && input == KEY_LEFT) ||
            (snakes[0].direction == KEY_DOWN && input == KEY_UP) || (snakes[0].direction == KEY_LEFT && input == KEY_RIGHT))
        {
                usleep(SLEEP_TIME);
        }
        else {
            moveSnake(&snakes[0], input);
        }
        if(collisionDiamond(&snakes[0])){
            growSnake(&snakes[0]);
        }
        if(collisionSnake(0) || snakes[0].alive==0){
            clear();
            mvaddch(snakes[0].body[0].y, snakes[0].body[0].x, 'X');
            mvprintw(0, COLS/2-5, "GAME OVER");
            for(int i =0; i<totalSnakes; i++){
                snakes[i].length = 0;
                snakes[i].alive = 0;
                free(snakes[i].body);
            }
            gameover = 1;
            refresh();
            break;
        }
        if(enemySnakes()==0){
            gameover = 1;
            mvprintw(0,COLS/2-4, "YOU WON!");
            refresh();
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