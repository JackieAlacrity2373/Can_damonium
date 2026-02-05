# 🎯 Can Damonium: From Recording to MVP Plugin

**Revised Plan: Single IR for MVP, then Expand**

---

## 📋 Executive Summary

**Old Plan**: Record 3 IRs → Build plugin with all 3  
**New Plan**: Record 1 IR → Build MVP plugin → Test → Then add 2 more  

**Why?**
- Faster validation of full workflow
- Catch issues early
- Working plugin in your DAW sooner
- Lower risk before recording other profiles

---

## 🚀 Quick Path to Working Plugin

### Total Time: ~1.5 hours

| # | Phase | Task | Time | Status |
|---|-------|------|------|--------|
| 1 | 2a | Record Regular Pringles Can IR | 30 min | Ready |
| 2 | 2b | Build MVP VST3 Plugin | 30 min | Ready |
| 3 | 2c | Test in DAW | 20 min | Ready |
| | | **MVP Complete** | **~1.5 hr** | **Today!** |

Then (Phase 3): Add Small & Grande IRs (later)

---

## 📖 Documentation (Updated for MVP)

### Start Here
1. **[MVP_WORKFLOW.md](MVP_WORKFLOW.md)** ← Read this first! (10 min)
   - Explains the revised plan
   - Timeline and workflow
   - Success criteria

### Then Record
2. **[RECORDING_CHECKLIST_SINGLE_IR.md](RECORDING_CHECKLIST_SINGLE_IR.md)** (15 min)
   - Record Regular Pringles Can only
   - Step-by-step guide
   - What to expect

### Then Build
3. **[PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md)** (20 min)
   - Create MVP plugin
   - Embed single IR
   - Build and test

### All Docs
- **[DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md)** - Complete index

---

## 🎙️ Phase 2a: Record Regular Pringles Can

**What**: Record one IR profile  
**Time**: ~30 minutes  
**File**: `Regular_Pringles_Can_YYYYMMDD_HHMMSS_48k.wav`  
**Location**: `Documents\Can_damonium\IRs\`  

**Steps**:
```powershell
# Launch recorder
h:\Can_damonium\build\src\recorder\Release\CanDamoniumRecorder.exe

# In app:
# 1. Step 1 (setup) → NEXT
# 2. Step 2 (config) → NEXT
# 3. Select "Regular Pringles Can" → RECORD
# 4. Wait ~14 seconds
# 5. Steps 4-5 auto-process
# 6. Done! File saved
```

**Verify**:
```powershell
Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\
```

Expected: `Regular_Pringles_Can_*.wav` (~150 MB)

---

## 🏗️ Phase 2b: Build MVP Plugin

**What**: Create VST3 plugin with Regular IR embedded  
**Time**: ~30 minutes  
**File**: `CanDamoniumPlugin.vst3`  

**Steps**:
```powershell
# Copy IR to plugin resources
Copy-Item "$env:USERPROFILE\Documents\Can_damonium\IRs\Regular_Pringles_Can*.wav" `
  h:\Can_damonium\src\plugin\Resources\ -Force

# Build plugin
cd h:\Can_damonium\build
cmake --build . --config Release

# Verify
Test-Path h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3
```

**Code Changes**:
- Create `ConvolverProcessor.h/cpp` (real-time convolution)
- Modify `PluginProcessor.cpp` (load IR, process audio)
- Update `CMakeLists.txt` (embed resource)

See: [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md)

---

## 🎵 Phase 2c: Test in DAW

**What**: Load plugin and verify convolution works  
**Time**: ~20 minutes  
**DAW**: Any VST3-compatible (Reaper, Studio One, Ableton 12+, Cubase 12+)  

**Steps**:
```powershell
# Install plugin
$plugin = "h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3"
$vst3 = "$env:PROGRAMFILES\Common Files\VST3"
Copy-Item $plugin $vst3 -Force

# In DAW:
# 1. Rescan plugins
# 2. Insert "Can Damonium Plugin" on track
# 3. Send audio to it
# 4. Listen to output
# 5. Verify convolution effect audible
```

**Success Indicators**:
- ✓ Plugin loads without crash
- ✓ Audio passes through
- ✓ You hear the convolution effect
- ✓ No glitches or artifacts
- ✓ Latency ~5-6 seconds (normal)

