# Sketchy UI System Reference

Master reference for the Ksoloti/Axoloti **sketchy** encoder + OLED stack used in blindlib and looper patches.

Related docs:
- [sketchy_naming_conventions.md](../sketchy_naming_conventions.md) — instance names, type namespaces
- [page_handling_strategy.md](../page_handling_strategy.md) — page switching and param persistence
- [6_shared_objects_strategy.md](../6_shared_objects_strategy.md) — shared dial object pools
- [sdram_array_strategy.md](../sdram_array_strategy.md) — SRAM2/SRAM3/SDRAM placement
- [SDCARD_CONFIG_WORKFLOW.md](../SDCARD_CONFIG_WORKFLOW.md) — font/config SD save/load
- [PRM_TO_SKETCHY_INTEGRATION.md](../PRM_TO_SKETCHY_INTEGRATION.md) — RBRT prm bridge

---

## Architecture

```
┌─────────────────┐     page index      ┌──────────────────┐
│ trellis / MIDI  │ ──────────────────► │ 8encoder_*       │
│ page selector   │                     │ (reads GPIO,     │
└─────────────────┘                     │  writes params)  │
                                        └────────┬─────────┘
                                                 │ read/write
                                                 ▼
┌─────────────────┐   register widgets   ┌──────────────────┐
│ sketchy_dials   │ ───────────────────► │ sketchy_render*  │
│ (config table)  │ ◄── dial_init/update │ (OLED thread)    │
└────────┬────────┘                      └────────┬─────────┘
         │ uses                                   │ draws via
         ▼                                        ▼
┌────────────────────────────────────────────────────────────┐
│ sketchy_objects │ sketchy_texts │ sketchy_params │ font   │
│ (layout table)  │ (strings)     │ (page×8 ints)  │ (glyphs)│
└────────────────────────────────────────────────────────────┘
         ▲
         │ optional read
┌────────┴────────┐        ┌─────────────────────┐
│ int param get   │        │ bool param get      │
│ float param get │        │ pulse uni / bi      │
└─────────────────┘        └─────────────────────┘
```

**Data flow:** encoders update `sketchy_params.array[page][param]`. Dials read that table for display and write back on turn. Other patch logic reads the same cells via param-get objects.

---

## Core table objects

| Object | Role | Typical instance name |
|--------|------|------------------------|
| `sketchy_objects` | Screen element registry (type, x, y, w, h, page, text_id) | `sketchy__objects` |
| `sketchy_texts` | String table for labels and values | `sketchy__texts` |
| `sketchy_params` | `int32_t array[pages][8]` parameter RAM | `sketchy__params` |
| `sketchy_font` | Bitmap font for renderer | `sketchy__font` |
| `sketchy_render*` | OLED draw thread + widget helpers | `sketchy__render` |

Every patch that uses sketchy dials needs **all five** (or embedded equivalents). Param-get objects only need `sketchy_params` present in the patch; they reference `parent->objectinstance_sketchy__params_i` directly — **no `<depends>` block** (same as `int param get`).

---

## Dial configuration (`encoder_dial_config_t`)

Used by `sketchy_dials*`, embedded patchobjs, and `sketchy_dials_library.axo`:

```c
typedef struct {
    uint8_t mode;           // see mode table below
    bool show_value;
    const char* label;
    const char* options;    // list options (mode 3) or "page,dial" (mode 6)
    float default_value;
    int16_t int_min;
    int16_t int_max;
    uint8_t presetIgnore;   // 0 = preset load writes cell; 1 = leave unchanged
    uint8_t domain;         // 0 = param (default), 1 = seq, 2 = global (snapshot system)
} encoder_dial_config_t;
```

`domain` is consumed by `sketchy_params_preset_pulse` v005+ for the snapshot system (see the dedicated section below). Older patches that still use 8-field initializers get `domain = 0` (param) automatically via C99 trailing-zero rules — fully backward compatible.

### Mode values

| Mode | Name | Param storage | Display widget |
|------|------|---------------|----------------|
| 0 | `MODE_FRAC_UNIPOLAR` | frac 0…1 | dial |
| 1 | `MODE_FRAC_BIPOLAR` | frac −1…+1 | dial (bipolar) |
| 2 | `MODE_INT` | integer min…max | intdisplay |
| 3 | `MODE_LIST` | index into options | select |
| 4 | `MODE_BAR_UNIPOLAR` | frac 0…1 | bar |
| 5 | `MODE_BAR_BIPOLAR` | frac −1…+1 | bar |
| 6 | `MODE_DUPLICATE` | mirrors ref cell | same widget as ref |
| 7 | `MODE_BOOL_PULSE_UNI` | **counter** (increments per CW detent) | dial locked at centre |
| 8 | `MODE_BOOL_PULSE_BI` | **signed counter** (+/− per detent) | dial locked at centre |
| 255 | `MODE_EMPTY` | — | no widget |

