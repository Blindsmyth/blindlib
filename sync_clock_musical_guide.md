# sync_clock_musical - Usage Guide

## Overview

`sync_clock_musical.axo` is a drop-in replacement for `sync clock dev 8` with **musical tempo interpretation** instead of arbitrary sample ranges.

**UUID:** `5224773f-18de-4231-8f92-b1f22bb9539E`  
**Location:** `/Users/simon/Dropbox/blindlib/objects/clock/sync_clock_musical.axo`

---

## Key Improvements Over sync clock dev 8

### What's BETTER:

✅ **Musical BPM Interpretation**
- Instead of `minDuration=3000, maxDuration=9000` (arbitrary samples)
- Uses `lowerBPM=90` (musical tempo threshold)
- Example: A 1-bar 89 BPM loop → automatically becomes 2-bar 178 BPM

✅ **Time Signature Awareness**
- `beats` inlet: numerator (e.g., 3 in 3/4, 4 in 4/4)
- `measure` inlet: denominator (e.g., 4 in 3/4, 8 in 7/8)
- Correctly calculates ppq per bar: 72 ppq for 3/4, 96 ppq for 4/4

✅ **BPM Display & Control**
- `bpm` outlet: current tempo in beats per minute
- `bpmValue` inlet + `setBPM` trigger: manually set tempo
- `detectedBars` outlet: how many bars the loop was interpreted as

✅ **Smart Bar Quantization**
- Automatically rounds to: 1, 2, 4, 8, or 16 bars
- Based on actual musical tempo, not arbitrary samples
- Respects time signature

### What's the SAME:

✅ All RBRT slot functionality (leader/follower system)  
✅ All sync inlets/outlets (syncPhase, syncDurSMPS, rollover)  
✅ Play/stop control with auto-start  
✅ 24ppq clock generation  
✅ External sync support  
✅ Quantization options  

---

## Inlet/Outlet Reference

### NEW Inlets (compared to sync clock dev 8):

| Inlet | Type | Description |
|-------|------|-------------|
| `beats` | int32 | Time signature numerator (e.g., 4 in 4/4, 3 in 3/4) |
| `measure` | int32 | Time signature denominator (e.g., 4 in 4/4, 8 in 7/8) |
| `lowerBPM` | int32 | Minimum tempo threshold (e.g., 90). Loops below this get doubled. |
| `setBPM` | bool32.rising | Trigger to manually set BPM |
| `bpmValue` | int32 | Manual BPM value (e.g., 120) |

### NEW Outlets (compared to sync clock dev 8):

| Outlet | Type | Description |
|--------|------|-------------|
| `bpm` | int32 | Current tempo in beats per minute |
| `detectedBars` | int32 | Number of bars the loop was interpreted as (1, 2, 4, 8, or 16) |
| `ppqPerBar` | int32 | 96th notes per bar based on time signature |

### EXISTING (same as sync clock dev 8):

All the standard RBRT sync inlets/outlets remain:
- `toSlot`, `setNow`, `inSync`, `setTempo`, `resetPhase`
- `syncSlot`, `syncPhase`, `syncDurSMPS`, `rollover`
- `clock24ppq`, `reset`, `isPlaying`
- etc.

---

## How the Musical Logic Works

### 1. Loop Recording & Interpretation

When you record a loop and set it as leader (`setNow` or `inSync`):

```c
// Example: Record a loop
loop_samples = 53333 // samples recorded
beats = 4            // 4/4 time
measure = 4
lowerBPM = 90        // minimum tempo

// Calculate samples per bar at lower BPM
quarter_note_duration = 60.0 / 90 = 0.667 seconds
samples_per_bar = 0.667 * 48000 * 4 = 32,000 samples

// Calculate bars
calculated_bars = 53333 / 32000 = 1.67 bars
// Round to valid bar count
detected_bars = 2  // rounds up to next power of 2

// Calculate actual BPM
BPM = (60 * 48000 * 4) / (53333 / 2) = 216 BPM
```

**Result:** Your loop is interpreted as **2 bars at 216 BPM**

### 2. Time Signature Impact

**4/4 Time (most common):**
- 4 beats per bar
- 96 ppq per bar (24ppq × 4 quarter notes)
- Standard rock/pop/electronic

**3/4 Time (waltz):**
- 3 beats per bar
- 72 ppq per bar (24ppq × 3 quarter notes)
- The loop is interpreted with 3-beat bars

**7/8 Time (odd meter):**
- 7 eighth notes per bar = 3.5 quarter notes
- 84 ppq per bar (24ppq × 3.5 quarter notes)
- Common in prog/metal

