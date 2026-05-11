# clock_master / clock_master_sram3 — transport vs loop (change log)

Purpose: avoid “fix A, break B” by recording **invariants** each time we touch leader / `setNow` / external sync.

## Invariants (agreed behaviour)

1. **`clock_transport_active`** = `clock_playing || (use_external && isClockIncoming)` — MIDI or internal ticker has taken ownership of advancing time.
2. While **`clock_transport_active`**, **`trig`** (slot commit): **never** derives `leader_tempo` via `scale_tempo_musical` from loop length **and never** nudges `pos` / `sync.syncer_pos` / `prev_clock_pos` — same idea as **`clock_is_master`** (fixed grid; loops only declare **BARS** via `calc_bars`).
3. **`clock_is_master`** = `clock_playing && !use_external` remains the internal-only path; **`clock_transport_active`** extends the same “grid locks tempo” rule when **external** clock is flowing.
4. **`prev_leader –1→slot`** block: same guard — **no** phase snaps or **`outlet_reset`** when **`clock_transport_active`**; only cold / no-clock paths realign phase.
5. **Stopped clock** (`!clock_transport_active`): loop may derive `leader_tempo` + baseline phase exactly as before (first loop from silence, etc.).

## Version map

| Object                | Outlet | Summary |
|----------------------|--------|---------|
| `clock_master.axo`   | **v029** | Unify trig + leader with `clock_transport_active` (fixes loops trading bar phase vs leader_tempo gymnastics). |
| `clock_master_sram3.axo` | **v006** | Same logic as SRAM1; fixed broken v004 shim in wrapper. |

## Debug log cues

- **`H8`** (`post-fix3`): trig took **slave branch** (`trig_slave_no_tempo_rewrite` / `clock_master*_trig_clock_slave`).
- Older **`H5`** / **`H7`**: superseded — do not use for diagnosing current behaviour.

## File locations

- `blindlib/objects/clock/clock_master.axo`
- `blindlib/objects/clock/clock_master_sram3.axo`

## Related: `loopsync dev 14` (Axolonatics)

**2026-05-11 — post-record / sync gap:** `nexttrig` was handled *inside* `if (ro < _ro)`, so a “trigger NOW” request from `slotstart`/`syncstart` or `inlet_resync` could wait until the next phase-wrap (up to ~one `mod` cycle), audibly delaying `wait && synctrig` → gate flip → case-4 `play`. Moved `nexttrig` handling **after** the `ro`-wrap block so it forces `synctrig` same k-tick. Instrumentation: **`H9`** `post-fix4` on that path.

Path: `Axolonatics/objects/rbrt_new/smplr/loopsync dev 14.axo`
