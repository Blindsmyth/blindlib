# Blindlib Development Rules

## Include Path Rules

### RBRT Classes Include Paths

**Location**: `/Users/simon/Dropbox/Axolonatics/objects/rbrt_new/rbrt_classes/`

**For objects in blindlib folder structure:**
```
/Users/simon/Dropbox/blindlib/objects/[category]/
```
Use: `../../Axolonatics/objects/rbrt_new/rbrt_classes/Smplr.h`

**For objects in Axolonatics folder structure:**
```
/Users/simon/Dropbox/Axolonatics/objects/rbrt_new/[category]/
```
Use: `../rbrt_classes/Smplr.h`

### Common RBRT Headers
- `Smplr.h` - Main smplr system
- `sync.h` - Synchronization classes
- `Poly.h` - Polyphonic voice management
- `Push.h` - Push controller integration

### Include Path Examples

#### ✅ CORRECT - Blindlib objects
```xml
<includes>
   <include>../../Axolonatics/objects/rbrt_new/rbrt_classes/Smplr.h</include>
</includes>
```

#### ✅ CORRECT - Axolonatics objects
```xml
<includes>
   <include>../rbrt_classes/Smplr.h</include>
</includes>
```

#### ❌ WRONG - These will fail
```xml
<!-- Wrong: Missing Axolonatics in path -->
<include>../../objects/rbrt_new/rbrt_classes/Smplr.h</include>

<!-- Wrong: Wrong number of levels -->
<include>../../../Axolonatics/objects/rbrt_new/rbrt_classes/Smplr.h</include>

<!-- Wrong: Case sensitivity -->
<include>../../Axolonatics/objects/rbrt_new/rbrt_classes/smplr.h</include>
```

## UUID Rules

### Format
- Must be 8-4-4-4-12 hex format
- Example: `a1b2c3d4-e5f6-7890-abcd-ef1234567890`

### Generation Pattern
- Use sequential hex values
- Avoid obvious patterns like `00000000-0000-0000-0000-000000000000`
- Keep track of used UUIDs to avoid conflicts

### Examples
```xml
<obj.normal id="my_object" uuid="a1b2c3d4-e5f6-7890-abcd-ef1234567890">
```

## Object Naming Rules

### File Naming
- Use lowercase with underscores: `my_object.axo`
- Avoid spaces in filenames
- Be descriptive but concise

### Object ID Naming
- Use spaces for readability: `my object`
- Match the filename concept but with spaces
- Keep consistent with existing patterns

### Examples
```xml
<!-- File: my_control_object.axo -->
<obj.normal id="my control object" uuid="...">
```

## MIDI Buffer Management

### Problem
Objects that send MIDI messages can overflow the serial buffer (SD2, 256 bytes), causing other MIDI functions to fail.

### Solution
```cpp
// Check buffer space before sending
if (sdGetTimeout(&SD2, TIME_IMMEDIATE) == MSG_TIMEOUT) {
    // Buffer has space, safe to send
    SendMidi3(MIDI_CONTROL_CHANGE + channel, cc, value);
} else {
    // Buffer full, skip this cycle
}
```

### Throttling Strategies
1. **Scan rate limiting**: Only scan 1 parameter per k-rate cycle
2. **Heavy throttling**: Only scan every 10th k-rate cycle
3. **Buffer checking**: Check space before each MIDI send
4. **Page-based**: Only send current page (8 params vs 64)

## Subpatch Access Rules

### Problem
Objects in subpatches cannot access parent objects directly using `parent->objectinstance_*`.

### Solutions

#### Option 1: Pass values through inlets
```xml
<!-- Main patch -->
<obj type="param get" page="1" param="0"/>
  ↓ [value outlet]
<!-- Subpatch -->
<obj type="param get subpatch" page="1" param="0"/>
  ↑ [value inlet]
```

#### Option 2: Use parent->parent (limited support)
```cpp
// Only works in specific cases
param_table = &parent->parent->objectinstance_sketchy__params_i;
```

#### Option 3: Direct parameter control
```cpp
// Bypass sketchy system entirely
smplr.prm[adr + param_offset] = value;
```

## Memory Management

### SDRAM Usage
- RBRT system uses shared SDRAM for parameters
- Each slot: 128 parameters × 4 bytes = 512 bytes
- Total for 128 slots: ~64KB

### Parameter Access
```cpp
uint16_t adr = slot * 128;  // slot * 128
smplr.prm[adr + param_offset] = value;
```

### Common Parameter Offsets
- `VOL_OFFSET = 8` (Volume)
- `PBASE_OFFSET = 7` (Base Pitch)
- `PLAYMODE_OFFSET = 3` (Play Mode)

## Testing Checklist

Before committing new objects:

- [ ] Include paths work from both blindlib and Axolonatics locations
- [ ] UUID follows 8-4-4-4-12 format
- [ ] MIDI objects have buffer overflow protection
- [ ] Subpatch objects work with inlet-based approach
- [ ] Parameter access uses correct offsets
- [ ] Object compiles without errors
- [ ] Documentation matches implementation

## Common Pitfalls

1. **Include path case sensitivity**: `Smplr.h` not `smplr.h`
2. **Wrong directory levels**: Count carefully from object location
3. **MIDI buffer overflow**: Always check buffer space
4. **Subpatch access**: Use inlet-based approach, not parent->parent
5. **UUID conflicts**: Keep track of used UUIDs
6. **Parameter offsets**: Use correct RBRT parameter numbers

## File Structure Reference

```
/Users/simon/Dropbox/
├── blindlib/objects/           # Our custom objects
│   ├── smplr/                 # RBRT-related objects
│   ├── encoder/               # Encoder/display objects
│   ├── midi/                  # MIDI objects
│   └── ...
└── Axolonatics/objects/        # Robert's original objects
    └── rbrt_new/
        ├── rbrt_classes/      # Header files
        │   ├── Smplr.h
        │   ├── sync.h
        │   └── ...
        └── smplr/             # RBRT smplr objects
```

---

**Last Updated**: 2025-01-27  
**Author**: AI Assistant  
**Version**: 1.0



