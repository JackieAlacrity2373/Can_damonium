# Recording Checklist: Regular Pringles Can (MVP)

**Goal**: Record one IR profile for MVP plugin validation

**Timeline**: ~45 minutes total
- Setup & verification: 15 min
- Recording: 15 min
- Verification: 5 min
- Buffer: 10 min

---

## ✅ Pre-Recording Setup (15 minutes)

### Equipment Checklist
- [ ] Regular size Pringles can amp is nearby
- [ ] Microphone available
- [ ] Audio interface connected
- [ ] Windows computer ready
- [ ] Recorder app built: `CanDamoniumRecorder.exe`

### Physical Setup

**Pringles Can Placement**
- [ ] Regular can positioned on stable surface
- [ ] No obstructions blocking the opening
- [ ] About 6-12 inches from microphone
- [ ] Microphone at mouth opening level

**Microphone Setup**
- [ ] Microphone positioned 6-12 inches from can opening
- [ ] Mic pointing at can opening
- [ ] Mic not touching any surfaces
- [ ] Shock mount if available

**Audio Interface**
- [ ] Connected to computer via USB
- [ ] Power is on
- [ ] Microphone input level set to -6dB (test, not clipping)
- [ ] Audio interface drivers installed

### Software Configuration

**Audio Interface Settings**
- [ ] Sample rate: 48 kHz ✓
- [ ] Bit depth: 24-bit ✓
- [ ] Buffer size: 256-512 samples
- [ ] Input: Microphone selected
- [ ] Output: Speakers/headphones (for monitoring)

**System Check**
- [ ] No background applications recording
- [ ] Antivirus not scanning (pause if needed)
- [ ] Windows audio properly configured

### Pre-Recording Audio Check (Optional but Recommended)

**Test Microphone Level**
```
1. Open Windows Sound Settings
2. Recording devices → Microphone
3. Speak into mic at normal level
4. Should show -20 to -10 dB (not clipping)
5. Adjust audio interface input level if needed
```

---

## 🎙️ Recording Session: Regular Pringles Can (15 minutes)

### Launch Application
```powershell
cd h:\Can_damonium\build\src\recorder\Release
.\CanDamoniumRecorder.exe
```

### Step-by-Step Workflow

#### Step 1: Setup ⏱️ ~1 minute
- Read setup instructions
- Verify physical placement complete
- **Click NEXT**

#### Step 2: Configuration ⏱️ ~2 minutes
- Read configuration checklist
- Verify audio interface is 48 kHz, 24-bit
- Test input level (should show green, not red)
- **Click NEXT**

#### Step 3: Recording ⏱️ ~15 seconds of actual recording
1. Dropdown: Select **"Regular Pringles Can"**
2. Click **RECORD**
   - You should hear test signal playing through speakers
   - Microphone captures the can's response
   - Progress bar shows recording progress
   - Watch for level meter (should show activity, not clipping)
3. Let it complete (14 seconds total)
4. Status will show "Processing..."
5. Wait for "Complete!" message

#### Step 4: Processing ⏱️ ~5-10 seconds (automatic)
- Application automatically:
  - Analyzes envelope
  - Trims tail below -80 dBFS
  - Normalizes to -3 dBFS
  - Verifies file integrity
- Shows progress bar

#### Step 5: Success! ⏱️ ~1 minute
- Displays success message
- Shows file location
- IR file saved to: `Documents\Can_damonium\IRs\`
- **Click DONE**

### During Recording
- ✓ Don't move microphone
- ✓ Don't touch anything
- ✓ Let the test signal play fully
- ✓ Watch level meter (should show response)
- ✓ Keep background quiet

### What to Expect
- **Hear**: Chirp sweep through speakers
- **See**: Microphone input level responding
- **Duration**: ~15 seconds total (includes processing)
- **Result**: One WAV file saved

---

## ✅ Post-Recording Verification (5 minutes)

### Verify File Was Saved
```powershell
# Check the file
$ir_file = Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\ | Sort-Object LastWriteTime -Descending | Select-Object -First 1

