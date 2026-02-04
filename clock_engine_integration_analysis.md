# Clock/Tempo Engine Integration Analysis
## 4 Track Dev Sim 0_4_9 vs Clock Barlength Stuff Exp 1_1_22

Date: October 16, 2025

## Executive Summary

You have two different clock/tempo systems:
1. **tap.io** - A standalone clock engine with MIDI sync, tap tempo, and manual BPM control
2. **sync clock dev 8** - An integrated looper synchronization system designed for multi-track sample playback

**Key Question:** How to integrate tap.io's superior tempo/MIDI features into the 4-track dev's RBRT sync system?

---

## Detailed Comparison

### 1. tap.io Clock Engine (Clock Barlength Patch)

**Location:** `/Users/simon/Dropbox/blindlib/objects/clock/tap.io.axo`

**Core Features:**
- ✅ **MIDI Clock Sync** - Receives and measures MIDI timing clock
- ✅ **Tap Tempo** - Calculate BPM from tapped intervals
- ✅ **Manual BPM Control** - +1/-1 BPM, half/double tempo
- ✅ **Parameter Table Integration** - BPM input/output with change detection
- ✅ **Multiple Sync Modes:**
  - Mode 0: Auto (clock dropout → internal)
  - Mode 1: Auto Special (MIDI stop → internal)
  - Mode 2: Internal only
  - Mode 3: External only
- ✅ **Time Signature Support** - Beats/measure inputs
- ✅ **Loop Length Analysis** - Converts sample counts to bars/beats
- ✅ **Smart Phase Generation** - Bar-accurate phasor based on 24ppq

**Key Inlets:**
```
- reset, play, start, stop (transport control)
- samples (loop length in samples)
- factor (96th notes multiplier)
- bpmPlus, bpmMinus, half, double (manual tempo)
- beats, measure (time signature)
- syncmode, waitforstart (MIDI behavior)
- bpm (param table input)
```

**Key Outlets:**
```
- slave (external clock active?)
- bpm (current tempo)
- 24ppq (clock pulse)
- start (reset pulse)
- 24ppqsmp (samples per 24ppq tick)
- barsamples (samples per bar)
- playing (transport state)
- bpmChanged (pulse for param updates)
```

**Implementation Highlights:**
- **MIDI Handler:** Direct `code.midihandler` for MIDI_TIMING_CLOCK, MIDI_START, MIDI_STOP
- **Clock Measurement:** Rolling average of 96 MIDI clock intervals
- **Internal Oscillator:** Phase accumulator for internal clock generation
- **BPM Calculation:** `48000.0 / currentAverage * 2.5`
- **Seamless Switching:** Auto-detects clock dropout after 12000 samples (~250ms)

---

### 2. sync clock dev 8 (4 Track Dev Patch)

**Location:** `/Users/simon/Dropbox/Axolonatics/objects/rbrt_new/smplr/sync clock dev 8.axo`

**Core Features:**
- ✅ **Multi-Slot Synchronization** - Coordinates multiple loops
- ✅ **Auto-Start** - Clock starts when first loop recorded
- ✅ **Play/Stop Control** - Transport with counter reset
- ✅ **Quantization** - Auto-quantize loop lengths (1/1, 1/2, 1/4, etc.)
- ✅ **External Sync Input** - Accept external phase/duration
- ✅ **Shared Memory** - Uses RBRT Smplr.h and sync.h classes
- ✅ **24ppq Generation** - Calculated from phase position

**Key Inlets:**
```
- toSlot (which loop to sync to)
- setNow, inSync (sync triggers)
- setTempo, tempo (manual tempo setting)
- resetPhase (reset counter)
- play, stop (transport)
- EXTsync, EXTphase, EXTdurSMPS (external sync)
```

**Key Outlets:**
```
- syncSlot (current master loop)
- syncPhase (position in bar)
- syncDurSMPS (bar length in samples)
- rollover (bar boundary pulse)
- clock24ppq (clock pulse)
- reset (counter reset)
- isPlaying (transport state)
```

**Implementation Highlights:**
- **RBRT Integration:** Uses `Smplr smplr` and `SYNC sync` objects
- **Leader System:** One loop becomes "leader" and sets tempo for all
- **Bar Calculation:** `sync.calc_bars(stempo, smplr.leader_tempo)`
- **Phase Generation:** `(float(pos) / smplr.leader_tempo) * (1 <<27)`
- **24ppq from Phase:** `(sync.syncer_pos * 96) >> 27`
- **No MIDI Handler:** No direct MIDI clock input

---

## What's Missing in Each System

