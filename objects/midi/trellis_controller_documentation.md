# Trellis Controller Documentation

## Overview
`trellis_controller.axo` is a comprehensive MIDI controller handler designed for the Adafruit Trellis 8x4 button grid (32 buttons, notes 36-63). It provides intelligent button mapping with shift functionality, combination detection, and lock-based debouncing to prevent accidental triggers.

## Hardware Layout
**Trellis 8x4 Grid (Notes 36-63)**
```
Row 1: 36  37  38  39  40  41  42  43
Row 2: 44  45  46  47  48  49  50  51
Row 3: 52  53  54  55  56  57  58  59
Row 4: 60  61  62  63  64  65  66  67
```

## Button Mapping

### Row 1 (Notes 36-43) - 4-Layer Loop Control
Each layer has a Rec button (even notes) and Sel button (odd notes):

| Layer | Rec Note | Sel Note | Functions |
|-------|----------|----------|-----------|
| 1 | 36 | 37 | Rec/Stop/Reverse |
| 2 | 38 | 39 | Rec/Stop/Reverse |
| 3 | 40 | 41 | Rec/Stop/Reverse |
| 4 | 42 | 43 | Rec/Stop/Reverse |

**Button Functions per Layer:**
- **Rec alone** → `rec` output (record gate)
- **Rec + Sel** (both held) → `stop` pulse (short hold) / `reset` pulse + clear toggles (long hold)
- **Shift + Rec** → Toggle `alt` output on/off (alternate function)
- **Shift + Sel** → Toggle `reverse` output on/off (reverse playback)
- **Sel alone** (no shift) → Changes `index` to layer number (1-4)

Note: `alt` and `reverse` are toggles - press once to turn on, press again to turn off. Triggering `reset` (long hold Rec+Sel) clears both toggles back to off.

### Row 2 (Notes 44-51) - Reserved
Currently unused, available for future expansion.

### Row 3 (Notes 52-59) - Reserved
Currently unused, available for future expansion.

### Row 4 (Notes 60-67) - Transport & Mode Selection

| Note | Primary Function | Shift Function |
|------|-----------------|----------------|
| 60 | Tap | Nudge- |
| 61 | Play | Nudge+ |
| 62 | **Shift Button** | - |
| 63 | Main (index 0) | System (index 9) |
| 64 | Fx (index 5) | - |
| 65 | Syn (index 6) | - |
| 66 | Inst (index 7) | - |
| 67 | Seq (index 8) | - |

## Outputs

### Gate Outputs (bool32)
Gate outputs are **HIGH when button is held**, **LOW when released**:

**Loop Control (4 layers):**
- `rec1`, `rec2`, `rec3`, `rec4` - Record gates (Rec alone, gate follows button)
- `stop1`, `stop2`, `stop3`, `stop4` - Stop pulses (Rec+Sel short hold)
- `reset1`, `reset2`, `reset3`, `reset4` - Reset pulses (Rec+Sel long hold > resetTime, clears alt/reverse toggles)
- `reverse1`, `reverse2`, `reverse3`, `reverse4` - Reverse toggles (Shift+Sel to toggle, reset clears)
- `alt1`, `alt2`, `alt3`, `alt4` - Alt function toggles (Shift+Rec to toggle, reset clears)

**Transport:**
- `tap` - Tap tempo gate (note 60, no shift)
- `play` - Play gate (note 61, no shift)
- `shift` - Shift button state (note 62)
- `nudgeMinus` - Nudge down gate (note 60 + shift)
- `nudgePlus` - Nudge up gate (note 61 + shift)
- `shiftHeld` - Shift state (same as `shift`)

### Index Outputs

**`index` (int32):**
Current mode/layer index:
- `0` = Main (note 63)
- `1-4` = Sel layers (notes 37, 39, 41, 43)
- `5` = Fx (note 64)
- `6` = Syn (note 65)
- `7` = Inst (note 66)
- `8` = Seq (note 67)
- `9` = System (note 63 + shift)

**`indexTrig` (bool32.pulse):**
Pulse output when index changes.

