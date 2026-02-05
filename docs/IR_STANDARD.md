# Can Damonium Impulse Response Standard (v1.0)

## Overview
This document defines the standardized format and methodology for capturing impulse responses (IRs) from amplifiers and audio equipment using the Can Damonium IR recording ecosystem.

## 1. Test Signal Specification

### Signal Type: Logarithmic Frequency Sweep (Chirp)
- **Range**: 20 Hz to 20,000 Hz (full human hearing range)
- **Type**: Logarithmic chirp (frequency increases exponentially)
- **Fade In/Out**: 50ms fade-in and fade-out to prevent clicks
- **Sweep Duration**: 10 seconds
- **Amplitude**: -3dBFS (decibels relative to full scale) to provide headroom

**Rationale**: 
- Linear sweeps don't evenly distribute energy across frequency range
- Logarithmic chirps are better for capturing full frequency response
- 10 seconds provides good frequency resolution
- -3dBFS avoids clipping while capturing transients

### Alternative Test Signals (for comparison)
- White noise burst (1-4 seconds, -3dBFS)
- MLS (Maximum Length Sequence) signal

## 2. Audio Format Specification

### Recording Parameters
- **Sample Rate**: 48,000 Hz (48 kHz)
- **Bit Depth**: 24-bit
- **Channels**: Mono (single microphone input)
- **File Format**: WAV (PCM, uncompressed)

**Rationale**:
- 48 kHz is standard in professional audio and provides 24 kHz bandwidth (>20 kHz hearing limit)
- 24-bit captures sufficient dynamic range and reduces quantization noise
- Mono simplifies capture and convolution processing
- WAV is universally compatible and lossless

## 3. Impulse Response Duration

### Captured Duration
- **Recording Length**: 14 seconds (4 seconds of silence before sweep + 10 seconds sweep)
- **Processing Duration**: 4-6 seconds (after decay analysis)

**Post-Processing**:
1. Record full sweep + tail
2. Analyze decay envelope
3. Trim when signal drops below -80 dBFS (noise floor)
4. Final IR typically 2-4 seconds depending on equipment characteristics

## 4. Recording Setup Requirements

### Audio Interface Requirements
- Minimum 24-bit, 48 kHz capability
- Line-level input/output (-10 dBV or +4 dBu standard)
- Low-noise preamp (preferably >90 dB SNR)

### Equipment Chain
```
Test Signal Generator → Amplifier → Microphone → Audio Interface → Recording
```

### Microphone Placement
- Microphone should be positioned perpendicular to speaker cone
- Distance: 6-12 inches from speaker (10 cm typical for Pringles cans)
- Avoid reflective surfaces nearby (treat with soft materials if needed)

## 5. File Naming Convention

```
{profile_name}_{serial_or_id}_{date}_{samplerate}.wav
```

**Examples**:
- `Small_Pringles_Original_2026-02-02_48k.wav`
- `Regular_Pringles_Custom_2026-02-02_48k.wav`
- `Grande_Pringles_Experiment_2026-02-02_48k.wav`

## 6. Quality Validation Checklist

Before accepting a recorded IR, verify:
- [ ] Duration matches specification (14 seconds total)
- [ ] Sample rate is exactly 48,000 Hz
- [ ] Bit depth is 24-bit
- [ ] Peak level doesn't exceed -0.5 dBFS (no clipping)
- [ ] Peak level is at least -18 dBFS (good signal capture)
- [ ] Frequency response is reasonably flat in critical bands
- [ ] No extraneous noise or artifacts

## 7. Processing Pipeline

### Steps:
1. **Load** 48 kHz, 24-bit WAV file
2. **Validate** file format and parameters
3. **Analyze** envelope decay
4. **Trim** to appropriate length (2-4 seconds typically)
5. **Normalize** peak to -3 dBFS for consistent convolution
6. **Save** as embedded resource in plugin or user library

## Future Enhancements

- Support for stereo IRs (left/right channels)
- Alternative sample rates (96 kHz, 192 kHz)
- Automated deconvolution techniques
- Frequency-dependent windowing

---

**Last Updated**: February 2, 2026
**Version**: 1.0
