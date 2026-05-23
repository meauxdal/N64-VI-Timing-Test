## VI timing test ROM for N64

[![Build N64 ROM](https://github.com/meauxdal/N64-VI-Timing-Test/actions/workflows/build.yml/badge.svg)](https://github.com/meauxdal/N64-VI-Timing-Test/actions/workflows/build.yml)

![image of the tool, NTSC progressive, version 3 point 0 point 0. rainbow test pattern background with black UI text](NTSC-v3.0.0.png)

this tool allows adjusting VI timing values in real time to observe hardware effects. 

the presets are the currently best known values for **composite** signals for each mode. testing using composite is critical because that is where the fine detail we are looking for is most observable. 

the best settings should see the composite "dot crawl" of the image look as stable as possible, with consistent color and overall image quality (within the normal expectations of N64 hardware, of course!). one tester described optimal MPAL as looking almost as good as S-Video.

please try the ROMs (each ROM corresponds to a best-known preset per region/mode combination, but the values are configurable in real-time to observe changes) and see if you can find anything that looks better on your setup. join the N64brew.dev Discord and ping me - @meauxdal.

notably, the system was designed for NTSC, so those presets are most likely already optimal. testing is welcome nonetheless.

PAL 1996 and PAL 1997 represent two observed presets for European/Australian/etc. PAL signals. they are near-identical (they produce the exact same line frequency and refresh rate), but there may be a visible difference. please test and report which one looks better on your television, or if another set of values looks better still.

---

## warning

**be careful with extreme values as you can exceed the ability of your device to sync to it.**

that said, you *can* use this to observe the handling of various timings with your devices. usb debug output is also available (useful for this purpose in particular). only tested using sc64deployer. debug output is effectively the same as the OSD.

---

## controls

this tool works very similarly to the [VI timing calculator](https://meauxdal.neocities.org/n64-vi-calculator). you can adjust VI registers dynamically and see the results:

- d-pad up & down: increases or decreases VI_H_TOTAL
- d-pad right & left: increases or decreases the leap pattern (0-31)
- c-up & c-down: increases or decreases LEAP_A
- c-right & c-left: increases or decreases LEAP_B

---

## abbreviations

- INT: interlaced
- PROG: progressive
- PAL_1996: original PAL leap configuration
- PAL_1997: revised PAL leap configuration
- PAL60: NTSC lines/timing with PAL color (libdragon implementation)

---

## brief explanation of leap

H_TOTAL represents the number of VI clocks per line. the N64 allows specifying two alternate line lengths to be applied to one line per VSYNC (the ~first line after VSYNC iiuc). the pattern applies either LEAP_A or LEAP_B for bit 0 or 1, respectively. pattern bit 0 is "always use LEAP_A", pattern 31 is "always use LEAP_B", and the bias between them changes as you increase the values. the math equations used for this are shown in my [vi timing calculator](https://meauxdal.neocities.org/n64-vi-calculator).

LEAP_A/B are clamped to >= VI_H_TOTAL as this tool is not intended to explore negative leap deltas. the N64brew.dev wiki Video Interface page describes negative effects related to negative leap deltas; this clamping is in the interest of generating useful presets.

---

## existing MPAL results

initial testing shows promise with the following profiles, thanks to N64brew Discord PAL-M tester AAIC: 

**MPAL_PROG (progressive):**  
H_TOTAL 3091  
Leap pattern: 0  
LEAP_A: 3098  
LEAP_B: 3098 (not used due to pattern 0 = always use LEAP_A)  

In libdragon preview macro convention, that is:  

    H_TOTAL: 772.75  
    Leap pattern: 0b00000  
    LEAP_A: 774.50  
    LEAP_B: 774.50

**MPAL_INT (interlaced):**  
H_TOTAL: 3091  
Leap pattern: 0  
LEAP_A: 3096  
LEAP_B: 3096 (not used due to pattern 0)  

    H_TOTAL: 772.75  
    Leap pattern: 0b00000  
    LEAP_A: 774.00  
    LEAP_B: 774.00  

---

see https://github.com/DragonMinded/libdragon/issues/884.

--- 

built with libdragon (trunk branch as of may 2026)