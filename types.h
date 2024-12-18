#ifndef TYPES_H
#define TYPES_H

// A point struct, with x,y coordinates to represent the points location in 2D space
typedef struct {
    int x,y;
} Point;

// Struct for bullets
typedef struct {
    int x, y;
    int dx, dy;        // Movement direction, e.g., (1,0) for right
    int speed;         // How many frames before moving one step
    int speed_counter; // Counts frames to control when the bullet moves
    int damage;
    int active;        // 1 if bullet is in use, 0 if not
} Bullet;


#endif