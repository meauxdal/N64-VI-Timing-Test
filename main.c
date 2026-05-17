#include <stdio.h>
#include <libdragon.h>

// CRT safe area -------------------------------------------------------------
#define SAFE_X  20
#define SAFE_Y  16

// VI register addresses (uncached KSEG1) ------------------------------------
#define VI_BASE                 0xA4400000
#define REG_VI_V_TOTAL          ((volatile uint32_t*)(VI_BASE + 0x18))
#define REG_VI_H_TOTAL          ((volatile uint32_t*)(VI_BASE + 0x1C))
#define REG_VI_H_TOTAL_LEAP     ((volatile uint32_t*)(VI_BASE + 0x20))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// populate preset string into overlay ---------------------------------------

#if defined(PRESET_NTSC)
#define PRESET_NAME "NTSC"
#elif defined(PRESET_MPAL_MATH)
#define PRESET_NAME "MPAL_MATH"
#elif defined(PRESET_MPAL_OLD)
#define PRESET_NAME "MPAL_OLD"
#elif defined(PRESET_MPAL_PREVIEW)
#define PRESET_NAME "MPAL_PREVIEW"
#elif defined(PRESET_PAL_1996)
#define PRESET_NAME "PAL_1996"
#elif defined(PRESET_PAL_1997)
#define PRESET_NAME "PAL_1997"
#else
#define PRESET_NAME "UNKNOWN"
#endif

// ---------------------------------------------------------------------------
// Presets - select via make PRESET=<name>, e.g. make PRESET=MPAL_MATH
// Default (no PRESET specified): NTSC
// VI_S = progressive half-line count. Interlaced = VI_S - 1.
// ---------------------------------------------------------------------------

#if defined(PRESET_MPAL_MATH)
  // MPAL progressive - math (lidnariq)
  #define FVI_NUM           6953850000LL
  #define FVI_DEN           143LL
  #define VI_S              526
  #define DEFAULT_H_TOTAL   3091
  #define DEFAULT_LEAP_PAT  0
  #define DEFAULT_LEAP_A    3091
  #define DEFAULT_LEAP_B    3091

#elif defined(PRESET_MPAL_OLD)
  // MPAL progressive - libdragon trunk / libultra
  #define FVI_NUM           6953850000LL
  #define FVI_DEN           143LL
  #define VI_S              526
  #define DEFAULT_H_TOTAL   3090
  #define DEFAULT_LEAP_PAT  4
  #define DEFAULT_LEAP_A    3099
  #define DEFAULT_LEAP_B    3098

#elif defined(PRESET_MPAL_PREVIEW)
  // MPAL - libdragon preview (interlaced profile used for both modes)
  #define FVI_NUM           6953850000LL
  #define FVI_DEN           143LL
  #define VI_S              526
  #define DEFAULT_H_TOTAL   3089
  #define DEFAULT_LEAP_PAT  0
  #define DEFAULT_LEAP_A    3101
  #define DEFAULT_LEAP_B    3101

#elif defined(PRESET_PAL_1996)
  // PAL progressive - 1996 / libdragon
  #define FVI_NUM           49656530LL
  #define FVI_DEN           1LL
  #define VI_S              626
  #define DEFAULT_H_TOTAL   3178
  #define DEFAULT_LEAP_PAT  21
  #define DEFAULT_LEAP_A    3183
  #define DEFAULT_LEAP_B    3184

#elif defined(PRESET_PAL_1997)
  // PAL progressive - 1997 / OS2.0H+
  #define FVI_NUM           49656530LL
  #define FVI_DEN           1LL
  #define VI_S              626
  #define DEFAULT_H_TOTAL   3178
  #define DEFAULT_LEAP_PAT  23
  #define DEFAULT_LEAP_A    3182
  #define DEFAULT_LEAP_B    3184

#else
  // PRESET_NTSC - default
  #define FVI_NUM           535500000LL
  #define FVI_DEN           11LL
  #define VI_S              526
  #define DEFAULT_H_TOTAL   3094
  #define DEFAULT_LEAP_PAT  0
  #define DEFAULT_LEAP_A    3094
  #define DEFAULT_LEAP_B    3094

#endif

