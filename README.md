# Can Damonium - Impulse Response VST3 Plugin

An impulse response convolver VST3 plugin featuring pre-loaded profiles from Pringles can amplifiers.

## Project Structure

- **src/profiler/** - Application for recording standardized IR profiles from equipment (used to create Small/Regular/Grande profiles)
- **src/plugin/** - VST3 plugin code (convolver, UI, IR library management)
- **src/recorder/** - Standalone Windows application for users to record and generate their own impulse responses
- **src/common/** - Shared utilities and common code
- **docs/IR_STANDARD.md** - Standardized IR format specification (48 kHz, 24-bit, logarithmic chirp)
- **resources/impulse_responses/** - Pre-recorded IR profiles and user library storage

## Building

### Prerequisites
- JUCE framework (add as git submodule or clone separately)
- CMake 3.24+
- Visual Studio 2019+ (for Windows VST3 development)
- Windows 10+ SDK

### Build Steps

```bash
# Clone the project
git clone <repo-url>
cd Can_damonium

# Add JUCE as a submodule (if not already present)
git submodule add https://github.com/juce-framework/JUCE.git JUCE

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . --config Release
```

## Workflow

### Phase 1: User IR Recording Tool (COMPLETE ✓)
- Standalone Recorder Application with 5-step guided workflow
- Test signal generation (logarithmic chirp, 20Hz-20kHz)
- Audio I/O and real-time capture
- WAV file I/O (48 kHz, 24-bit, mono)
- **Status**: Ready to build and use

### Phase 2a: Record Single IR for MVP
- Record **Regular Pringles Can IR only** (MVP approach)
- Use CanDamoniumRecorder application
- File saved to: `Documents\Can_damonium\IRs\`
- **Timeline**: ~15 minutes
- **Why Single IR?**: Validate full workflow before scaling

### Phase 2b: Build MVP VST3 Plugin
- Create convolver processor
- Embed Regular Pringles Can IR as resource
- Basic UI (no controls needed for MVP)
- Test end-to-end in DAW
- **Timeline**: ~30 minutes

### Phase 2c: Test & Verify
- Load plugin in DAW
- Process audio through convolver
- Verify convolution works correctly
- **Timeline**: ~15 minutes
- **Success**: Plugin loads, audio flows, effect audible

### Phase 3: Expand to Full Plugin
- Record Small Pringles Can IR
- Record Grande Pringles Can IR
- Add profile selector to plugin UI
- Update plugin to load selected IR
- **Timeline**: After MVP validates

### Phase 4 (Later): Distribute User Tool
- Package Recorder application for general users
- Users record their own amp IRs
- Share libraries and profiles

## Plugin Features

- **Pre-loaded Profiles**: Small, Regular, and Grande Pringles can amp IRs
- **Custom IR Loading**: Load user-generated impulse response files
- **Library Management**: Save and organize custom IRs
- **Real-time Convolution**: Apply IR to audio signals in real-time

## Development Status

- [x] JUCE project scaffolding
- [x] IR Standard specification (docs/IR_STANDARD.md)
- [x] User IR Recorder application (complete UI & workflow)
- [x] Test signal generator (logarithmic chirp, 20Hz-20kHz)
- [x] WAV file I/O (save 48kHz, 24-bit recordings to disk)
- [x] Recording pipeline (capture → validate → trim → normalize → save)
- [ ] Build and test the recorder application
- [ ] **Record Regular Pringles Can IR** (MVP - one profile)
- [ ] **Build MVP VST3 plugin** (1 IR, convolver only)
- [ ] **Test MVP plugin in DAW** (end-to-end validation)
- [ ] Record Small & Grande IR profiles (Phase 3)
- [ ] Expand plugin to 3-profile selector (Phase 3)

## Ready to Record & Build MVP Plugin

**See quick-start guide: [QUICK_REFERENCE.md](QUICK_REFERENCE.md)**

**Complete guides:**
- Recording: [BUILD_AND_RECORD_GUIDE.md](docs/BUILD_AND_RECORD_GUIDE.md)
- MVP Plugin: [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md)
- After Recording: [AFTER_RECORDING_NEXT_STEPS.md](AFTER_RECORDING_NEXT_STEPS.md)

**Timeline for MVP:**
1. Build Recorder (5 min)
2. Record Regular Pringles Can (15 min)
3. Build MVP Plugin (30 min)
4. Test in DAW (15 min)
5. **Total: ~1 hour** to working convolver plugin

**Why MVP with Single IR?**
- Get end-to-end workflow validated
- Catch issues early before recording 3 profiles
- Faster to working plugin
- Still add other profiles later (Phase 3)

## License

TBD
