# Page Handling Strategy Document

## Goal & Terminology Clarification

### System Overview:
We have **1 encoder object** that:
1. **Reads data from 8 physical rotary encoders** (relative movement)
2. **Interprets this data** using a page-based configuration system
3. **Stores/retrieves values** from the param_table for persistence
4. **Displays encoder values** as dials (handled by renderer object)

### Page System:
- **Multiple pages** (0-7) each containing configuration for all 8 encoders
- **Global configuration arrays** define encoder behavior per page:
  - Mode (FRAC_UNIPOLAR, FRAC_BIPOLAR, INT, SELECTOR)
  - Min/Max values (for INT and SELECTOR modes)
  - Default values (for all modes)
  - Labels and display properties

### Page Switching Process:
1. **Display switching**: Handled by renderer object (separate concern)
2. **Value loading**: Encoder object must read stored values from param_table for the new page
3. **Configuration switching**: Encoder object applies new page's mode/min/max/default settings

### Key Insight:
- **Physical encoders**: Always the same 8 encoders
- **Logical function**: Changes based on current page configuration
- **Value persistence**: Each page maintains its own set of 8 stored values
- **Relative encoding**: Encoders need to know current position to apply relative changes correctly

## Current Issues & Inconsistencies

### 1. **Default Value Handling**
**Problem**: Default values are not properly initialized or restored when switching pages.

**Current Behavior**:
- `dials[encoderIndex].default_value = 0` is hardcoded to 0 for all encoders
- No mechanism to store/restore actual default values per page
- Values reset inconsistently when switching pages

**Impact**: Users lose their settings when switching between pages, making the system unreliable.

### 2. **Page Change Logic Inconsistencies**

#### A. Value Initialization Strategy
**Current Implementation**:
```c
if (currentMode == MODE_INT || currentMode == MODE_SELECTOR) {
    loadIntValue(i, inlet_a);  // Loads minimum value, not saved value
} else {
    encoderPos[i] = 0.0f;      // Always resets to 0
}
```

**Problems**:
- INT/SELECTOR modes load minimum values instead of saved values
- FRAC modes always reset to 0.0f
- No persistence of user-adjusted values

#### B. Display Update Timing
**Current Implementation**:
- `setupDisplay()` called on every page change ✓ (GOOD - this is when we want to reconfigure)
- `dial_update()` called every krate for all dials ✓ (GOOD - needed for smooth display)
- `dial_init()` called on every page change (potentially expensive)

**Problems**:
- `dial_init()` might be expensive to call repeatedly
- But the timing of setup calls is actually correct

#### C. Parameter Storage Inconsistency
**Current Implementation**:
- Values stored in `param_table->array[page][firstparam + i]`
- But local variables (`encoderPos[]`, `intValues[]`) don't sync with param_table
- No bidirectional synchronization

### 3. **Array Index Management**
**Current Issues**:
- Multiple ways to calculate array indices: `inlet_a * 8 + i` vs `page * 8 + encoderIndex`
- Some functions use `layer` parameter, others use `page` - terminology inconsistency
- Bounds checking inconsistent across functions

### 4. **Mode-Specific Value Handling**
**Problems**:
- Different value ranges and constraints per mode
- No unified approach to convert between internal representation and param_table values
- INT modes use different data structures (`intValues[]`) than FRAC modes (`encoderPos[]`)

## Proposed Strategy

### Phase 1: Unify Data Model
1. **Single Source of Truth**: Make `param_table` the authoritative storage
2. **Eliminate Redundant Storage**: Remove local `encoderPos[]` and `intValues[]` arrays
3. **Consistent Indexing**: Use single formula for array indexing throughout

### Phase 2: Fix Value Persistence
1. **Load Actual Values**: When switching pages, load saved values from param_table
2. **Default Value System**: Create proper default value arrays per mode/page
3. **Bidirectional Sync**: Ensure changes update param_table immediately

### Phase 3: Optimize Display Updates
1. **Keep Display Setup on Page Change**: `setupDisplay()` timing is correct
2. **Optimize dial_init()**: Maybe cache initialization or make it lighter
3. **Current update pattern is fine**: dial_update() every krate is appropriate

