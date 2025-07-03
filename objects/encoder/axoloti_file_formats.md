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
- Contains `<patchobj>` elements that reference objects
- Used for complete patches/projects
- Example structure:
```xml
<patch appVersion="1.0.12">
   <patchobj type="patch/object" uuid="..." name="..." x="..." y="...">
      <params/>
      <attribs/>
   </patchobj>
   <nets>...</nets>
</patch>
```

### Common Mistakes
❌ Creating .axo files with `<patch><patchobj>` structure (causes XML parsing errors)
❌ Using `<object>` root element (missing required wrapper)
✅ .axo files must use `<objdefs><obj.normal>` structure

### Memory Aid
- .a**x**o = o**bject** definition = `<objdefs><obj.normal>` structure
- .a**x**p = **patch** = `<patch><patchobj>` structure 