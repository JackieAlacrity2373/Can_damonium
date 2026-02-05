# MVP Workflow Summary: Record 1 → Build Plugin → Test → Expand

**Total Timeline**: ~2 hours from start to working plugin in DAW

---

## 📋 The Plan (Revised for MVP)

### Why Single IR First?
✅ Validate full workflow quickly  
✅ Catch issues before recording 3 profiles  
✅ Get to working plugin faster  
✅ Easier to debug with one variable  
✅ Add other profiles once MVP is proven  

---

## 🚀 Complete Workflow

### Phase 2a: Record Single IR (Regular Pringles Can)
**Time**: ~30 minutes  
**What**: Record Regular Pringles Can IR using CanDamoniumRecorder app  
**Output**: `Regular_Pringles_Can_YYYYMMDD_HHMMSS_48k.wav` (~150 MB)

**Steps**:
1. Launch recorder app
2. Select "Regular Pringles Can" from dropdown
3. Click RECORD
4. Wait for processing (14 seconds + 5-10 second processing)
5. File saved automatically to `Documents\Can_damonium\IRs\`

**Documentation**: [RECORDING_CHECKLIST_SINGLE_IR.md](RECORDING_CHECKLIST_SINGLE_IR.md)

**Success Criteria**:
- ✓ File exists
- ✓ File is 100-200 MB
- ✓ Can verify in Windows
- ✓ Optional: Listen in Audacity to verify quality

---

### Phase 2b: Build MVP VST3 Plugin (1 IR)
**Time**: ~30 minutes  
**What**: Create convolver plugin with Regular Pringles Can IR embedded  
**Output**: `CanDamoniumPlugin.vst3` (ready to load in DAW)

**Development Steps**:
1. Copy IR to `src/plugin/Resources/`
2. Create ConvolverProcessor class
3. Modify PluginProcessor to load IR and process audio
4. Update CMakeLists.txt to embed resource
5. Build release configuration
6. Verify `.vst3` file created

**Commands**:
```powershell
# Copy IR
Copy-Item "$env:USERPROFILE\Documents\Can_damonium\IRs\Regular_Pringles_Can*.wav" `
  h:\Can_damonium\src\plugin\Resources\ -Force

# Build
cd h:\Can_damonium\build
cmake --build . --config Release

# Check output
Test-Path h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3
```

**Documentation**: [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md)

**Success Criteria**:
- ✓ No build errors
- ✓ `.vst3` file exists
- ✓ File size reasonable (~5-10 MB)

---

### Phase 2c: Test MVP Plugin in DAW
**Time**: ~20 minutes  
**What**: Load plugin in DAW and verify convolution works  
**Output**: Working convolver plugin, proven in DAW

**Test Steps**:
1. Install plugin to VST3 folder
2. Open DAW and rescan plugins
3. Insert plugin on audio track
4. Feed audio through plugin
5. Listen to output
6. Verify convolution effect is audible
7. Check no crashes

**Install Command**:
```powershell
$plugin = "h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3"
$vst3 = "$env:PROGRAMFILES\Common Files\VST3"
Copy-Item $plugin $vst3 -Force
```

**Expected Results**:
- ✓ Plugin loads in DAW
- ✓ No crash on instantiate
- ✓ Audio flows through
- ✓ You hear convolution effect
- ✓ No audio glitches
- ✓ Latency ~5-6 seconds (normal for this IR)

**Success Criteria**:
- ✓ Plugin appears in DAW
- ✓ Plugin doesn't crash
- ✓ Audio passes through
- ✓ Convolution effect audible
- ✓ Works on different audio

---

## ✅ MVP Success Checklist

