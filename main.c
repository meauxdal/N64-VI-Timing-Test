#include <stdio.h>
#include <libdragon.h>

// VI register addresses (uncached KSEG1) ------------------------------------
#define VI_BASE                 0xA4400000
#define REG_VI_V_TOTAL          ((volatile uint32_t*)(VI_BASE + 0x18))
#define REG_VI_H_TOTAL          ((volatile uint32_t*)(VI_BASE + 0x1C))
#define REG_VI_H_TOTAL_LEAP     ((volatile uint32_t*)(VI_BASE + 0x20))

// ---------------------------------------------------------------------------
// Preset definition
// ---------------------------------------------------------------------------

typedef struct {
    const char *name;

    long long fvi_num;
    long long fvi_den;

    int vi_s;

    int default_h_total;
    int default_pat;
    int default_leap_a;
    int default_leap_b;

    resolution_t resolution;
    int fb_width;
    int fb_height;

    int safe_x;
    int safe_y;
} preset_t;

// ---------------------------------------------------------------------------
// Presets
// ---------------------------------------------------------------------------

static const preset_t preset_ntsc = {
    .name = "NTSC",

    .fvi_num = 535500000LL,
    .fvi_den = 11LL,

    .vi_s = 526,

    .default_h_total = 3094,
    .default_pat     = 0,
    .default_leap_a  = 3094,
    .default_leap_b  = 3094,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 16,
};

static const preset_t preset_mpal_math = {
    .name = "MPAL_MATH",

    .fvi_num = 6953850000LL,
    .fvi_den = 143LL,

    .vi_s = 526,

    .default_h_total = 3091,
    .default_pat     = 0,
    .default_leap_a  = 3091,
    .default_leap_b  = 3091,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 16,
};

static const preset_t preset_mpal_old = {
    .name = "MPAL_OLD",

    .fvi_num = 6953850000LL,
    .fvi_den = 143LL,

    .vi_s = 526,

    .default_h_total = 3090,
    .default_pat     = 4,
    .default_leap_a  = 3099,
    .default_leap_b  = 3098,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 16,
};

static const preset_t preset_mpal_preview = {
    .name = "MPAL_PREVIEW",

    .fvi_num = 6953850000LL,
    .fvi_den = 143LL,

    .vi_s = 526,

    .default_h_total = 3089,
    .default_pat     = 0,
    .default_leap_a  = 3101,
    .default_leap_b  = 3101,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 16,
};

static const preset_t preset_pal_1996 = {
    .name = "PAL_1996",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .vi_s = 626,

    .default_h_total = 3178,
    .default_pat     = 21,
    .default_leap_a  = 3183,
    .default_leap_b  = 3184,

    .resolution = RESOLUTION_320x288,
    .fb_width   = 320,
    .fb_height  = 288,

    .safe_x = 20,
    .safe_y = 20,
};

static const preset_t preset_pal_1997 = {
    .name = "PAL_1997",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .vi_s = 626,

    .default_h_total = 3178,
    .default_pat     = 23,
    .default_leap_a  = 3182,
    .default_leap_b  = 3184,

    .resolution = RESOLUTION_320x288,
    .fb_width   = 320,
    .fb_height  = 288,

    .safe_x = 20,
    .safe_y = 20,
};

// ---------------------------------------------------------------------------
// Active preset selection
// ---------------------------------------------------------------------------

#if defined(PRESET_MPAL_MATH)
    #define ACTIVE_PRESET preset_mpal_math
#elif defined(PRESET_MPAL_OLD)
    #define ACTIVE_PRESET preset_mpal_old
#elif defined(PRESET_MPAL_PREVIEW)
    #define ACTIVE_PRESET preset_mpal_preview
#elif defined(PRESET_PAL_1996)
    #define ACTIVE_PRESET preset_pal_1996
#elif defined(PRESET_PAL_1997)
    #define ACTIVE_PRESET preset_pal_1997
#else
    #define ACTIVE_PRESET preset_ntsc
#endif

static const preset_t *preset = &ACTIVE_PRESET;

// ---------------------------------------------------------------------------
// Write VI timing registers directly.
// ---------------------------------------------------------------------------

static void sanitize_timing(int *h_total, int *pat, int *leap_a, int *leap_b)
{
    if (*h_total < 1)
        *h_total = 1;

    if (*leap_a < *h_total)
        *leap_a = *h_total;

    if (*leap_b < *h_total)
        *leap_b = *h_total;

    if (*pat < 0)
        *pat = 0;

    if (*pat > 31)
        *pat = 31;
}

static void apply_vi_timing(int h_total, int pat, int leap_a, int leap_b, int s)
{
    sanitize_timing(&h_total, &pat, &leap_a, &leap_b);

    *REG_VI_V_TOTAL = (s - 1) & 0x3FF;

    *REG_VI_H_TOTAL = (uint32_t)(
        ((pat & 0x1F) << 16) |
        ((h_total - 1) & 0xFFF)
    );

    *REG_VI_H_TOTAL_LEAP = (uint32_t)(
        (((leap_a - 1) & 0xFFF) << 16) |
        ((leap_b - 1) & 0xFFF)
    );
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

static timing_t compute_timing(
    const preset_t *preset,
    int h_total,
    int pat,
    int leap_a,
    int leap_b,
    int s)
{
    timing_t t = {0};

    long long den_h = preset->fvi_den * (long long)h_total;

    t.fh_int  = (int)(preset->fvi_num / den_h);
    t.fh_frac = (int)((preset->fvi_num % den_h) * 100LL / den_h);

    long long fv_num = 2LL * preset->fvi_num;
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
        {255, 255, 255},
        {255, 255,   0},
        {  0, 255, 255},
        {  0, 255,   0},
        {255,   0, 255},
        {255,   0,   0},
        {  0,   0, 255},
    };

    int bar_w = preset->fb_width / 7;

    for (int i = 0; i < 7; i++) {
        int x0 = i * bar_w;
        int w  = (i == 6) ? preset->fb_width - x0 : bar_w;

        uint32_t color = graphics_make_color(
            bars[i][0],
            bars[i][1],
            bars[i][2],
            255
        );

        graphics_draw_box(disp, x0, 0, w, preset->fb_height, color);
    }
}