### tap.io Lacks:
- ❌ Multi-loop coordination
- ❌ Shared memory structures for multiple tracks
- ❌ Auto-quantization
- ❌ Loop leader system
- ❌ Integration with RBRT Smplr framework

### sync clock dev 8 Lacks:
- ❌ MIDI clock sync
- ❌ Tap tempo
- ❌ Manual BPM adjustment (+/-/half/double)
- ❌ Multiple sync modes
- ❌ BPM display/parameter integration
- ❌ Clock dropout detection
- ❌ Time signature awareness beyond bar length

---

## Integration Strategies

### Option 1: **Replace sync clock dev 8 with Enhanced tap.io** ⭐ RECOMMENDED

**Approach:** Extend tap.io to output the signals that sync clock dev 8 provides.

**Required Additions to tap.io:**
1. **Add outlets:**
   - `syncPhase` (frac32.positive) - Current position in bar (0 to 1<<27)
   - `syncDurSMPS` (int32.positive) - Bar length in samples
   - `rollover` (bool32.pulse) - Bar boundary trigger

2. **Phase calculation:**
```c
// In code.krate after 24ppq generation
static uint32_t barPos = 0;
static uint32_t prevBarPos = 0;
bool barRollover = false;

if (playing) {
    // Increment position by 16 samples per k-rate cycle
    barPos += 16;
    if (barPos >= barsamples) {
        barRollover = true;
        barPos = 0;
    }
    
    // Calculate phase (0 to 1<<27)
    outlet_syncPhase = (uint32_t)((float)barPos / barsamples * (1<<27));
    outlet_syncDurSMPS = barsamples;
    outlet_rollover = barRollover;
} else {
    outlet_syncPhase = 0;
    outlet_rollover = false;
}
```

3. **Connect to existing loopsync objects:**
   - tap.io `syncPhase` → `loopsync dev 14` `syncPhase` inlet
   - tap.io `syncDurSMPS` → `loopsync dev 14` `sync duration` inlet

**Pros:**
- ✅ Keep all tap.io features (MIDI, tap tempo, BPM control)
- ✅ Minimal changes to 4-track dev patch structure
- ✅ loopsync objects already designed to accept external phase
- ✅ No RBRT framework modifications needed

**Cons:**
- ⚠️ Bypasses RBRT Smplr leader system
- ⚠️ Need to test with multiple polyphonic loopers
- ⚠️ May need to adjust auto-start logic

---

### Option 2: **Add MIDI/Tap Features to sync clock dev 8**

**Approach:** Extend sync clock dev 8 with tap.io's MIDI and tap tempo code.

**Required Additions:**
1. Add MIDI handler code from tap.io
2. Add tap tempo measurement arrays
3. Add manual BPM control logic
4. Add sync mode selection
5. Modify tempo setting to accept:
   - MIDI clock timing
   - Tap intervals
   - Manual +/- adjustments

**Pros:**
- ✅ Keeps RBRT integration intact
- ✅ Maintains leader/follower system
- ✅ Auto-quantization preserved

**Cons:**
- ⚠️ More complex code modifications
- ⚠️ Need to reconcile leader tempo with external MIDI clock
- ⚠️ Risk of conflicts between RBRT tempo and tap tempo
- ⚠️ Harder to maintain

---

### Option 3: **Hybrid System with tap.io as Master**

**Approach:** Use tap.io for master clock generation, feed tempo into sync clock dev 8.

**Architecture:**
```
tap.io → tempo (bpm) → sync clock dev 8 → syncPhase → loopers
       → 24ppq                           → clock24ppq
       → playing                         → rollover
```

**Required Connections:**
1. tap.io `bpm` outlet → sync clock dev 8 `tempo` inlet
2. tap.io `playing` → sync clock dev 8 `play` inlet
3. tap.io `bpmChanged` → sync clock dev 8 `setTempo` inlet
4. Keep sync clock dev 8 for phase generation and looper coordination

**Pros:**
- ✅ Separation of concerns
- ✅ Easy to implement
- ✅ Both systems do what they're best at
- ✅ Can switch between systems easily

**Cons:**
- ⚠️ Two clock objects (more DSP usage)
- ⚠️ Need to ensure tempo sync stays tight
- ⚠️ BPM → samples conversion happening twice

---

## Recommended Implementation Plan

### Phase 1: Test tap.io Integration (LOW RISK)
1. Create `tap.io_extended.axo` with syncPhase/syncDurSMPS outlets
2. Create test patch with single looper
3. Verify phase accuracy and rollover timing
4. Test with MIDI clock sync
5. Test tap tempo functionality

