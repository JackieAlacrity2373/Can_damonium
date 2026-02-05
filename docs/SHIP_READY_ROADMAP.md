# Ship-Ready Feature Checklist & Roadmap

## Current Status: Phase 1 Complete - Phase 2 Pending IR Recordings

### Phase 1 ✅ COMPLETE: Core Convolution Engine
- [x] JUCE 7 integration
- [x] Real-time FFT-based convolution
- [x] Multi-channel support (mono/stereo detection)
- [x] Sample rate matching (96kHz native)
- [x] Audio device selection (Focusrite + S/PDIF)
- [x] Test tone generator (440Hz reference)
- [x] Real-time input/output metering
- [x] IR file loading and management
- [x] IR Recorder app with logging

### Phase 2 🚀 IN PROGRESS: UI Customization
- [x] Can flavor color selection (10 flavors)
- [x] Can size window resizing (3 sizes)
- [x] Dynamic UI layout adjustment
- [x] Flavor-themed background coloring
- [ ] **TODO:** Record Small Can IR (4.75") @ 96kHz
- [ ] **TODO:** Record Large Can IR (11.5") @ 96kHz
- [ ] **TODO:** VST3 packaging & testing
- [ ] **TODO:** Cross-platform testing (macOS/Linux)

---

## Implementation Summary

### UI Features Added

#### 1. Can Flavor Selector
```
Can Flavor: [Original/Plain ▼]
```
**10 Pringles Flavors with Themed Colors:**
1. Original/Plain → Red (#E31937)
2. Sour Cream & Onion → Green (#2E8B57)
3. Cheddar Cheese → Gold (#FF8C00)
4. Jalapeño → Red-Orange (#FF6347)
5. Screamin' Dill Pickle → Teal (#20B2AA)
6. Rotisserie Chicken → Brown (#A0522D)
7. Soft-Shell Crab → Tan (#D2B48C)
8. Blueberry & Hazelnut → Purple (#9370DB)
9. Pumpkin Pie Spice → Dark Orange (#D2691E)
10. Grilled Shrimp → Salmon Pink (#FA8072)

#### 2. Can Size Selector
```
Can Size: [Regular (9.25") ▼]
```
**Three Window Sizes (proportional to real cans):**
| Size | Dimensions | Ratio to Regular |
|------|-----------|------------------|
| Small (4.75") | 360 × 280 | 51.3% |
| Regular (9.25") | 700 × 550 | 100% |
| Large (11.5") | 870 × 685 | 124.3% |

**Features:**
- Window automatically resizes when selection changes
- All UI controls reposition proportionally
- Color background updates with flavor selection
- 15% color tint for visual feedback while maintaining readability

### Technical Details

**Files Modified:**
- `src/plugin/PluginEditor.h` - Added flavor/size data structures
- `src/plugin/PluginEditor.cpp` - Implemented UI selectors and sizing logic

**Build Status:** ✅ SUCCESSFUL
```
CanDamoniumPlugin_Standalone.vcxproj → SUCCESSFUL
```

**Plugin Executable:** 
```
H:\Can_damonium\build\src\plugin\CanDamoniumPlugin_artefacts\Release\Standalone\Can Damonium.exe
```

---

## Immediate Roadmap Items

### 1. Record Small Can IR (Next Session)
**Objective:** Create 96kHz IR matching Small Pringles can dimensions (4.75")

**Steps:**
1. Obtain physical small Pringles can (if available) or equivalent sized can
2. Position vertically at recording location
3. Launch: `Can Damonium IR Recorder.exe`
4. Set Audio → Audio Settings to Focusrite (96kHz native)
5. Click "Start Recording"
6. Follow on-screen chirp sequence
7. File auto-saves to: `Documents\Can_damonium\IRs\SmallCan_[DATE]_96k.wav`

**Acceptance Criteria:**
- [ ] File created at correct location
- [ ] Sample rate: 96kHz
- [ ] Duration: ~10 seconds (similar to regular)
- [ ] Can be selected in flavor dropdown IR list
- [ ] Plugin loads and produces non-silent output

### 2. Record Large Can IR (Session After)
**Objective:** Create 96kHz IR matching Large Pringles can dimensions (11.5")

**Steps:** (Same as Small, with Large can)

**Acceptance Criteria:**
- [ ] File created at correct location
- [ ] Sample rate: 96kHz
- [ ] Duration: ~10 seconds
- [ ] Can be selected in flavor dropdown IR list
- [ ] Plugin loads and produces non-silent output

### 3. Unified IR Selector (Enhancement)
**Objective:** Link flavor selection to automatic IR file matching

**Proposed Logic:**
```
Flavor: Original/Plain → Auto-loads "SmallCan" IR if selected size is "Small"
Flavor: Original/Plain → Auto-loads "RegularCan" IR if selected size is "Regular"
Flavor: Original/Plain → Auto-loads "LargeCan" IR if selected size is "Large"
```

**Benefits:**
- User selects size + flavor
- Correct IR automatically loaded
- More intuitive user experience

### 4. Clean Shipping Package
**Objective:** Prepare for distribution/demo

**Deliverables:**
- [x] Working Standalone plugin (Can Damonium.exe)
- [x] Working IR Recorder app (Can Damonium IR Recorder.exe)
- [ ] Pre-loaded IRs for all 3 sizes
- [ ] User guide PDF
- [ ] Example usage video (optional)
- [ ] VST3 plugin package (optional)

---

## Current Window Layout

```
═══════════════════════════════════════════════════════════
                    Can Damonium
═══════════════════════════════════════════════════════════

Can Flavor: [Original/Plain ▼]    Can Size: [Regular ▼]

IR Status: Loaded

Sample Rate: 96000 Hz  |  Block: 480  |  Callbacks: 102

Audio: Speakers (Focusrite USB Audio)

IR File: [Regular_Pringles_Can_... ▼]                    [+]

[    Reload IR    ]

[Bypass: OFF]  [Test Tone: OFF]  [Audio Settings]

╔═══════════════════════════════════════════════════════════╗
║  Input    ████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 0.16%  ║
║ Convolution ████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 0.07% ║
║  Output   ████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 0.07% ║
╚═══════════════════════════════════════════════════════════╝
```

---

## Color Scheme Examples

### Regular/Plain Can (Red #E31937)
```
Background: Dark grey with 15% red tint
Text: White
Meters: Green (normal) → Orange (mid) → Red (peak)
```

### Sour Cream & Onion (Green #2E8B57)
```
Background: Dark grey with 15% green tint
Text: White
Meters: Same
```

---

## Next Session Quick Start

### If continuing UI work:
```bash
# Launch plugin
.\build\src\plugin\CanDamoniumPlugin_artefacts\Release\Standalone\Can Damonium.exe

# Check logs for initialization
cat $env:USERPROFILE\Documents\Can_damonium\CanDamonium.log | Select-String "flavor|size"
```

### If recording IRs:
```bash
# Launch IR Recorder
.\build\src\recorder\CanDamoniumRecorder_artefacts\Release\Can Damonium IR Recorder.exe

# Check generated IRs
ls $env:USERPROFILE\Documents\Can_damonium\IRs\
```

---

## Success Metrics

### ✅ Phase 1 Complete
- Convolution working with sustained output (not fading)
- Input meter shows ambient noise (~0.15%)
- Output meter shows convolved signal (~0.07%)
- IR Recorder functional (recorded 96kHz native)
- Device switching works (Focusrite + S/PDIF verified)

### ✅ Phase 2 - UI Ready
- Can flavor selector fully functional with 10 options
- Can size selector resizes window smoothly
- Color background updates with flavor
- All controls reposition correctly on resize
- Build system stable (no compile errors)

### 🚀 Phase 2 - Ready for Shipping
- [ ] All 3 can IRs recorded (Small, Regular, Large)
- [ ] VST3 packaging complete
- [ ] Cross-platform testing passed
- [ ] User documentation finalized
- [ ] Demo video ready (optional)

