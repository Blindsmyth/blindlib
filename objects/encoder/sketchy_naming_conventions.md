# Sketchy System Naming Conventions

## CRITICAL RULE: Always use base names in dependencies and references

Even when using expanded objects (`sketchy_render_exp`, `sketchy_objects_exp`, etc.) in patches, the object definitions themselves must use the base naming conventions.

## Dependencies (in <depends> section)

**ALWAYS USE:**
```xml
<depend>sketchy_font</depend>
<depend>sketchy_params</depend>
<depend>sketchy_objects</depend>
<depend>sketchy_texts</depend>
<depend>sketchy_render</depend>
```

**NEVER USE:**
```xml
<depend>sketchy_objects_exp</depend>
<depend>sketchy_texts_exp</depend>
<depend>sketchy_render_exp</depend>
```

## Type References

**ALWAYS USE:**
```c
rootc::instancesketchy__render::dial_t *dials;
rootc::instancesketchy__render::intdisplay_t *intdisplays;
rootc::instancesketchy__render::select_t *selects;
rootc::instancesketchy__render::bar_t *bars;
```

**NEVER USE:**
```c
rootc::instancesketchy__render_exp::dial_t *dials;
```

## Function Calls

**ALWAYS USE:**
```c
parent->instancesketchy__render_i.dial_init(&dials[array_index]);
parent->instancesketchy__render_i.dial_update(&dials[array_index]);
parent->instancesketchy__objects_i.registerEntry();
parent->instancesketchy__texts_i.registerEntry();
parent->instancesketchy__params_i.array[page][param];
```

**NEVER USE:**
```c
parent->instancesketchy__render_exp_i.dial_init();
parent->instancesketchy__objects_exp_i.registerEntry();
```

## Why This Matters

1. **Consistency**: All sketchy objects use the same base naming pattern
2. **Compatibility**: Objects can work with both regular and expanded versions
3. **Compilation**: The Axoloti compiler expects these specific naming patterns
4. **Drop-in replacement**: Objects with consistent naming can replace each other

## Example: sketchy_dials vs sketchy_dials_exp

Both objects use identical naming conventions:
- Same dependencies (`sketchy_render`, not `sketchy_render_exp`)
- Same type references (`instancesketchy__render::dial_t`)
- Same function calls (`instancesketchy__render_i.dial_init()`)

The only difference is internal implementation (e.g., `maxpages` attribute, SDRAM allocation size).

## Memory Aid

**"Base names in code, expanded names in patches"**
- Object definitions (.axo files) = base names
- Patch files (.axp files) = can use expanded names 