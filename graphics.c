#include <ncurses.h>
#include "config.h"
#include "types.h"
#include "graphics.h"

// Declare externs for the globals defined in main.c
extern Point snake[];
extern int snake_dir[];
extern int snake_length;
extern Point food;
extern int direction;
extern int row, col;
extern int frame_count;
extern int is_game_over;
extern Point farmer;
extern int farmer_direction_index;
extern int farmer_move_counter;
extern int farmer_steps_taken;
extern int farmer_steps_threshold;
extern int farmer_shoot_cooldown;
extern int farmer_shoot_cooldown_duration;
extern Bullet bullets[];
extern int score;
extern int high_score;

void draw_ui() {
    mvprintw(GAME_AREA_BOTTOM + 2, 2, "Score: %d", snake_length - 3);
    mvprintw(GAME_AREA_BOTTOM + 3, 2, "Use arrow keys to move.");
    mvprintw(GAME_AREA_BOTTOM + 4, 2, "Press 'q' to quit.");
}

void draw_border() {
    static int animation_frame = 0;
    char vert_border_chars[] = {'#', '-', '#', '-'};
    char hor_border_chars[] = {'|', '!', '|', '!'};
    char current_vert_char = vert_border_chars[animation_frame];
    char current_hor_char = hor_border_chars[animation_frame];

    // Sets color attribute for all the borders
    attron(COLOR_PAIR(3));
    for (int c = GAME_AREA_LEFT; c <= GAME_AREA_RIGHT; c++) {
        mvprintw(GAME_AREA_TOP, c, "%c", current_vert_char);
        mvprintw(GAME_AREA_BOTTOM, c, "%c", current_vert_char);
    }

    for (int r = GAME_AREA_TOP; r <= GAME_AREA_BOTTOM; r++) {
        mvprintw(r, GAME_AREA_LEFT, "%c", current_hor_char);
        mvprintw(r, GAME_AREA_RIGHT, "%c", current_hor_char);
    }
    attroff(COLOR_PAIR(3));     // color attribute off after drawing

    animation_frame = (animation_frame + 1) % 4;
}

void draw_bullets() {
    attron(COLOR_PAIR(3));
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            mvprintw(bullets[i].y, bullets[i].x, "*");
        }
    }
    attroff(COLOR_PAIR(3));
}

void draw_game() {
    if (frame_count % 15 == 0) {
        draw_border();
    }
    frame_count++;
    draw_ui();

    // Sets color attribute for the food
    attron(COLOR_PAIR(1)); 
    mvprintw(food.y, food.x, "O");
    attroff(COLOR_PAIR(1));

    // Draw the snake
    for (int i = 0; i < snake_length; i++) {
        char segment_char;
        switch (snake_dir[i]) {
            case KEY_UP:    segment_char = '^'; break;
            case KEY_DOWN:  segment_char = 'v'; break;
            case KEY_LEFT:  segment_char = '<'; break;
            case KEY_RIGHT: segment_char = '>'; break;
            default:        segment_char = 'X'; break;
        }

        // Sets color attribute for the snake
        attron(COLOR_PAIR(2));
        mvprintw(snake[i].y, snake[i].x, "%c", segment_char);
        attroff(COLOR_PAIR(2));
    }

    // Draw the NPC
    attron(COLOR_PAIR(4));
    mvprintw(farmer.y, farmer.x, "F");
    attroff(COLOR_PAIR(4));

    draw_bullets();

    refresh();
}