// ---------------------------------------------------------------------------
// Write VI timing registers directly.
// REG_VI_V_TOTAL:      bits 9:0 = S - 1 (effective half-lines per frame)
// REG_VI_H_TOTAL:      bits 20:16 = 5-bit leap pattern, bits 11:0 = h_total-1
// REG_VI_H_TOTAL_LEAP: bits 27:16 = leap_a-1, bits 11:0 = leap_b-1
// LEAP_A and LEAP_B are clamped to >= h_total.
// Note: SERRATE (VI_CTRL bit 6) is not toggled. L/R adjusts half-lines only
// ---------------------------------------------------------------------------
static void apply_vi_timing(int h_total, int pat, int leap_a, int leap_b, int s)
{
    if (leap_a < h_total) leap_a = h_total;
    if (leap_b < h_total) leap_b = h_total;
    if (pat < 0)  pat = 0;
    if (pat > 31) pat = 31;

    *REG_VI_V_TOTAL      = (s - 1) & 0x3FF;
    *REG_VI_H_TOTAL      = ((pat & 0x1F) << 16) | ((h_total - 1) & 0xFFF);
    *REG_VI_H_TOTAL_LEAP = (((leap_a - 1) & 0xFFF) << 16) | ((leap_b - 1) & 0xFFF);
}

// ---------------------------------------------------------------------------

typedef struct {
    int fh_int;
    int fh_frac;
    int fv_int;
    int fv_frac;
    int delta_a;
    int delta_b;
    int avg_whole;
    int avg_tenths;
} timing_t;

static timing_t compute_timing(int h_total, int pat, int leap_a, int leap_b, int s)
{
    timing_t t = {0};

    long long den_h = FVI_DEN * (long long)h_total;
    t.fh_int  = (int)(FVI_NUM / den_h);
    t.fh_frac = (int)((FVI_NUM % den_h) * 100LL / den_h);

    long long fv_num = 2LL * FVI_NUM;
    long long fv_den = den_h * (long long)s;
    t.fv_int  = (int)(fv_num / fv_den);
    t.fv_frac = (int)((fv_num % fv_den) * 100LL / fv_den);

    t.delta_a = leap_a - h_total;
    t.delta_b = leap_b - h_total;

    int total_extra = 0;
    for (int i = 0; i < 5; i++)
        total_extra += ((pat >> i) & 1) ? t.delta_b : t.delta_a;

    int avg10 = total_extra * 2;
    t.avg_whole  = avg10 / 10;
    t.avg_tenths = avg10 % 10;

    return t;
}

// ---------------------------------------------------------------------------

static void draw_color_bars(surface_t *disp)
{
    static const uint8_t bars[7][3] = {
        {255, 255, 255}, // white
        {255, 255,   0}, // yellow
        {  0, 255, 255}, // cyan
        {  0, 255,   0}, // green
        {255,   0, 255}, // magenta
        {255,   0,   0}, // red
        {  0,   0, 255}, // blue
    };

    int bar_w = 320 / 7;

    for (int i = 0; i < 7; i++) {
        int x0 = i * bar_w;
        int w  = (i == 6) ? 320 - x0 : bar_w;
        uint32_t color = graphics_make_color(bars[i][0], bars[i][1], bars[i][2], 255);
        graphics_draw_box(disp, x0, 0, w, 240, color);
    }
}

// ---------------------------------------------------------------------------
// Draw overlay: parameters, derived timing estimates, register readback.
// ---------------------------------------------------------------------------
static void draw_overlay(surface_t *disp, int h_total, int pat, int leap_a, int leap_b, int s)
{
    char buf[64];
    timing_t t = compute_timing(h_total, pat, leap_a, leap_b, s);

    uint32_t reg_vt   = *REG_VI_V_TOTAL;
    uint32_t reg_ht   = *REG_VI_H_TOTAL;
    uint32_t reg_leap = *REG_VI_H_TOTAL_LEAP;

    int progressive = (s == VI_S);

    int y = SAFE_Y;
    graphics_set_color(graphics_make_color(0, 0, 0, 255), 0);

// text section -------------------------------------------------------------

    snprintf(buf, sizeof(buf), "VI TIMING TEST [%s]", PRESET_NAME);
    graphics_draw_text(disp, SAFE_X + 48, y, buf); y += 48;

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), "H_TOTAL:      %d", h_total);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;

    snprintf(buf, sizeof(buf), "LEAP PAT:     %d (0b%c%c%c%c%c)",
        pat,
        (pat >> 4) & 1 ? '1' : '0',
        (pat >> 3) & 1 ? '1' : '0',
        (pat >> 2) & 1 ? '1' : '0',
        (pat >> 1) & 1 ? '1' : '0',
        (pat >> 0) & 1 ? '1' : '0');
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;

    snprintf(buf, sizeof(buf), "LEAP_A:       %d  deltaA: +%d", leap_a, t.delta_a);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;

    snprintf(buf, sizeof(buf), "LEAP_B:       %d  deltaB: +%d", leap_b, t.delta_b);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;

    snprintf(buf, sizeof(buf), "avg/VSYNC:    %d.%d clk", t.avg_whole, t.avg_tenths);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 16;

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), "~fV:          %d.%02d Hz", t.fv_int, t.fv_frac);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;
    
    snprintf(buf, sizeof(buf), "~fH:          %d.%02d Hz", t.fh_int, t.fh_frac);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 16;

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), "REG V_TOTAL:  0x%08lX", (unsigned long)reg_vt);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;

    snprintf(buf, sizeof(buf), "REG H_TOTAL:  0x%08lX", (unsigned long)reg_ht);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;

    snprintf(buf, sizeof(buf), "REG LEAP:     0x%08lX", (unsigned long)reg_leap);
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 16;

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), "L/R - even/odd halflines (P/I)");
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;
    snprintf(buf, sizeof(buf), "DPAD U/D: H_TOTAL  DPAD L/R: PAT");
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;
    snprintf(buf, sizeof(buf), "C U/D: LEAP_A      C L/R: LEAP_B");
    graphics_draw_text(disp, SAFE_X + 16, y, buf); y += 12;
}

