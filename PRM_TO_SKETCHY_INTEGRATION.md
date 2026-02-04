# RBRT PRM System to Sketchy Parameter System Integration

## Overview

This document explains how the RBRT prm (parameter) system works and how to bridge it with the Sketchy parameter system.

---

## RBRT PRM System Architecture

### Memory Structure

The RBRT system stores parameters in a **shared SDRAM array**:

```cpp
static int32_t *prm;  // Parameter array
```

**Layout:**
- `prm[0-127]` = Slot 0 parameters
- `prm[128-255]` = Slot 1 parameters
- `prm[256-383]` = Slot 2 parameters
- etc...

**Access Pattern:**
```cpp
uint16_t adr = slot * 128;
int32_t value = smplr.prm[adr + parameter_offset];
```

### Parameter Offsets (per slot)

| Offset | Parameter | Description |
|--------|-----------|-------------|
| 0 | START | Sample start position |
| 1 | END | Sample end position |
| 2 | STARTPOINT | Playback startpoint |
| 3 | PLAYMODE | Playback mode |
| 4 | SMOD | Start modulation |
| 5 | LMOD | Length modulation |
| 6 | DIRECTION | Play direction |
| 7 | PBASE | Base pitch |
| 8 | VOL | Volume |
| 9 | PMOD | Pitch modulation |
| 10 | VOLMOD | Volume modulation |
| 11 | ATTACK | Attack time |
| 12 | RELEASE | Release time |
| 13 | BARS | Duration in bars |
| 14 | RAW | Raw/unsynced flag |
| 15 | SPMOD | Startpoint modulation |
| ... | ... | Custom parameters |
| 125 | RO | Rollover count |
| 126 | POS | Current position |
| 127 | PLAY | Playing flag |

### RBRT PRM Objects

**Reading Parameters:**
- `prm f.axo` - Read float parameter
- `prm i.axo` - Read integer parameter
- `prm f attr.axo` - Read with parameter index attribute
- `prm i attr.axo` - Read int with attribute

**Writing Parameters:**
- `prm GUI.axo` - GUI-based parameter editor with mass-mod
- `prm labeled.axo` - Named parameter access

**Key Feature: Mass Modulation**
```cpp
void mod(uint8_t param, int32_t value, bool massmod) {
    if (!massmod) {
        smplr.prm[adr+param] = value;  // Single slot
    } else {
        for (i = 0; i < smplr.slots; i++) {
            smplr.prm[(i << 7) + param] = value;  // All slots
        }
    }
}
```

---

## Sketchy Parameter System Architecture

### Memory Structure

The Sketchy system uses a **page-based parameter table**:

```cpp
#define MAX_PAGES 32
#define MAX_PARAMS_PER_PAGE 8
static int32_t param_array[MAX_PAGES][MAX_PARAMS_PER_PAGE];
```

**Layout:**
- **Pages**: 32 pages (can represent different contexts/scenes)
- **Parameters per page**: 8 (one per encoder)
- **Total parameters**: 256 (32 × 8)

**Access Pattern:**
```cpp
param_table->array[page][param] = value;
```

### Sketchy Components

1. **sketchy_params.axo** - Central parameter storage table
2. **sketchy_dials.axo** - Configuration for display and encoder modes
3. **8encoder_input.axo** - Reads encoder hardware
4. **sketchy_render.axo** - OLED display renderer
5. **param_write_int.axo** - External parameter writer

### Parameter Modes

The Sketchy system supports multiple parameter types:
- `MODE_FRAC_UNIPOLAR` (0) - 0.0 to 1.0
- `MODE_FRAC_BIPOLAR` (1) - -1.0 to 1.0
- `MODE_INT` (2) - Integer range
- `MODE_LIST` (3) - Selector/enum
- `MODE_BAR_UNIPOLAR` (4) - Bar display 0-1
- `MODE_BAR_BIPOLAR` (5) - Bar display -1 to 1
- `MODE_DUPLICATE` (6) - Mirror another parameter

---

## Key Differences

