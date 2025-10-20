# SD Card Config Loading Workflow

## Problem
Currently, config structs are hardcoded in `init_config_data()` which stores initialization data in SRAM (flash section), then copies it to SDRAM at runtime. This wastes precious SRAM space.

## Solution
Load config structs directly from SD card into SDRAM, bypassing SRAM entirely.

## Current Structure

### encoder_dial_config_t (32 bytes each)
```cpp
typedef struct {
    uint8_t mode;           // 1 byte: 0=frac uni, 1=frac bi, 2=int, 3=list, 4=bar uni, 5=bar bi, 6=dup
    bool show_value;        // 1 byte: Show/hide value
    const char* label;      // 4 bytes: Label text pointer
    const char* options;    // 4 bytes: List options or duplicate ref pointer
    float default_value;    // 4 bytes: Default value
    int16_t int_min;        // 2 bytes: Min value
    int16_t int_max;        // 2 bytes: Max value
    // + padding = ~32 bytes total per struct
} encoder_dial_config_t;
```

**Total data**: 64 configs (8 pages × 8 dials) = ~2KB

### Challenge: String Storage
The `label` and `options` fields are pointers to strings. We need to:
1. Store strings in a separate section
2. Store string offsets/indices instead of pointers
3. Reconstruct pointers after loading

## Binary File Format

### File: `config_dials.bin`

```
[HEADER - 16 bytes]
  - Magic: "AXOCFG01" (8 bytes) - identifies file type/version
  - Config count: uint16_t (2 bytes) - should be 64
  - String section offset: uint32_t (4 bytes) - byte offset to string data
  - String section size: uint16_t (2 bytes) - total string data size

[CONFIG DATA - 1536 bytes = 64 × 24 bytes each]
  For each of 64 configs:
    - mode: uint8_t (1 byte)
    - show_value: uint8_t (1 byte)
    - label_offset: uint16_t (2 bytes) - offset into string section
    - options_offset: uint16_t (2 bytes) - offset into string section (0xFFFF = NULL)
    - default_value: float (4 bytes)
    - int_min: int16_t (2 bytes)
    - int_max: int16_t (2 bytes)
    - reserved: uint8_t[12] (12 bytes) - for future expansion

[STRING SECTION - variable size, ~1KB typical]
  - All label strings concatenated (null-terminated)
  - All options strings concatenated (null-terminated)
  - Indexed by offsets from config data
```

**Total file size**: ~2.5KB (very compact!)

## Workflow

### Phase 1: Save Current Config to SD Card

```
[sketchy_dials] → [sketchy_config_save] → SD:/config_dials.bin
       ↓                    ↓
   configs[8][8]    Serialize to binary
```

**Object**: `sketchy_config_save.axo`
- Inlet: `trig` (rising edge to save)
- Inlet: `filename` (default: "config_dials.bin")
- Reads from `parent->objectinstance_sketchy__dials_i.configs`
- Serializes to binary format
- Writes to SD card

### Phase 2: Load Config from SD Card

```
SD:/config_dials.bin → [sketchy_config_load] → [sketchy_dials]
                              ↓                        ↓
                       Deserialize binary      configs[8][8] in SDRAM
```

**Object**: `sketchy_config_load.axo`
- Inlet: `trig` (rising edge to load)
- Inlet: `filename` (default: "config_dials.bin")
- Allocates SDRAM for:
  - Config structs
  - String buffer
- Reads from SD card directly into SDRAM
- Reconstructs pointers
- Updates `sketchy_dials` configs pointer

### Phase 3: Modified sketchy_dials

**Add attribute**: `load_from_sd` (bool, default: false)
```cpp
if (attr_load_from_sd) {
    // Skip init_config_data() - will be loaded from SD
    // Just allocate SDRAM arrays
} else {
    // Use current hardcoded init_config_data()
}
```

## Implementation Strategy

### Step 1: Create Save Object (sketchy_config_save.axo)

