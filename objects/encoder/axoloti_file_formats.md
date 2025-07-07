# Axoloti File Format Reference

## CRITICAL: File Structure Differences

### .axo files (Object Definitions)
- Start with `<objdefs appVersion="1.0.12">` root element
- Contains `<obj.normal id="name" uuid="unique-uuid">` element
- NO `<patch>` or `<patchobj>` wrapper
- Used for creating reusable objects
- Example structure:
```xml
<objdefs appVersion="1.0.12">
   <obj.normal id="object_name" uuid="unique-uuid">
      <sDescription>...</sDescription>
      <author>...</author>
      <inlets/>
      <outlets/>
      <code.declaration><![CDATA[...]]></code.declaration>
      <code.init><![CDATA[...]]></code.init>
      <code.krate><![CDATA[...]]></code.krate>
   </obj.normal>
</objdefs>
```

### .axp files (Patch Files) 
- Start with `<patch>` root element
- Contains object instances that reference .axo objects
- Used for complete patches/projects
- **CRITICAL**: Object references use `type="directory/object_name"` format
- **REQUIRED**: All objects MUST have `name`, `x`, and `y` attributes
- Example structure:
```xml
<patch appVersion="1.0.12">
   <!-- Standard objects use type="category/name" -->
   <obj type="ctrl/dial p" uuid="..." name="page_input" x="98" y="14">
      <params/>
      <attribs/>
   </obj>
   
   <!-- Custom objects in encoder directory -->
   <obj type="encoder/template_renderer" uuid="..." name="renderer" x="280" y="14">
      <params/>
      <attribs/>
   </obj>
   
   <!-- Patch objects (embedded definitions) -->
   <patchobj type="patch/object" uuid="..." name="..." x="..." y="...">
      <params/>
      <attribs/>
      <object id="patch/object" uuid="...">
         <!-- Embedded object definition -->
      </object>
   </patchobj>
   
   <nets>
      <net>
         <source obj="page_input" outlet="out"/>
         <dest obj="renderer" inlet="page"/>
      </net>
   </nets>
</patch>
```

### Common Mistakes

#### .axo Object Files
❌ Creating .axo files with `<patch><patchobj>` structure (causes XML parsing errors)
❌ Using `<object>` root element (missing required wrapper)
✅ .axo files must use `<objdefs><obj.normal>` structure

#### .axp Patch Files
❌ Missing `x`, `y`, or `name` attributes on objects (causes "ValueRequiredException")
❌ Using `type="patch/object"` for custom objects (should be `type="directory/object_name"`)
❌ Incorrect object type paths (must match directory structure)
✅ All objects must have positioning: `name="..." x="280" y="14"`
✅ Custom objects use directory path: `type="encoder/template_renderer"`
✅ Standard objects use category: `type="ctrl/dial p"`

### Memory Aid
- .a**x**o = o**bject** definition = `<objdefs><obj.normal>` structure
- .a**x**p = **patch** = `<patch><obj type="...">` structure

### Error Debugging
- **"ValueRequiredException" for field 'x'**: Missing `x="..."` attribute on object
- **"ValueRequiredException" for field 'y'**: Missing `y="..."` attribute on object  
- **Object not found**: Check `type="directory/object_name"` path matches file location
- **XML parsing error**: Wrong root element or structure for file type 