**Example config line** (embedded `sketchy_dials` init):

```c
configs[0][0] = (encoder_dial_config_t){6, false, "PNS", "5,0", 8.0f, 0, 127, 0};
//                                      ^mode              ^dup ref  ^min ^max ^presetIgnore (domain=0=param implied)

// Seq-domain cell (snapshot-swapped with activeseqsnap selector):
configs[14][0] = (encoder_dial_config_t){2, true,  "STEP", "", 16.0f, 1, 32, 0, 1};
//                                                                            ^dom=1 (seq)

// Global cell (never swapped, never overwritten by a load):
configs[15][0] = (encoder_dial_config_t){2, true,  "PSNP", "", 0.0f, 0, 3, 1, 2};
//                                                                       ^ign  ^dom=2 (global)
```

---

## Pulse encoder modes (7 / 8)

Pulse modes turn a dial into a **step counter** stored in `sketchy_params`, while the OLED shows a fixed centre position (no visible value change).

- **Mode 7 (uni):** each clockwise detent increments the cell; CCW does nothing (or holds).
- **Mode 8 (bi):** CW increments, CCW decrements the cell.

The renderer needs `pulseVisualMode` on `dial_t` (set by dials code). Use the **`*_pulse` renderer/encoder/dials siblings** (see below) or extend an embedded `sketchy_dials` patchobj with the same mode-7/8 handling as `sketchy_dials_library.axo`.

### Reading pulse events in the patch

Use param-get objects that **diff** the counter against the previous k-rate sample:

| Object | Path | Outlets | When it fires |
|--------|------|---------|---------------|
| `bool param get pulse uni` | `encoder/` | `pulse` | one k-period pulse when cell **increases** |
| `bool param get pulse bi` | `encoder/` | `plus`, `minus` | pulse on increase / decrease |

Attributes: `page` (−127…127), `param` (0…7). Wire `pulse` → `logic/pulse` or any `bool32` inlet.

**Pattern** (same as `int param get` — no depends, no threads):

```c
param_table = &parent->objectinstance_sketchy__params_i;
v = param_table->array[attr_page][attr_param];
outlet_pulse = (v > prevValue);
prevValue = v;
```

Version outlet: `v002` (= 2).

---

## `*_pulse` sibling objects (ABI-safe upgrades)

**Rule:** Do not change behaviour/layout of objects already used in shipping patches. Add a **sibling** `.axo` with a new `id`, new UUID, and `v001` (or higher) version outlet.

Pulse toolchain siblings (UUIDs `953D6`…`953EA` in blindlib `.cursorrules`):

| Family | Legacy | Pulse sibling |
|--------|--------|---------------|
| Dials | `sketchy_dials`, `_exp`, `_simple`, … | `sketchy_dials_pulse`, … |
| Renderer | `sketchy_render_kso_fw_sram3` | `sketchy_render_kso_fw_sram3_pulse` |
| Encoder | `8encoder_integrated_kso` | `8encoder_integrated_kso_pulse` |
| Preset | `sketchy_params_preset` | `sketchy_params_preset_pulse` |
| MIDI send | `sketchy_params_midi_send_fixed` | `…_fixed_pulse` |

New patches that need pulse modes wire the `*_pulse` types. Old patches keep legacy objects unchanged.

**Version outlets:** every new object exposes `v001`, `v002`, …; value matches suffix. Display in patch to confirm loaded revision.

---

## Embedded dials vs library object

| Approach | Where config lives | Best for |
|----------|-------------------|----------|
| **Embedded patchobj** | `code.init` inside `.axp` | Looper patches with custom per-page tables (e.g. `4 track dev_sim 1_1_6`) |
| **`sketchy_dials_library.axo`** | editable in object attribute / template | New patches, reuse without embedding |
| **`sketchy_dials_hardcoded.axo`** | fixed table in `.axo` | Demos, single-purpose builds |

When embedding, copy mode-7/8 support from `sketchy_dials_library.axo` (`init_dial`, `update_dial`, `init_param_table`, `displayToEncoderValue`).

---

## Page names and pager display

**`pager_fixed`** (embedded or object) shows current page name + `[current/total]`.

