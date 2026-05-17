#include <stdio.h>
#include <libdragon.h>

// VI register addresses (uncached KSEG1)
#define VI_BASE             0xA4400000
#define VI_H_TOTAL          ((volatile uint32_t*)(VI_BASE + 0x1C))
#define VI_H_TOTAL_LEAP     ((volatile uint32_t*)(VI_BASE + 0x20))

// ---------------------------------------------------------------------------

// MPAL progressive (Math/lidnariq)
//#define DEFAULT_H_TOTAL     3091
//#define DEFAULT_LEAP_PAT    0
//#define DEFAULT_LEAP_A      3091
//#define DEFAULT_LEAP_B      3091

// MPAL progressive (older libdragon)
//#define DEFAULT_H_TOTAL     3090
//#define DEFAULT_LEAP_PAT    4
//#define DEFAULT_LEAP_A      3099
//#define DEFAULT_LEAP_B      3098

// MPAL (libdragon preview)
//#define DEFAULT_H_TOTAL     3089
//#define DEFAULT_LEAP_PAT    0
//#define DEFAULT_LEAP_A      3101
//#define DEFAULT_LEAP_B      3101

// ---------------------------------------------------------------------------

// NTSC
#define DEFAULT_H_TOTAL     3094
#define DEFAULT_LEAP_PAT    0
#define DEFAULT_LEAP_A      3094
#define DEFAULT_LEAP_B      3094

// PAL (libdragon / 1996)
//#define DEFAULT_H_TOTAL     3178
//#define DEFAULT_LEAP_PAT    21
//#define DEFAULT_LEAP_A      3183
//#define DEFAULT_LEAP_B      3184

// PAL (1997)
//#define DEFAULT_H_TOTAL     3178
//#define DEFAULT_LEAP_PAT    23
//#define DEFAULT_LEAP_A      3182
//#define DEFAULT_LEAP_B      3184

// ---------------------------------------------------------------------------
// Write VI timing registers directly to hardware.
// VI_H_TOTAL:      bits 20:16 = 5-bit leap pattern, bits 11:0 = h_total-1
// VI_H_TOTAL_LEAP: bits 27:16 = leap_a-1, bits 11:0 = leap_b-1
// LEAP_A and LEAP_B are clamped to >= h_total (per wiki: values smaller
// than H_TOTAL cause undesired side effects including skipped hsyncs).
// ---------------------------------------------------------------------------
static void apply_vi_timing(int h_total, int pat, int leap_a, int leap_b)
{
    // Clamp
    if (leap_a < h_total) leap_a = h_total;
    if (leap_b < h_total) leap_b = h_total;
    if (pat < 0)  pat = 0;
    if (pat > 31) pat = 31;

    *VI_H_TOTAL      = ((pat & 0x1F) << 16) | ((h_total - 1) & 0xFFF);
    *VI_H_TOTAL_LEAP = (((leap_a - 1) & 0xFFF) << 16) | ((leap_b - 1) & 0xFFF);
}

// ---------------------------------------------------------------------------
// Draw seven vertical color bars filling the screen.
// Standard color bar order: white, yellow, cyan, green, magenta, red, blue.
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
// Draw current parameter values as text overlay.
// ---------------------------------------------------------------------------
static void draw_overlay(surface_t *disp, int h_total, int pat, int leap_a, int leap_b)
{
    char buf[48];

    graphics_set_color(graphics_make_color(0, 0, 0, 255), 0);

    snprintf(buf, sizeof(buf), "     VI TIMING TEST");
    graphics_draw_text(disp, 8, 8, buf);

    snprintf(buf, sizeof(buf), "     H_TOTAL:    %d", h_total);
    graphics_draw_text(disp, 8, 24, buf);

    snprintf(buf, sizeof(buf), "     LEAP PAT:   %d (0b%c%c%c%c%c)",
        pat,
        (pat >> 4) & 1 ? '1' : '0',
        (pat >> 3) & 1 ? '1' : '0',
        (pat >> 2) & 1 ? '1' : '0',
        (pat >> 1) & 1 ? '1' : '0',
        (pat >> 0) & 1 ? '1' : '0');
    graphics_draw_text(disp, 8, 40, buf);

    snprintf(buf, sizeof(buf), "     LEAP_A:     %d", leap_a);
    graphics_draw_text(disp, 8, 56, buf);

    snprintf(buf, sizeof(buf), "     LEAP_B:     %d", leap_b);
    graphics_draw_text(disp, 8, 72, buf);

    graphics_set_color(graphics_make_color(0, 0, 0, 180), 0);
    snprintf(buf, sizeof(buf), "DPAD U/D: H_TOTAL   DPAD L/R: PAT");
    graphics_draw_text(disp, 8, 210, buf);
    snprintf(buf, sizeof(buf), "C U/D: LEAP_A       C L/R: LEAP_B");
    graphics_draw_text(disp, 8, 222, buf);
}

// ---------------------------------------------------------------------------
// Log current values to debug output.
// ---------------------------------------------------------------------------
static void log_values(int h_total, int pat, int leap_a, int leap_b)
{
    debugf("H_TOTAL=%d PAT=%d(0b%c%c%c%c%c) LEAP_A=%d LEAP_B=%d | "
           "VI_H_TOTAL=0x%08lX VI_H_TOTAL_LEAP=0x%08lX\n",
           h_total, pat,
           (pat >> 4) & 1 ? '1' : '0',
           (pat >> 3) & 1 ? '1' : '0',
           (pat >> 2) & 1 ? '1' : '0',
           (pat >> 1) & 1 ? '1' : '0',
           (pat >> 0) & 1 ? '1' : '0',
           leap_a, leap_b,
           *VI_H_TOTAL, *VI_H_TOTAL_LEAP);
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

    // Apply starting values immediately
    apply_vi_timing(h_total, pat, leap_a, leap_b);
    log_values(h_total, pat, leap_a, leap_b);

    while (1) {
        joypad_poll();
        joypad_buttons_t keys = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        bool changed = false;

        // D-pad up/down -> H_TOTAL
        if (keys.d_up)   { h_total++; changed = true; }
        if (keys.d_down) { h_total--; changed = true; }

        // D-pad left/right -> LEAP pattern
        if (keys.d_right) { pat++; changed = true; }
        if (keys.d_left)  { pat--; changed = true; }

        // C-up/C-down -> LEAP_A
        if (keys.c_up)   { leap_a++; changed = true; }
        if (keys.c_down) { leap_a--; changed = true; }

        // C-left/C-right -> LEAP_B
        if (keys.c_right) { leap_b++; changed = true; }
        if (keys.c_left)  { leap_b--; changed = true; }

        // L/R currently unassigned (V_TOTAL removed as too dangerous)

        if (changed) {
            // Clamp before apply
            if (h_total < 1) h_total = 1;
            if (leap_a < h_total) leap_a = h_total;
            if (leap_b < h_total) leap_b = h_total;
            if (pat < 0)  pat = 0;
            if (pat > 31) pat = 31;

            apply_vi_timing(h_total, pat, leap_a, leap_b);
            log_values(h_total, pat, leap_a, leap_b);
        }

        surface_t *disp = display_get();
        draw_color_bars(disp);
        draw_overlay(disp, h_total, pat, leap_a, leap_b);
        display_show(disp);
    }

    return 0;
}