### Phase 4: Standardize Mode Handling
1. **Unified Value Conversion**: Single functions to convert between modes and param_table
2. **Consistent Constraints**: Apply constraints at single point
3. **Mode-Specific Defaults**: Proper default values per mode

## Implementation Plan

### Step 1: Create Default Value Arrays
```c
// Default values per page/encoder combination
const float defaultValues[64] = {
    // Page 0: Oscillator defaults
    0.5f, 0.0f, 0.0f, 0.0f, 0.25f, 0.0f, 1.0f, 0.0f,
    // ... etc for all 8 pages
};
```

### Step 2: Implement Value Loading Function
```c
void loadPageValues(int page) {
    for (int i = 0; i < 8; i++) {
        int arrayIndex = page * 8 + i;
        float storedValue = param_table->array[page][attr_firstparam + i];
        
        // If value is uninitialized (0), use default
        if (storedValue == 0) {
            param_table->array[page][attr_firstparam + i] = defaultValues[arrayIndex];
        }
    }
}
```

### Step 3: Centralize Value Updates
```c
void updateEncoderValue(int encoderIndex, float newValue, int page) {
    int arrayIndex = page * 8 + encoderIndex;
    int currentMode = encoderModes[arrayIndex];
    
    // Apply constraints
    float constrainedValue = applyModeConstraints(newValue, currentMode, encoderIndex);
    
    // Store in param_table (single source of truth)
    param_table->array[page][attr_firstparam + encoderIndex] = constrainedValue;
    
    // Update dial display
    dials[encoderIndex].current_value = constrainedValue;
    parent->instancesketchy__render_i.dial_update(&dials[encoderIndex]);
}
```

### Step 4: Optimize Page Switching
```c
void switchToPage(int newPage) {
    if (newPage == lastA) return; // No change needed
    
    // Load values for new page
    loadPageValues(newPage);
    
    // Update dial configurations
    for (int i = 0; i < 8; i++) {
        updateDialConfig(i, newPage);
    }
    
    lastA = newPage;
}
```

## Expected Outcomes

1. **Consistent Behavior**: Values persist correctly across page changes
2. **Proper Defaults**: Each encoder has appropriate default values per page
3. **Performance**: Reduced redundant operations
4. **Maintainability**: Clearer, more consistent code structure
5. **User Experience**: Reliable, predictable behavior

## Risk Mitigation

1. **Backward Compatibility**: Ensure existing patches continue to work
2. **Testing Strategy**: Test all mode combinations across all pages
3. **Gradual Rollout**: Implement changes incrementally to isolate issues
4. **Fallback Plan**: Keep current version as backup during transition

## Clear Goals Summary

### Primary Goals:
1. **Relative Encoder Tracking**: When page switches, encoders must know their current position on the new page to apply relative changes correctly
2. **Value Persistence**: Each page maintains independent stored values that survive page switching
3. **Configuration Application**: Each page applies its own mode/min/max/default settings to the same 8 physical encoders
4. **Proper Initialization**: When switching to a page for the first time, apply default values

### Technical Requirements:
1. **On Page Switch**: 
   - Load stored values from `param_table->array[new_page][encoder_index]`
   - If value is uninitialized (or 0), use configured default value
   - Update local encoder position variables to match loaded values
   - Apply new page's mode constraints and min/max limits

2. **On Encoder Movement**:
   - Apply relative change to current position
   - Respect current page's mode constraints (frac/int/selector)
   - Store updated value back to param_table immediately
   - Update display via dial system

3. **Configuration System**:
   - Mode arrays define behavior per page/encoder combination
   - Min/max arrays define limits for INT/SELECTOR modes  
   - Default value arrays define initial values per page/encoder
   - Label arrays define display text per page/encoder

### Success Criteria:
- ✅ Encoder positions persist correctly across page switches
- ✅ Default values are applied appropriately for new pages
- ✅ Relative encoder movement works smoothly after page switches
- ✅ Each page maintains independent 8-encoder configurations
- ✅ Display updates reflect correct values and labels per page

This clarified strategy ensures the encoder object properly handles the page-based configuration system while maintaining value persistence and relative encoder tracking. 