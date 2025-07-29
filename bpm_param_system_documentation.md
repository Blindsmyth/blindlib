# BPM Parameter System Documentation

## Overview

The BPM Parameter System allows bidirectional communication between the `tap.io` clock object and the sketchy param table, enabling BPM values to be:
1. **Set from encoders** → param table → `tap.io` (internal BPM control)
2. **Set from external MIDI clock** → `tap.io` → param table (external BPM sync)

This system prevents feedback loops by carefully tracking the source of BPM changes.

## Components

### 1. Modified `tap.io` Object

**File**: `objects/clock/tap.io.axo`

**New Features**:
- **BPM Input**: `int32 bpm` inlet for receiving BPM from param table
- **BPM Change Output**: `bool32 bpmChanged` outlet that pulses when BPM changes from external source
- **Source Tracking**: Internal flags to track whether BPM came from external clock or param table

**Key Logic**:
```c
// BPM input handling
int32_t lastInletBpm; // Track last BPM input value
bool bpmFromExternal; // Flag to indicate if BPM was set from external clock
bool bpmFromParam; // Flag to indicate if BPM was set from param table

// Only update from param table if not from external clock
if (bpmUpdateNeeded && !bpmFromExternal) {
    setBpmFromParam(inlet_bpm);
}

// Only allow manual BPM adjustments if not from external or param sources
if (!bpmFromExternal && !bpmFromParam) {
    // Handle bpmPlus, bpmMinus, half, double buttons
}

// Output pulse when BPM changes from external source
outlet_bpmChanged = bpmFromExternal;
```

### 2. New `param_write_int` Object

**File**: `objects/encoder/param_write_int.axo`
**UUID**: `5224773f-18de-4231-8f92-b1f22bb95387`

**Purpose**: Generic object that writes integer values to the param table when triggered. Used for BPM and other parameter updates from external sources.

**Inlets**:
- `int32 value`: Value to write to param table
- `bool32 trigger`: Pulse to trigger writing to param table
- `int32 page`: Page number for param table (default: 0)
- `int32 param`: Parameter index for storage (default: 0)

**Key Logic**:
```c
// Only write to param table when triggered and value changed
void safeWriteValue(int32_t value, bool trigger, int32_t page, int32_t param) {
    if (trigger && !lastTrigger && value != lastValue) {
        writeValueToParamTable(value, page, param);
        lastValue = value;
    }
    lastTrigger = trigger;
}
```

## Usage Scenarios

### Scenario 1: Encoder Controls BPM (Internal Mode)

```
Encoder → sketchy_dials → param_table → get_param_int → tap.io
```

1. User turns encoder
2. `sketchy_dials` updates param table
3. `get_param_int` reads from param table
4. `tap.io` receives BPM via `bpm` inlet
5. `tap.io` sets `bpmFromParam = true`, `bpmFromExternal = false`
6. Manual BPM controls (bpmPlus, bpmMinus, etc.) are disabled
7. External MIDI clock is ignored (if in internal mode)

### Scenario 2: External MIDI Clock Controls BPM

```
MIDI Clock → tap.io → param_write_int → param_table
```

1. External MIDI clock arrives at `tap.io`
2. `tap.io` calculates BPM from clock timing
3. `tap.io` sets `bpmFromExternal = true`, `bpmFromParam = false`
4. `tap.io` outputs `bpmChanged = true` pulse
5. `param_write_int` receives pulse and writes BPM to param table
6. Manual BPM controls are disabled
7. Param table BPM input is ignored

### Scenario 3: Display Current BPM

```
param_table → get_param_int → sketchy_dials (display)
```

1. `get_param_int` reads current BPM from param table
2. `sketchy_dials` displays the BPM value
3. Works regardless of BPM source (encoder or external clock)

## Feedback Loop Prevention

The system prevents feedback loops through several mechanisms:

### 1. Source Tracking
- `bpmFromExternal`: Set when BPM comes from MIDI clock
- `bpmFromParam`: Set when BPM comes from param table
- These flags are mutually exclusive

### 2. Conditional Updates
```c
// Only update from param table if not from external clock
if (bpmUpdateNeeded && !bpmFromExternal) {
    setBpmFromParam(inlet_bpm);
}

// Only write to param table when triggered and value changed
if (trigger && !lastTrigger && value != lastValue) {
    writeValueToParamTable(value, page, param);
}
```

### 3. Manual Control Disabling
When BPM comes from external or param sources, manual controls (bpmPlus, bpmMinus, half, double) are disabled to prevent conflicts.

## Test Patch

**File**: `patches/test_bpm_param_system.axp`

This patch demonstrates the complete system:
- `tap.io` with BPM input/output
- `bpm_param_writer` for writing to param table
- `sketchy_params` for parameter storage
- `get_param_int` for reading from param table

## Integration with Sketchy System

### Required Dependencies
- `sketchy_params`: Parameter table storage
- `sketchy_dials`: For encoder input and display
- `get_param_int`: For reading from param table

### Typical Setup
1. Configure `sketchy_dials` with BPM encoder on desired page/param
2. Connect `get_param_int` to read BPM from same page/param
3. Connect `get_param_int` output to `tap.io` BPM input
4. Connect `tap.io` BPM output and `bpmChanged` to `bpm_param_writer`
5. Configure `bpm_param_writer` to write to same page/param

## Advantages

1. **Bidirectional**: BPM can be set from encoders or external clock
2. **No Feedback Loops**: Careful source tracking prevents conflicts
3. **Display Integration**: Current BPM always visible on sketchy display
4. **Flexible**: Works with any page/param combination
5. **Efficient**: Only updates when values actually change

## Limitations

1. **Single BPM Storage**: Only one BPM value can be stored per page/param
2. **Source Priority**: External clock takes priority over encoder input
3. **Manual Controls**: Manual BPM controls are disabled when using external/param sources

## Future Enhancements

1. **Multiple BPM Storage**: Support for multiple BPM values
2. **Source Blending**: Allow mixing of internal and external BPM sources
3. **BPM Smoothing**: Add interpolation for BPM changes
4. **BPM History**: Track BPM changes over time 