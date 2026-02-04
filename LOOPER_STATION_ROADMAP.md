# Ksoloti Looper Station - Development Roadmap

## Current State (v1.5)

### ✅ Implemented Features

#### Core Looping Functionality
- **4-Layer Looper System** - Independent recording and playback for 4 layers
- **State Management** - 5 states per layer (Empty, Recording, Stopped, Playing, Overdubbing)
- **Layer Control** - Record, Stop, Reset, Select, Reverse, and Alt functions
- **BPM Synchronization** - Tap tempo integration with global BPM parameter system
- **Quantized Recording** - Beat-aligned recording start/stop

#### Hardware Interface
- **Trellis Controller Integration** - 32-button grid (8x4) with full button mapping
- **8-Encoder System** - Physical rotary encoders with 4x2 grid layout
- **OLED Display** - Visual feedback for encoder parameters
- **Page System** - Multiple parameter pages (8 pages supported)
- **Encoder Modes** - 6 different encoder types (Frac Unipolar/Bipolar, Integer, List, Bar, Duplicate)

#### Visual Feedback
- **LED Feedback System** - Dual-mode operation (Looper states / Toggle states)
- **State-to-Velocity Conversion** - MIDI velocity mapping for LED brightness
- **Shift Mode Switching** - Context-aware LED display
- **Real-time Updates** - Immediate visual feedback on state changes

#### Parameter Management
- **Parameter Storage System** - Persistent parameter table with page-based organization
- **Automatic Save/Load** - Values maintained across page changes
- **Real-time Parameter Updates** - Immediate parameter changes during editing
- **Parameter Access Objects** - Get/set parameters from pages

#### MIDI Integration
- **External Controller Support** - Full MIDI CC and note output
- **MIDI Channel Configuration** - Configurable channels (1-16)
- **MIDI Device Support** - DIN, USB Host, USB Device, Internal
- **LED Feedback via MIDI** - Send note messages for external LED controllers

#### Audio Processing
- **Stereo Audio Support** - Full stereo input/output
- **Audio Mixing** - 4-layer stereo mixing (`sum4_stereo.axo`)
- **S-rate Processing** - Real-time audio processing

#### Advanced Features
- **Lock-Based Debouncing** - Intelligent button combination handling
- **Combination Priority** - Proper handling of button combinations
- **Edge Detection** - Immediate state updates on mode changes
- **UUID Management System** - Automated conflict prevention

