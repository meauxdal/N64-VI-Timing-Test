## VI timing test ROM for N64

[![Build N64 ROM](https://github.com/meauxdal/N64-VI-Timing-Test/actions/workflows/build.yml/badge.svg)](https://github.com/meauxdal/N64-VI-Timing-Test/actions/workflows/build.yml)

this tool allows adjusting VI timing values in real time to observe hardware effects. in particular, the initial goal is to be able to quickly iterate appropriate presets for MPAL

see https://github.com/DragonMinded/libdragon/issues/884 for more details.

---

#### disclaimer 1: this was coded with some degree of LLM assistance. in particular, i got claude and chatgpt to build off my prior test ROMs (for audio interface and leap testing) to iterate something that could minimally build, boot, draw some graphics + debug OSD, and enable debug output. i mostly iterated on it myself once the initial functionality appeared to be working. 

---

it works on hardware (and ares) and does appear to change the values in question. i tested it via summercart 64 + NTSC on my own CRT. i started lowering the H_TOTAL register, and once the fV got low enough, the signal started visually degrading (manifesting as a "bend" at the top of the image). it recovered as i raised the value back up. register profiling indicates the appropriate values are being modified (printed onscreen and via debug).  my currently connected N64 is RGB modded, so i didn't really see any difference at all until it started to reach the boundaries of my CRT's capability to sync to it.


---

this tool works very similarly to my [vi timing calculator](https://meauxdal.neocities.org/n64-vi-calculator). you can adjust VI registers dynamically and see the results:

- d-pad up & down: increases or decreases VI_H_TOTAL
- d-pad right & left: increases or decreases the leap pattern (0-31)
- c-up & c-down: increases or decreases LEAP_A
- c-right & c-left: increases or decreases LEAP_B

LEAP_A/B are clamped to >= VI_H_TOTAL as this tool is not intended to explore negative leap deltas. this can easily be changed if desired.

---

#### disclaimer 2: be mindful that i've not implemented any further guardrails. i'm not responsible if you manage to damage your tv or whatever. i don't think there's significant likelihood of actual harm, but i won't make any such guarantee.

---

L/R switch between the number of half-lines to nominal progressive/interlaced values. the intent is to allow for interlaced testing, but more work probably needs to be done here. it _does_ produce interlaced timing values; however, _for now it should be considered untested._ these values are clamped to 526/525 for NTSC/MPAL, 626/625 for PAL.

- L/R buttons: L = even/prog (default), R = odd/interlaced. 

this is not true interlaced. SERRATE is not set, etc. 

---

each ROM has different default values:

mpal
- **mpal_math** - lidnariq's calculated mpal profile
- **mpal_old** - the old mpal progressive-only profile - no longer exists in libdragon preview
- **mpal_preview** - the old mpal interlaced-only profile. applies to both interlaced and progressive in libdragon preview

ntsc
- **ntsc**

pal
- **pal_1996** - original pal profile (libdragon)
- **pal_1997** - "corrected" pal profile in libultra (not used in libdragon)

---

#### disclaimer 3: i don't have a PAL nor MPAL machine. the PAL ROM appears to work as expected in ares, but the bottom of the screen flickers due to how i'm setting the screen size i guess. i don't *think* this matters for our purposes here but worth noting. if it ends up mattering i doubt it'd be hard to fix

---

other VI registers are not (intentionally) touched. SERRATE, BURST, etc. i only attempted to a) adjust the relevant registers and b) observe they were having the intended effect.

---

using libdragon trunk, in case that makes a difference (i don't think it does for this)


