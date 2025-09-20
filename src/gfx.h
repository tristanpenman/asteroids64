#ifndef __GFX_H
#define __GFX_H

#ifndef SCREEN_WD
#define SCREEN_WD 640
#endif

#ifndef SCREEN_HT
#define SCREEN_HT 480
#endif

#define SCREEN_RATIO ((float)SCREEN_HT / (float)SCREEN_WD)

#include <nusys.h>

// The maximum length of the display list of one task
#define GFX_GLIST_LEN 2048

// Number of gfx lists
#define GLX_GLIST_COUNT 15

// Modeling matrices
typedef struct {
    Mtx modeling;
    Mtx rotation;
    Mtx scale;
} Dynamic;

extern Dynamic gfx_dynamic;
extern Gfx* glistp;
extern Gfx gfx_glist[GLX_GLIST_COUNT][GFX_GLIST_LEN];

void gfx_init(void);
void gfx_rcp_init(void);
void gfx_clear_cfb(void);

#endif
