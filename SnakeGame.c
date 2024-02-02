#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

// Define the maximum length of the snake
#define MAX_LENGTH 100

// Define a structure to represent a segment of the snake
typedef struct {
    int x; // X-coordinate of the segment
    int y; // Y-coordinate of the segment
} SnakeSegment;

// Define a structure to represent the food in the game
typedef struct {
    int x; // X-coordinate of the food
    int y; // Y-coordinate of the food
    int value; // Value of the food
} Food;

// Declare an array to represent the snake's body segments
SnakeSegment snakeBody[MAX_LENGTH];
// Initialize the initial length of the snake
int snakeLength = 5;

// Declare a variable to represent the food in the game
Food food;

// Variable to keep track of the number of foods eaten
int foodsEaten = 0;

// Variable to store the duration of the current food
int foodDuration = 0;

// Declare a volatile variable to indicate if an interrupt signal is received
volatile sig_atomic_t interrupted = 0;

// Function to handle the interrupt signal
void handleInterrupt(int signum) {
    // Do nothing, just acknowledge the signal
    interrupted = 1;
}

// Function to draw the border of the game area
void drawBorder(int orig_x, int orig_y, int max_X, int max_Y) {
    int x = 0;
    int y = 0;
    if (max_X + max_Y < 100)
    {
        char draw[] = ".";

        // Draw top border
        while (x < max_X) {
            mvprintw(y, x, draw);
            x++;
        }
        x--;

        // Draw right border
        while (y < max_Y) {
            mvprintw(y, x, draw);
            y++;
        }
        y--;

        // Draw bottom border
        while (x > 0) {
            mvprintw(y, x, draw);
            x--;
        }
        x = 0;

        // Draw left border
        while (y > 0) {
            mvprintw(y, x, draw);
            y--;
        }
        y = 0;
    }
    
}

// Function to generate food at a random location with a random value between 1 and 9
void generateFood(int max_X, int max_Y) {
    // Generate random coordinates for the food
    food.x = rand() % (max_X - 2) + 1; // Avoid placing food on the border
    food.y = rand() % (max_Y - 2) + 1;

    // Generate a random value for the food between 1 and 9
    food.value = rand() % 9 + 1;

    // Set a random duration for the food (between 1 and 9)
    foodDuration = rand() % (500 - 100)  + 1;

    // Draw the food on the screen
    mvprintw(food.y, food.x, "%d", food.value);
}

// Function to generate a new food with a random duration
void generateFoodWithRandomDuration(int max_X, int max_Y) {
    generateFood(max_X, max_Y);  // Generate new food
}

// Function to handle food expiration and regeneration
void handleFoodExpiration(int max_X, int max_Y) {
    if (foodDuration > 0) {
        foodDuration--;

        // Check for expiration
        if (foodDuration == 0) {
            // Clear the expired food from the screen
            mvprintw(food.y, food.x, " ");

            // Generate a new food with random duration
            generateFoodWithRandomDuration(max_X, max_Y);
        }
    }
}

