#ifndef __DEFINES_H
#define __DEFINES_H

// asteroids
#define ASTEROID_SPEED_MIN 0.05f
#define ASTEROID_SPEED_MAX 0.11f
#define MAX_ASTEROIDS 100
#define NO_ASTEROID_RADIUS 0.19f   /* Asteroids spawn >= distance from ship */
#define NUM_ASTEROID_SHAPES 4

// bullets
#define BULLET_DELAY 0.25f
#define BULLET_SPEED 0.8f
#define MAX_BULLETS 10
#define MAX_BULLET_DISTANCE (1.0f / 2.0f - SHIP_PIVOT)

// player
#define SHIP_ACCELERATION 0.45f
#define SHIP_EXPLOSION_SHARDS 5
#define SHIP_PIVOT 0.025f
#define SHIP_RADIUS 0.038f
#define SHIP_ROTATION_SPEED 4.0f

#endif
