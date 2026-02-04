# Ksoloti Looper Station Manual

## Overview

The Ksoloti Looper Station is a comprehensive 4-layer looping system built around the Adafruit Trellis 8x4 button grid controller. It provides professional-grade looping capabilities with intelligent LED feedback, shift-based mode switching, and seamless integration with the Ksoloti platform.

## System Architecture

### Core Components

1. **Trellis Controller** (`trellis_controller.axo`) - Main input interface
2. **8-Encoder System** (`8encoder_input.axo`, `8encoder_dials_fused.axo`) - Parameter control
3. **LED Feedback System** (`trellis_led_feedback_combined.axo`) - Visual state indication
4. **Looper Engine** - 4-layer recording and playback system
5. **Parameter Management** - BPM sync and parameter control
6. **MIDI Integration** - External controller support

### Hardware Requirements

- **Ksoloti Core** - Main processing unit
- **Adafruit Trellis 8x4** - 32-button grid controller (MIDI notes 36-67)
- **8 Rotary Encoders** - Physical encoders for parameter control
- **Audio Interface** - Stereo input/output
- **MIDI Interface** - For external controller integration

## 8-Encoder System

### Overview

The looper station features 8 physical rotary encoders organized in a 4x2 grid layout, providing direct parameter control with visual feedback through the OLED display. The encoders are grouped into pages, allowing access to different parameter sets.

### Physical Layout

```
Top Row:    [Enc1] [Enc2] [Enc3] [Enc4]
Bottom Row: [Enc5] [Enc6] [Enc7] [Enc8]
```

**Position Mapping:**
- **Top Row**: Y = 13px, X = 26, 52, 78, 104px
- **Bottom Row**: Y = 39px, X = 26, 52, 78, 104px
- **Size**: 24x24 pixels each

### Page System

The encoders support multiple pages (typically 8 pages), with each page containing different parameter configurations:

- **Page 0**: Main looper parameters
- **Page 1**: Layer-specific controls
- **Page 2**: Effects and processing
- **Page 3**: Advanced settings
- **Page 4-7**: Additional parameter sets

### Encoder Modes

Each encoder can operate in different modes depending on the parameter type:

| Mode | Type | Range | Display | Description |
|------|------|-------|---------|-------------|
| 0 | Fractional Unipolar | 0.0 to 1.0 | Dial | Positive-only parameters (volume, mix) |
| 1 | Fractional Bipolar | -1.0 to 1.0 | Dial | Bipolar parameters (pan, pitch) |
| 2 | Integer | min to max | Text | Integer values (tempo, note values) |
| 3 | List/Selector | 0 to max | Text | Selection from predefined options |
| 4 | Bar Unipolar | 0.0 to 1.0 | Vertical Bar | Visual level indicators |
| 5 | Bar Bipolar | -1.0 to 1.0 | Vertical Bar | Bipolar level indicators |
| 6 | Duplicate | - | - | Reference to another parameter |
| 255 | Empty | - | - | Unconfigured/disabled |

### Parameter Storage

The encoder system uses a sophisticated parameter storage system:

- **Parameter Table**: Stores all encoder values per page
- **Automatic Save/Load**: Values are saved when switching pages
- **Persistent Storage**: Values maintained across page changes
- **Real-time Updates**: Immediate parameter updates during editing

### Encoder Input Processing

**Objects**: `8encoder_input.axo`, `8encoder_dials_fused.axo`

**Features**:
- **Acceleration**: Faster turning increases increment speed
- **Direction Detection**: Clockwise/counterclockwise input
- **Edge Detection**: Trigger-based processing
- **Mode-specific Logic**: Different behavior per encoder mode

**Inlets**:
- `page` - Current page selection
- `trig1-8` - Encoder trigger inputs
- `dir1-8` - Encoder direction inputs

**Attributes**:
- `maxpages` - Maximum number of pages (default 8)
- `firstparam` - Starting parameter index
- `acceltime` - Acceleration time threshold
- `accelmultiplier` - Maximum acceleration multiplier

### Visual Feedback

The encoders provide rich visual feedback through the OLED display:

- **Dial Display**: Circular dials for fractional parameters
- **Text Display**: Numeric values for integer parameters
- **Bar Display**: Vertical bars for level indicators
- **Label Display**: Parameter names and units
- **Value Display**: Current parameter values

### Integration with Looper

The encoder system integrates seamlessly with the looper functionality:

- **Page Selection**: Trellis controller index output selects encoder pages
- **Parameter Control**: Direct control of looper parameters
- **Real-time Updates**: Immediate parameter changes
- **Visual Confirmation**: OLED display shows current values