| Aspect | RBRT PRM | Sketchy |
|--------|----------|---------|
| **Organization** | Slot-based (128 params × N slots) | Page-based (8 params × 32 pages) |
| **Addressing** | `[slot * 128 + param]` | `[page][param]` |
| **Use Case** | Per-sample parameters | Global/scene parameters |
| **GUI** | Simple on-screen knobs | OLED + encoders |
| **Scope** | Sample-specific | Patch-wide |
| **Mass Mod** | Built-in (all slots at once) | Not built-in |
| **Value Format** | Mostly 27-bit fractional | Mixed (int/frac/lists) |

---

## Integration Strategies

### Strategy 1: Direct Mapping (Slot → Page)

Map each RBRT slot to a Sketchy page:
- **Slot 0** → Page 0
- **Slot 1** → Page 1
- **Slot 2** → Page 2
- etc.

**Pros:**
- 1:1 mapping
- Easy to understand
- Each slot gets full 8-parameter control

**Cons:**
- Limited to 32 slots (sketchy page limit)
- Wastes parameters if slots use fewer than 8

### Strategy 2: Parameter Bank Mapping

Use Sketchy pages as "parameter banks" for a single selected slot:
- **Page 0** → Slot parameters 0-7 (playback)
- **Page 1** → Slot parameters 8-15 (envelope)
- **Page 2** → Slot parameters 16-23 (custom)
- **Selected slot** → Inlet/parameter

**Pros:**
- Access all 128 parameters per slot
- One UI for any slot
- Efficient parameter use

**Cons:**
- More complex addressing
- Need slot selection mechanism

### Strategy 3: Hybrid Approach

Combine both strategies:
- **Pages 0-7**: Direct control of slots 0-7 (8 key params each)
- **Pages 8-31**: Parameter banks for selected slot

**Pros:**
- Quick access to multiple slots
- Deep dive into single slot when needed
- Flexible

**Cons:**
- Most complex to implement
- Need clear UI conventions

---

## Implementation: Bridge Objects

### Object 1: prm_to_sketchy (Read RBRT → Write Sketchy)

Maps RBRT parameters to Sketchy parameter table.

```xml
<inlets>
  <int32 name="slot"/>           <!-- Source slot -->
  <int32 name="page"/>           <!-- Target page -->
  <int32 name="param_start"/>    <!-- RBRT parameter offset (0-127) -->
  <bool32 name="update"/>        <!-- Trigger update -->
</inlets>
```

**Function:**
- Reads 8 consecutive RBRT parameters starting at `param_start`
- Writes them to Sketchy page `page`, params 0-7
- Updates on trigger

### Object 2: sketchy_to_prm (Read Sketchy → Write RBRT)

Maps Sketchy parameters to RBRT prm array.

```xml
<inlets>
  <int32 name="page"/>           <!-- Source page -->
  <int32 name="slot"/>           <!-- Target slot -->
  <int32 name="param_start"/>    <!-- RBRT parameter offset -->
  <bool32 name="massmod"/>       <!-- Apply to all slots -->
</inlets>
```

**Function:**
- Reads 8 Sketchy parameters from page
- Writes them to RBRT slot parameters starting at `param_start`
- Supports mass-mod (write to all slots)

### Object 3: prm_sketchy_sync (Bidirectional)

Two-way sync between RBRT and Sketchy.

```xml
<inlets>
  <int32 name="slot"/>
  <int32 name="page"/>
  <bool32 name="sketchy_to_prm"/>  <!-- Direction control -->
  <bool32 name="prm_to_sketchy"/>
</inlets>
```

**Function:**
- Monitors both systems
- Syncs changes bidirectionally
- Handles conflicts (last-write-wins)

---

## Example Use Cases

### Use Case 1: Multi-Slot Looper with Sketchy Control

