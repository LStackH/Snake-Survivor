#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>  // For usleep()

// Own header files
#include "config.h"
#include "types.h"
#include "graphics.h"

// Global variables and initialization
Point snake[SNAKE_MAX_LENGTH];      // Snake is represented as an array of points, each with it's own x,y coordinates (2D Space)
int snake_dir[SNAKE_MAX_LENGTH];    // Snake segment direction, to represent the direction/animation of the snake with appropriate chars
int snake_length;
Point food;
int score;
int high_score = 0;

int direction;
int row, col;
int frame_count;
int is_game_over;

// NPC variables
Point farmer;
int farmer_direction_index; // 0=right, 1=down, 2=left, 3=up
int farmer_move_counter;    // To control NPC movement speed

// Define minimum and maximum values
int steps_min = 5;
int steps_max = 50;
int cooldown_min = 5;
int cooldown_max = 30;

int farmer_steps_taken = 0;       // How many steps since last shoot
int farmer_steps_threshold = 30;       // Move x steps before shooting
int farmer_shoot_cooldown = 0;    // How long to wait before moving again after shooting
int farmer_shoot_cooldown_duration = 30; // Frames to wait (e.g., 30 frames)

// Array for bullets
Bullet bullets[MAX_BULLETS];

void initialize_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }
}

void initialize_game() {
    snake_length = 3; 
    direction = KEY_RIGHT;
    frame_count = 0;
    is_game_over = 0;
    score = 0;

    for (int i = 0; i < snake_length; i++) {
        snake[i].x = GAME_AREA_LEFT + 10;  
        snake[i].y = GAME_AREA_TOP + 10;       
        snake_dir[i] = KEY_RIGHT;
    }

    food.x = GAME_AREA_LEFT + 15;
    food.y = GAME_AREA_TOP + 15;

    // Initialize the NPC
    // Place him at the top border and he'll move clockwise
    farmer.x = GAME_AREA_LEFT - 1;
    farmer.y = GAME_AREA_TOP - 1;
    farmer_direction_index = 0; // Start moving right
    farmer_move_counter = 0;
    // Reset the bullet array
    initialize_bullets();

    // Update new values for thresholds to remain unpredictable
    farmer_steps_threshold = steps_min + rand() % (steps_max - steps_min + 1);
    farmer_shoot_cooldown_duration = cooldown_min + rand() % (cooldown_max - cooldown_min + 1);
}


void update_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        // Before updating position, erase the bullet at the old position
        mvprintw(bullets[i].y, bullets[i].x, " ");

        bullets[i].speed_counter++;
        if (bullets[i].speed_counter >= bullets[i].speed) {
            bullets[i].speed_counter = 0;
            // Move the bullet
            bullets[i].x += bullets[i].dx;
            bullets[i].y += bullets[i].dy;

            // Check out of bounds
            if (bullets[i].x < 0 || bullets[i].x > col ||
                bullets[i].y < 0 || bullets[i].y > row) {
                bullets[i].active = 0;
                continue;
            }

            // Check collision with snake
            // Just loop through snake segments:
            for (int s = 0; s < snake_length; s++) {
                if (snake[s].x == bullets[i].x && snake[s].y == bullets[i].y) {
                    // Hit snake, reduce length
                    snake_length -= bullets[i].damage;
                    if (snake_length < 3) snake_length = 3; // Minimum length
                    bullets[i].active = 0;
                    break;
                }
            }
        }
    }
}

void spawn_bullet(int x, int y, int direction, int speed, int damage) {
    // Directions: 0=right, 1=down, 2=left, 3=up
    int dx, dy;
    switch(direction){
        case 0: //moving right, shoot down
            dx = 0;
            dy = 1;
            break;
        case 1: //moving down, shoot left
            dx = -1;
            dy = 0;
            break;
        case 2: //moving left, shoot up
            dx = 0;
            dy = -1;
            break;
        case 3: //moving up, shoot right
            dx = 1;
            dy = 0;
            break;
        default:
            dx = 0;
            dy = 0;
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].dx = dx;
            bullets[i].dy = dy;
            bullets[i].speed = speed;
            bullets[i].speed_counter = 0;
            bullets[i].damage = damage;
            bullets[i].active = 1;
            break;
        }
    }
}

