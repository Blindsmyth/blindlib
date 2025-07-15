# Sketchy Object Naming Conventions

## Overview

This document defines the naming conventions used throughout the sketchy object system to ensure consistency and prevent conflicts.

## Object ID to Instance Name Mapping

### Core Pattern
Object IDs follow this pattern: `sketchy_[name]` → `instancesketchy__[name]_i`

### Examples
- `sketchy_render` → `instancesketchy__render_i`
- `sketchy_dials` → `instancesketchy__dials_i`
- `sketchy_objects` → `instancesketchy__objects_i`
- `sketchy_texts` → `instancesketchy__texts_i`
- `sketchy_params` → `instancesketchy__params_i`
- `sketchy_font` → `instancesketchy__font_i`

### Shared/Variant Objects
For shared or variant objects, use unique object IDs but reference them with original names:
- Object ID: `sketchy_render_shared` → Reference: `instancesketchy__render_i`
- Object ID: `sketchy_dials_shared` → Reference: `instancesketchy__dials_i`
- Object ID: `sketchy_render_exp` → Reference: `instancesketchy__render_i`
- Object ID: `sketchy_dials_exp` → Reference: `instancesketchy__dials_i`

**CRITICAL**: New objects have unique IDs but are referenced by the original names to maintain compatibility.

## Type Declaration Conventions

### Core Pattern
Type declarations use: `rootc::instancesketchy__[name]::[type]_t`

### Examples
- `rootc::instancesketchy__render::dial_t`
- `rootc::instancesketchy__render::intdisplay_t`
- `rootc::instancesketchy__render::select_t`
- `rootc::instancesketchy__render::bar_t`
- `rootc::instancesketchy__render_shared::dial_state_t`

## Table Access Conventions

### Core Pattern
Table access uses: `parent->instancesketchy__[name]_i`

### Examples
- `parent->instancesketchy__objects_i`
- `parent->instancesketchy__texts_i`
- `parent->instancesketchy__params_i`
- `parent->instancesketchy__font_i`

## Function Call Conventions

### Core Pattern
Function calls use: `parent->instancesketchy__[name]_i.[function_name]()`

### Examples
- `parent->instancesketchy__render_i.dial_init(&dial)`
- `parent->instancesketchy__render_i.dial_update(&dial)`
- `parent->instancesketchy__render_i.update_dial_position(index, &state)`  // Even for shared objects!

## Important Rules

### 1. Never Change Existing Object IDs
- Existing object IDs like `sketchy_render` and `sketchy_dials` must remain unchanged
- Many other objects depend on these exact names
- Changing them would break the entire ecosystem

### 2. Use Unique IDs for New Objects
- New objects should have unique IDs (e.g., `sketchy_render_shared`)
- This prevents conflicts with existing objects
- Allows both old and new objects to coexist

### 3. Follow the Pattern Consistently
- Object ID: `sketchy_[name]`
- Instance name: `instancesketchy__[name]_i`
- Type namespace: `rootc::instancesketchy__[name]::`

### 4. Double Underscore in Type Namespaces
- Type declarations use double underscore: `rootc::instancesketchy__render::`
- Instance access uses single underscore: `parent->instancesketchy__render_i`

## Common Mistakes to Avoid

### ❌ Don't Do This
```c
// Wrong: Changing existing object ID
<obj.normal id="sketchy_render" uuid="...">  // This breaks existing patches

// Wrong: Using different reference names for new objects
parent->instancesketchy__render_shared_i  // Should be instancesketchy__render_i

// Wrong: Missing underscore
rootc::instancesketchy_render::dial_t     // Should be instancesketchy__render
```

### ✅ Do This Instead
```c
// Correct: Keep existing IDs unchanged
<obj.normal id="sketchy_render" uuid="...">  // Original stays the same

// Correct: New object with unique ID
<obj.normal id="sketchy_render_shared" uuid="...">  // New shared version

// Correct: Use original reference names for compatibility
parent->instancesketchy__render_i  // Same as original, even for new objects

// Correct: Proper type namespace
rootc::instancesketchy__render::dial_state_t  // Use original namespace
```

## Migration Strategy

### For New Objects
1. Choose a unique object ID (e.g., `sketchy_render_shared`)
2. **BUT reference it with original names**: `instancesketchy__render_i` (not `instancesketchy__render_shared_i`)
3. **Use original type namespaces**: `rootc::instancesketchy__render::` (not `rootc::instancesketchy__render_shared::`)

**MEMORY AID**: "New object ID, old reference names"

### For Existing Objects
1. Never change existing object IDs
2. Maintain backward compatibility
3. If improvements are needed, create new objects with unique IDs

## Ecosystem Compatibility

### Existing Dependencies
Many objects depend on the exact naming:
- `8encoder_input.axo` → `parent->instancesketchy__dials_i`
- `sketchy_dials.axo` → `parent->instancesketchy__render_i`
- `int_param_get.axo` → `parent->instancesketchy__params_i`

### Breaking Changes
Changing existing object IDs would break:
- All existing patches
- All objects that reference the original names
- The entire sketchy ecosystem

## Summary

1. **Object IDs are sacred** - never change existing ones
2. **New objects get unique IDs** - `sketchy_render_shared`, `sketchy_dials_shared`
3. **BUT reference with original names** - `instancesketchy__render_i`, `instancesketchy__dials_i`
4. **Use original type namespaces** - `rootc::instancesketchy__render::`
5. **Preserve ecosystem** - existing objects must continue to work

**CRITICAL RULE**: "New object ID, old reference names" - This prevents compatibility issues.

This ensures the sketchy system remains stable and extensible while allowing new optimizations and features to be added safely. 