## Lock-Based Debouncing

The object uses a smart lock system to prevent accidental triggers when releasing button combinations:

### How It Works

**Problem:** When releasing "Shift + Rec", if user releases Shift slightly before Rec, the Rec button could accidentally trigger.

**Solution:** Lock flags that require full button release before retriggering.

### Lock Logic per Layer

```cpp
// Lock when shift or combo is active
if (shift_held || (rec_held && sel_held)) {
    rec_locked = 1;
}

// Unlock ONLY when button is physically released
if (!rec_held) {
    rec_locked = 0;
}

// Rec only triggers if not locked
rec_output = !shift && rec_held && !sel_held && !rec_locked;
```

### Example Sequence

1. **Press Shift + Rec:**
   - `shift_held = true` → `rec_locked = true`
   - `reverse` output = HIGH ✅

2. **Release Shift** (Rec still held):
   - `shift_held = false`
   - `rec_locked` still = true (locked!)
   - `rec` output = LOW (blocked) ✅

3. **Release Rec:**
   - `rec_held = false` → `rec_locked = false`
   - Button is now unlocked for next press

4. **Press Rec alone:**
   - `rec_locked = false` (not locked)
   - `rec` output = HIGH ✅

### Benefits
- ✅ No timing-based debounce needed
- ✅ Works regardless of release order
- ✅ Deterministic behavior
- ✅ No false triggers

## Usage Example

### Basic Looper Control
```
trellis_controller → rec1 → looper layer 1 rec inlet
                  → stop1 → looper layer 1 stop inlet
                  → reverse1 → looper layer 1 reverse inlet
                  → index → layer selector
```

### With Shift Functions
```
Hold Shift (62) + Press Rec (36) → alt1 toggles on/off
Hold Shift (62) + Press Sel (37) → reverse1 toggles on/off
Hold Rec (36) + Press Sel (37) short → stop1 pulse
Hold Rec (36) + Press Sel (37) long → reset1 pulse, alt1 and reverse1 clear to off
Press Sel (37) alone → index = 1, indexTrig pulse
Press Main (63) alone → index = 0
Press Main (63) + Shift → index = 9 (System)
```

## Technical Details

### MIDI Implementation
- Uses efficient `gate[128]` array for all note states
- MIDI handler updates gate array and index values
- K-rate computes all gate logic from array
- Supports Note On/Off and All Notes Off CC

### Memory Usage
- Gate array: 128 bytes
- Lock flags: 4 bytes (4 layers)
- Previous state tracking: ~10 bytes
- Total: ~150 bytes

### Performance
- All logic computed in K-rate
- No S-rate processing
- Minimal CPU overhead
- Scales well with polyphony

## Integration Notes

### MIDI Channel
- Controlled by `attr_midichannel` (standard Axoloti MIDI attribute)
- Set in object properties

### Default State
- Index defaults to 0 (Main mode) on startup
- All gates initialize to LOW
- All locks initialize to unlocked

### Combination Priority
1. **Stop** (Rec+Sel, no shift) - Highest priority
2. **Reverse** (Shift + Rec/Sel) - Second priority
3. **Rec alone** (if not locked) - Normal operation
4. **Index change** (Sel alone, no shift) - Normal operation

## Tips & Best Practices

1. **Shift Functions:** Always release shift LAST to avoid accidental single-button triggers
2. **Stop Function:** Press and hold both Rec and Sel together for reliable stop
3. **Index Selection:** Sel buttons (37,39,41,43) double as index selectors when pressed alone
4. **Visual Feedback:** Use index output to drive LED feedback on which mode is active
5. **Layer Management:** Connect index output to layer routers for automatic layer switching

## Version History

- **v1.2** - Changed `alt` and `reverse` to toggles (press to toggle on/off), reset clears toggles
- **v1.1** - Added `alt1-4` outlets for Shift+Rec specifically (separate from reverse)
- **v1.0** - Initial release with 4-layer control, shift functions, and lock-based debouncing

## Author
Simon / AI Assistant

## License
BSD

---

*Last Updated: October 2025*