### 3. Lower BPM Threshold

The `lowerBPM` inlet prevents unrealistically slow tempos:

**Example with lowerBPM = 90:**
- Record 1-bar loop at 85 BPM → interpreted as **2 bars at 170 BPM** ✅
- Record 1-bar loop at 45 BPM → interpreted as **4 bars at 180 BPM** ✅
- Record 1-bar loop at 120 BPM → stays as **1 bar at 120 BPM** ✅

This keeps everything musically playable and in-tempo.

---

## Integration with 4 Track Dev

### Step 1: Replace sync clock dev 8

In your patch, find:
```xml
<obj type="rbrt_new/smplr/sync clock dev 8" uuid="..." name="sync_1" x="..." y="...">
```

Replace with:
```xml
<obj type="clock/sync_clock_musical" uuid="5224773f-18de-4231-8f92-b1f22bb9539E" name="sync_1" x="..." y="...">
   <params/>
   <attribs>
      <combo attributeName="setTempoTo" selection="cycles"/>
      <combo attributeName="quantization" selection="auto"/>
      <spinner attributeName="defaultBeats" value="4"/>
      <spinner attributeName="defaultMeasure" value="4"/>
      <spinner attributeName="defaultLowerBPM" value="90"/>
   </attribs>
</obj>
```

### Step 2: Add Time Signature Controls

Add UI controls for time signature:

```xml
<!-- Beats control (numerator) -->
<obj type="ctrl/i" uuid="..." name="beats_ctl" x="..." y="...">
   <params>
      <int32 name="value" value="4"/>
   </params>
</obj>

<!-- Measure control (denominator as radio) -->
<obj type="ctrl/i radio 4 h" uuid="..." name="measure_ctl" x="..." y="...">
   <params>
      <int32.hradio name="value" value="2"/>  <!-- 0=1, 1=2, 2=4, 3=8 -->
   </params>
</obj>
<obj type="phi/math/exp i" uuid="..." name="exp_measure" x="..." y="..."/>

<!-- Lower BPM threshold -->
<obj type="ctrl/i" uuid="..." name="lower_bpm" x="..." y="...">
   <params>
      <int32 name="value" value="90"/>
   </params>
</obj>
```

### Step 3: Connect Time Signature

```xml
<net>
   <source obj="beats_ctl" outlet="out"/>
   <dest obj="sync_1" inlet="beats"/>
</net>
<net>
   <source obj="exp_measure" outlet="out"/>
   <dest obj="sync_1" inlet="measure"/>
</net>
<net>
   <source obj="lower_bpm" outlet="out"/>
   <dest obj="sync_1" inlet="lowerBPM"/>
</net>
```

### Step 4: Add BPM Display

```xml
<!-- BPM display -->
<obj type="disp/i" uuid="..." name="bpm_display" x="..." y="...">
   <params/>
</obj>

<net>
   <source obj="sync_1" outlet="bpm"/>
   <dest obj="bpm_display" inlet="in"/>
</net>

<!-- Or send to OLED -->
<obj type="tiar/conv/i_to_c" uuid="..." name="bpm_to_text" x="..." y="...">
   <params/>
   <attribs>
      <table attributeName="prefix" table="BPM"/>
   </attribs>
</obj>

<net>
   <source obj="sync_1" outlet="bpm"/>
   <dest obj="bpm_to_text" inlet="i"/>
</net>
<net>
   <source obj="bpm_to_text" outlet="o"/>
   <dest obj="OLED128x64niceWOScope_1" inlet="line1"/>
</net>
```

### Step 5: Optional Manual BPM Control

```xml
<!-- BPM value control -->
<obj type="ctrl/i" uuid="..." name="manual_bpm" x="..." y="...">
   <params>
      <int32 name="value" value="120"/>
   </params>
</obj>

<!-- Set BPM button -->
<obj type="ctrl/button" uuid="..." name="set_bpm_btn" x="..." y="...">
   <params>
      <bool32.mom name="b" value="0"/>
   </params>
</obj>

<net>
   <source obj="manual_bpm" outlet="out"/>
   <dest obj="sync_1" inlet="bpmValue"/>
</net>
<net>
   <source obj="set_bpm_btn" outlet="o"/>
   <dest obj="sync_1" inlet="setBPM"/>
</net>
```

---

## Usage Examples

### Example 1: Standard 4/4 Looper

**Setup:**
- beats = 4
- measure = 4
- lowerBPM = 90

**Scenario:**
1. Record first loop (5 seconds = 240,000 samples)
2. Object calculates: ~2 bars at 120 BPM
3. BPM outlet shows: 120
4. detectedBars outlet shows: 2
5. Clock starts automatically
6. All subsequent loops sync to this 2-bar, 120 BPM master

