#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define ABS(x) (((x) < 0) ? (-(x)) : (x) )
#define ENEMY_RANDOMNESS 30
#define COLS 100
#define LINES 100
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

struct point* diamonds = NULL;
int totalSnakes = 5;
int totalDiamonds = 5;
char gameover = 0;
char error = 0;
struct point lastP;
pthread_mutex_t lock;


// creates the snakes in random locations
void initSnakes();
// sets diamonds in random locations
void initDiamonds();
// moves the snake to the desired direction (if it is no opposite to the current)
void moveSnake(struct snake *snake, int direction);
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
    totalSnakes = 15;
    totalDiamonds = totalSnakes * 2;
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
    initSnakes();
    initDiamonds();
    if(error){
        free(snakes);
        free(diamonds);
        return -1;
    }


    pthread_t ui_thread;
    pthread_t enemiesThread[totalSnakes-1];
    for(int i = 0; i < totalSnakes-1; i++){
        enemiesThread[i] = 0;
    }
    if(enemiesThread == NULL){
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
    for(int s=0;s<totalSnakes-1;s++){
        pthread_join(enemiesThread[s], NULL);         
    }


    // sleep(5);

    for(int i =0; i<totalSnakes; i++){
        if(snakes[i].body!=NULL)
            free(snakes[i].body);
        snakes[i].body = NULL;
    }
    
    if(snakes != NULL){
        free(snakes);
        snakes = NULL;
    }
    if(diamonds != NULL){
        free(diamonds);
        diamonds = NULL;
    }
    pthread_mutex_destroy(&lock); 
    return 0;
}

void growSnake(struct snake *snake){
    snake->growing = 1;
    struct point* newBody = calloc(snake->length+1, sizeof(struct point));
    struct point* oldBody = snake->body;
    newBody[snake->length].x = -1;
    newBody[snake->length].y = -1;
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
        case 1: 
            if(snake->direction != 3)
                snake->body[0].y = (snake->body[0].y-1)%(LINES-1);
            else{
                block = 1;
            }
            break;
        case 2:
            if(snake->direction != 4)
                snake->body[0].x = (snake->body[0].x+1)% (COLS-1);               
            else{
                block = 1;
            }
            break;
        case 4:
            if(snake->direction != 2)
                snake->body[0].x = (snake->body[0].x-1)% (COLS-1);
            else{
                block = 1;
            }
            break;
        case 3:
            if(snake->direction != 1)
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
        if(snake->direction == 3)
            snake->body[0].y = (snake->body[0].y+1)%(LINES-1);
        else if(snake->direction == 1)
            snake->body[0].y = (snake->body[0].y-1)%(LINES-1);
        else if(snake->direction == 4)
            snake->body[0].x = (snake->body[0].x-1)% (COLS-1);
        else if(snake->direction == 2)
            snake->body[0].x = (snake->body[0].x+1)% (COLS-1);
    }
    if(!block){
        snake->direction = direction;
    }
}

void initSnakes(){
    int symbols[7] = {'1','*', '@', '$', '2', '3', '4'};
    time_t t;
    srand((unsigned) time(&t));
    for(int i = 0; i<totalSnakes; i++){
        if(i == 0){
            snakes[i].length = 1;
            snakes[i].body = calloc(snakes[0].length, sizeof(struct point));
            snakes[i].body[0].x = COLS/2;
            snakes[i].body[0].y = LINES/2;
            snakes[i].direction = 2;
            snakes[i].symbol = symbols[0];
        }
        else {
            snakes[i].length = rand()%3 +1;
            snakes[i].body = calloc(snakes[i].length, sizeof(struct point));
            int x = 0, y = 0;
            do {
                x = 1 + rand() % (COLS-2);
                y = 1 + rand() % (LINES-2);
            } while (!locationAvailable(x, y) || (
                (x>snakes[0].body[0].x-10 && x<snakes[0].body[0].x+10) &&
                (y>snakes[0].body[0].y-10 && y<snakes[0].body[0].y+10))
            );
            if(error){
                return;
            }
            snakes[i].body[0].x = x;
            snakes[i].body[0].y = y;
            for(int j = 1; j < snakes[i].length; j++){
                snakes[i].body[j].x = -1;
                snakes[i].body[j].y = -1;
            }
            snakes[i].direction = rand() % 4 + 1;
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
            move = 4;
        }else {
            move = 2;
        }
    }else if(xdistance == 0){
        if(ydistance<0) {
            move = 1;
        }else {
            move = 3;
        }
    }
    else{
        if(ABS(ydistance) < ABS(xdistance)){
            if(ydistance<0) {
                move = 1;
            }else {
                move = 3;
            }
        }else{
            if(xdistance<0) {
                move = 4;
            }else {
                move = 2;
            }
        }
    }
    //The posible movements of the snake is basically perfect.
    //adding randomness to the movements.
    rnd = rand() % 100;
    if(rnd < ENEMY_RANDOMNESS) return rand()%4 + 1;
    return move;
}

char collisionDiamond(struct snake *snake){
    char flag = 0; 
    for(int i = 0; i<totalDiamonds;i++){
        if(
            snake->body[0].x 
            == 
            diamonds[i].x 
            && 
            snake->body[0].y 
            == 
            diamonds[i].y){
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
    } 
    char flag = 0;
    for(int i = 0; i<totalSnakes;i++){
        if((i==position) || !snakes[i].alive) continue;
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
    if(position == 0){
        for(int i = 1; i < snake.length; i++){
            if(snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y){
                flag = 1;
            }
        }
    }
    if(flag){
        snakes[position].alive = 0;
        printf("snake %d collisioned\n", position);
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
    int input = 0;
    while(!gameover){
        moveSnake(&snakes[0], 2);
        if(collisionDiamond(&snakes[0])){
            growSnake(&snakes[0]);
        }
        if(collisionSnake(0) || snakes[0].alive==0){
            printf("lose\n");
            printf("Snake %d length %d x %d y %d\n", 0, snakes[0].length, snakes[0].body[0].x, snakes[0].body[0].y);
            for(int i =0; i<totalSnakes; i++){
                snakes[i].length = 0;
                snakes[i].alive = 0;
            }
            gameover = 1;
            break;
        }
        if(enemySnakes()==0){
            gameover = 1;
            printf("win");
            break;
        }
        for(int i = 0; i < totalSnakes; i++){
            if(snakes[i].alive)
                printf("Snake %d length %d x %d y %d\n", i, snakes[i].length, snakes[i].body[0].x, snakes[i].body[0].y);
        }
    }
    return 0;
}
