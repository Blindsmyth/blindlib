# Ring Optimization Strategy for Axoloti Object Systems

## Problem Statement

The core challenge was **object table overflow** in Axoloti patches due to excessive ring objects:
- 8 encoders × 8 pages = 64+ dial objects
- Each dial creates virtual knob rings (types 99/100) 
- Compilation fails when object limit exceeded
- Need to optimize "under the hood" while maintaining functionality

## Key Insight: Coordinate-Based Ring Reuse

### The Breakthrough Concept
User's key insight: **"cant we have 1 ring fixed that we call per element and then align using the x and y coordinates?"**

This revealed that since x,y coordinates for dials already exist in the system, separate ring objects for each dial are redundant. A single shared ring could be positioned dynamically using existing coordinate data.

### Hard-Coded Coordinate Discovery
Found existing coordinate arrays in the system:
- `circle_coords[38][2]` - predefined circle positions
- `indicator_coords[31][2]` - predefined indicator positions

This means the layout geometry is already solved - we just need to reuse it efficiently.

## Strategic Approaches Explored

### 1. Complex Reuse System (Failed)
**Concept:** Dynamic object type switching with reuse pools
- Created `sketchy_render_reuse.axo` and `sketchy_dials_reuse.axo`
- Tried to prevent renderer from creating rings automatically
- **Failure:** Compilation errors due to struct incompatibilities

### 2. Shared Circles System (Partial)
**Concept:** 8 shared circle objects instead of 64+ individual rings
- Used type switching (101=shared ring, 102=indicator only)
- **Failure:** Struct type mismatches, complex state management

### 3. Ring Reuse with Dynamic Visibility (Complex)
**Concept:** 8 shared ring objects with dynamic positioning
- Type 101 rings positioned based on active fractional dials
- Reduced objects by 88% in theory
- **Failure:** Always-on ring creation in `dial_init`/`dial_update` functions

### 4. Simple Object-Per-Position (Final Attempt)
**Concept:** 8 objects (one per position) + 1 shared ring
- Object types switch based on configuration (dial/int/select/bar)
- Single ring positions itself at first fractional dial
- **Status:** Compiled but functionality issues reported

## Core Technical Challenges

### 1. Renderer Interference
The `sketchy_render.axo` renderer's `dial_init` and `dial_update` functions **always** create virtual knob objects (types 99/100) regardless of configuration. This conflicts with any shared ring optimization.

### 2. Struct Compatibility Issues
Multiple compilation failures due to:
- Missing struct members (`inlet_selected_dial`)
- Wrong function signatures (functions only take struct pointer)
- Incorrect field names (`w`/`h` vs `width`/`height`, `label` vs `param_label`)

### 3. Object Type Management
Axoloti's object system expects consistent object types. Dynamic switching between dial/int/select/bar objects proved complex due to different update requirements and struct layouts.

## Key Strategic Insights

### 1. The "8 Elements" Concept
**Core Goal:** "8 elements that could be a dial int, etc" with "the ring initialised only once from table and then be aligned per page according to xy coordinates."

This captures the essential optimization: fixed positions with dynamic content and shared visual elements.

### 2. Under-the-Hood Optimization
**Principle:** "keep the functionality of the old system and optimise it under the hood"

Users shouldn't notice any functional changes - only the internal object count should be reduced.

### 3. Coordinate Reuse is Key
Since coordinate systems already exist, the optimization should focus on:
- Reusing position calculations
- Sharing visual ring elements
- Avoiding duplicate ring creation

## Recommended Future Approaches

### Option A: Renderer Modification
Modify `sketchy_render.axo` to:
- Add shared ring support natively
- Prevent automatic ring creation for certain object types
- Use coordinate arrays more efficiently

### Option B: Custom Lightweight Renderer
Create a new renderer that:
- Uses hard-coded coordinate arrays
- Implements shared ring system from ground up
- Maintains API compatibility with existing dial configurations

### Option C: Object Pool System
Implement at the object level:
- Pre-allocate 8 position objects
- Pre-allocate 1-2 shared rings
- Dynamically assign content without creating new objects

## Technical Requirements for Success

### 1. Ring Positioning System
```c
// Single ring that moves based on active fractional dial
void position_shared_ring(int page, int position) {
    int x = COL_X[position % 4];
    int y = (position < 4) ? ROW1_Y : ROW2_Y;
    // Apply to shared ring object
}
```

### 2. Object Type Switching
```c
// Switch object behavior without creating new objects
void configure_position(int pos, config_t* config) {
    switch(config->mode) {
        case DIAL: setup_dial_behavior(pos, config); break;
        case INT:  setup_int_behavior(pos, config); break;
        // etc.
    }
}
```

### 3. Coordinate Reuse
```c
// Leverage existing coordinate arrays
extern const int circle_coords[38][2];
extern const int indicator_coords[31][2];
```

## Success Metrics

A successful implementation should achieve:
- **Object Count:** Reduce from 64+ to <16 total objects
- **Functionality:** 100% feature parity with original system
- **Performance:** No noticeable latency increase
- **Compatibility:** Works with existing patch files

## Lessons Learned

1. **Start Simple:** Complex dynamic systems often fail due to unexpected interactions
2. **Renderer Integration:** The renderer is the bottleneck - optimization must work with it, not against it
3. **Coordinate Reuse:** Existing coordinate systems are the key to efficient positioning
4. **Object Lifecycle:** Understanding when/how objects are created is crucial for optimization

## Next Steps

1. Analyze `sketchy_render.axo` dial functions to understand ring creation timing
2. Experiment with renderer modifications for shared ring support
3. Consider lightweight custom renderer as alternative path
4. Test coordinate reuse concepts in isolation before full integration

This strategy provides a foundation for future ring optimization attempts while preserving the core insights and avoiding repeated exploration of failed approaches. 