- **`list` attribute:** comma-separated page names. Count **must match** usable pages.
- **`num_pagenames`** = token count of `list` at init.
- **`firstpage`:** offset between hardware page index and first list entry.

If the dial config defines 16 pages but `list` only has 10 names, pages 10+ show blank titles and `[N/10]` caps early. Extend `list` and add explicit `configs[10][0]`…`configs[15][7]` lines (do not rely on copy loops unless intentional).

Example (16 pages):

```
Main,Layer 1,Layer 2,Layer 3,Layer 4,Syn,Fx,DubS,Seq,Inst,Sdchn,X,Y,Preset,Free1,Free2
```

---

## Presets and `presetIgnore`

`sketchy_params_preset_pulse` saves/loads the param table from SD. File naming depends on object version:

- **v004 and earlier:** `preset0.bin`, `preset1.bin`, …  Flat `int32 array[NUM_PAGES][NUM_PARAMS]`.
- **v005 and later (snapshot system):** `snap0.bin`, `snap1.bin`, …  Flat `int32 snapshot_storage[4][NUM_PAGES][NUM_PARAMS]`.

Old `presetN.bin` files survive on disk for rollback; new patches simply ignore them.

`presetIgnore` keeps its meaning across versions:

- **`presetIgnore = 0`** (default): cell is overwritten on preset load.
- **`presetIgnore = 1`**: cell keeps live RAM value across load (use for page index, MIDI-mapped globals, etc.).

---

## Snapshot system (`sketchy_params_preset_pulse` v005+)

Every preset file holds **4 parallel snapshots** of param-domain cells (A/B/C/D) and **4 parallel snapshots** of seq-domain cells (A/B/C/D). Two selector cells in `sketchy_params` (default `[15][0]` = `activeparamsnap`, `[15][1]` = `activeseqsnap`) live-swap snapshots in memory with zero file I/O — effectively **16 micro-configurations per preset file** (4×4 combos).

### Per-cell domain

The `domain` field on `encoder_dial_config_t` partitions every cell into one of three classes:

| Value | Name | Behavior |
|-------|------|----------|
| `0` | `param` (default) | Snapshot-swapped when `activeparamsnap` changes. Saved as the param slice. Targeted by `loadparamsX` inlets. |
| `1` | `seq` | Snapshot-swapped when `activeseqsnap` changes. Saved as the seq slice. Targeted by `loadseqX` inlets. |
| `2` | `global` | Never swapped. Never overwritten by per-snapshot loads. Holds performance state (preset SEL, save/load triggers, the snapshot selectors themselves). |

`presetIgnore = 1` is **orthogonal**: such a cell is left alone on any load regardless of its domain.

### Preset object inlets (v005)

| Inlet | Type | Purpose |
|-------|------|---------|
| `print` | bool32 | Dump live cells (with domain + ignore flags) to the log |
| `index` | int32 | Preset slot (selects `snapN.bin`) |
| `save` | rising | Sync live → active snapshot slot (both domains), then write full bank |
| `loadall` | rising | Read file, populate all 4 snapshot slots; refresh live from active param + seq slots |
| `loadparamsA..D` | rising | Read file, copy snapshot `X` param-domain cells into the in-memory snapshot bank; if `X` is currently active, also refresh live |
| `loadseqA..D` | rising | Same as above but for seq-domain cells |
| `loaded` | pulse (outlet) | **v009:** one k-rate pulse when a load job finishes and live array is updated. Wire to MIDI redraw, seq counter reset, launchpad redraw — **not** the Load button pulse (load is async). |

### Patch wiring after load (reference: `4 track dev_sim 1_2_1 try snapshots.axp`)

Preset load runs on a background thread. Triggering `sketchy_params_midi_send_fixed_pulse` `redraw` from the **Load button pulse** fires before params are applied — the second board keeps old CC values.

**Correct pattern:**

```
Load button pulse  →  sketchy_params.loadall   (only)
sketchy_params.loaded  →  kdelay → midi_send.redraw
                       →  counter.r            (reset seq step / phase)
                       →  seq_launch.redraw    (refresh trellis LEDs)
                       →  seq_euclidean.resync (v002: latch steps/hits/offset, skip regen)
```

Optional: keep a separate periodic redraw (e.g. `pulse_1` → `or` → `kdelay` → `redraw`) for startup; OR in the `loaded` path for post-preset refresh.

### Object revisions (preset / MIDI / seq)