### Recording Complete
- [ ] IR file saved
- [ ] File is named: `Regular_Pringles_Can_*.wav`
- [ ] File size: 100-200 MB
- [ ] File location: `Documents\Can_damonium\IRs\`

### Plugin Built
- [ ] Build completes without errors
- [ ] Plugin file exists: `.vst3`
- [ ] Plugin file size: 5-10 MB

### Plugin Tested
- [ ] Loads in DAW
- [ ] Doesn't crash
- [ ] Audio flows through
- [ ] Effect is audible
- [ ] Quality sounds good

### MVP Complete ✓
- [ ] Everything above works
- [ ] No major issues found
- [ ] Ready for Phase 3

---

## 📊 Timeline Breakdown

| Phase | Task | Time | Status |
|-------|------|------|--------|
| 2a | Record Regular IR | 30 min | Ready |
| 2b | Build MVP plugin | 30 min | Ready |
| 2c | Test in DAW | 20 min | Ready |
| 2 | **MVP Total** | **~1.5 hours** | **Ready** |

---

## 🎯 Phase 3: After MVP Works (Later)

Once MVP is validated:

### Phase 3a: Record Additional IRs
1. Record Small Pringles Can IR
2. Record Grande Pringles Can IR
3. Verify both files saved

### Phase 3b: Expand Plugin
1. Add profile selector UI
2. Load selected IR dynamically
3. Support all 3 profiles

### Phase 3c: Polish
1. Add level controls
2. Add wet/dry mix
3. Show IR waveform
4. Add bypass button

---

## 📁 Key Files & Locations

### Recordings
- **Location**: `Documents\Can_damonium\IRs\`
- **MVP File**: `Regular_Pringles_Can_20260202_*.wav`

### Plugin
- **Source**: `h:\Can_damonium\src\plugin\`
- **Built**: `h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3`
- **Install**: `C:\Program Files\Common Files\VST3\CanDamoniumPlugin.vst3`

### Recorder
- **Built**: `h:\Can_damonium\build\src\recorder\Release\CanDamoniumRecorder.exe`

### Documentation
- **This File**: MVP Workflow Summary
- **Recording**: [RECORDING_CHECKLIST_SINGLE_IR.md](RECORDING_CHECKLIST_SINGLE_IR.md)
- **Plugin Dev**: [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md)
- **After Recording**: [AFTER_RECORDING_NEXT_STEPS.md](AFTER_RECORDING_NEXT_STEPS.md)
- **Quick Ref**: [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

---

## 🚦 Status Indicators

### Current Status
- ✅ Recorder app: READY
- ✅ Documentation: READY
- 🔲 MVP Recording: NOT STARTED
- 🔲 MVP Plugin: NOT STARTED
- 🔲 MVP Testing: NOT STARTED

### Next Action
**Build recorder and record Regular Pringles Can IR**

```powershell
# Verify build exists
Test-Path h:\Can_damonium\build\src\recorder\Release\CanDamoniumRecorder.exe

# Launch
h:\Can_damonium\build\src\recorder\Release\CanDamoniumRecorder.exe
```

---

## 📖 Documentation Map

**Start here for each phase**:
- **Recording**: [RECORDING_CHECKLIST_SINGLE_IR.md](RECORDING_CHECKLIST_SINGLE_IR.md)
- **Plugin Development**: [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md)
- **Testing & Next**: [AFTER_RECORDING_NEXT_STEPS.md](AFTER_RECORDING_NEXT_STEPS.md)
- **Quick Commands**: [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
- **Full Archive**: [DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md)

---

## 🎯 Success Criteria

**MVP is "done" when**:
1. Single IR recorded (Regular Pringles Can)
2. Plugin builds without errors
3. Plugin loads in DAW
4. Audio passes through plugin
5. Convolution effect is audible
6. No crashes or major issues

**Then**:
- Add other 2 profiles (Phase 3)
- Polish UI and controls
- Ready for production

---

**Status**: ✅ Ready to start MVP

**Next Step**: Record Regular Pringles Can IR  
**Estimated Time**: 30 minutes for recording + 30 min build plugin + 20 min test = **~1.5 hours total**

---

*MVP Workflow - February 2, 2026*