### Example 2: 3/4 Waltz

**Setup:**
- beats = 3
- measure = 4
- lowerBPM = 80

**Scenario:**
1. Record first loop (4 seconds = 192,000 samples)
2. Object calculates: ~2 bars of 3/4 at 90 BPM
3. ppqPerBar = 72 (instead of 96)
4. 24ppq clock adjusts to 3-beat bars
5. All loopers follow 3/4 time

### Example 3: Manual BPM Override

**Setup:**
- Record a loop at whatever tempo
- Detected as 4 bars at 135 BPM
- You want exactly 140 BPM

**Action:**
1. Set `manual_bpm` control to 140
2. Press `set_bpm_btn`
3. Loop stretches/compresses to 140 BPM
4. Still 4 bars, but now at exact tempo

---

## Attributes (Object Settings)

Set these in the object's `<attribs>` section:

| Attribute | Default | Range | Description |
|-----------|---------|-------|-------------|
| `defaultBeats` | 4 | 1-16 | Default time signature numerator if inlet not connected |
| `defaultMeasure` | 4 | 1-16 | Default time signature denominator if inlet not connected |
| `defaultLowerBPM` | 90 | 40-180 | Default lower BPM limit if inlet not connected |
| `setTempoTo` | cycles | cycles/raw | Same as sync clock dev 8 |
| `quantization` | auto | auto/1/2/4/8/16/32 | Same as sync clock dev 8 |

---

## Troubleshooting

### Problem: BPM is way too high/low

**Cause:** Wrong time signature or lowerBPM setting  
**Solution:** 
- Check `beats` and `measure` inlets
- Adjust `lowerBPM` threshold
- Remember: 3/4 has 3 beats, not 4

### Problem: Loops not staying in sync

**Cause:** Probably same as sync clock dev 8 issues  
**Solution:**
- Check all `loopsync` objects are connected to `syncPhase`
- Verify `syncDurSMPS` is connected
- Make sure play/stop states are correct

### Problem: BPM outlet shows 0 or weird value

**Cause:** No leader set yet, or detected_bars = 0  
**Solution:**
- Record first loop and trigger `setNow`
- Check that loop has valid length (not too short)
- Defaults to 120 BPM if calculation fails

### Problem: Want odd time signatures (5/4, 7/8, etc.)

**Solution:**
- beats = 5, measure = 4 for 5/4
- beats = 7, measure = 8 for 7/8
- beats = 13, measure = 16 for 13/16
- ppqPerBar will calculate automatically

---

## Comparison: Old vs New

### Old Way (sync clock dev 8):
```
Record loop → 6500 samples
minDuration = 3000
maxDuration = 9000
Result: Accepted, but what tempo is it? 🤷
```

### New Way (sync_clock_musical):
```
Record loop → 53333 samples
beats = 4, measure = 4, lowerBPM = 90
Result: 2 bars at 120 BPM 🎵
```

**The new way is MUSICAL!**

---

## Next Steps

1. **Test in simple patch:**
   - One looper
   - sync_clock_musical
   - BPM display
   - Record and verify BPM calculation

2. **Integrate into 4-track dev:**
   - Replace sync clock dev 8
   - Add time signature controls
   - Add BPM display to OLED
   - Test multi-track sync

3. **Experiment with time signatures:**
   - Try 3/4 waltz
   - Try 7/8 odd meter
   - Try 5/4 prog rock

4. **Add to encoder dial system:**
   - Map BPM to dial for quick adjust
   - Map time signature to another dial
   - Update sketchy_params display

---

## Files Created

1. **sync_clock_musical.axo** - The main object
   - Location: `/Users/simon/Dropbox/blindlib/objects/clock/sync_clock_musical.axo`
   - UUID: `5224773f-18de-4231-8f92-b1f22bb9539E`

2. **tap.io.extended.axo** - Alternative approach (not used in this integration)
   - Location: `/Users/simon/Dropbox/blindlib/objects/clock/tap.io.extended.axo`
   - UUID: `5224773f-18de-4231-8f92-b1f22bb9539D`

3. **Documentation:**
   - `clock_engine_integration_analysis.md` - Full analysis
   - `sync_clock_musical_guide.md` - This file

---

## Questions?

The object is designed to be a drop-in replacement, so most connections should work as-is. The main additions are:
- Connect `beats`, `measure`, `lowerBPM` inlets
- Read `bpm`, `detectedBars` outlets
- Display/use BPM as needed

**Happy looping! 🎵**


