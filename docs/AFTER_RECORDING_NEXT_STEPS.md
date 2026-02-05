# After Recording: Next Steps to MVP Plugin

**Status**: You've successfully recorded `Regular_Pringles_Can_YYYYMMDD_HHMMSS_48k.wav` ✓

This guide shows exactly what comes next.

---

## 🎯 Your IR Recording

- **File**: `Regular_Pringles_Can_YYYYMMDD_HHMMSS_48k.wav`
- **Location**: `Documents\Can_damonium\IRs\`
- **Format**: 48 kHz, 24-bit mono, PCM WAV
- **Duration**: ~3-6 seconds
- **Ready**: Yes! ✅

---

## 📋 Phase 2: MVP VST3 Plugin (Single IR)

### What You'll Build
A working VST3 plugin that:
- Loads the Regular Pringles Can IR
- Processes audio through convolution
- Works in your DAW
- Can be tested end-to-end

### Why Single IR First?
- ✅ Validate full recording → plugin → test workflow
- ✅ Catch any issues early
- ✅ Faster to MVP
- ✅ Easier to debug
- ✅ Then add other 2 profiles

---

## 🚀 Next Steps (In Order)

### Step 1: Locate Your Recording ⏱️ 1 min
```powershell
# Verify the file exists
$ir_file = Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\ | Sort-Object LastWriteTime -Descending | Select-Object -First 1
Write-Host "Your IR file: $($ir_file.Name)"
Write-Host "File size: $($ir_file.Length / 1MB)MB"
```

Expected output:
```
Your IR file: Regular_Pringles_Can_20260202_140325_48k.wav
File size: 156.3MB
```

### Step 2: Prepare Plugin Project ⏱️ 5 min

The plugin skeleton already exists at `src/plugin/`. We need to:
1. Add IR embedding system
2. Create convolver processor
3. Build basic UI

### Step 3: Embed IR in Plugin ⏱️ 10 min

Convert WAV to binary resource:
```powershell
# Copy IR to plugin resources
Copy-Item "$env:USERPROFILE\Documents\Can_damonium\IRs\Regular_Pringles_Can*.wav" `
  h:\Can_damonium\src\plugin\Resources\ -Force
```

### Step 4: Build MVP Plugin ⏱️ 3-5 min

```powershell
cd h:\Can_damonium\build
cmake --build . --config Release
# Check for: CanDamoniumPlugin.vst3
```

### Step 5: Test in DAW ⏱️ 10-15 min

- Copy plugin to VST3 folder
- Load in your DAW (Reaper, Ableton, Studio One, etc.)
- Feed audio through it
- Listen to convolved output

### Step 6: Verify Results ⏱️ 5 min

✓ Plugin loads
✓ No crashes
✓ Audio passes through
✓ You hear the IR effect
✓ Output sounds like Pringles can amp

---

## 📝 Detailed Instructions

### Create Plugin Resources Directory
```powershell
New-Item -ItemType Directory -Path h:\Can_damonium\src\plugin\Resources -Force
```

### Copy Your Recording
```powershell
$latest_ir = Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\ | Sort-Object LastWriteTime -Descending | Select-Object -First 1
Copy-Item $latest_ir.FullName h:\Can_damonium\src\plugin\Resources\regular_pringles.wav -Force
Write-Host "Copied: $($latest_ir.Name)"
```

### Build Command
```powershell
cd h:\Can_damonium\build
cmake --build . --config Release --verbose
```

### Plugin Location (After Build)
```
h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3
```

### Install for DAW (Windows)
```powershell
# VST3 plugins folder
$vst3_folder = "$env:PROGRAMFILES\Common Files\VST3"

# Copy plugin
Copy-Item h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3 $vst3_folder -Force

# Rescan plugins in your DAW
Write-Host "✓ Plugin installed to: $vst3_folder"
Write-Host "Rescan plugins in your DAW to see it"
```

---

## 🔄 Complete Workflow Checklist

### Before Building Plugin
- [ ] Recording completed: `Regular_Pringles_Can_*.wav` exists
- [ ] File is in: `Documents\Can_damonium\IRs\`
- [ ] File size: 100-200 MB ✓
- [ ] Built on `build/` directory exists
- [ ] CMakeLists.txt configured ✓

### Building MVP Plugin
- [ ] `Resources/` directory created
- [ ] IR copied to `src/plugin/Resources/`
- [ ] CMake build executes without errors
- [ ] `.vst3` file generated in Release folder
- [ ] No build warnings (acceptable)

### Testing in DAW
- [ ] Plugin installed to VST3 folder
- [ ] DAW rescanned plugins
- [ ] Plugin loads without crash
- [ ] Plugin appears in plugin list
- [ ] Can instantiate (add to track)
- [ ] Audio passes through
- [ ] Effect is audible
- [ ] No audio glitches

### Success Criteria
- ✅ Plugin loads
- ✅ Plugin doesn't crash
- ✅ Audio flows through
- ✅ You can hear the Pringles can IR effect

---

## 📊 Timeline

| Step | Task | Time |
|------|------|------|
| 1 | Verify IR file | 1 min |
| 2 | Prepare plugin | 5 min |
| 3 | Embed IR | 10 min |
| 4 | Build plugin | 5 min |
| 5 | Test in DAW | 15 min |
| **Total** | **MVP Complete** | **~40 min** |

---

## 🚨 If Something Goes Wrong

### Build Fails
```powershell
# Clean rebuild
cd h:\Can_damonium\build
cmake --build . --config Release --clean-first
# or
rm -r h:\Can_damonium\build
mkdir h:\Can_damonium\build
cd h:\Can_damonium\build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Plugin Doesn't Load
- [ ] Check VST3 folder path
- [ ] Verify plugin file exists: `CanDamoniumPlugin.vst3`
- [ ] Try 64-bit version (not 32-bit)
- [ ] Check DAW compatibility (Reaper, Studio One, Ableton 12+)

### No Audio
- [ ] Check DAW track routing
- [ ] Verify IR file copied to Resources/
- [ ] Check plugin window (should show waveform)
- [ ] Try mono input (1 channel)

### Audio Distorts
- [ ] Normal - convolution is processing
- [ ] Adjust input/output levels
- [ ] Plugin has no level control yet (MVP)

See [BUILD_AND_RECORD_GUIDE.md](docs/BUILD_AND_RECORD_GUIDE.md#troubleshooting) for more.

---

## 🎯 After MVP Works

### Phase 2a: Test & Verify
- ✓ Record audio through it
- ✓ Save output file
- ✓ Verify sound quality
- ✓ Compare with original IR

### Phase 2b: Add UI
- Add input/output level control
- Add wet/dry mix control
- Show IR waveform
- Add bypass button

### Phase 3: Add Other Profiles
- Record Small Pringles Can IR
- Record Grande Pringles Can IR
- Add profile selector to plugin UI
- Update plugin to load selected IR

---

## 📖 Documentation Reference

For detailed information:
- Build issues: [BUILD_AND_RECORD_GUIDE.md](docs/BUILD_AND_RECORD_GUIDE.md)
- Technical specs: [IR_STANDARD.md](docs/IR_STANDARD.md)
- Quick commands: [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
- Architecture: [PROJECT_TREE.md](PROJECT_TREE.md)
- Phase 2 details: [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md) (coming next)

---

## ✅ You're Ready!

Your IR is recorded and validated. Time to build the MVP plugin and get it working end-to-end.

**Next command to run:**
```powershell
# Verify IR exists
Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\
```

Once you see your file, you're ready to start Phase 2.

---

*Last Updated: February 2, 2026*
*Status: Ready for MVP Plugin Development*
