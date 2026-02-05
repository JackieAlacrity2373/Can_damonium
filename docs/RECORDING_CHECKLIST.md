# Recording Session Checklist

## Pre-Recording Equipment Verification

### Audio Interface Setup
- [ ] Audio interface connected via USB
- [ ] Power cable connected (if separate)
- [ ] Drivers installed (Windows shows device)
- [ ] Sample rate set to 48 kHz in interface settings
- [ ] Bit depth set to 24-bit

### Microphone Setup
- [ ] Microphone cable connected to interface input
- [ ] Microphone powered (XLR phantom power if needed)
- [ ] Input level set to appropriate gain (-12dB to -6dB target)
- [ ] No clipping lights/indicators on interface

### Amplifier Setup (Small Pringles Can)
- [ ] Pringles can amp powered on
- [ ] Speaker is functioning
- [ ] Volume set to normal operating level
- [ ] Tone controls set to desired settings
- [ ] Positioned securely, won't move

### Microphone Positioning (Small IR)
- [ ] Microphone 6-10 inches from speaker
- [ ] Positioned perpendicular to speaker cone (aim at center)
- [ ] Microphone stand stable, won't vibrate
- [ ] No cables in way of microphone
- [ ] Room quiet (doors/windows closed)

### Recording Environment
- [ ] Quiet room (no fans, air conditioning off if possible)
- [ ] No other applications playing audio
- [ ] Notifications muted (email, chat, etc.)
- [ ] Phone on silent
- [ ] No traffic noise nearby

---

## Recording Session: Small Pringles Can

### Step 1: Launch Application
```powershell
.\build\src\recorder\Release\CanDamoniumRecorder.exe
```
- [ ] Application window opens
- [ ] Title shows "Can Damonium IR Recorder"
- [ ] Step 1 instructions visible

### Step 2: Physical Setup Instructions
- [ ] Read setup instructions in app
- [ ] Verify all equipment positioned correctly
- [ ] Click NEXT to proceed

### Step 3: Audio Configuration Guide
- [ ] Read configuration checklist
- [ ] Verify audio interface settings:
  - [ ] 48,000 Hz sample rate
  - [ ] 24-bit bit depth
  - [ ] Input level showing signal (-12 to -6 dB)
  - [ ] No red clipping indicators
- [ ] Click NEXT when ready

### Step 4: Recording
- [ ] Equipment selector shows "Small Pringles Can"
- [ ] Click RECORD button
- [ ] Level meter shows real-time input signal
- [ ] Progress bar starts filling
- [ ] You hear frequency sweep (20Hz-20kHz chirp)
- [ ] Microphone captures the response
- [ ] Wait for recording to auto-complete (~14 seconds)
- [ ] DO NOT touch anything during recording

### Step 5: Processing
- [ ] Status shows "Processing..."
- [ ] Application validates format, analyzes envelope
- [ ] Takes 2-5 seconds
- [ ] Status updates to "✓ IR successfully saved"

### Step 6: Success
- [ ] Step 5 complete screen shows confirmation
- [ ] Message shows file saved to Documents\Can_damonium\IRs\
- [ ] Filename visible (something like: Small_Pringles_Can_20260202_153045_48k.wav)

### Verification
- [ ] Check file was created:
```powershell
Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\
```
- [ ] Should show: `Small_Pringles_Can_*_48k.wav`
- [ ] File size should be ~100-150 MB

---

## Recording Session: Regular Pringles Can

### Equipment Change
- [ ] Remove Small Pringles Can
- [ ] Reposition Regular Pringles Can
- [ ] Adjust microphone distance if needed (6-12 inches)
- [ ] Check equipment is stable

### Recording Steps
- [ ] In app, click "NEW RECORDING" button
- [ ] Equipment selector changes to "Regular Pringles Can"
- [ ] Follow Steps 1-5 above (same as Small)

### Verification
- [ ] Check file created:
```powershell
Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\
```
- [ ] Should show: `Regular_Pringles_Can_*_48k.wav`

---

## Recording Session: Grande Pringles Can

### Equipment Change
- [ ] Remove Regular Pringles Can
- [ ] Reposition Grande Pringles Can
- [ ] Adjust microphone distance if needed (6-12 inches)
- [ ] Check equipment is stable

### Recording Steps
- [ ] In app, click "NEW RECORDING" button
- [ ] Equipment selector changes to "Grande Pringles Can"
- [ ] Follow Steps 1-5 above (same as Small)

### Verification
- [ ] Check file created:
```powershell
Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\
```
- [ ] Should show: `Grande_Pringles_Can_*_48k.wav`

---

## Final Verification - All Three IRs

```powershell
$path = "$env:USERPROFILE\Documents\Can_damonium\IRs\"
Get-ChildItem $path | Measure-Object -Property Length -Sum
```

Expected result:
- [ ] 3 files total
- [ ] Files: Small_*, Regular_*, Grande_*
- [ ] Each file ~100-200 MB
- [ ] Total ~300-600 MB

### Optional: Verify with Audacity

1. Open Audacity
2. File → Open → Documents\Can_damonium\IRs\Small_Pringles_Can_*_48k.wav
3. Verify:
   - [ ] Sample rate shows 48000 Hz
   - [ ] Shows "Mono" (1 channel)
   - [ ] Duration is 3-6 seconds
   - [ ] Waveform shows frequency sweep pattern
   - [ ] No clipping at waveform edges
4. Repeat for Regular and Grande

---

## Success! 

All three impulse responses recorded and saved:
- ✅ Small_Pringles_Can_*_48k.wav
- ✅ Regular_Pringles_Can_*_48k.wav
- ✅ Grande_Pringles_Can_*_48k.wav

**Next step**: Embed these IR files into the VST3 plugin and implement convolution.

---

## Troubleshooting During Recording

### Issue: No sound playing from amp
**Solution**:
- Check audio interface output cable
- Verify app isn't muted (operating system volume)
- Ensure amp is powered and volume is up
- Try re-running application

### Issue: Microphone input showing no signal
**Solution**:
- Check microphone cable connections
- Verify microphone is powered (phantom power)
- Check input level on audio interface
- Try different input on interface
- Move microphone closer to speaker

### Issue: Recording shows high clipping
**Solution**:
- Lower input level on audio interface
- Move microphone further from speaker
- Reduce amp volume slightly
- Try re-recording

### Issue: Application crashes during recording
**Solution**:
- Close other applications
- Update audio drivers
- Restart computer
- Try again

### Issue: WAV file not created
**Solution**:
- Check Documents\Can_damonium\ folder exists
- IRs\ subfolder should be created automatically
- Check disk space available
- Verify Write permissions to Documents folder

---

**Estimated Time**: 15 minutes per IR = 45 minutes total for all three recordings

**Ready to Record**: ✅ YES

Proceed with recording session when equipment is ready!
