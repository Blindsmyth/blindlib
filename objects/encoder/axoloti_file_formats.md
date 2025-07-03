# Axoloti File Format Reference

## CRITICAL: File Structure Differences

### .axo files (Object Definitions)
- Start with `<object>` root element
- NO `<patch>` or `<patchobj>` wrapper
- Used for creating reusable objects
- Example structure:
```xml
<object id="patch/object" uuid="unique-uuid">
   <sDescription>...</sDescription>
   <author>...</author>
   <inlets/>
   <outlets/>
   <code.declaration><![CDATA[...]]></code.declaration>
   <code.init><![CDATA[...]]></code.init>
   <code.krate><![CDATA[...]]></code.krate>
</object>
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

### Common Mistake
❌ Creating .axo files with `<patch><patchobj>` structure (causes XML parsing errors)
✅ .axo files must ONLY contain the `<object>` element

### Memory Aid
- .a**x**o = o**bject** = `<object>` root
- .a**x**p = **patch** = `<patch>` root 