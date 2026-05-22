hardware bug on mpal

- early MPAL motherboards (NUS-CPU(M)-02) 480i color bug: top portion of screen is wrong color
- reports indicate 100% repro on affected hardware w/ libultra 480i software, including Mystical Ninja (97), World Driver Championship (2000), and Majora's Mask (2000); strong implication this was not ever fixed in software
- possibly fixed in hardware by MAV-NUS or equivalent later revision (unconfirmed)
- AAIC's board revision unknown

libdragon workaround

- `__display_callback` in `display.c` writes per-field V_BURST values every vblank for MPAL interlaced:
  - field 0: `0x000b0202`
  - field 1: `0x000e0204`
- Gated on `get_tv_type() == TV_MPAL && interlaced`

early test -> later test

- Progressive → interlaced → progressive round-trip triggers the color bug in progressive, which never previously occurred
- Root cause: `apply_vi_timing` doesn't write V_BURST; libdragon's callback writes interlaced V_BURST values every field while interlaced, then stops when you return to progressive, leaving the wrong value in the register
- `display_init` sets the correct progressive V_BURST via `vi_write_config`, but nothing restores it after the round-trip

a button test

- Manual V_BURST reset via button press did not fix the color bug
- Did destabilize interlaced (field shake), consistent with writing V_BURST unsynchronized to vblank
- Color bug persisting suggests V_BURST alone may not be the root cause of the original hardware bug, or the values used were wrong

questions

- What does `vi_config_presets` write for MPAL progressive V_BURST? (needs vi.c)
- What does libultra write to V_BURST for MPAL, if anything?
- Exact board revision of AAIC's unit
- Whether MAV-NUS is confirmed as the fix point

---

- Write correct progressive MPAL V_BURST in `apply_vi_timing` when `s` is even, rather than relying on register retaining `display_init` value — requires confirming the correct value from `vi_config_presets`