### 📊 System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Trellis Controller                        │
│  (32 buttons: Layer Control + Transport + Mode Selection)   │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    8-Encoder System                          │
│  (8 encoders: Parameter Control + OLED Display)             │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                  Looper Engine (4 Layers)                    │
│  States: Empty, Recording, Stopped, Playing, Overdubbing    │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│              Audio Processing & Mixing                       │
│  (Stereo Input → Looper → Mixer → Stereo Output)           │
└─────────────────────────────────────────────────────────────┘
```

---

## Missing Features / Roadmap

### 🎯 Priority 1: BPM Control & Display

#### Current State
- Tap tempo button exists (Button 60)
- BPM parameter writer exists (`bpm_param_writer.axo`)
- BPM synchronization works with looper engine
- **Missing**: Visual display of current BPM on OLED
- **Missing**: Direct encoder control for BPM adjustment

#### Implementation Requirements

**1. BPM Display on OLED**
- **Objective**: Show current BPM value on OLED display
- **Location**: Encoder page (likely Page 0, Encoder 2 position)
- **Format**: Integer display (e.g., "120 BPM")
- **Update Rate**: Real-time updates when BPM changes
- **Dependencies**: 
  - `sketchy_texts` or `sketchy_objects` for display
  - BPM parameter reading object
  - Integration with encoder display system

**2. BPM Encoder Control**
- **Objective**: Allow encoder adjustment of BPM value
- **Encoder Mode**: Integer mode (range: 60-200 BPM typical)
- **Encoder Page**: Page 0, Encoder 2 (or designated position)
- **Integration**: 
  - Connect encoder to `bpm_param_writer.axo`
  - Update display in real-time
  - Maintain tap tempo functionality

**3. BPM Parameter Access**
- **Objective**: Read BPM from parameter system
- **Implementation**: 
  - Use `int param get page kso.axo` or similar
  - Map to encoder page parameter
  - Display on OLED

#### Technical Notes
- BPM parameter is likely stored in a specific parameter page
- Need to determine which parameter index stores BPM
- May need to create new object for BPM display if not using standard int display

---

### 🎯 Priority 2: BPM Doubling/Halving

#### Current State
- BPM can be set via tap tempo or parameter
- No quick adjustment for tempo doubling/halving
- **Missing**: Fast tempo correction buttons/functions

#### Implementation Requirements

**1. BPM Doubling Function**
- **Objective**: Double the current BPM value (e.g., 60 → 120)
- **Trigger**: Button combination or dedicated button
- **Constraints**: 
  - Max BPM limit (e.g., 200 BPM)
  - If doubling exceeds max, wrap to half of doubled value
- **Implementation**: 
  - Read current BPM
  - Multiply by 2 (or set to 2x)
  - Write back via `bpm_param_writer.axo`
  - Update display

**2. BPM Halving Function**
- **Objective**: Halve the current BPM value (e.g., 120 → 60)
- **Trigger**: Button combination or dedicated button
- **Constraints**: 
  - Min BPM limit (e.g., 30 BPM)
  - If halving goes below min, wrap to double of halved value
- **Implementation**: 
  - Read current BPM
  - Divide by 2 (or set to 1/2)
  - Write back via `bpm_param_writer.axo`
  - Update display

**3. Button Mapping Options**
- **Option A**: Shift + Tap Tempo (hold) → Double
- **Option B**: Shift + Play (hold) → Halve
- **Option C**: New dedicated buttons on Trellis
- **Option D**: Encoder button press (long hold on BPM encoder)

**4. Visual Feedback**
- **LED Feedback**: Flash LED on successful double/halve
- **OLED Display**: Update BPM display immediately
- **Audio Feedback**: Optional beep/click sound

#### Technical Notes
- Need to handle edge cases (very high/low BPM values)
- Consider musical rounding (e.g., 66.67 → 67 BPM)
- May need to sync with looper engine to avoid timing issues
- Should work with both tap tempo and encoder-set BPM

---

### 🎯 Priority 3: Live Instrument Routing

#### Current State
- Audio input exists (stereo)
- Audio goes to looper engine
- **Missing**: Routing control for live instruments
- **Missing**: Separate routing for live vs. looped audio

#### Implementation Requirements

**1. Routing Configuration**
- **Objective**: Control where live instrument audio is routed
- **Options**:
  - **Direct Output**: Live audio goes directly to output (dry)
  - **Loop Input**: Live audio goes to looper input (wet)
  - **Both**: Live audio goes to both output and looper
  - **Muted**: Live audio is muted/not routed

**2. Routing Control Interface**
- **Option A**: Encoder control (Page 1, dedicated encoder)
  - Mode: List selector ("Direct", "Loop", "Both", "Muted")
- **Option B**: Button control (Trellis button)
  - Toggle between routing modes
  - LED feedback for current mode
- **Option C**: Parameter page with multiple routing options

**3. Audio Routing Implementation**
- **Objects Needed**: Audio routing/mixing objects
- **Routing Logic**:
  - Route live input to output (if enabled)
  - Route live input to looper (if enabled)
  - Mix live and looped audio appropriately
- **Implementation**: 
  - May need to modify `sum4_stereo.axo` or create new routing object
  - Add audio routing switches/mixers

**4. Per-Layer Routing (Advanced)**
- **Future Enhancement**: Route live audio to specific layers
- **Future Enhancement**: Route live audio to different layers simultaneously

#### Technical Notes
- Audio routing requires S-rate processing
- Need to maintain low latency for live performance
- Consider audio buffer implications
- May need additional audio mixer objects

---

### 🎯 Priority 4: Live Instrument Volume Control

#### Current State
- Master volume control may exist
- Layer volume controls may exist
- **Missing**: Dedicated volume control for live instrument input

#### Implementation Requirements

**1. Live Input Volume Control**
- **Objective**: Control volume of live instrument input
- **Range**: 0.0 to 1.0 (or 0-127 MIDI-style)
- **Encoder Mode**: Fractional Unipolar (0.0 to 1.0)
- **Display**: Dial or bar display with percentage

**2. Volume Control Interface**
- **Location**: Encoder page (Page 0 or Page 1)
- **Encoder**: Dedicated encoder for live input volume
- **Display**: 
  - Current volume level
  - Visual indicator (dial or bar)
  - Label: "Live Vol" or "Input Vol"

**3. Volume Control Implementation**
- **Audio Processing**: 
  - Apply gain to live input signal
  - Apply before routing (affects both direct and looped paths)
  - Or apply separately to direct and looped paths
- **Parameter Storage**: 
  - Store volume in parameter table
  - Persist across page changes
  - Recall on page return

**4. Volume Metering (Optional)**
- **Future Enhancement**: Visual level meter on OLED
- **Future Enhancement**: Peak indicator
- **Future Enhancement**: Clip warning

#### Technical Notes
- Volume control requires S-rate audio processing
- Need to handle gain staging properly
- Consider logarithmic vs. linear volume curves
- May need to integrate with existing audio mixer

---

## Implementation Priority

### Phase 1: BPM Features (High Priority)
1. **BPM Display** - Visual feedback is essential
2. **BPM Encoder Control** - Direct control improves usability
3. **BPM Doubling/Halving** - Quick tempo correction

**Estimated Complexity**: Medium
**Dependencies**: BPM parameter system, encoder display system
**Impact**: High - Core functionality improvement

### Phase 2: Audio Routing (Medium Priority)
1. **Basic Routing Control** - Simple routing options
2. **Routing Interface** - Encoder or button control
3. **Audio Implementation** - Routing logic and audio processing

**Estimated Complexity**: Medium-High
**Dependencies**: Audio routing objects, mixer system
**Impact**: Medium - Performance workflow improvement

### Phase 3: Live Input Volume (Medium Priority)
1. **Volume Control** - Encoder-based volume control
2. **Volume Display** - Visual feedback
3. **Audio Integration** - Apply gain to live input

**Estimated Complexity**: Medium
**Dependencies**: Audio processing, encoder system
**Impact**: Medium - Usability improvement

---

## Technical Considerations

### Memory Constraints
- **Current Usage**: ~300 bytes for control system
- **Additional Objects**: 
  - BPM display: ~50 bytes
  - Routing control: ~30 bytes
  - Volume control: ~20 bytes
- **Total Impact**: ~100 bytes additional (minimal)

### CPU Considerations
- **Current**: All control logic in K-rate
- **Audio Routing**: Requires S-rate processing
- **Volume Control**: Requires S-rate processing
- **Impact**: Minimal - standard audio processing

### Integration Points

#### BPM Features
- **BPM Parameter**: `bpm_param_writer.axo`
- **Display**: `sketchy_objects` or `sketchy_texts`
- **Encoder**: `8encoder_input.axo` + `8encoder_dials_fused.axo`
- **Parameter Access**: `int param get page kso.axo`

#### Audio Routing
- **Audio Input**: Stereo input objects
- **Audio Output**: Stereo output objects
- **Routing Logic**: New routing object or modify `sum4_stereo.axo`
- **Control**: Encoder or button control

#### Volume Control
- **Audio Processing**: Gain/volume object
- **Control**: Encoder input
- **Display**: Encoder display system
- **Parameter**: Parameter table storage

---

## Testing Requirements

### BPM Features
- [ ] BPM display updates correctly when BPM changes
- [ ] Encoder control adjusts BPM smoothly
- [ ] Tap tempo still works with encoder control
- [ ] Doubling/halving works correctly
- [ ] BPM limits are respected
- [ ] Display shows correct BPM value

### Audio Routing
- [ ] Routing modes work correctly
- [ ] Audio latency is acceptable
- [ ] No audio dropouts during routing changes
- [ ] All routing combinations work
- [ ] LED feedback shows current routing mode

### Volume Control
- [ ] Volume control affects live input correctly
- [ ] Volume display updates in real-time
- [ ] Volume persists across page changes
- [ ] No audio artifacts when adjusting volume
- [ ] Volume range is appropriate (0.0 to 1.0)

---

## Future Enhancements (Out of Scope)

### Advanced Features
- **Per-Layer Routing**: Route live audio to specific layers
- **Input Effects**: Apply effects to live input before routing
- **Volume Metering**: Visual level meters and peak indicators
- **BPM Sync Options**: Sync to external MIDI clock
- **BPM Presets**: Save/recall common BPM values
- **Tempo Mapping**: Musical tempo divisions/multiplications
- **Audio Compression**: Automatic gain control for live input
- **Input Monitoring**: Solo/mute for live input

### User Experience
- **Configuration Pages**: Dedicated pages for routing/volume
- **Quick Presets**: One-button routing/volume presets
- **Visual Feedback**: Enhanced LED feedback for routing modes
- **Parameter Locking**: Lock certain parameters during performance

---

## Version History

### v1.5 (Current)
- Core looper functionality
- LED feedback system
- Encoder system
- Basic parameter management

### v1.6 (Planned)
- BPM display and control
- BPM doubling/halving
- Live instrument routing
- Live instrument volume control

---

## Notes

### Development Guidelines
- Follow existing UUID management system
- Maintain backward compatibility with existing patches
- Use established encoder page system
- Follow audio processing best practices
- Test thoroughly with actual hardware

### Documentation Updates
- Update `LOOPER_STATION_MANUAL.md` with new features
- Document new encoder pages/parameters
- Document new button combinations
- Update object reference section

---

*Last Updated: January 2025*
*Roadmap Version: 1.0*