| Object | Version | Notes |
|--------|---------|-------|
| `sketchy_params_preset_pulse` | v009 | `loaded` outlet; worker in `.sram3.text.ppp*` (v008) |
| `sketchy_params_midi_send_fixed_pulse` | v002 | Redraw: rising-edge start, run to completion (no abort on falling edge) |
| `seq_euclidean` | v002 | `resync` rising inlet — call after preset load to avoid immediate pattern regen |

### Memory placement (v008+: SDRAM data, SRAM3 code + maps + stack)

v005–v007 moved **data** (maps, stack) toward SRAM3. That helps CCM but **does not** shrink SRAM1 code pressure. The snapshot worker (`ThreadPreset2`, ~2.6 KiB) lives in default `.text` and was the main SRAM1 overflow source on dense patches.

**v008** places preset worker functions in **SRAM3 code** (`.sram3.text.ppp*`) using the same macro pattern as `brain_lpr_v3_sram3.axo` (`PRESET_PULSE_SRAM3_FN(n)` — unique prefix per `.axo`).

**Data placement (unchanged from v005):**

- **SDRAM** — live `array`, `snapshot_storage[4]`, `file_buf` (load scratch)
- **SRAM3 data** — `_ignore_map`, `_domain_map`, thread stack (`preset_spawn_thread` static local; section attr on class members is rejected by C++ codegen)
- **SRAM1** — scalars + pointers only in `code.declaration`

Thread stack pattern (v007+): pointer-free spawn helper with function-local `static stkalign_t _waPreset[…] __attribute__((section(".sram3")))` — not `WORKING_AREA` in `code.declaration` (C++ forbids section attrs on class data members).

Footprint per object instance, 16×8 patch:

| Region | Bytes | Items |
|--------|------:|-------|
| SDRAM  | 512   | live `array` |
| SDRAM  | 2048  | `snapshot_storage[4][16][8]` |
| SDRAM  | 2048  | `file_buf[4][16][8]` |
| **SDRAM total** | **4608** | |
| SRAM3 data | ~1.3 KiB | maps + thread stack |
| SRAM3 text | ~2.6 KiB+ | `ThreadPreset2`, helpers (v008) |
| SRAM1  | ~64   | pointers + scalars in `code.declaration` |

External readers access cells via `parent->objectinstance_sketchy__params_i.array[p][q]` regardless of backing region.

### File format (`snapN.bin`)

```
int32_t snapshot_storage[4][NUM_PAGES][NUM_PARAMS]   // flat little-endian
```

For a 16×8 patch: 4 × 16 × 8 × 4 = **2048 bytes per file**.

**Size-tolerant load:** load functions read up to `min(file_size, sizeof(snapshot_storage))` bytes. If you grow `pages` later, old `snapN.bin` files still load (head wins; tail stays at defaults). If you shrink `pages`, larger files load only the portion that fits. No format-version bump needed when adding pages.

### Manual-edit-wins (`seq_p/` sequencer family)

Sequencer pattern + lock bits live in 4 seq-domain cells per "seq mirror page" (default page 14):

| Cell | Holds |
|------|-------|
| `[seqpage][4]` | `PatternLo16` — bits for steps 0…15 |
| `[seqpage][5]` | `PatternHi16` — bits for steps 16…31 |
| `[seqpage][6]` | `LockLo16` — manual-edit lock bits 0…15 |
| `[seqpage][7]` | `LockHi16` — manual-edit lock bits 16…31 |

- `seq_p/seq_table_toggle_shift32` toggles a pattern bit on MIDI input **and** sets the corresponding lock bit.
- `seq_p/seq_euclidean` regenerates a pattern when steps/hits/offset change, with `newpat = (pat & lock) | (gen & ~lock)` — locked steps survive. **v002:** `resync` rising inlet latches current steps/hits/offset without regenerating (use after `loaded`).
- `seq_p/seq_data` exposes a `clearlocks` rising-edge inlet that zeros all lock bits **and** does a fresh Euclidean regeneration so you get a clean slate.

Because pattern + lock cells are `domain = seq`, switching `activeseqsnap` A→B swaps both rhythm and manual-edit history together.

### Growing pages later

The preset object is fully parametric. To grow from e.g. 16 to 24 pages, **no preset-object code changes** are needed:

1. Bump `pages` attribute on the `sketchy_params_preset_pulse` instance.
2. Extend the embedded `sketchy_dials` patchobj: bump `maxpages`, extend `_configs[N][8]` decl, add `configs[p][q] = …` lines with `domain` tags for the new pages.
3. Bump `maxpages` on `8encoder_integrated_kso_pulse` and `sketchy_params_midi_send_fixed_pulse`.
4. Append page names to `pager_fixed`'s `list` attribute.

