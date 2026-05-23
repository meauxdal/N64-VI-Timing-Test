#include <stdio.h>
#include <libdragon.h>

// VI register addresses (uncached KSEG1) ------------------------------------
#define VI_BASE             0xA4400000
#define REG_VI_CTRL         ((volatile uint32_t*)(VI_BASE + 0x00))
#define REG_VI_V_BURST      ((volatile uint32_t*)(VI_BASE + 0x0C))
#define REG_VI_CURRENT      ((volatile uint32_t*)(VI_BASE + 0x10))
#define REG_VI_V_TOTAL      ((volatile uint32_t*)(VI_BASE + 0x18))
#define REG_VI_H_TOTAL      ((volatile uint32_t*)(VI_BASE + 0x1C))
#define REG_VI_H_TOTAL_LEAP ((volatile uint32_t*)(VI_BASE + 0x20))
#define REG_VI_V_VIDEO      ((volatile uint32_t*)(VI_BASE + 0x28))
#define REG_VI_Y_SCALE      ((volatile uint32_t*)(VI_BASE + 0x34))

// ---------------------------------------------------------------------------
// Preset definition
// ---------------------------------------------------------------------------

typedef struct {
    const char *name;

    long long fvi_num;
    long long fvi_den;

    int s_progressive;
    int s_interlaced;
    int default_s;

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
// MPAL Presets
// ---------------------------------------------------------------------------

static const preset_t preset_mpal_prog __attribute__((unused)) = {
    .name = "MPAL_PROG",

    .fvi_num = 6953850000LL,
    .fvi_den = 143LL,

    .s_progressive = 526,
    .s_interlaced  = 525,
    .default_s     = 526,

    .default_h_total = 3091,
    .default_pat     = 0,
    .default_leap_a  = 3098,
    .default_leap_b  = 3098,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 16,
};

static const preset_t preset_mpal_int __attribute__((unused)) = {
    .name = "MPAL_INT",

    .fvi_num = 6953850000LL,
    .fvi_den = 143LL,

    .s_progressive = 526,
    .s_interlaced  = 525,
    .default_s     = 525,

    .default_h_total = 3091,
    .default_pat     = 0,
    .default_leap_a  = 3096,
    .default_leap_b  = 3096,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 16,
};

// ---------------------------------------------------------------------------
// NTSC Presets
// ---------------------------------------------------------------------------

static const preset_t preset_ntsc_int __attribute__((unused)) = {
    .name = "NTSC_INT",

    .fvi_num = 535500000LL,
    .fvi_den = 11LL,

    .s_progressive = 526,
    .s_interlaced  = 525,
    .default_s     = 525,

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

static const preset_t preset_ntsc_prog __attribute__((unused)) = {
    .name = "NTSC_PROG",

    .fvi_num = 535500000LL,
    .fvi_den = 11LL,

    .s_progressive = 526,
    .s_interlaced  = 525,
    .default_s     = 526,

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

// ---------------------------------------------------------------------------
// PAL96 Presets
// ---------------------------------------------------------------------------

static const preset_t preset_pal_1996_int __attribute__((unused)) = {
    .name = "PAL_1996_INT",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .s_progressive = 626,
    .s_interlaced  = 625,
    .default_s     = 625,

    .default_h_total = 3178,
    .default_pat     = 21,
    .default_leap_a  = 3183,
    .default_leap_b  = 3184,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 20,
};

static const preset_t preset_pal_1996_prog __attribute__((unused)) = {
    .name = "PAL_1996_PROG",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .s_progressive = 626,
    .s_interlaced  = 625,
    .default_s     = 626,

    .default_h_total = 3178,
    .default_pat     = 21,
    .default_leap_a  = 3183,
    .default_leap_b  = 3184,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 20,
};

// ---------------------------------------------------------------------------
// PAL97 Presets
// ---------------------------------------------------------------------------

static const preset_t preset_pal_1997_int __attribute__((unused)) = {
    .name = "PAL_1997_INT",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .s_progressive = 626,
    .s_interlaced  = 625,
    .default_s     = 625,

    .default_h_total = 3178,
    .default_pat     = 23,
    .default_leap_a  = 3182,
    .default_leap_b  = 3184,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 20,
};

static const preset_t preset_pal_1997_prog __attribute__((unused)) = {
    .name = "PAL_1997_PROG",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .s_progressive = 626,
    .s_interlaced  = 625,
    .default_s     = 626,

    .default_h_total = 3178,
    .default_pat     = 23,
    .default_leap_a  = 3182,
    .default_leap_b  = 3184,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 20,
};

// ---------------------------------------------------------------------------
// PAL60 Presets
// ---------------------------------------------------------------------------

static const preset_t preset_pal60_int __attribute__((unused)) = {
    .name = "PAL60_INT",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .s_progressive = 526,
    .s_interlaced  = 525,
    .default_s     = 525,

    .default_h_total = 3156,
    .default_pat     = 0,
    .default_leap_a  = 3156,
    .default_leap_b  = 3156,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 20,
};


static const preset_t preset_pal60_prog __attribute__((unused)) = {
    .name = "PAL60_PROG",

    .fvi_num = 49656530LL,
    .fvi_den = 1LL,

    .s_progressive = 526,
    .s_interlaced  = 525,
    .default_s     = 526,

    .default_h_total = 3156,
    .default_pat     = 0,
    .default_leap_a  = 3156,
    .default_leap_b  = 3156,

    .resolution = RESOLUTION_320x240,
    .fb_width   = 320,
    .fb_height  = 240,

    .safe_x = 20,
    .safe_y = 20,
};


// ---------------------------------------------------------------------------
// Active preset selection
// ---------------------------------------------------------------------------


#if defined(PRESET_MPAL_PROG)
    #define ACTIVE_PRESET preset_mpal_prog
#elif defined(PRESET_MPAL_INT)
    #define ACTIVE_PRESET preset_mpal_int

#elif defined(PRESET_NTSC_INT)
    #define ACTIVE_PRESET preset_ntsc_int
#elif defined(PRESET_NTSC_PROG)
    #define ACTIVE_PRESET preset_ntsc_prog

#elif defined(PRESET_PAL_1996_INT)
    #define ACTIVE_PRESET preset_pal_1996_int
#elif defined(PRESET_PAL_1996_PROG)
    #define ACTIVE_PRESET preset_pal_1996_prog

#elif defined(PRESET_PAL_1997_INT)
    #define ACTIVE_PRESET preset_pal_1997_int
#elif defined(PRESET_PAL_1997_PROG)
    #define ACTIVE_PRESET preset_pal_1997_prog

#elif defined(PRESET_PAL60_INT)
    #define ACTIVE_PRESET preset_pal60_int
#elif defined(PRESET_PAL60_PROG)
    #define ACTIVE_PRESET preset_pal60_prog

#else
    #error "No VI preset defined"
#endif

static const preset_t *preset = &ACTIVE_PRESET;

// ---------------------------------------------------------------------------

static void sanitize_timing(int *h_total, int *pat, int *leap_a, int *leap_b)
{
    if (*h_total < 1)       *h_total = 1;
    if (*leap_a < *h_total) *leap_a  = *h_total;
    if (*leap_b < *h_total) *leap_b  = *h_total;
    if (*pat < 0)           *pat     = 0;
    if (*pat > 31)          *pat     = 31;
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

    // Set or clear SERRATE (bit 6 of VI_CTRL) based on scan type.
    // Interlaced = odd s; progressive = even s.
    uint32_t ctrl = *REG_VI_CTRL;

    if (s % 2 == 1)
        ctrl |=  (1u << 6);
    else
        ctrl &= ~(1u << 6);

    *REG_VI_CTRL = ctrl;
}

// ---------------------------------------------------------------------------

typedef struct {
    double fh;
    double fv;

    int delta_a;
    int delta_b;

    int avg_whole;
    int avg_tenths;
} timing_t;


static timing_t compute_timing(
    const preset_t *p,
    int h_total,
    int pat,
    int leap_a,
    int leap_b,
    int s)
{
    timing_t t = {0};

    // Leap deltas
    t.delta_a = leap_a - h_total;
    t.delta_b = leap_b - h_total;

    // Total extra clocks over the 5-VSYNC leap cycle
    int total_extra = 0;
    for (int i = 0; i < 5; i++)
        total_extra += ((pat >> i) & 1) ? t.delta_b : t.delta_a;

    // Average extra clocks per VSYNC display field (whole.tenths)
    int avg10 = total_extra * 2;
    t.avg_whole  = avg10 / 10;
    t.avg_tenths = avg10 % 10;

    // Leap-adjusted average line length:
    //   L_avg = (5 * L * S + 2 * total_extra) / (5 * S)
    double fvi   = (double)p->fvi_num / (double)p->fvi_den;
    double l_avg = (5.0 * (double)h_total * (double)s + 2.0 * (double)total_extra)
                   / (5.0 * (double)s);

    t.fh = fvi / l_avg;
    t.fv = 2.0 * fvi / ((double)s * l_avg);

    return t;
}

// ---------------------------------------------------------------------------

static void draw_color_bars(surface_t *disp)
{
    static const uint8_t bars[7][3] = {
        {255, 255, 255},  // white
        {255, 255,   0},  // yellow
        {  0, 255, 255},  // cyan
        {  0, 255,   0},  // green
        {255,   0, 255},  // magenta
        {255,   0,   0},  // red
        {  0,   0, 255},  // blue
    };

    int bar_w = preset->fb_width / 7;

    for (int i = 0; i < 7; i++) {
        int x0 = i * bar_w;
        int w  = (i == 6) ? preset->fb_width - x0 : bar_w;
        uint32_t color = graphics_make_color(bars[i][0], bars[i][1], bars[i][2], 255);
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

    timing_t t = compute_timing(preset, h_total, pat, leap_a, leap_b, s);

    uint32_t reg_vt   = *REG_VI_V_TOTAL;
    uint32_t reg_ht   = *REG_VI_H_TOTAL;
    uint32_t reg_leap = *REG_VI_H_TOTAL_LEAP;

    int y = preset->safe_y;

// ---------------------------------------------------------------------------

    graphics_set_color(graphics_make_color(0, 0, 0, 255), 0);

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), "        VI TIMING TEST");
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), "       PRESET: %s", preset->name);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 16;

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), "      H_TOTAL: %d", h_total);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), " LEAP PATTERN: %d (0b%c%c%c%c%c)",
        pat,
        (pat >> 4) & 1 ? '1' : '0',
        (pat >> 3) & 1 ? '1' : '0',
        (pat >> 2) & 1 ? '1' : '0',
        (pat >> 1) & 1 ? '1' : '0',
        (pat >> 0) & 1 ? '1' : '0');
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), "       LEAP_A: %d  deltaA: +%d", leap_a, t.delta_a);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), "       LEAP_B: %d  deltaB: +%d", leap_b, t.delta_b);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), "    avg/VSYNC: %d.%d clk", t.avg_whole, t.avg_tenths);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 16;

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), " REFRESH (fV): %.7f Hz", t.fv);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), "    LINE (fH): %.4f Hz", t.fh);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 16;