### Phase 2: Replace in 4-Track Dev (MEDIUM RISK)
1. Backup current 4 track dev_sim 0_4_9.axp
2. Replace sync clock dev 8 with tap.io_extended
3. Connect outlets to existing loopsync objects
4. Add UI controls for:
   - Sync mode selector (i radio 4)
   - BPM display
   - +/- BPM buttons
   - Tap tempo button
   - Half/double tempo buttons
5. Test single track recording and playback
6. Test multi-track sync

### Phase 3: Add Clock Barlength Features (HIGH VALUE)
1. Add `LoopLengthToBars` object from Clock Barlength patch
2. Connect first loop recording length to bar calculator
3. Auto-set tap.io factor based on detected bars
4. Add bar phasor display
5. Add quantization selector for first loop

### Phase 4: Parameter Integration (POLISH)
1. Connect tap.io BPM to sketchy_params for display
2. Use encoder dial for BPM control
3. Add BPM to OLED display
4. Store/recall BPM with presets

---

## Code Template: tap.io_extended.axo

Here's the modification to add to tap.io:

```c
// Add to outlets:
<frac32.positive name="syncPhase" description="Current position in bar (0 to 1<<27)"/>
<int32.positive name="syncDurSMPS" description="Bar length in samples"/>
<bool32.pulse name="rollover" description="Bar boundary pulse"/>

// Add to code.declaration:
uint32_t barPos;
bool prevBarRollover;

// Add to code.init:
barPos = 0;
prevBarRollover = false;

// Add to code.krate (after barsamples calculation):
// Generate syncPhase output for looper synchronization
if (playing && barsamples > 0) {
    // Increment position (16 samples per k-rate at 48kHz)
    barPos += 16;
    
    // Handle rollover
    bool currentRollover = false;
    if (barPos >= barsamples) {
        currentRollover = true;
        barPos = barPos % barsamples; // Wrap around
    }
    
    // Calculate phase (0 to 1<<27 = full bar)
    outlet_syncPhase = (uint32_t)((double)barPos / barsamples * (1<<27));
    outlet_syncDurSMPS = barsamples;
    outlet_rollover = currentRollover && !prevBarRollover; // Pulse only on transition
    
    prevBarRollover = currentRollover;
} else {
    outlet_syncPhase = 0;
    outlet_syncDurSMPS = barsamples;
    outlet_rollover = false;
    barPos = 0;
    prevBarRollover = false;
}
```

---

## Testing Checklist

### Basic Functionality:
- [ ] tap.io generates stable 24ppq at 120 BPM
- [ ] syncPhase ramps 0 → (1<<27) smoothly
- [ ] rollover pulses exactly at bar boundaries
- [ ] BPM display matches set tempo

### MIDI Sync:
- [ ] Receives MIDI clock and calculates correct BPM
- [ ] Switches to internal clock on dropout
- [ ] MIDI start/stop work correctly
- [ ] External BPM updates parameter table

### Tap Tempo:
- [ ] Tapping 4 beats sets correct tempo
- [ ] Time signature affects bar length calculation
- [ ] Tap timeout works (returns to default)

### Manual Control:
- [ ] +1/-1 BPM buttons work
- [ ] Half/double buttons work
- [ ] Changes update parameter display

### Looper Integration:
- [ ] First loop records correctly
- [ ] Subsequent loops sync to first loop
- [ ] Overdubs stay in sync
- [ ] Quantization works with new clock

### Multi-Track:
- [ ] All 4 tracks stay in sync
- [ ] Polyphonic voices sync correctly
- [ ] Stop/start maintains sync
- [ ] Reset works across all tracks

---

## Questions to Consider

1. **Loop Length Detection:** How should the system handle the first loop recording?
   - Should it auto-detect bars like Clock Barlength patch?
   - Should user pre-set tempo before recording?
   - Should it use tap tempo before recording?

2. **Sync Mode Default:** Which sync mode should be default?
   - Mode 0 (Auto) for MIDI clock with fallback?
   - Mode 2 (Internal) for standalone use?

3. **BPM Range:** What are sensible min/max BPM limits?
   - Current Clock Barlength uses 90 BPM as lower limit
   - Typical range: 60-180 BPM? 40-240 BPM?

4. **Quantization:** Should tempo changes be quantized?
   - Immediate tempo change vs. wait for next bar?
   - How does this interact with running loops?

5. **Parameter Memory:** Where should BPM be stored?
   - In sketchy_params table?
   - Separate preset system?
   - Both?

---

## Next Steps

**IMMEDIATE:** I can create the `tap.io_extended.axo` file for you with the necessary modifications.

**THEN:** Test it in a simple patch before integrating into 4-track dev.

**FINALLY:** Create a new version of 4 track dev (0_5_0?) with the new clock engine.

Would you like me to proceed with creating the extended tap.io object?


