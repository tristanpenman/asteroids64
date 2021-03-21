#include <assert.h>
#include <nusys.h>
#include <stdlib.h>
#include <string.h>

#include "canvas.h"
#include "collision.h"
#include "data.h"
#include "defines.h"
#include "entities.h"
#include "gfx.h"
#include "mathdefs.h"
#include "shape.h"
#include "vec.h"

// nusys input
extern NUContData contdata[1];
extern u8 contPattern;

// useful constants
static const float factor = 1.0f / 60.0f;
static const float degrees_to_radians = M_PI / 180.0f;

// ship wrapping constants
static const float max_x =  1.0 / 2.0f + SHIP_RADIUS;
static const float min_x = -1.0 / 2.0f - SHIP_RADIUS;
static const float max_y =  SCREEN_RATIO / 2.0f + SHIP_RADIUS;
static const float min_y = -SCREEN_RATIO / 2.0f - SHIP_RADIUS;

// asteroid state
static struct asteroid asteroids[MAX_ASTEROIDS];
static unsigned int starting_asteroids;

// bullets
static struct bullet bullets[MAX_BULLETS];

// player state
static struct player player;

// shapes
static uint8_t asteroid_shapes[NUM_ASTEROID_SHAPES];
static uint8_t bullet_shape;
static uint8_t player_frame_1_shape;
static uint8_t player_frame_2_shape;

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static unsigned int num_asteroids_for_level(int level) {
    switch (level) {
        case 1:
            return 4;
        case 2:
            return 6;
        case 3:
            return 8;
        case 4:
            return 10;
        default:
            break;
    }

    return 11;
}

void check_fire_button(float f)
{
    unsigned int i;
    unsigned int num_player_bullets = 0;

    if (player.reloading == true) {
        player.reload_delay += f;
        if (player.reload_delay >= BULLET_DELAY) {
            player.reload_delay = 0.0f;
            player.reloading = false;
        } else {
            return;
        }
    }

    if (player.state != PS_NORMAL) {
        return;
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (true == bullets[i].visible) {
            num_player_bullets++;
        }
    }

    if (num_player_bullets >= MAX_BULLETS) {
        return;
    }

    if (KS_DOWN == player.keys.fire) {
        for (i = 0; i < MAX_BULLETS; i++) {
            if (false == bullets[i].visible) {
                bullet_init(&bullets[i], &player.pos, player.rot);

                break;
            }
        }

        player.reload_delay = 0.0f;
        player.reloading = true;
    }
}

/******************************************************************************
 *
 * Collision helpers
 *
 *****************************************************************************/

void check_collisions(unsigned int *asteroids_hit)
{
    float dx, dy;
    unsigned int i, j;

    player.hit = 0;

    // Check for asteroid collisions
    for (j = 0; j < MAX_ASTEROIDS; j++) {
        if (asteroids[j].visible == false) {
            continue;
        }

        if (player.state == PS_NORMAL) {
            bool collision = collision_test_shapes(
                &player_frame_1_shape_data, &player.pos, player.rot, 1.0f,
                &asteroid_shape_data[asteroids[j].shape], &asteroids[j].pos, 0, asteroids[j].scale);

            if (collision) {
                player.hit++;
            }
        }
    }
}

/******************************************************************************
 *
 * Update logic
 *
 *****************************************************************************/

void level_draw()
{
    int i;
    char conbuf[20];

    canvas_start_drawing(true);
    canvas_draw_line_segments(player_frame_1_shape, player.pos, player.rot, vec_2d_unit);

    for (i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].visible) {
            canvas_draw_line_segments(asteroid_shapes[asteroids[i].shape], asteroids[i].pos, 0, vec_2d_unit);
        }
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].visible) {
            canvas_draw_line_segments(bullet_shape, bullets[i].pos, bullets[i].rot, vec_2d_unit);
        }
    }

    canvas_finish_drawing(false);

    // Print score using debug output
    sprintf(conbuf, "%d", player.score);
    nuDebConTextPos(0, 3, 3);
    nuDebConCPuts(0, conbuf);

    // Player hit?
    sprintf(conbuf, "%d", player.hit);
    nuDebConTextPos(0, 5, 3);
    nuDebConCPuts(0, conbuf);

    {
        int num_bullets = 0;
        int i = 0;
        for (i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].visible) {
                num_bullets++;
            }
        }

        // Fire?
        sprintf(conbuf, "%d", num_bullets);
        nuDebConTextPos(0, 7, 3);
        nuDebConCPuts(0, conbuf);
    }

    nuDebConDisp(NU_SC_SWAPBUFFER);
}

void level_update()
{
    float f = 1.f / 60.f;
    int i;
    static float rot;

    // Data reading of controller 1
    nuContDataGetEx(contdata, 0);

    if (contdata[0].stick_x < -20) {
        player.rot -= SHIP_ROTATION_SPEED;
    } else if (contdata[0].stick_x > 20) {
        player.rot += SHIP_ROTATION_SPEED;
    }

    if (player.rot > 360.0) {
        player.rot -= 360.0;
    } else if (player.rot < 0.0) {
        player.rot += 360.0;
    }

    rot = player.rot * degrees_to_radians;

    player.keys.fire = contdata[0].button & A_BUTTON ? KS_DOWN : KS_UP;

    check_fire_button(f);

    if (contdata[0].button & B_BUTTON) {
        player.vel.x += sinf(rot) * SHIP_ACCELERATION * factor;
        player.vel.y -= cosf(rot) * SHIP_ACCELERATION * factor;
    }

    player.pos.x += player.vel.x * factor;
    player.pos.y += player.vel.y * factor;

    if (player.pos.y > max_y) {
        player.pos.y = min_y;
    } else if (player.pos.y < min_y) {
        player.pos.y = max_y;
    }

    if (player.pos.x > max_x) {
        player.pos.x = min_x;
    } else if (player.pos.x < min_x) {
        player.pos.x = max_x;
    }

    for (i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].visible) {
            struct vec_2d lim;
            lim.x = 1.f / 2.f + asteroids[i].radius;
            lim.y = 0.75f / 2.f + asteroids[i].radius;
            asteroid_update(&asteroids[i], f, &lim);
        }
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].visible) {
            bullet_update(&bullets[i], f);
        }
    }

    {
        unsigned int asteroids_hit = 0;
        check_collisions(&asteroids_hit);
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void level_init(unsigned int level, unsigned int lives, unsigned int score)
{
    int i;

    canvas_reset();

    player_frame_1_shape = canvas_load_shape(&player_frame_1_shape_data);
    player_frame_2_shape = canvas_load_shape(&player_frame_2_shape_data);

    for (i = 0; i < NUM_ASTEROID_SHAPES; i++) {
        asteroid_shapes[i] = canvas_load_shape(&asteroid_shape_data[i]);
    }

    bullet_shape = canvas_load_shape(&bullet_shape_data);

    starting_asteroids = num_asteroids_for_level(level);
    memset(asteroids, 0, sizeof(struct asteroid) * MAX_ASTEROIDS);
    for (i = 0; i < starting_asteroids; i++) {
        asteroid_init(&asteroids[i]);
    }

    player.rot = 0;
    player.pos.x = 0;
    player.pos.y = 0;
    player.vel.x = 0;
    player.vel.y = 0;
}

void level_loop(bool draw)
{
    level_update();

    if (draw) {
        level_draw();
    }
}
