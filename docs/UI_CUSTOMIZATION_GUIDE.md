# Can Damonium UI Customization Guide

## Overview
The Can Damonium VST plugin now features dynamic UI customization with resizable windows and Pringles flavor-themed color schemes.

## Features Implemented

### 1. Can Size Selection
The window size automatically adjusts based on the selected can size to match real-world proportions:

| Can Size | Dimensions | Aspect Ratio | Status |
|----------|-----------|--------------|--------|
| **Small** | 360 × 280 | 1.29:1 | 📋 Pending IR Recording |
| **Regular** | 700 × 550 | 1.27:1 | ✅ Currently Implemented |
| **Large** | 870 × 685 | 1.27:1 | 📋 Pending IR Recording |

**How to use:**
1. Open the plugin UI
2. Find the "Can Size" dropdown (next to "Can Flavor")
3. Select: Small, Regular, or Large
4. Window automatically resizes and repositions UI elements

### 2. Flavor Color Customization
Choose from 10 popular Pringles flavors, each with a unique color scheme:

| # | Flavor | Color | Hex Code | Notes |
|---|--------|-------|----------|-------|
| 1 | Original/Plain | Red | #E31937 | Classic iconic red |
| 2 | Sour Cream & Onion | Green | #2E8B57 | Traditional favorite |
| 3 | Cheddar Cheese | Gold/Orange | #FF8C00 | Warm cheesy tone |
| 4 | Jalapeño | Spicy Red-Orange | #FF6347 | Represents heat |
| 5 | Screamin' Dill Pickle | Teal | #20B2AA | Cool pickle vibes |
| 6 | Rotisserie Chicken | Warm Brown | #A0522D | Savory meat flavor |
| 7 | Soft-Shell Crab | Tan/Beige | #D2B48C | Subtle seafood tone |
| 8 | Blueberry & Hazelnut | Purple | #9370DB | Premium/sweet |
| 9 | Pumpkin Pie Spice | Dark Orange | #D2691E | Autumn seasonal |
| 10 | Grilled Shrimp | Salmon Pink | #FA8072 | Seafood option |

**How to use:**
1. Open the plugin UI
2. Find the "Can Flavor" dropdown (top-left)
3. Select your preferred flavor
4. Background color subtly tints to the flavor's primary color (15% blend with dark grey for readability)

### 3. Dynamic Window Resizing
The UI layouts automatically reposition and resize based on your selected can size:

**Small (360×280):**
- Compact layout
- All controls still accessible
- Ideal for floating plugin windows
- Meters scaled appropriately

**Regular (700×550):**
- Default size
- All controls clearly visible
- Optimal for most DAWs
- Full-size meters

**Large (870×685):**
- Expanded layout
- Extra space for future controls
- Suitable for main monitor
- Large, easy-to-read elements

## Technical Implementation

### Code Structure

#### Header File: `PluginEditor.h`
```cpp
// Can sizing data
struct CanSize {
    int width;
    int height;
    const char* name;
};
static constexpr CanSize canSizes[3] = {
    {360, 280, "Small"},
    {700, 550, "Regular"},
    {870, 685, "Large"}
};

// Flavor data with RGB colors
struct FlavorInfo {
    const char* name;
    uint32_t rgbColor;  // 0xRRGGBB format
};
static constexpr FlavorInfo flavors[10] = { ... };
```

#### Key Methods:
- `updateCanFlavor(int flavorIndex)` - Changes flavor and repaints UI
- `updateCanSize(int sizeIndex)` - Resizes window and repositions controls
- `getCurrentFlavorColor()` - Returns juce::Colour from flavor RGB data

#### UI Dropdowns:
- **Can Flavor Selector:** Top-left dropdown with 10 options
- **Can Size Selector:** Top-center dropdown with 3 options
- **IR Selector:** Loads impulse response files
- **Other Controls:** Reload IR, Bypass, Test Tone, Audio Settings buttons

### Color Implementation
- Flavor colors are stored as `uint32_t` in `0xRRGGBB` format
- Converted to JUCE `Colour` objects at paint time
- Background uses 15% blend with dark grey for subtle theming
- Maintains text readability while providing visual feedback

### Window Sizing Logic
```cpp
void PluginEditor::updateCanSize(int sizeIndex)
{
    // Set new dimensions
    setSize(canSizes[sizeIndex].width, canSizes[sizeIndex].height);
    
    // UI elements automatically reposition in resized() method
    // (uses calculated coordinates based on getWidth()/getHeight())
}
```

## Future Enhancements

### Phase 2 Roadmap
1. **Small Can IR Recording** - Record at native 96kHz
2. **Large Can IR Recording** - Record at native 96kHz
3. **Custom Color Picker** - Allow users to define custom flavor colors
4. **Preset Management** - Save/load favorite flavor+size combinations
5. **Flavor-Specific Effects** - Different convolver settings per flavor (future)
6. **Multi-Platform Testing** - Test window resizing on macOS and Linux

### Recording New IRs
To add new IRs for Small and Large cans:

1. **Prepare Can:**
   - Small can: 4.75" tall
   - Large can: 11.5" tall
   - Both at matching orientation

2. **Use IR Recorder App:**
   - Launch: `Can Damonium IR Recorder.exe`
   - Set Audio → Audio Settings to your device
   - Click "Start Recording"
   - Pringles IR Recorder generates logarithmic chirp
   - Follow on-screen instructions
   - Automatic normalization and WAV save

3. **File Location:**
   - Saved to: `Documents\Can_damonium\IRs\`
   - Format: `[CanSize]_[Date]_[Time]_96k.wav`
   - Plugin auto-scans this directory on startup

4. **Naming Convention (Recommended):**
   ```
   SmallCan_[YYYYMMDD]_[HHMMSS]_96k.wav
   LargeCan_[YYYYMMDD]_[HHMMSS]_96k.wav
   ```

## Usage Notes

### Best Practices
1. **Recording IRs:** Always record at device's native sample rate (96kHz for Focusrite)
2. **Can Orientation:** Keep can vertical during recording for consistency
3. **Flavor Selection:** Choose flavor that matches your physical can
4. **Size Selection:** Should match your physical can dimensions
5. **Window Resizing:** May require DAW refresh in some hosts

### Troubleshooting
- **Window doesn't resize?** Ensure VST host allows plugin window resizing
- **Colors look wrong?** Check your display color profile
- **IR won't load?** Ensure file is 96kHz mono or stereo WAV
- **Dropdown not responding?** Try clicking directly on dropdown arrow

## Files Modified
- `src/plugin/PluginEditor.h` - Added flavor/size data structures
- `src/plugin/PluginEditor.cpp` - Added UI controls and sizing logic
- `docs/FLAVOR_ROADMAP.md` - Roadmap document (this section)

## Testing Checklist
- [x] Can Flavor dropdown selectable
- [x] Can Size dropdown resizes window
- [x] Colors update on flavor change
- [x] All UI elements reposition correctly
- [x] IR loading still works with new UI
- [x] Test tone button functional
- [x] Audio Settings accessible
- [ ] Small can IR recorded and loaded
- [ ] Large can IR recorded and loaded
- [ ] Cross-platform testing (macOS/Linux)