```
Workflow:
1. Use recordlogic for 3 slots (0, 1, 2)
2. Map each slot to Sketchy pages 0, 1, 2
3. Control key parameters per slot:
   - Encoder 0: Volume (RBRT param 8)
   - Encoder 1: Pitch (RBRT param 7)
   - Encoder 2: Length Mod (RBRT param 5)
   - Encoder 3: Feedback (custom param)
   - Encoder 4: Pan (custom param)
   - Encoder 5: Attack (RBRT param 11)
   - Encoder 6: Release (RBRT param 12)
   - Encoder 7: Playmode (RBRT param 3)
```

**Objects needed:**
- `sketchy_to_prm` (one per slot or one shared)
- Page selector tied to active slot from recordlogic

### Use Case 2: Deep Parameter Editor

```
Workflow:
1. Select a slot with button/encoder
2. Navigate parameter banks (pages)
3. Edit any of 128 parameters
4. Save to preset
```

**Bank organization:**
- Page 0: Playback (START, END, STARTPOINT, PLAYMODE, etc.)
- Page 1: Modulation (SMOD, LMOD, DIRECTION, PMOD, etc.)
- Page 2: Dynamics (VOL, VOLMOD, ATTACK, RELEASE, etc.)
- Page 3: Sync (BARS, RAW, SPMOD, etc.)
- Pages 4-15: Custom parameters

### Use Case 3: Performance Mixer

```
Workflow:
1. Sketchy page 0 = Mixer for slots 0-7
2. Each encoder controls volume for one slot
3. Use prm_sketchy_sync for live updates
```

**Implementation:**
- 8 encoders → 8 slot volumes
- Direct RBRT param 8 (VOL) access
- Real-time display of levels

---

## Value Conversion

RBRT uses 27-bit fractional values (0x08000000 = 1.0).
Sketchy uses various formats.

### Conversion Functions:

**RBRT → Sketchy (0-64 unipolar):**
```cpp
int32_t rbrt_to_sketchy_uni(int32_t rbrt_value) {
    return (rbrt_value >> 21);  // 0x08000000 → 64
}
```

**RBRT → Sketchy (-64 to +64 bipolar):**
```cpp
int32_t rbrt_to_sketchy_bi(int32_t rbrt_value) {
    return ((rbrt_value >> 21) - 64);  // Center at 0
}
```

**Sketchy → RBRT (unipolar):**
```cpp
int32_t sketchy_to_rbrt_uni(int32_t sketchy_value) {
    return (sketchy_value << 21);  // 64 → 0x08000000
}
```

**Sketchy → RBRT (bipolar):**
```cpp
int32_t sketchy_to_rbrt_bi(int32_t sketchy_value) {
    return ((sketchy_value + 64) << 21);
}
```

---

## Recommended Approach

For a **3-slot looper with Sketchy control**, I recommend:

### Configuration:

**Strategy 1 implementation** (Direct Mapping):
- Page 0 → Slot 0 parameters
- Page 1 → Slot 1 parameters  
- Page 2 → Slot 2 parameters
- Page 3 → Master/global controls

**Parameter assignments per page/slot:**
1. **VOL** (Volume) - RBRT param 8
2. **PBASE** (Pitch) - RBRT param 7
3. **LMOD** (Length) - RBRT param 5
4. **DIRECTION** - RBRT param 6
5. **ATTACK** - RBRT param 11
6. **RELEASE** - RBRT param 12
7. **FEEDBACK** - Custom param 16
8. **PAN** - Custom param 17

**Objects:**
- 1× `sketchy_params` (parameter storage)
- 1× `sketchy_dials` (configuration)
- 1× `8encoder_input` (hardware interface)
- 1× `sketchy_render` (OLED display)
- 1× `sketchy_to_prm` (bridge object)
- Tie page to recordlogic active slot

This gives you **real-time encoder control** of key loop parameters with **visual feedback** on the OLED.

---

## Next Steps

1. Create `sketchy_to_prm` bridge object
2. Create `prm_to_sketchy` bridge object (optional, for initialization)
3. Configure `sketchy_dials` with appropriate modes for each parameter
4. Wire recordlogic `activeslot` to Sketchy `page` input
5. Test and iterate parameter mappings

Would you like me to create these bridge objects?