### Usage Examples

#### Basic Encoder Setup
```
trellis_controller → index → 8encoder_input → page
encoder_hardware → 8encoder_input → trig1-8, dir1-8
8encoder_input → parameter_system → looper_parameters
```

#### Page-based Parameter Control
```
// Page 0: Main looper controls
Encoder 1: Master Volume (0.0-1.0)
Encoder 2: Tempo (60-200 BPM)
Encoder 3: Loop Length (1-32 beats)
Encoder 4: Quantization (1/4, 1/8, 1/16)

// Page 1: Layer controls
Encoder 1: Layer 1 Volume (0.0-1.0)
Encoder 2: Layer 2 Volume (0.0-1.0)
Encoder 3: Layer 3 Volume (0.0-1.0)
Encoder 4: Layer 4 Volume (0.0-1.0)
```

## Trellis Controller Interface

### Button Layout

The Trellis controller is organized into 4 rows with specific functions:

```
Row 1: 36  37  38  39  40  41  42  43  (Layer Control)
Row 2: 44  45  46  47  48  49  50  51  (Reserved)
Row 3: 52  53  54  55  56  57  58  59  (Reserved)
Row 4: 60  61  62  63  64  65  66  67  (Transport & Modes)
```

### Layer Control (Row 1: Notes 36-43)

Each layer has two buttons:
- **Rec Button** (Even notes: 36, 38, 40, 42) - Record/Stop functions
- **Sel Button** (Odd notes: 37, 39, 41, 43) - Select/Reverse functions

#### Layer Functions

| Function | Button Combination | Output | Description |
|----------|-------------------|---------|-------------|
| **Record** | Rec alone | `rec1-4` gate | Start/stop recording on layer |
| **Stop** | Rec + Sel (short hold) | `stop1-4` pulse | Stop layer playback |
| **Reset** | Rec + Sel (long hold) | `reset1-4` pulse | Clear layer and reset toggles |
| **Select Layer** | Sel alone | `index` = 1-4 | Switch to layer mode |
| **Reverse Toggle** | Shift + Sel | `reverse1-4` toggle | Toggle reverse playback |
| **Alt Function** | Shift + Rec | `alt1-4` toggle | Toggle alternate function |

### Transport & Mode Selection (Row 4: Notes 60-67)

| Note | Primary Function | Shift Function | Output |
|------|-----------------|----------------|---------|
| 60 | Tap Tempo | Nudge- | `tap` gate / `nudgeMinus` gate |
| 61 | Play | Nudge+ | `play` gate / `nudgePlus` gate |
| 62 | **Shift** | - | `shift` gate / `shiftHeld` gate |
| 63 | Main Mode | System Mode | `index` = 0 / 9 |
| 64 | FX Mode | - | `index` = 5 |
| 65 | Synth Mode | - | `index` = 6 |
| 66 | Instrument Mode | - | `index` = 7 |
| 67 | Sequencer Mode | - | `index` = 8 |

## LED Feedback System

### Dual-Mode Operation

The LED feedback system operates in two distinct modes based on the shift state:

#### Shift OFF Mode (Looper States)
- **LEDs show**: Current looper layer states
- **Mapping**: 
  - Layer 1: Note `startnote` (default 36)
  - Layer 2: Note `startnote + 2` (default 38)
  - Layer 3: Note `startnote + 4` (default 40)
  - Layer 4: Note `startnote + 6` (default 42)
- **Velocities**: Based on looper state array `{0, 3, 0, 19, 60}`
  - State 0: Velocity 0 (LED off)
  - State 1: Velocity 3 (dim)
  - State 2: Velocity 0 (LED off)
  - State 3: Velocity 19 (medium)
  - State 4: Velocity 60 (bright)

#### Shift ON Mode (Toggle States)
- **LEDs show**: Reverse and Alt toggle states
- **Mapping**:
  - Reverse toggles: Sel buttons (37, 39, 41, 43)
  - Alt toggles: Rec buttons (36, 38, 40, 42)
- **Velocities**: Configurable via attributes
  - `revVelocity` (default 19) - Reverse toggle brightness
  - `altVelocity` (default 60) - Alt toggle brightness

### LED Feedback Object

**Object**: `trellis_led_feedback_combined.axo`

**Inlets**:
- `shift` - Shift state (controls mode)
- `reverse1-4` - Reverse toggle states (shift mode)
- `alt1-4` - Alt toggle states (shift mode)
- `state0-2` - Looper states (non-shift mode)

**Attributes**:
- `revVelocity` (0-127) - Reverse LED brightness
- `altVelocity` (0-127) - Alt LED brightness
- `startnote` (0-127) - Starting MIDI note for looper states
- `midichannel` (1-16) - MIDI channel
- `device` - MIDI device selection

