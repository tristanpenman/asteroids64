#include <nusys.h>
#include <PR/os_vi.h>

#include "gfx.h"

Gfx gfx_glist[GLX_GLIST_COUNT][GFX_GLIST_LEN];
Dynamic gfx_dynamic;
Gfx* glistp;

#ifndef GFX_FRAMEBUFFER_COUNT
#define GFX_FRAMEBUFFER_COUNT 3
#endif

static u16 __attribute__((aligned(64))) framebuffers[GFX_FRAMEBUFFER_COUNT][SCREEN_HT * SCREEN_WD];
static u16* framebuffer_ptrs[GFX_FRAMEBUFFER_COUNT];
static u16 __attribute__((aligned(64))) depth_buffer[SCREEN_HT * SCREEN_WD];

// Wrapper for memory alignment
static Vp vp;

static void gfx_update_viewport(void)
{
    // Conversion from (-1,-1,-1)-(1,1,1), with a 2-bit decimal suffix
    vp.vp.vscale[0] = SCREEN_WD * 2;
    vp.vp.vscale[1] = SCREEN_HT * 2;
    vp.vp.vscale[2] = G_MAXZ / 2;
    vp.vp.vscale[3] = 0;

    vp.vp.vtrans[0] = SCREEN_WD * 2;
    vp.vp.vtrans[1] = SCREEN_HT * 2;
    vp.vp.vtrans[2] = G_MAXZ / 2;
    vp.vp.vtrans[3] = 0;
}

// Display list to initialise RDP
static Gfx setup_rdpstate[] = {
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
#ifdef LOW_RESOLUTION
    // Bayer color dithering perturbs each pixel to hide 5-bit-per-channel banding
    // At 320×240 the VI’s resampling filter averages neighboring pixels while doubling the image,
    // which causes dithering to be smoothed out
    gsDPSetColorDither(G_CD_BAYER),
#else
    // At 640x480, dithered pixels are sent straight to the display. This 4x4 Bayer pattern
    // shows up as 'grain'. We disable this to ensure solid blacks.
    gsDPSetColorDither(G_CD_DISABLE),
    // Have to disable alpha dithering too
    gsDPSetAlphaDither(G_AD_DISABLE),
#endif
    gsSPEndDisplayList()
};

// Display list to initialise RSP
static Gfx setup_rspstate[] = {
    gsSPViewport(&vp),
    gsSPClearGeometryMode(0xFFFFFFFF),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK),
    gsSPTexture(0, 0, 0, 0, G_OFF),
    gsSPEndDisplayList()
};

void gfx_init(void)
{
    int i;

    nuGfxInit();

    for (i = 0; i < GFX_FRAMEBUFFER_COUNT; i++) {
        framebuffer_ptrs[i] = framebuffers[i];
    }

    nuGfxSetCfb(framebuffer_ptrs, GFX_FRAMEBUFFER_COUNT);
    nuGfxSetZBuffer(depth_buffer);

#ifdef LOW_RESOLUTION
    osViSetMode(&osViModeNtscLpf1);
#else
    osViSetMode(&osViModeNtscHpf1);
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF | OS_VI_GAMMA_DITHER_OFF | OS_VI_DITHER_FILTER_ON);
#endif

    gfx_update_viewport();
}

void gfx_rcp_init(void)
{
    gfx_update_viewport();

    // Setting the RSP segment register
    gSPSegment(glistp++, 0, 0x0);

    // Setting RSP
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(setup_rspstate));

    // Setting RDP
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(setup_rdpstate));

    gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
}

void gfx_clear_cfb(void)
{
    // Setting addresses of the frame buffer/Z-buffer and clear them using
    // nuGfxZBuffer (the address of the Z-buffer) and nuGfxCfb_ptr (the address
    // of the frame buffer) which are global variables of NuSYSTEM

    // Clear the Z-buffer
    gDPSetDepthImage(glistp++, OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetCycleType(glistp++, G_CYC_FILL);
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b,SCREEN_WD, OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetFillColor(glistp++, (GPACK_ZDZ(G_MAXFBZ,0) << 16 | GPACK_ZDZ(G_MAXFBZ,0)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
    gDPPipeSync(glistp++);

    // Clear the frame buffer
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, osVirtualToPhysical(nuGfxCfb_ptr));
    gDPSetFillColor(glistp++, (GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
    gDPPipeSync(glistp++);
}