// ---------------------------------------------------------------------------

static void draw_overlay(
    surface_t *disp,
    int h_total,
    int pat,
    int leap_a,
    int leap_b,
    int s)
{
    char buf[64];

    timing_t t = compute_timing(
        preset,
        h_total,
        pat,
        leap_a,
        leap_b,
        s
    );

    uint32_t reg_vt   = *REG_VI_V_TOTAL;
    uint32_t reg_ht   = *REG_VI_H_TOTAL;
    uint32_t reg_leap = *REG_VI_H_TOTAL_LEAP;

    int y = preset->safe_y;

    graphics_set_color(graphics_make_color(0, 0, 0, 255), 0);

    snprintf(buf, sizeof(buf), "VI TIMING TEST [%s]", preset->name);
    graphics_draw_text(disp, preset->safe_x + 54, y, buf);
    y += 36;

    snprintf(buf, sizeof(buf), "     H_TOTAL: %d", h_total);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "    LEAP PAT: %d (0b%c%c%c%c%c)",
        pat,
        (pat >> 4) & 1 ? '1' : '0',
        (pat >> 3) & 1 ? '1' : '0',
        (pat >> 2) & 1 ? '1' : '0',
        (pat >> 1) & 1 ? '1' : '0',
        (pat >> 0) & 1 ? '1' : '0');

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "      LEAP_A: %d  deltaA: +%d",
        leap_a,
        t.delta_a);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "      LEAP_B: %d  deltaB: +%d",
        leap_b,
        t.delta_b);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "   avg/VSYNC: %d.%d clk",
        t.avg_whole,
        t.avg_tenths);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 16;

    snprintf(buf, sizeof(buf),
        "         ~fV: %d.%02d Hz",
        t.fv_int,
        t.fv_frac);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "         ~fH: %d.%02d Hz",
        t.fh_int,
        t.fh_frac);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 16;

    snprintf(buf, sizeof(buf),
        "         MODE: %s",
        (s == preset->vi_s) ? "PROGRESSIVE" : "INTERLACED");

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 16;

    snprintf(buf, sizeof(buf),
        " REG V_TOTAL: 0x%08lX",
        (unsigned long)reg_vt);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        " REG H_TOTAL: 0x%08lX",
        (unsigned long)reg_ht);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "    REG LEAP: 0x%08lX",
        (unsigned long)reg_leap);

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 20;

    snprintf(buf, sizeof(buf),
        "L/R - even/odd halflines (P/I)");

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "DPAD U/D: H_TOTAL  DPAD L/R: PAT");

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf),
        "C U/D: LEAP_A      C L/R: LEAP_B");

    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
}

// ---------------------------------------------------------------------------
// debug output
// ---------------------------------------------------------------------------

static void log_values(int h_total, int pat, int leap_a, int leap_b, int s)
{
    timing_t t = compute_timing(
        preset,
        h_total,
        pat,
        leap_a,
        leap_b,
        s
    );

    debugf(
        "S=%d(%s) H_TOTAL=%d PAT=%d(0b%c%c%c%c%c) "
        "LEAP_A=%d LEAP_B=%d dA=%d dB=%d "
        "avg=%d.%d ~fH=%d.%02d ~fV=%d.%02d | "
        "REG_VT=0x%08lX REG_HT=0x%08lX REG_LEAP=0x%08lX\n",

        s,
        (s == preset->vi_s) ? "P" : "I*",

        h_total,
        pat,

        (pat >> 4) & 1 ? '1' : '0',
        (pat >> 3) & 1 ? '1' : '0',
        (pat >> 2) & 1 ? '1' : '0',
        (pat >> 1) & 1 ? '1' : '0',
        (pat >> 0) & 1 ? '1' : '0',

        leap_a,
        leap_b,

        t.delta_a,
        t.delta_b,

        t.avg_whole,
        t.avg_tenths,

        t.fh_int,
        t.fh_frac,

        t.fv_int,
        t.fv_frac,

        (unsigned long)*REG_VI_V_TOTAL,
        (unsigned long)*REG_VI_H_TOTAL,
        (unsigned long)*REG_VI_H_TOTAL_LEAP
    );
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(void)
{
    display_init(
        preset->resolution,
        DEPTH_16_BPP,
        2,
        GAMMA_NONE,
        ANTIALIAS_RESAMPLE_FETCH_ALWAYS
    );

    joypad_init();
    debug_init_usblog();

    int h_total = preset->default_h_total;
    int pat     = preset->default_pat;
    int leap_a  = preset->default_leap_a;
    int leap_b  = preset->default_leap_b;
    int s       = preset->vi_s;

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

        if (keys.l) { s = preset->vi_s;     changed = true; }
        if (keys.r) { s = preset->vi_s - 1; changed = true; }

        if (changed) {
            sanitize_timing(&h_total, &pat, &leap_a, &leap_b);

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
