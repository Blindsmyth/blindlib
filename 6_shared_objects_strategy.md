# 6 Shared Objects Strategy

## Overview

This strategy implements a memory-efficient shared object system using only 6 pre-registered objects (one for each dial type) that are dynamically repositioned rather than creating new objects for each dial position.

## Core Concept

Instead of creating 48+ objects (8 positions × 6 types), we create just 6 shared objects total and dynamically adjust their positions, types, and properties during updates.

## Type Definitions

Using the same type definitions as the dials object:

```c
#define MODE_FRAC_UNIPOLAR 0  // 0.0 to 1.0, positive only (dial)
#define MODE_FRAC_BIPOLAR  1  // -1.0 to 1.0, bipolar (dial)  
#define MODE_INT           2  // min to max, integer (intdisplay)
#define MODE_LIST          3  // selector mode (select)
#define MODE_BAR_UNIPOLAR  4  // 0.0 to 1.0, vertical bar (bar)
#define MODE_BAR_BIPOLAR   5  // -1.0 to 1.0, vertical bar (bar)
#define MODE_DUPLICATE     6  // duplicate another parameter (handled specially)
```

## Shared Object Structure

```c
typedef struct {
    uint16_t main_object_id;    // dial/bar/intdisplay/select
    uint16_t label_object_id;   // label background
    uint16_t label_text_id;     // label text
    uint16_t value_object_id;   // value background (for bars, intdisplays, selects)
    uint16_t value_text_id;     // value text (for bars, intdisplays, selects)
} shared_object_set_t;

shared_object_set_t shared_objects[6]; // One set per type (0-5)
```

## Memory Savings

### Original System
- 64 dials × 3 objects per dial = 192 objects
- 64 bars × 5 objects per bar = 320 objects  
- 64 intdisplays × 5 objects per intdisplay = 320 objects
- 64 selects × 5 objects per select = 320 objects
- **Total: ~1152 objects**

### 6 Shared Objects System
- 6 shared object sets × 5 objects per set = 30 objects
- **Total: 30 objects**
- **Memory reduction: ~97%**

## Implementation Strategy

### 1. Initialization
```c
void init_shared_objects() {
    for (int type = 0; type < 6; type++) {
        // Register all component objects for this type
        shared_objects[type].main_object_id = object_table->registerEntry();
        shared_objects[type].label_object_id = object_table->registerEntry();
        shared_objects[type].label_text_id = text_table->registerEntry();
        shared_objects[type].value_object_id = object_table->registerEntry();
        shared_objects[type].value_text_id = text_table->registerEntry();
        
        // Set up basic properties (type-specific)
        setup_shared_object_type(type);
    }
}
```

### 2. Update Pattern
```c
void update_dial(uint8_t page, uint8_t dial_index, uint8_t type) {
    // Calculate position
    int x = COL_X[dial_index % 4];
    int y = (dial_index < 4) ? ROW1_Y : ROW2_Y;
    
    // Update shared object positions and properties
    reposition_shared_object(type, x, y, page);
    update_shared_object_value(type, page, dial_index);
}
```

### 3. Position Calculation
```c
const uint8_t DIAL_WIDTH = 24;
const uint8_t DIAL_HEIGHT = 24;
const uint8_t ROW1_Y = 13;
const uint8_t ROW2_Y = 39;
const uint8_t COL_X[4] = {26, 52, 78, 104};
```

## Advantages

1. **Massive Memory Reduction**: 97% fewer objects
2. **Predictable Performance**: Fixed object count, no dynamic allocation
3. **Simple Updates**: Just reposition existing objects
4. **Type Compatibility**: Uses same type definitions as dials object
5. **Easy Maintenance**: Clear separation of concerns

## Challenges

1. **Sequential Updates**: Only one dial can be updated at a time (per type)
2. **State Management**: Need to track which dial is currently using each shared object
3. **Page Visibility**: Must ensure objects are only visible on correct pages

## Naming Convention

Following the established pattern:
- Object ID: `sketchy_render_shared` (unique)
- Instance name: `instancesketchy__render_i` (original)
- Type namespace: `rootc::instancesketchy__render` (original)

### Inlet/Outlet/Attribute/Parameter Names
- **UNDERSCORES ARE FORBIDDEN** in inlet/outlet/attribute/parameter names
- Use camelCase instead: `sequenceLength` not `sequence_length`
- Axoloti converts underscores to double underscores in generated code, causing compilation errors
- Examples: `sequencelength`, `trackoffset`, `isstep`, `currentlength`
- This is different from type namespaces which use double underscores: `rootc::instancesketchy__render::`

## Compatibility

This approach maintains full compatibility with:
- Existing dial configurations
- Parameter table structure  
- Object table interface
- Text table interface
- All existing type definitions

## Future Extensions

1. **Multiple Renderers**: Could support multiple renderer instances
2. **Dynamic Types**: Could add new dial types without changing core structure
3. **Performance Monitoring**: Easy to track object usage and performance 