int main() {
    // Initialize the game state variable
    int gameRun = 1;
    // Seed the random number generator with the current time
    time_t t;
    srand((unsigned)time(&t));

    // Initialize the starting position of the snake's head
    int posX = 50;
    int posY = 50;
    // Initialize the user input variable
    char input = 'Q';

    // Variables to store the dimensions of the game window
    int max_x = 0;
    int max_y = 0;

    // Variables to represent the current direction of the snake
    bool north = false;
    bool south = false;
    bool east = false;
    bool west = false;

    // Initialize the ncurses library
    initscr();
    // Configure terminal settings for the game
    noecho();  // Disable echoing of characters
    cbreak();  // Disable line buffering
    curs_set(0);  // Hide the cursor
    getmaxyx(stdscr, max_y, max_x); // Get the dimensions of the game window
    posX = max_x / 2;  // Set the initial X-coordinate of the snake's head
    posY = max_y / 2; // Set the initial Y-coordinate of the snake's head
    keypad(stdscr, true); // Enable special keys
    halfdelay(1);  // Set a delay for getch() to wait for user input

    // Set up signal handling for interrupt (Ctrl+C)
    signal(SIGINT, handleInterrupt);

    // Initialize the snake's body segments
    for (int i = 0; i <= 4; i++) {
        // Set the X-coordinate of the current segment to the initial X-coordinate of the snake's head
        snakeBody[i].x = posX;
        // Set the Y-coordinate of the current segment to an offset from the initial Y-coordinate of the snake's head
        snakeBody[i].y = posY + i;
    }

    generateFoodWithRandomDuration(max_x, max_y);  // Initial food generation with duration

    // Randomly select an initial direction for the snake
    int initialDirection = rand() % 4;
    switch (initialDirection) {
        case 0:
            north = true;
            break;
        case 2:
            east = true;
            break;
        case 3:
            west = true;
            break;
        default:
            north = true;
            break;
    }

    // Main game loop
    while (gameRun == 1) {
        //drawBorder(posX, posY, max_x, max_y);
            drawBorder(posX, posX, max_x, max_y);
        // Handle interrupt signal (ignore it)
        if (interrupted) {
            interrupted = 0;  // Reset the interrupted flag
        }

        // Update the head of the snake based on the current direction
        if (north) {
            posY--;
        } else if (south) {
            posY++;
        } else if (east) {
            posX++;
        } else if (west) {
            posX--;
        }

        // Check for self-collision
        for (int i = 0; i < snakeLength; i++) {
            if (posX == snakeBody[i].x && posY == snakeBody[i].y) {
                // End the game if the snake collides with itself
                gameRun = 0;
                drawBorder(posX, posY, max_x, max_y);
                mvprintw(max_y / 2, max_x / 2 - 10, "Game Over! Foods Eaten: %d", foodsEaten);
                refresh();
                sleep(5);
                clear();
                endwin();
                return 0;
            }
        }

        // Check for collision with food
        if (posX == food.x && posY == food.y) {
            // Increase snake length to the value of the food
            for (int i = 0; i < food.value; i++) {
                if (snakeLength < MAX_LENGTH) {
                    snakeBody[snakeLength] = snakeBody[snakeLength - 1];
                    snakeLength++;
                    foodsEaten++;
                }
            }

            generateFoodWithRandomDuration(max_x, max_y);  // Generate new food with duration
        }

        // Update the positions of the snake body segments
        for (int i = snakeLength - 1; i > 0; i--) {
            snakeBody[i] = snakeBody[i - 1];
        }

        // Update the position of the snake's head
        snakeBody[0].x = posX;
        snakeBody[0].y = posY;

        // Handle food expiration and regeneration
        handleFoodExpiration(max_x, max_y);

        // Draw the snake on the screen
        for (int i = 0; i < snakeLength; i++) {
            mvprintw(snakeBody[i].y, snakeBody[i].x, "o");
        }

        // Draw the food with its remaining duration
        mvprintw(food.y, food.x, "%d", food.value);

        // Get user input to change the direction of the snake
        int key = getch();
        if (key != ERR) {
            input = key;
            switch (input) {
                case 'd':
                    north = false;
                    south = false;
                    east = true;
                    west = false;
                    break;
                case 'a':
                    north = false;
                    south = false;
                    east = false;
                    west = true;
                    break;
                case 's':
                    north = false;
                    south = true;
                    east = false;
                    west = false;
                    break;
                case 'w':
                    north = true;
                    south = false;
                    east = false;
                    west = false;
                    break;
            }
        }

        // Check for collision with the game borders
        if (posX <= 0 || posX >= max_x || posY <= 0 || posY >= max_y) {
            // End the game if the snake collides with the game borders

            // Check for win condition
            int perimeter = 2 * (max_x + max_y - 2);  // Exclude corners counted twice
            if (snakeLength >= perimeter / 10) {
                gameRun = 2;  // Win condition met
                mvprintw(max_y / 2, max_x / 2 - 10, "Congratulations! You Win! Foods Eaten: %d", foodsEaten);
            } else {
                // Loss condition
                mvprintw(max_y / 2, max_x / 2 - 10, "Game Over! Foods Eaten: %d", foodsEaten);
            }

            refresh();
            sleep(5);
            clear();
            endwin();
            return 0;
        }

        // Refresh the screen and introduce a delay to control game speed
        refresh();
        usleep(25000);
        clear();
    }

    // End the game and clean up resources
    endwin();
    return 0;
}