Write-Host "IR File: $($ir_file.Name)"
Write-Host "Size: $($ir_file.Length / 1MB) MB"
Write-Host "Last Modified: $($ir_file.LastWriteTime)"
```

Expected Output:
```
IR File: Regular_Pringles_Can_20260202_140325_48k.wav
Size: 156.3 MB
Last Modified: 2/2/2026 2:03:25 PM
```

### Verify File Specifications

**File Properties**
- [ ] Filename: `Regular_Pringles_Can_YYYYMMDD_HHMMSS_48k.wav`
- [ ] Format: WAV (48 kHz, 24-bit, mono)
- [ ] Size: 100-200 MB (reasonable for ~5 sec @ 48kHz 24-bit)
- [ ] Modified time: Just now ✓

### Optional: Listen to Recording

**In Windows Media Player or Audacity**
1. Open file from: `Documents\Can_damonium\IRs\`
2. Listen to recording
3. Should sound like:
   - ~1 second ramp-up
   - Peak amplitude in middle
   - ~2-4 second tail decay
   - All from Pringles can reverb

**Quality Check**
- [ ] No clipping/distortion
- [ ] Clean recording (no noise)
- [ ] Obvious can resonance
- [ ] Tail trails off smoothly
- [ ] Total duration: 3-6 seconds

---

## 🎯 Success Checklist

- [ ] Application built successfully
- [ ] Pre-recording setup complete
- [ ] Audio interface verified at 48 kHz, 24-bit
- [ ] Recording session completed
- [ ] File saved to correct location
- [ ] File has correct name format
- [ ] File size is reasonable (100-200 MB)
- [ ] File is valid WAV format
- [ ] Recording quality is good (no clipping)
- [ ] Effect is audible (can resonance obvious)

---

## 📊 Summary

**Regular Pringles Can IR Successfully Recorded! ✓**

Your recording is ready for:
1. **Phase 2b**: Build MVP VST3 plugin with this IR
2. **Phase 2c**: Test plugin in your DAW

Next: See [PHASE_2_VST3_MVP.md](PHASE_2_VST3_MVP.md) or [AFTER_RECORDING_NEXT_STEPS.md](AFTER_RECORDING_NEXT_STEPS.md)

---

## 🚨 Troubleshooting

### Application Crashes During Recording
```powershell
# Restart and try again
# If persists, rebuild:
cd h:\Can_damonium\build
cmake --build . --config Release --clean-first
```

### No File Created
- [ ] Check: `Documents\Can_damonium\IRs\` folder exists (app creates it)
- [ ] Check: Disk has space (need ~200 MB)
- [ ] Check: Read permissions on Documents folder
- [ ] Rebuild and try again

### File Too Small (< 50 MB)
- [ ] Recording may have been cut short
- [ ] Try again with longer settings (if available)
- [ ] Check microphone was on during recording

### Clipping/Distortion
- [ ] Lower input level on audio interface
- [ ] Move microphone further from can
- [ ] Try again with reduced levels

### No Sound/Silence in Recording
- [ ] Check microphone is plugged in
- [ ] Check audio interface input selected
- [ ] Check Windows recording device set to correct interface
- [ ] Test with another application first

### Application Won't Launch
```powershell
# Make sure build completed
Test-Path h:\Can_damonium\build\src\recorder\Release\CanDamoniumRecorder.exe

# If missing, rebuild:
cd h:\Can_damonium\build
cmake --build . --config Release
```

---

## 📝 Notes

- **Duration**: Recording + processing takes ~15-20 seconds total
- **File Size**: Regular can IR at 48kHz 24-bit produces ~100-200 MB file
- **One Profile**: This MVP records only Regular Pringles Can
- **Other Profiles**: Small and Grande IRs added later after MVP validates
- **Next Step**: Build plugin with this single IR

---

**Status**: Ready to record Regular Pringles Can IR

*Last Updated: February 2, 2026*