## Looper Engine

### 4-Layer Architecture

The looper system supports 4 independent layers, each with:

- **Recording capability** - Record audio input to layer
- **Playback control** - Play, stop, reverse
- **State management** - Track recording/playback states
- **Synchronization** - BPM-synced recording and playback

### Layer States

Each layer can be in one of 5 states:

| State | Value | Description | LED Behavior |
|-------|-------|-------------|--------------|
| 0 | Empty | No audio recorded | LED off |
| 1 | Recording | Currently recording | Dim LED (velocity 3) |
| 2 | Stopped | Recorded but stopped | LED off |
| 3 | Playing | Playing back recorded audio | Medium LED (velocity 19) |
| 4 | Overdubbing | Recording over existing audio | Bright LED (velocity 60) |

### State-to-Velocity Conversion

**Object**: `state_to_velocity_midi.axo`

Converts looper states to MIDI velocities for LED feedback:

```c
uint8_t velocities[5] = {0, 3, 0, 19, 60};
```

**Inlets**:
- `active` - Controls when MIDI is sent
- `state0-2` - Looper states to convert

**Behavior**:
- Only sends MIDI when `active` is high
- Sends all states immediately on active rising edge
- Sends individual states only when they change

## Parameter Management

### BPM Synchronization

The looper integrates with the BPM parameter system for synchronized recording and playback:

- **Tap Tempo** - Button 60 for tempo input
- **BPM Parameter Writer** - Syncs looper to global BPM
- **Quantized Recording** - Record start/stop aligned to beat

### Parameter Access

**Objects**:
- `int param get page kso.axo` - Get all 8 parameters from a page
- `int param get half page kso.axo` - Get 4 parameters from half a page
- `param_write_int.axo` - Write integer parameters

## MIDI Integration

### External Controller Support

The system supports external MIDI controllers through:

- **MIDI CC Send** (`midi_cc_send.axo`) - Send control changes
- **MIDI Note Output** - Send note messages for LED feedback
- **MIDI Pitch Bend** (`midi_pitchbend_int.axo`) - Send pitch bend data

### MIDI Channel Configuration

All MIDI objects use the standard Axoloti MIDI channel attribute:
- Set `attr_midichannel` in object properties
- Default channel: 1
- Range: 1-16

## Audio Processing

### Stereo Support

**Object**: `sum4_stereo.axo`

Provides stereo audio mixing capabilities:
- 4 stereo inputs (in1L/R, in2L/R, in3L/R, in4L/R)
- 2 stereo outputs (outL, outR)
- S-rate processing for real-time audio

### Audio Routing

Typical audio flow:
```
Audio Input → Looper Engine → Audio Mixer → Audio Output
            ↓
         LED Feedback ← MIDI Controller
```

## Usage Examples

### Basic Looper Setup

```
trellis_controller → rec1 → looper_layer_1_rec
                  → stop1 → looper_layer_1_stop
                  → reverse1 → looper_layer_1_reverse
                  → index → layer_selector
```

### LED Feedback Setup

```
trellis_controller → shiftHeld → trellis_led_feedback_combined → shift
                  → reverse1-4 → trellis_led_feedback_combined → reverse1-4
                  → alt1-4 → trellis_led_feedback_combined → alt1-4
recordlogic → slot0state-2state → trellis_led_feedback_combined → state0-2
```

### State-to-Velocity Setup

```
recordlogic → active_signal → state_to_velocity_midi → active
            → slot0state-2state → state_to_velocity_midi → state0-2
state_to_velocity_midi → MIDI_Out → External_Controller
```

## Advanced Features

### Lock-Based Debouncing

The trellis controller uses intelligent lock-based debouncing to prevent accidental triggers:

- **Problem**: Releasing "Shift + Rec" could accidentally trigger Rec alone
- **Solution**: Lock flags that require full button release before retriggering
- **Benefit**: No timing-based debounce needed, works regardless of release order

### Combination Priority

Button combinations are processed in priority order:

1. **Stop** (Rec+Sel, no shift) - Highest priority
2. **Reverse/Alt** (Shift + Rec/Sel) - Second priority  
3. **Rec alone** (if not locked) - Normal operation
4. **Index change** (Sel alone, no shift) - Normal operation

### Edge Detection

Both LED feedback modes use edge detection for immediate state updates:

- **Shift rising edge** (0→1): Sends all toggle states immediately
- **Shift falling edge** (1→0): Sends all looper states immediately
- **Active rising edge**: Sends all looper states immediately

