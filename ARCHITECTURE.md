# Snakes and the Gang
> A multithreaded version of the video game Slither.io


## Introduction

The presented project is a  multithreaded version of the video game Slither.io. This version will be a Computer vs Human game. Each enemy is independent, and the number of enemies is configurable. This document pretends to show the software architecture of the software project.

## Requirements

* The game's layout can be static.
* Food dots can be randomly located in the layout.
* Number of food dots must be configured in the game's start.
* The main snake gamer must be controlled by the user.
* Enemy snakes are autonomous entities that will move a random way.
* Enemy snakes and main snake should respect the layout limits and walls.
* Enemy snakes’ number can be configured on game's start.
* Each enemy's behavior will be implemented as a separated thread.
* Enemy snakes and main sneak threads must use the same map or game layout data structure resource.
* Display obtained main snake's scores.
* Main snake grows one unit on every eaten food dot.
* Enemy snakes can also grow when they touch food dots.
* Main snake loses when it has been hit 10 times by other snake or it touches the limits or walls.
* Main snake wins the game when all food dots have eaten, and main snake has the largest length.

## Authorized Modifications

* Walls are not required. Game can use modular math to avoid snakes goes outside limits. (If a snake goes to the outside left limit of the screen, it will appear in the right side and vice versa. This is also required in the vertical axis.
* All snakes only have one life. They lose its life if they collision with other snakes.
* Food when ate, appears in another new position
* Royal Battle mode game. Main snake wins when no enemy snakes are left.

## Proposed Architecture

For the creation of the project we decided to develop the game directly in console, with a non-traditional GUI. To do this we can defined our inputs and outputs as:


| Input                      | Output                                                   |
|----------------------------|----------------------------------------------------------|
| Keyboard up-arrow          |     User snake moves up (if it is not moving down)       |
|     Keyboard up-down       |     User snake moves down (if it is not moving up)       |
|     Keyboard up-left       |     User snake moves left (if it is not moving right)    |
|     Keyboard up-right      |     User snake moves right (if it is not moving left)    |
|     Keyboard escape key    |     Finishes the game                                    |


The software works with threads allowing us to control multiple parts of the system almost simultaneously. We can define the threads as:
* GUI Manager: controls the GUI drawing and the main snake
* Enemy snakes: One thread per enemy, it has an small AI for the snakes to follow the player

## Sequence Diagram

![Squence diagram](/seq.png)

## Structures description

_Point_
Attributes: int x, int y.

This structure defines the coordinates of a point in the grid in the form (x,y). 

_Snake_
Attributes: unsigned int direction, struct point * body, int length char growing, int symbol, char alive.

A snake is basically an array of points ( body ) of given length (length). The snake is always moving in a given (direction). Its possible states of the snake are: alive/dead (alive), or (growing) in case it just ate a diamond. The snake is represented in the UI grid with a given (symbol).


## Function description

### void initSnakes()
* Creates all snakes in random locations with random initial lengths.
* Sets their grid symbols.

### void drawSnake(struct snake snake)
* Draws each point of the body of the snake into the grid

### void moveSnake( struct snake * snake, int direction)
* Moves the snake given its next direction.

### void growSnake(struct snake * snake)
* Adds a new point in the snake’s body.
* Increases the length of the snake.

### void initDiamonds()
* Sets diamonds in random locations

### void placeDiamond(int diamond)
* Sets the desired diamond in a random location

###  char locationAvaliable(int x, int y)
* Given the coordinate of a point, it looks if there is a snake or diamond in the given location.

###  void initUI()
* Initializes the Ncurses library.

###  void *manageUI(void *vargp)
* This routine is run on the main thread. It manages the state of the game, the elements drawed into the grid and also listens to the keyboard and manages the kinematics of the main snake.

###  void drawScore()
* Writes into the grid the first place and the user score.

###  int getDistanceSquared(int x1, int y1, int x2, int y2)
* Returns the Euclidian metric between 2 points. (The distance squared).

###  char collisionSnake(int indexSnake)
* Given a snake, checks collisions with the other snakes.

###  char collisionDiamond(struct snake *snake)
* Given a snake, checks collisions with all the diamonds in the grid.

###  int countEnemySnakes()
* Returns the number of enemy snakes still alive.

###  int *moveEnemy(void *vargp)
* Manages the kinematics of the enemy snake.

###  int calculateEnemyMove(struct snake *enemy)
* This function calculates the shortest distance from the head of an enemy snake to the (head of the) main snake, then it calculates the shortest path. Generate a move according to that shortest path. In order to simulate a more realistic movement, is added certain (probabilistic) degree of randomness.

###  int main()
* Runs the whole game, gets the initial parameters set by the user. Manages memory resources used by the game.

## Used libraries
_NCurses:_ Provides an API that allows the programmer to write text-based user interfaces in the terminal. The API provides functions to move the cursor, create windows, produce colors, play with mouse, etc. 
_Pthread:_ Allows a program to control multiple differente flows of work that overlap in time. 


See [SnakesAndTheGang](https://github.com/AlexGz9851/SnakesAndTheGang) to watch the original repo and commits.