// ---------------------------------------------------------------------------
// debug output
// ---------------------------------------------------------------------------
static void log_values(int h_total, int pat, int leap_a, int leap_b, int s)
{
    timing_t t = compute_timing(h_total, pat, leap_a, leap_b, s);
    debugf("S=%d(%s) H_TOTAL=%d PAT=%d(0b%c%c%c%c%c) LEAP_A=%d LEAP_B=%d "
           "dA=%d dB=%d avg=%d.%d ~fH=%d.%02d ~fV=%d.%02d | "
           "REG_VT=0x%08lX REG_HT=0x%08lX REG_LEAP=0x%08lX\n",
           s, (s == VI_S) ? "P" : "I*",
           h_total, pat,
           (pat >> 4) & 1 ? '1' : '0',
           (pat >> 3) & 1 ? '1' : '0',
           (pat >> 2) & 1 ? '1' : '0',
           (pat >> 1) & 1 ? '1' : '0',
           (pat >> 0) & 1 ? '1' : '0',
           leap_a, leap_b,
           t.delta_a, t.delta_b,
           t.avg_whole, t.avg_tenths,
           t.fh_int, t.fh_frac,
           t.fv_int, t.fv_frac,
           (unsigned long)*REG_VI_V_TOTAL,
           (unsigned long)*REG_VI_H_TOTAL,
           (unsigned long)*REG_VI_H_TOTAL_LEAP);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(void)
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE_FETCH_ALWAYS);
    joypad_init();
    debug_init_usblog();

    int h_total = DEFAULT_H_TOTAL;
    int pat     = DEFAULT_LEAP_PAT;
    int leap_a  = DEFAULT_LEAP_A;
    int leap_b  = DEFAULT_LEAP_B;
    int s       = VI_S;  // start progressive

    apply_vi_timing(h_total, pat, leap_a, leap_b, s);
    log_values(h_total, pat, leap_a, leap_b, s);

    while (1) {
        joypad_poll();
        joypad_buttons_t keys = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        bool changed = false;

        if (keys.d_up)    { h_total++; changed = true; }
        if (keys.d_down)  { h_total--; changed = true; }
        if (keys.d_right) { pat++;     changed = true; }
        if (keys.d_left)  { pat--;     changed = true; }
        if (keys.c_up)    { leap_a++;  changed = true; }
        if (keys.c_down)  { leap_a--;  changed = true; }
        if (keys.c_right) { leap_b++;  changed = true; }
        if (keys.c_left)  { leap_b--;  changed = true; }

        // L = progressive, R = interlaced (S only; SERRATE not toggled)
        if (keys.l) { s = VI_S;     changed = true; }
        if (keys.r) { s = VI_S - 1; changed = true; }

        if (changed) {
            if (h_total < 1) h_total = 1;
            if (leap_a < h_total) leap_a = h_total;
            if (leap_b < h_total) leap_b = h_total;
            if (pat < 0)  pat = 0;
            if (pat > 31) pat = 31;

            apply_vi_timing(h_total, pat, leap_a, leap_b, s);
            log_values(h_total, pat, leap_a, leap_b, s);
        }

        surface_t *disp = display_get();
        draw_color_bars(disp);
        draw_overlay(disp, h_total, pat, leap_a, leap_b, s);
        display_show(disp);
    }

    return 0;
}