// ---------------------------------------------------------------------------

    snprintf(buf, sizeof(buf), "  REG V_TOTAL: 0x%08lX", (unsigned long)reg_vt);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), "  REG H_TOTAL: 0x%08lX", (unsigned long)reg_ht);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 12;

    snprintf(buf, sizeof(buf), "     REG LEAP: 0x%08lX", (unsigned long)reg_leap);
    graphics_draw_text(disp, preset->safe_x + 16, y, buf);
    y += 40;

// ---------------------------------------------------------------------------

    graphics_draw_text(disp, preset->safe_x + 10, y, "DPAD U/D: H_TOTAL  C U/D: LEAP_A");
    y += 12;
    graphics_draw_text(disp, preset->safe_x + 10, y, "DPAD L/R: PATTERN  C L/R: LEAP_B");
}

// ---------------------------------------------------------------------------
// debug output
// ---------------------------------------------------------------------------

static void log_values(int h_total, int pat, int leap_a, int leap_b, int s)
{
    timing_t t = compute_timing(preset, h_total, pat, leap_a, leap_b, s);

    debugf(
        "S=%d(%s) H_TOTAL=%d PAT=%d(0b%c%c%c%c%c) "
        "LEAP_A=%d LEAP_B=%d deltaA=%d deltaB=%d "
        "avg=%d.%d ~fH=%.6f ~fV=%.6f | "
        "VI_V_TOTAL=0x%08lX VI_H_TOTAL=0x%08lX VI_H_TOTAL_LEAP=0x%08lX\n",

        s,
        (s == preset->s_progressive) ? "P" : "I",
        h_total, pat,

        (pat >> 4) & 1 ? '1' : '0',
        (pat >> 3) & 1 ? '1' : '0',
        (pat >> 2) & 1 ? '1' : '0',
        (pat >> 1) & 1 ? '1' : '0',
        (pat >> 0) & 1 ? '1' : '0',

        leap_a, leap_b,
        t.delta_a, t.delta_b,
        t.avg_whole, t.avg_tenths,
        t.fh, t.fv,

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

#if defined(PRESET_PAL_1996) || defined(PRESET_PAL_1997)
    // Reposition active video window for PAL frame timing.
    // libdragon initializes VI_V_VIDEO for NTSC blanking (V_START=0x025).
    // PAL blanking ends later (V_START=0x05F); without this correction the
    // active region is mispositioned and the bottom of the framebuffer
    // produces garbage output.
    // V_START=0x05F (95 half-lines), V_END=0x23F (575 half-lines) = 240 lines.
    // Note: VI_V_VIDEO is latched at frame start; takes effect next frame.
    *REG_VI_V_VIDEO = (0x05FUL << 16) | 0x23FUL;
#endif

    joypad_init();
    debug_init_usblog();

    int h_total = preset->default_h_total;
    int pat     = preset->default_pat;
    int leap_a  = preset->default_leap_a;
    int leap_b  = preset->default_leap_b;
    int s       = preset->default_s;

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
//        if (keys.l)       { s = preset->s_progressive; changed = true; }
//        if (keys.r)       { s = preset->s_interlaced;  changed = true; }

        if (changed) {
            sanitize_timing(&h_total, &pat, &leap_a, &leap_b);
            apply_vi_timing(h_total, pat, leap_a, leap_b, s);
            log_values(h_total, pat, leap_a, leap_b, s);
        }

        surface_t *disp = display_get();

        // Per-field Y_OFFSET correction.
        // Interlaced: offset the odd field by half a scanline (0x200 in 0.10 format)
        // so the two fields composite correctly rather than bobbing.
        // Progressive: restore 1:1 scale with no offset.
        if (s % 2 == 1) {
            int field = *REG_VI_CURRENT & 1;
            *REG_VI_Y_SCALE = (field ? (0x200u << 16) : 0u) | 0x400u;
        } else {
            *REG_VI_Y_SCALE = 0x400u;
        }

        draw_color_bars(disp);
        draw_overlay(disp, h_total, pat, leap_a, leap_b, s);
        display_show(disp);
    }

    return 0;
}
