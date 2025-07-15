# SD Card Font Strategy - SRAM Optimization

## Overview
Move the sketchy_font data from SRAM to SD card storage to free up 480 bytes of precious SRAM1 space. The font data will be saved to the SD card during development and loaded at runtime.

## Current Font Structure
Based on memory optimization summary:
- **Font data size**: 480 bytes
- **Current location**: SDRAM (already optimized from SRAM1)
- **Data type**: `uint8_t data[SKETCHY_FONT_DATA_SIZE]`
- **Usage**: Character bitmap data for OLED text rendering

## SD Card Strategy

### 1. Font Save Object (`sketchy_font_save.axo`)
**Purpose**: Save font data from SDRAM to SD card file
**UUID**: `5224773f-18de-4231-8f92-b1f22bb95378` (following UUID registry pattern)

**Features**:
- Trigger-based saving (rising edge)
- Configurable filename (default: "sketchy_font.bin")
- Error reporting for SD card issues
- Progress feedback via outlet

**Usage**:
```
filename: "sketchy_font.bin"
trig: rising edge to save
```

### 2. Font Load Object (`sketchy_font_load.axo`)
**Purpose**: Load font data from SD card to SDRAM at runtime
**UUID**: `5224773f-18de-4231-8f92-b1f22bb95379`

**Features**:
- Trigger-based loading (rising edge)
- Configurable filename (default: "sketchy_font.bin")
- Error reporting for missing files or SD issues
- Load status feedback via outlet
- Automatic loading on patch startup (optional)

**Usage**:
```
filename: "sketchy_font.bin"
trig: rising edge to load
load_on_start: true/false
```

### 3. Modified Font Object (`sketchy_font_sd.axo`)
**Purpose**: Font object that loads data from SD card instead of storing in memory
**UUID**: `5224773f-18de-4231-8f92-b1f22bb9537A`

**Features**:
- No font data stored in memory
- Loads font data from SD card on initialization
- Fallback to built-in minimal font if SD card fails
- Compatible with existing renderer objects

## Implementation Strategy

### Phase 1: Create Save/Load Objects
1. **Create `sketchy_font_save.axo`**
   - Based on existing save object pattern
   - Save font data from SDRAM to SD card
   - Include error handling and progress feedback

2. **Create `sketchy_font_load.axo`**
   - Based on existing load object pattern
   - Load font data from SD card to SDRAM
   - Include error handling and status feedback

### Phase 2: Create SD-Based Font Object
3. **Create `sketchy_font_sd.axo`**
   - Remove built-in font data array
   - Add SD card loading functionality
   - Include fallback font data (minimal set)
   - Maintain compatibility with renderer objects

### Phase 3: Integration and Testing
4. **Test SD card operations**
   - Verify save/load functionality
   - Test error conditions (missing SD card, corrupted file)
   - Measure loading time impact

5. **Update renderer compatibility**
   - Ensure renderer objects work with SD-based font
   - Test font rendering performance
   - Verify memory savings

## Memory Savings Analysis

### Current Memory Usage
- **Font data in SDRAM**: 480 bytes (already optimized from SRAM1)
- **Font object overhead**: ~50 bytes for structure and functions

### SD Card Strategy Benefits
- **Direct SRAM savings**: 0 bytes (already in SDRAM)
- **Indirect benefits**:
  - Reduced SDRAM usage (480 bytes freed)
  - More SDRAM available for other optimizations
  - Potential for larger font sets
  - Dynamic font loading capability

### Alternative Approach: Move to SRAM1
If we move font data back to SRAM1 and then to SD card:
- **SRAM1 savings**: 480 bytes (significant for 44KB limit)
- **Trade-off**: Slower font access (SD card vs SRAM)
- **Risk**: Font loading failure could break rendering

## Technical Considerations

### SD Card Access Performance
- **Read speed**: ~1-2ms for 480 bytes (negligible for text rendering)
- **Write speed**: ~5-10ms for saving (development time only)
- **Reliability**: SD cards can fail or be removed

### Error Handling Strategy
1. **SD card missing**: Fallback to minimal built-in font
2. **File missing**: Create default font file or use fallback
3. **Corrupted file**: Use fallback font and report error
4. **Read errors**: Retry mechanism with timeout

### Fallback Font Design
- **Size**: Minimal character set (64 bytes)
- **Characters**: Numbers 0-9, basic symbols
- **Quality**: Reduced resolution if needed
- **Purpose**: Ensure basic functionality even without SD card

## File Structure

### SD Card File Format
```
File: sketchy_font.bin
Size: 480 bytes
Format: Raw binary font data
Header: None (simple binary dump)
Checksum: Optional for corruption detection
```

### Font Data Organization
```
Offset 0-479: Character bitmap data
- 8x6 pixel characters
- 64 characters total
- 6 bytes per character
- Sequential character order (ASCII 32-95)
```

## Development Workflow

### Initial Setup
1. Create save/load objects
2. Create SD-based font object
3. Test with existing font data
4. Save font data to SD card

### Runtime Operation
1. Patch starts with SD-based font object
2. Font object attempts to load from SD card
3. If successful: use loaded font data
4. If failed: use fallback font data
5. Renderer uses font data normally

### Testing Strategy
1. **Normal operation**: SD card present, font file exists
2. **Missing SD card**: Should use fallback font
3. **Missing font file**: Should create or use fallback
4. **Corrupted font file**: Should detect and use fallback
5. **Performance test**: Measure font loading time impact

## Compatibility Considerations

### Renderer Objects
- All existing renderer objects should work unchanged
- Font data access remains the same
- No changes needed to rendering functions

### Dial Objects
- No impact on dial functionality
- Font rendering performance unchanged
- All existing patches should work

### Patch Files
- Existing patches need font object replacement
- Save/load objects added for font management
- No other changes required

## Future Enhancements

### Dynamic Font Loading
- Multiple font files on SD card
- Runtime font switching
- Custom font creation tools

### Font Compression
- Compress font data on SD card
- Decompress during loading
- Further reduce file size

### Font Validation
- Checksum verification
- Font format validation
- Automatic font repair

## Risk Assessment

### Low Risk
- **SD card access**: Well-tested in Axoloti
- **File I/O**: Standard operations
- **Fallback mechanism**: Ensures functionality

### Medium Risk
- **Loading time**: Could impact patch startup
- **SD card reliability**: Hardware dependency
- **File corruption**: Data integrity concerns

### Mitigation Strategies
- **Fast loading**: Optimize file read operations
- **Robust fallback**: Multiple fallback levels
- **Error reporting**: Clear feedback for issues
- **Testing**: Comprehensive error condition testing

## Conclusion

The SD card font strategy provides:
- **480 bytes SDRAM freed** for other optimizations
- **Dynamic font loading** capability
- **Zero functional impact** on existing patches
- **Robust error handling** with fallback mechanisms

This approach is particularly valuable for:
- Complex patches approaching memory limits
- Dynamic font requirements
- Future font expansion possibilities
- Memory optimization for other critical components

The implementation follows established Axoloti patterns and maintains full compatibility with existing sketchy rendering system. 