Old `snapN.bin` files keep loading thanks to the size-tolerant read; new pages stay at defaults.

---

## Param get / write objects

| Object | Purpose |
|--------|---------|
| `int param get` / `get_param_int_kso` | read one cell as int |
| `param get` | read one cell as frac |
| `bool param get pulse uni/bi` | edge-detect pulse counters |
| `param_write_int` / `param_write_frac` | write on trigger |
| `int param get half page kso` | read 4 cells (half page) |

All use `parent->objectinstance_sketchy__params_i` — **patch must contain a `sketchy_params` instance** named conventionally so codegen emits `objectinstance_sketchy__params_i`.

---

## Ksoloti build notes (fuse-2)

### Patch compile order / object placement

The patcher generates one `.cpp` from all objects; **generation order can depend on canvas position** (historically related to `y` coordinate). If compile fails with errors that do not match the object you added (e.g. `ThreadX` / `chThdCreateStatic` in the renderer while adding a simple param-get):

1. **Move the new object lower on the canvas** (larger `y` in `.axp`) and regenerate.
2. Or temporarily remove other recently added objects to isolate the conflict.
3. Regenerate code before compile (don't compile stale `.cpp`).

This is a patcher/codegen quirk, not a runtime behaviour issue.

### Thread functions on fuse-2 / ChibiOS

ChibiOS `tfunc_t` is `void (*)(void*)`. Objects with display threads must cast where needed:

```c
Thd = chThdCreateStatic(waThreadX, sizeof(waThreadX), NORMALPRIO-1, (tfunc_t)ThreadX, (void *)this);
```

`sketchy_params_preset_pulse` already uses `(tfunc_t)ThreadPreset`. Legacy render objects may still use bare `ThreadX`; if compile errors persist after placement workaround, add the cast in the `*_pulse` renderer sibling.

### New param-get objects

Follow **`int param get`** exactly:

- No `<depends>` section
- `code.declaration`: `rootc::objectinstance_sketchy__params* param_table;`
- `code.init`: `param_table = &parent->objectinstance_sketchy__params_i;`
- No threads, no SD RAM arrays in the getter itself

---

## Looper patch checklist (pulse + presets)

For patches like `4 track dev_sim 1_1_6 try presets.axp`:

1. Wire `*_pulse` objects: `sketchy_render_kso_fw_sram3_pulse`, `8encoder_integrated_kso_pulse`, `sketchy_params_preset_pulse`, `sketchy_params_midi_send_fixed_pulse`.
2. Extend embedded `sketchy_dials` with modes 7/8 if not using library dials.
3. Set `pager_fixed` `list` to cover all configured pages.
4. Set `maxpages` on encoder + embedded dials consistently (e.g. 16).
5. Add `bool param get pulse uni/bi` for any counter cell that should trigger patch logic.
6. Confirm revision outlets (`v001`/`v002`) on display if debugging object load.

---

## Object index (by role)

**Tables:** `sketchy_objects`, `sketchy_texts`, `sketchy_params`, `sketchy_font`

**Render:** `sketchy_render`, `sketchy_render_exp`, `sketchy_render_kso_fw_sram3`, `sketchy_render_kso_fw_sram3_pulse`, …

**Dials:** `sketchy_dials`, `sketchy_dials_exp`, `sketchy_dials_library`, `sketchy_dials_*_pulse`, embedded patchobj

**Input:** `8encoder_integrated_exp`, `8encoder_integrated_kso_pulse`, …

**Preset/MIDI:** `sketchy_params_preset_pulse` (v009 snapshot + `loaded` outlet), `sketchy_params_midi_send_fixed_pulse` (v002 redraw fix), …

**Reference patch:** `Axolonatics/Simons Patches/rbrt looper/4 track dev_sim 1_2_1 try snapshots.axp` — snapshot system + correct post-load wiring.

**Getters:** `int param get`, `get_param_int_kso`, `bool param get pulse uni`, `bool param get pulse bi`

**Snapshot-aware sequencer (`seq_p/`):** `seq_data`, `seq_euclidean`, `seq_launch_shift32`, `seq_table_toggle_shift32`, `seq_step_length`. Drop-in net replacements for the legacy `seq/...` objects that stored pattern data in an external table; the new ones store pattern + manual-edit lock bits in seq-domain cells of `sketchy_params`, so the data is part of the seq snapshot bank.

UUID registry: `/Users/simon/Dropbox/blindlib/.cursorrules`
