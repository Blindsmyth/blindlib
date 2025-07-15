# SDRAM Array Strategy for Axoloti Objects

## Overview
This document outlines the technique for moving large arrays from SRAM to SDRAM in Axoloti objects, providing significant memory savings for objects that need to store large amounts of data.

## The Problem
- **SRAM is limited**: Axoloti has limited SRAM (Static RAM) which is fast but scarce
- **Large arrays consume SRAM**: Big arrays declared normally consume precious SRAM
- **SDRAM is abundant**: Axoloti has much more SDRAM (Dynamic RAM) available but requires special allocation

## The Solution: SDRAM Array Technique

### Step 1: Declare Pointer in Local Data
In the **Local Data** section, declare a pointer to your chosen data type:

```cpp
// Examples:
uint8_t *midi_data;        // For MIDI values (0-127)
int16_t *audio_buffer;     // For audio samples
float *parameter_history;  // For floating point values
uint32_t *large_counters;  // For big integer values
```

**Key Points:**
- The `*` makes it a pointer (points to memory location)
- Choose data type based on your value range
- This pointer itself takes minimal SRAM (just 4 bytes)

### Step 2: Create Shadow Array in Init Code
In the **Init Code** section, create the actual array in SDRAM:

```cpp
// Template:
static [data_type] _[array_name][[array_size]] __attribute__ ((section (".sdram")));

// Examples:
static uint8_t _midi_data[8192] __attribute__ ((section (".sdram")));
static int16_t _audio_buffer[44100] __attribute__ ((section (".sdram")));
static float _parameter_history[1000] __attribute__ ((section (".sdram")));
```

**Key Points:**
- `static` keeps it persistent
- `_[name]` convention for the shadow array (underscore prefix)
- `__attribute__ ((section (".sdram")))` forces SDRAM allocation
- Size in square brackets `[SIZE]`

### Step 3: Point the Pointer to the Shadow Array
Still in **Init Code**, connect your pointer to the shadow array:

```cpp
// Template:
[pointer_name] = &_[shadow_array_name][0];

// Examples:
midi_data = &_midi_data[0];
audio_buffer = &_audio_buffer[0];
parameter_history = &_parameter_history[0];
```

**Key Points:**
- `&` gets the address of the first element
- `[0]` specifies the first element
- Now your pointer points to SDRAM memory

### Step 4: Use as Normal Array
After setup, use the pointer exactly like a regular array:

```cpp
// In K-rate or other code sections:
midi_data[index] = value;           // Write to array
int current_value = midi_data[index]; // Read from array
```

## Complete Example: Parameter Storage Object

```cpp
// In Local Data:
float *param_values;    // Pointer for parameter storage
uint16_t param_count;   // Current number of parameters

// In Init Code:
#define MAX_PARAMS 2048
static float _param_values[MAX_PARAMS] __attribute__ ((section (".sdram")));
param_values = &_param_values[0];
param_count = 0;

// Initialize array
for (int i = 0; i < MAX_PARAMS; i++) {
    param_values[i] = 0.0f;
}

// In K-rate Code:
// Store parameter
if (param_count < MAX_PARAMS) {
    param_values[param_count] = inlet_value;
    param_count++;
}

// Retrieve parameter
if (index < param_count) {
    outlet_value = param_values[index];
}
```

## Memory Optimization Guidelines

### 1. Array Size Considerations
- **Small arrays (<100 elements)**: May stay in SRAM for speed
- **Medium arrays (100-1000 elements)**: Good candidates for SDRAM
- **Large arrays (>1000 elements)**: Definitely use SDRAM

### 2. Performance Considerations
- **SDRAM is slower** than SRAM but difference is usually negligible
- **Access patterns**: Sequential access is faster than random access
- **Caching**: Frequently accessed values can be cached in local variables

## Common Use Cases

### 1. Parameter Storage Systems
```cpp
// For objects like sketchy_params with many parameters
static int32_t _param_table[8][128] __attribute__ ((section (".sdram")));
int32_t (*param_table)[128] = _param_table;
```

### 2. Audio Buffers
```cpp
// For delay lines, reverbs, etc.
static int16_t _delay_buffer[48000] __attribute__ ((section (".sdram")));
int16_t *delay_buffer = &_delay_buffer[0];
```

### 3. Lookup Tables
```cpp
// For wavetables, envelopes, etc.
static uint16_t _wavetable[4096] __attribute__ ((section (".sdram")));
uint16_t *wavetable = &_wavetable[0];
```

### 4. Configuration Arrays
```cpp
// For complex configuration data
static uint8_t _config_data[1024] __attribute__ ((section (".sdram")));
uint8_t *config_data = &_config_data[0];
```

## Best Practices

### 1. Naming Conventions
- **Pointer**: `array_name` (descriptive name)
- **Shadow Array**: `_array_name` (underscore prefix)
- **Constants**: `#define MAX_ARRAY_SIZE 1024`

### 2. Initialization
- Always initialize arrays to known values
- Use loops for large arrays
- Consider default values that make sense for your application

### 3. Bounds Checking
```cpp
// Always check array bounds
if (index >= 0 && index < ARRAY_SIZE) {
    array[index] = value;
}
```

### 4. Memory Layout
- Group related arrays together
- Consider alignment for performance
- Document memory usage in comments

## Troubleshooting

### Common Issues:
1. **Compilation errors**: Check syntax of `__attribute__` declaration
2. **Runtime crashes**: Usually bounds checking issues
3. **Performance issues**: Consider access patterns and caching

### Debug Tips:
- Use smaller arrays first to test the technique
- Add bounds checking during development
- Monitor memory usage in development tools

## Memory Savings Examples

### Example 1: MIDI Sequencer
```cpp
// Before (SRAM): 
int sequences[8][64];  // 8 sequences × 64 steps × 4 bytes = 2KB SRAM

// After (SDRAM):
static uint8_t _sequences[8][64] __attribute__ ((section (".sdram")));
uint8_t (*sequences)[64] = _sequences;  // 8 × 64 × 1 = 512 bytes SDRAM
// Savings: 1.5KB SRAM freed, 75% memory reduction
```

### Example 2: Parameter Storage
```cpp
// Before (SRAM):
float params[8][128];  // 8 pages × 128 params × 4 bytes = 4KB SRAM

// After (SDRAM):
static int16_t _params[8][128] __attribute__ ((section (".sdram")));
int16_t (*params)[128] = _params;  // 8 × 128 × 2 = 2KB SDRAM
// Savings: 4KB SRAM freed, 50% memory reduction
```

## Integration with Existing Objects

### For sketchy_params Integration:
```cpp
// Replace large parameter arrays with SDRAM versions
static int32_t _param_array[8][128] __attribute__ ((section (".sdram")));
int32_t (*array)[128] = _param_array;
```

### For Audio Processing:
```cpp
// Replace audio buffers with SDRAM versions
static int16_t _audio_buffer[BUFFER_SIZE] __attribute__ ((section (".sdram")));
int16_t *audio_buffer = &_audio_buffer[0];
```

## Conclusion

The SDRAM array technique provides:
- **Significant memory savings** (often 50-75% reduction)
- **Larger array capabilities** (limited by SDRAM, not SRAM)
- **Minimal performance impact** for most use cases
- **Easy implementation** with copy-paste templates

This technique is essential for complex objects that need to store large amounts of data while preserving precious SRAM for real-time processing. 