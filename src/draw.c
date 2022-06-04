#include <stdio.h>

#include "canvas.h"
#include "defines.h"

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void draw_score(int score)
{
    static char buffer[SCORE_BUFFER_SIZE];

    sprintf(buffer, "%u", score);

#ifdef N64
    canvas_draw_text(buffer, -0.45f, -0.33f, 0.65f);
#else
    canvas_draw_text(buffer, -0.45f, -0.4f, 0.35f);
#endif
}