```cpp
code.krate:
  if (trig rising edge) {
    1. Open file for writing
    2. Write header (magic, count, offsets)
    3. Build string section:
       - Collect all unique strings
       - Calculate offsets
    4. Write config data (with string offsets)
    5. Write string section
    6. Close file
  }
```

### Step 2: Create Load Object (sketchy_config_load.axo)

```cpp
code.init:
  - Allocate SDRAM for string buffer (2KB)
  
code.krate:
  if (trig rising edge) {
    1. Open file for reading
    2. Read header, validate magic
    3. Allocate SDRAM for configs if needed
    4. Read config data directly into SDRAM
    5. Read string section into SDRAM string buffer
    6. Reconstruct string pointers:
       - For each config:
         - config.label = string_buffer + label_offset
         - config.options = (options_offset == 0xFFFF) ? NULL : string_buffer + options_offset
    7. Update sketchy_dials.configs pointer
    8. Close file
  }
```

### Step 3: Modify sketchy_dials (optional enhancement)

Add attribute to support loading from file:
```cpp
<attribs>
  <combo name="config_source">
    <MenuEntries>
      <string>Hardcoded</string>
      <string>SD Card</string>
    </MenuEntries>
  </combo>
</attribs>

code.init:
  if (attr_config_source == 0) { // Hardcoded
    // Allocate SDRAM and call init_config_data()
  } else { // SD Card
    // Just allocate SDRAM, wait for load object
  }
```

## Advanced Workflow: Config Editor

For creating custom configs without recompiling:

```
[sketchy_config_editor] → [sketchy_config_save] → SD:/custom_config.bin
         ↓                                                    ↓
  Set mode, label,                              [sketchy_config_load]
  options, ranges                                        ↓
  per page/dial                                    [sketchy_dials]
```

**Future enhancement**: Create a config editor patch that lets you:
- Select page/dial
- Set mode, label, options
- Set ranges (min/max)
- Preview changes
- Save to SD card

## Benefits

✅ **SRAM savings**: ~2KB of SRAM freed (no hardcoded init data)  
✅ **Flexibility**: Multiple config files for different instruments  
✅ **Runtime config**: Change configs without recompiling  
✅ **Portability**: Share config files between patches  
✅ **Version control**: Easy to backup/restore configs  

## Memory Comparison

| Method | SRAM Usage | SDRAM Usage | Flash Usage |
|--------|------------|-------------|-------------|
| **Current** (hardcoded) | ~2KB (init data) | 2KB (configs) | ~5KB (code + data) |
| **SD Card** (proposed) | ~0KB | 4KB (configs + strings) | ~3KB (code only) |
| **Net savings** | **+2KB SRAM** | -2KB SDRAM | +2KB flash |

**Worth it?** YES! SRAM is the most precious resource. SDRAM is plentiful (8MB).

## File Management

### Recommended file naming:
- `config_dials.bin` - default dial configs
- `config_synth.bin` - synth preset
- `config_drums.bin` - drum machine preset
- `config_fx.bin` - effects processor preset

### Patch workflow:
```
[Load Synth Config]
      ↓
[sketchy_config_load filename="config_synth.bin"]
      ↓
[sketchy_dials] - now uses synth config
      ↓
[Your synth patch...]
```

## Next Steps

1. **Create `sketchy_config_save.axo`** - Save current configs to SD
2. **Create `sketchy_config_load.axo`** - Load configs from SD into SDRAM
3. **Test workflow** - Save default configs, load them back
4. **Create config variants** - Different instrument configs
5. **Optional**: Create config editor for runtime editing

## Notes

- **Thread safety**: Load should happen during init or when audio is paused
- **Error handling**: Validate file magic, check file size
- **Backwards compat**: Keep hardcoded init as default, SD loading as option
- **String limits**: Max string length 32 chars per label/options field
- **File corruption**: Add CRC/checksum to header for validation