## Troubleshooting

### Common Issues

1. **LEDs not updating**
   - Check MIDI channel settings
   - Verify shift state is correct
   - Ensure active signals are connected

2. **Buttons not responding**
   - Check MIDI device configuration
   - Verify button mapping
   - Check for lock states

3. **Audio not recording**
   - Check audio input levels
   - Verify looper engine connections
   - Check BPM synchronization

### Debug Tips

1. **Use index output** to verify mode switching
2. **Monitor gate outputs** to check button states
3. **Check MIDI messages** with MIDI monitor
4. **Verify parameter values** with parameter objects

## Performance Considerations

### Memory Usage

- **Trellis Controller**: ~150 bytes
- **LED Feedback**: ~50 bytes
- **State Management**: ~20 bytes per layer
- **Total System**: ~300 bytes

### CPU Usage

- **All logic in K-rate** - No S-rate processing
- **Efficient MIDI handling** - Gate array approach
- **Minimal overhead** - Scales well with polyphony

## Integration Notes

### Ksoloti Platform

- **Firmware**: Compatible with Ksoloti 1.1.0+
- **Memory**: Uses SRAM2 for buffers, SDRAM for large arrays
- **Threading**: Safe for multi-threaded environments

### External Controllers

- **MIDI Standard**: Full MIDI 1.0 compliance
- **Channel Support**: 1-16 channels
- **Device Support**: DIN, USB Host, USB Device, Internal

## UUID Management

### Preventing UUID Conflicts

The looper station uses a strict UUID management system to prevent object loading conflicts:

#### **Automated Tools**
- **`check_uuid_conflicts.sh`** - Scans all .axo files for duplicate UUIDs
- **`generate_next_uuid.sh`** - Generates the next available UUID in sequence
- **Pre-commit hook** - Automatically checks for conflicts before git commits

#### **UUID Workflow**
1. **Before creating objects**: Run `./check_uuid_conflicts.sh`
2. **Generate UUID**: Run `./generate_next_uuid.sh` 
3. **Create object**: Use the generated UUID
4. **Update registry**: Add to `.cursorrules` immediately
5. **Verify**: Run `./check_uuid_conflicts.sh` again

#### **UUID Format**
- **Pattern**: `5224773f-18de-4231-8f92-b1f22bb953XX`
- **Format**: 8-4-4-4-12 hex format
- **Sequence**: Increment last two hex digits (70, 71, 72, ..., 9A, 9B, 9C, etc.)

#### **Critical Rules**
- ✅ **Never reuse UUIDs** - Even deleted objects permanently retire their UUIDs
- ✅ **Check external objects** - Verify against Axolonatics objects
- ✅ **Document immediately** - Update `.cursorrules` before any other work
- ❌ **Never skip verification** - Always run conflict checks

## Version History

- **v1.0** - Initial looper station implementation
- **v1.1** - Added LED feedback system
- **v1.2** - Combined LED feedback object
- **v1.3** - Added state-to-velocity conversion
- **v1.4** - Improved edge detection and mode switching
- **v1.5** - Added UUID conflict prevention system

## Author

Simon / AI Assistant

## License

BSD

---

*Last Updated: January 2025*

## Appendix: Object Reference

### Core Objects

| Object | File | Purpose |
|--------|------|---------|
| trellis_controller | `objects/midi/trellis_controller.axo` | Main input interface |
| 8encoder_input | `objects/encoder/8encoder_input.axo` | Encoder input processing |
| 8encoder_dials_fused | `objects/encoder/8encoder_dials_fused.axo` | Encoder display system |
| trellis_led_feedback_combined | `objects/midi/trellis_led_feedback_combined.axo` | LED feedback system |
| state_to_velocity_midi | `objects/encoder/state_to_velocity_midi.axo` | State conversion |
| sum4_stereo | `objects/encoder/sum4_stereo.axo` | Stereo audio mixing |

### Parameter Objects

| Object | File | Purpose |
|--------|------|---------|
| int param get page kso | `objects/ksoloti/int param get page kso.axo` | Get page parameters |
| int param get half page kso | `objects/ksoloti/int param get half page kso.axo` | Get half page parameters |
| param_write_int | `objects/encoder/param_write_int.axo` | Write parameters |

### MIDI Objects

| Object | File | Purpose |
|--------|------|---------|
| midi_cc_send | `objects/encoder/midi_cc_send.axo` | Send MIDI CC |
| midi_pitchbend_int | `objects/encoder/midi_pitchbend_int.axo` | Send pitch bend |
| poly_cc_output | `objects/encoder/poly_cc_output.axo` | Polyphonic CC output |