void update_npc() {
    // If we are currently in shooting cooldown, do not move the farmer
    if (farmer_shoot_cooldown > 0) {
        farmer_shoot_cooldown--;
        return;
    }

    // If we've moved enough steps, time to shoot and pause
    if (farmer_steps_taken >= farmer_steps_threshold) {
        // Trigger the shooting action (for now just simulate the pause)
        spawn_bullet(farmer.x, farmer.y, farmer_direction_index, 3, 1);
        farmer_shoot_cooldown = farmer_shoot_cooldown_duration; // Pause movement for this many frames
        farmer_steps_taken = 0; // Reset steps taken
        return; // On the frame shooting happens, we don't move
    }

    // Move NPC after counter threshold
    farmer_move_counter++;
    if (farmer_move_counter < 2) {
        return;
    }
    farmer_move_counter = 0;

    // Erase old position
    mvprintw(farmer.y, farmer.x, " ");

    // NPC moves around the border in a loop:
    // Directions: 0=right, 1=down, 2=left, 3=up
    switch (farmer_direction_index) {
        case 0: // moving right along top row
            farmer.x++;
            if (farmer.x >= GAME_AREA_RIGHT + 1) { 
                farmer.x = GAME_AREA_RIGHT + 1; 
                farmer_direction_index = 1; // turn down
            }
            break;
        case 1: // moving down along right column
            farmer.y++;
            if (farmer.y >= GAME_AREA_BOTTOM + 1) {
                farmer.y = GAME_AREA_BOTTOM + 1;
                farmer_direction_index = 2; // turn left
            }
            break;
        case 2: // moving left along bottom row
            farmer.x--;
            if (farmer.x <= GAME_AREA_LEFT - 1) {
                farmer.x = GAME_AREA_LEFT - 1;
                farmer_direction_index = 3; // turn up
            }
            break;
        case 3: // moving up along left column
            farmer.y--;
            if (farmer.y <= GAME_AREA_TOP - 1) {
                farmer.y = GAME_AREA_TOP - 1;
                farmer_direction_index = 0; // turn right
            }
            break;
    }

    farmer_steps_taken++;
}

void update_game() {
    Point old_tail = snake[snake_length - 1];

    for (int i = snake_length - 1; i > 0; i--) {
        snake_dir[i] = snake_dir[i - 1]; 
        snake[i] = snake[i - 1];          
    }

    snake_dir[0] = direction;
    switch (direction) {
        // Game 2D space, -y is up and +y is down
        // -x left, +x right
        case KEY_UP:
            snake[0].y--; break;
        case KEY_DOWN:
            snake[0].y++; break;
        case KEY_LEFT:  
            snake[0].x--; break;
        case KEY_RIGHT: 
            snake[0].x++; break;
    }

    mvprintw(old_tail.y, old_tail.x, " ");

    // Check boundary collision
    if (snake[0].x <= GAME_AREA_LEFT || snake[0].x >= GAME_AREA_RIGHT ||
        snake[0].y <= GAME_AREA_TOP  || snake[0].y >= GAME_AREA_BOTTOM) {
        is_game_over = 1;
        return;
    }

    // Check self-collision
    for (int i = snake_length - 1; i > 0; i--) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            is_game_over = 1; 
            return;
        }
    }

    // Check food
    if (snake[0].x == food.x && snake[0].y == food.y) {
        snake_length++;
        if (snake_length >= SNAKE_MAX_LENGTH) {
            is_game_over = 1;
            return;
        }

        food.x = GAME_AREA_LEFT + 1 + (rand() % (GAME_AREA_RIGHT - GAME_AREA_LEFT - 2));
        food.y = GAME_AREA_TOP + 1 + (rand() % (GAME_AREA_BOTTOM - GAME_AREA_TOP - 2));
    }
}

void handle_input() {
    int ch = getch();
    if ((ch == KEY_UP && direction != KEY_DOWN) ||
        (ch == KEY_DOWN && direction != KEY_UP) ||
        (ch == KEY_LEFT && direction != KEY_RIGHT) ||
        (ch == KEY_RIGHT && direction != KEY_LEFT)) {
        direction = ch;
    }
}

void update_score(){
    score = snake_length-3;
}

// High level main game loop, calls all necessary functions until game over flag is set
void playGame() {
    initialize_game();
    nodelay(stdscr, TRUE);

    clear();
    while (!is_game_over) {
        handle_input();
        update_game();
        update_npc();
        update_bullets();
        draw_game();
        update_score();
        usleep(DELAY);
    }

    // Game over screen, waits until a key is pressed before running to end
    clear();
    if (snake_length >= SNAKE_MAX_LENGTH) {
        mvprintw(row / 2, (col - 35) / 2, "You won! Press any key to return to menu.");
    } else {
        if (score > high_score){
            high_score = score;
            mvprintw((row / 2) -1, (col-15) / 2, "New high score: %d", score);
            mvprintw(row / 2, (col - 40) / 2, "Game Over! Press any key to return to menu.");
        } else {
            mvprintw((row / 2) -1, (col-15) / 2, "Your score: %d", score);
            mvprintw(row / 2, (col - 40) / 2, "Game Over! Press any key to return to menu.");
        }
    }
    refresh();

    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
    // Returns back to main()'s - while loop, that throws us back to main_menu()
}

int main_menu() {
    clear();
    mvprintw(row / 2, (col - 11) / 2, "Snake Game");
    mvprintw((row / 2) + 1, (col - 18) / 2, "Press 'p' to play");
    mvprintw((row / 2) + 2, (col - 18) / 2, "Press 'q' to quit");
    refresh();

    nodelay(stdscr, FALSE);
    int ch = getch();
    nodelay(stdscr, TRUE);

    return ch;
}

int main() {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, row, col);

    // Give a new seed for rand, with current time to make rand more random
    srand((unsigned int)time(NULL));

    // Initialize colors
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_RED, -1);   // Food: Red
        init_pair(2, COLOR_GREEN, -1); // Snake: Green
        init_pair(3, COLOR_YELLOW, -1); // Border: yellow
        init_pair(4, COLOR_MAGENTA, -1); // NPC: Magenta
    }

    while (1) {
        int choice = main_menu();
        if (choice == 'p') {
            playGame();
        } else if (choice == 'q') {
            break;
        }
    }

    endwin();
    return 0;
}
