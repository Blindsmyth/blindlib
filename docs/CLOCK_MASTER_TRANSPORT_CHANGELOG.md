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
| `clock_master.axo`   | **v030** | Transport-active semantics; **instrumentation removed** (2026-05-11). |
| `clock_master_sram3.axo` | **v007** | Same as SRAM1; **instrumentation removed** (2026-05-11). |

## Runtime verification (2026-05-11)

Pre-strip logs showed:

- **`loopsync dev 14`**: **`H9`** `nexttrig_immediate` (`post-fix4`) on same k-tick as `nexttrig` — confirms immediate `synctrig` path.
- **`loopsync` case4**: **`H1`** `play=1 via gate&!gon` after record with expected `pos`/`lim`.
- **`recordlogic_refactored_mute`**: **`H4`** on `recfeedback` fall.

Temporary `LogTextMessage` blocks have been **removed** from `clock_master*` (previously `H2`/`H8`), `loopsync dev 14` (`H1`/`H2`/`H9`), and `recordlogic_refactored_mute` (`H1`/`H4`).

## File locations

- `blindlib/objects/clock/clock_master.axo`
- `blindlib/objects/clock/clock_master_sram3.axo`

## Related: `loopsync dev 14` (Axolonatics)

**2026-05-11 — post-record / sync gap:** `nexttrig` was handled *inside* `if (ro < _ro)`, so a “trigger NOW” request from `slotstart`/`syncstart` or `inlet_resync` could wait until the next phase-wrap (up to ~one `mod` cycle), audibly delaying `wait && synctrig` → gate flip → case-4 `play`. **Fix:** handle `nexttrig` **after** the `ro`-wrap block so it forces `synctrig` on the same k-tick.

Path: `Axolonatics/objects/rbrt_new/smplr/loopsync dev 14.axo`
