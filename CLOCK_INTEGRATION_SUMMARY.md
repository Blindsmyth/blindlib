# Clock Engine Integration - Summary

**Date:** October 16, 2025  
**Task:** Integrate tap.io clock engine features into 4-track dev sync system

---

## Decision: Enhanced sync clock dev (Musical Approach) ✅

After analyzing both systems, we went with **Option 2: Add musical features to sync clock dev** because:
- ✅ Keeps RBRT slot system (multi-loop coordination)
- ✅ Keeps leader/follower mechanism
- ✅ Adds musical BPM-based interpretation
- ✅ True drop-in replacement for existing 4-track dev patch

---

## What Was Created

### 1. sync_clock_musical.axo ⭐ PRIMARY OBJECT

**Location:** `/Users/simon/Dropbox/blindlib/objects/clock/sync_clock_musical.axo`  
**UUID:** `5224773f-18de-4231-8f92-b1f22bb9539E`

**What it does:**
- Replaces `sync clock dev 8` with musical tempo logic
- Instead of arbitrary sample ranges (min/max duration), uses **lowerBPM threshold**
- Interprets loops based on **time signature** (beats/measure)
- Auto-quantizes to **1, 2, 4, 8, or 16 bars**
- Outputs **BPM** for display
- Supports **manual BPM override**

**Key Innovation:**
```
OLD: "Is my loop between 3000-9000 samples?" → boring
NEW: "My 89 BPM 1-bar loop becomes 178 BPM 2-bar" → musical!
```

### 2. tap.io.extended.axo (Alternative Approach)

**Location:** `/Users/simon/Dropbox/blindlib/objects/clock/tap.io.extended.axo`  
**UUID:** `5224773f-18de-4231-8f92-b1f22bb9539D`

This was the initial approach (add looper sync to tap.io) but we pivoted to the better solution. Kept for reference.

---

## New Features in sync_clock_musical

### NEW Inlets:
- `beats` - Time signature numerator (3 in 3/4, 4 in 4/4)
- `measure` - Time signature denominator (4 in 3/4, 8 in 7/8)
- `lowerBPM` - Minimum tempo threshold (e.g., 90 BPM)
- `setBPM` - Trigger to manually set BPM
- `bpmValue` - Manual BPM value

### NEW Outlets:
- `bpm` - Current tempo in BPM (for display/OLED)
- `detectedBars` - How many bars loop was interpreted as (1/2/4/8/16)
- `ppqPerBar` - 96th notes per bar (varies with time signature)

### SAME as sync clock dev 8:
- All RBRT slot management
- syncPhase, syncDurSMPS, rollover outputs
- 24ppq clock generation
- Play/stop control
- External sync support

---

## How to Integrate into 4 Track Dev

### Quick Integration Steps:

1. **Replace object:**
   ```xml
   <!-- OLD -->
   <obj type="rbrt_new/smplr/sync clock dev 8" uuid="6db88432-..." .../>
   
   <!-- NEW -->
   <obj type="clock/sync_clock_musical" uuid="5224773f-18de-4231-8f92-b1f22bb9539E" .../>
   ```

2. **Add time signature controls:**
   - `ctrl/i` for beats (4)
   - `ctrl/i radio 4 h` + `phi/math/exp i` for measure (4)
   - `ctrl/i` for lowerBPM (90)

3. **Connect them:**
   ```
   beats_ctl → sync_1.beats
   measure_exp → sync_1.measure
   lower_bpm → sync_1.lowerBPM
   ```

4. **Display BPM:**
   ```
   sync_1.bpm → disp/i or → tiar/conv/i_to_c → OLED
   ```

5. **Test!**

---

## Musical Examples

### 4/4 Time (Standard):
- Record 2-second loop → **1 bar at 120 BPM**
- Record 5-second loop → **2 bars at 115 BPM**

### 3/4 Time (Waltz):
- beats=3, measure=4
- Record 4-second loop → **2 bars at 90 BPM** (3 beats per bar)
- Clock outputs 72 ppq per bar (not 96)

### 7/8 Time (Odd Meter):
- beats=7, measure=8
- Record 3.5-second loop → **2 bars at 103 BPM**
- Clock outputs 84 ppq per bar

### Lower BPM Threshold:
- lowerBPM = 90
- Record 1-bar at 85 BPM → **auto-doubles to 2-bar at 170 BPM** ✅
- Keeps everything in playable tempo range

---

## Documentation Files

1. **clock_engine_integration_analysis.md**
   - Full analysis of both clock systems
   - Comparison of tap.io vs sync clock dev 8
   - Three integration strategies explored

2. **sync_clock_musical_guide.md** ⭐ READ THIS FIRST
   - Complete usage guide
   - Inlet/outlet reference
   - Integration instructions
   - Examples and troubleshooting

3. **CLOCK_INTEGRATION_SUMMARY.md** (this file)
   - Quick overview
   - What was created
   - Next steps

---

## UUID Registry Updated

`.cursorrules` file updated with new UUIDs:
- tap.io.extended.axo: `5224773f-18de-4231-8f92-b1f22bb9539D`
- sync_clock_musical.axo: `5224773f-18de-4231-8f92-b1f22bb9539E`

Next available UUID: `5224773f-18de-4231-8f92-b1f22bb9539F`

---

## Testing Checklist

Before integrating into full 4-track dev:

- [ ] Create simple test patch with single looper
- [ ] Test basic recording and BPM calculation
- [ ] Verify BPM display shows correct tempo
- [ ] Test 4/4 time signature
- [ ] Test 3/4 time signature  
- [ ] Test manual BPM override
- [ ] Test lower BPM threshold (record slow loop)
- [ ] Verify 24ppq clock output
- [ ] Test with multiple slots

After tests pass:

- [ ] Backup current 4 track dev_sim 0_4_9.axp
- [ ] Create 4 track dev_sim 0_5_0.axp with new clock
- [ ] Replace sync clock dev 8 with sync_clock_musical
- [ ] Add time signature UI controls
- [ ] Add BPM display to OLED/sketchy
- [ ] Test 4-track recording and sync
- [ ] Test overdubs stay in sync
- [ ] Test play/stop/reset

---

## Why This Is Better

### Before (sync clock dev 8):
```
Loop: ?????? samples
Min: 3000, Max: 9000
Result: "It's within range!" 
BPM: ??? (no idea)
```

### After (sync_clock_musical):
```
Loop: 53333 samples
Time: 4/4, Lower BPM: 90
Result: "2 bars at 120 BPM"
BPM: 120 ✅
```

**The system now thinks musically!** 🎵

---

## Next Steps

1. **Test the object** in a simple patch
2. **Integrate** into 4-track dev (create v0.5.0)
3. **Add UI** for time signature and BPM display
4. **Experiment** with different time signatures
5. **Add to encoder dial system** for live BPM adjustment

---

## Questions or Issues?

Read the detailed guide: `sync_clock_musical_guide.md`

The object is designed as a drop-in replacement, so most of your existing connections should work unchanged. The main additions are connecting the time signature inlets and reading the BPM outlet.

**Enjoy your musical looper!** 🎹🥁🎸