---

## ✅ MVP Success = All Three Phases Work

**Phase 2a**: 1 IR recorded  
**Phase 2b**: Plugin built  
**Phase 2c**: Plugin tested in DAW  

**Result**: Working convolver plugin with Pringles can IR ✓

---

## 🎯 After MVP: Phase 3 (Later)

Once MVP is proven:

### Phase 3a: Record Other Profiles
- Record Small Pringles Can
- Record Grande Pringles Can

### Phase 3b: Expand Plugin
- Add profile selector UI
- Support all 3 IRs
- Load selected profile

### Phase 3c: Polish (Optional)
- Add level controls
- Add wet/dry mix
- Show waveform
- Add bypass

---

## 🗺️ Documentation Map

**MVP Path**:
```
MVP_WORKFLOW.md
    ↓
RECORDING_CHECKLIST_SINGLE_IR.md (Record Regular)
    ↓
PHASE_2_VST3_MVP.md (Build plugin)
    ↓
Test in DAW (Success!)
```

**All Documentation**:
- [DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md) ← Complete guide
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) ← Quick commands
- [BUILD_AND_RECORD_GUIDE.md](docs/BUILD_AND_RECORD_GUIDE.md) ← Original complete guide

---

## 💡 Why This Approach?

**Risks Reduced**:
- ✓ Find issues with 1 profile, not 3
- ✓ Validate full workflow sooner
- ✓ Get working plugin in DAW today
- ✓ Easier to debug problems

**Benefits**:
- ✓ Faster MVP
- ✓ Lower risk
- ✓ Proof of concept early
- ✓ Add profiles when proven

**Timeline**:
- Old: Record all 3 (45 min) → Build plugin (1.5 hr) = 2+ hours
- New: Record 1 (15 min) → Build plugin (30 min) → Test (20 min) = **1.5 hours**

---

## 📊 Comparison

| Metric | Original Plan | MVP Plan |
|--------|---------------|----------|
| IRs for MVP | 3 | 1 (Regular) |
| Recording Time | 45 min | 15 min |
| Plugin Dev Time | 1.5 hr | 30 min |
| Test Time | 15 min | 20 min |
| **Total Time** | **~2.5 hr** | **~1.5 hr** |
| Risk Level | Higher | Lower |
| Working Plugin | After 2+ hours | After 1.5 hours |

---

## 🚀 Start Here

1. **Read**: [MVP_WORKFLOW.md](MVP_WORKFLOW.md) (10 min)
2. **Record**: [RECORDING_CHECKLIST_SINGLE_IR.md](RECORDING_CHECKLIST_SINGLE_IR.md) (30 min)
3. **Build**: [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md) (30 min)
4. **Test**: Launch plugin in DAW (20 min)

**Total**: ~1.5 hours to working plugin ✓

---

## ❓ FAQ

### Why not record all 3 first?
Recording all 3 means if anything goes wrong, you might need to redo them all. With MVP, you validate the workflow with 1, then record others when proven.

### Will the MVP plugin work?
Yes. It will load Regular Pringles Can IR and convolve audio through it. No fancy UI, but it works.

### When do I record Small & Grande?
After MVP is tested and working. See Phase 3 in [MVP_WORKFLOW.md](MVP_WORKFLOW.md).

### Can I record all 3 before building plugin?
Yes! If you prefer, use [RECORDING_CHECKLIST.md](docs/RECORDING_CHECKLIST.md) for all 3 profiles first. But MVP approach is recommended for lower risk.

### What if plugin crashes?
See troubleshooting in [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md). Usually easy fixes like rebuild or resource path.

---

## ✨ Summary

**Old Plan**: Record 3 → Build plugin with all 3 → Maybe it works

**New Plan**: Record 1 → Build & test → Works! → Add 2 more

**Result**: Faster MVP, lower risk, proof of concept today

---

**Status**: ✅ Ready to start MVP workflow

**Next Action**: Read [MVP_WORKFLOW.md](MVP_WORKFLOW.md)

---

*MVP Plan - February 2, 2026*
*Can Damonium: Single IR to Working Plugin*
