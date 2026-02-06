# Convolution Implementation: Can_damonium vs JUCE Demo

## Key Differences Found

### 1. **IR Loading Mechanism**: JUCE Demo vs Can_damonium

#### JUCE Demo (Correct Approach)
```cpp
// In process() - called every audio callback (audio thread)
convolution.loadImpulseResponse (std::move (buf.buffer),
                                 buf.sampleRate,
                                 Convolution::Stereo::yes,
                                 Convolution::Trim::yes,
                                 Convolution::Normalise::yes);
```

**Key insight**: Loads IR **inside every process() call**, but only when new buffer is available (via try-lock).

#### Can_damonium (Current Approach - **PROBLEM**)
```cpp
// In PluginEditor.cpp (UI thread)
processor.loadImpulseResponse (selectedIR.file);  // Blocks UI!

// In PluginProcessor::processBlock()
if (convolutionEngine)
    convolutionEngine->processBlock (buffer);  // No concurrent IR loading
```

**The problem**: 
- IR is loaded on **UI thread** via `loadImpulseResponse()` 
- `processBlock()` doesn't check for new IR
- File I/O blocks the UI

---

### 2. **Thread Safety**: JUCE Uses SpinLock, Can_damonium Doesn't

#### JUCE Demo
```cpp
class BufferTransfer
{
    template <typename Fn>
    void get (Fn&& fn)  // Called from audio thread
    {
        const SpinLock::ScopedTryLockType lock (mutex);  // Non-blocking try-lock
        if (lock.isLocked() && newBuffer)
        {
            fn (buffer);  // Load IR safely
            newBuffer = false;
        }
    }
};
```

**Result**: Non-blocking check. If UI thread is setting IR, audio thread skips this frame safely.

#### Can_damonium
```cpp
// ConvolutionEngine.h
"Check if IR is deferred, but no try-lock"
if (deferredIRFile.existsAsFile())
{
    juce::Logger::writeToLog("Loading deferred IR...");
    loadImpulseResponse(deferredIRFile);  // Could block if called from audio thread
}
```

**Result**: No spinlock protection. Risk of race conditions or blocking.

---

### 3. **When IR Loading Happens**

#### JUCE Demo
- **UI sets IR** → `bufferTransfer.set()` queues buffer (fast)
- **Every audio callback** → `bufferTransfer.get()` checks if new IR queued (non-blocking try-lock)
- **If available** → loads IR right there in `process()`

#### Can_damonium
- **UI selects IR** → `processor.loadImpulseResponse()` called directly (blocks)
- **prepareToPlay()** → checks deferred file, loads it (but only once on init)
- **processBlock()** → just uses loaded IR if available (no checking for new IR)

---

### 4. **Missing: Per-Block IR Check**

#### JUCE Demo
```cpp
void process (ProcessContextReplacing<float> context)
{
    // CHECK FOR NEW IR EVERY BLOCK (non-blocking)
    bufferTransfer.get ([this] (BufferWithSampleRate& buf)
    {
        convolution.loadImpulseResponse (...);
    });

    convolution.process (context);  // Apply IR
}
```

#### Can_damonium
```cpp
void ConvolutionEngine::processBlock (juce::AudioBuffer<float>& buffer)
{
    // NO CHECK FOR NEW IR HERE
    // Only checks bypass/loaded status
    if (bypass.load() || !irLoaded.load())
        return;

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    convolver.process (context);
}
```

---

## Root Cause of Your Issue

**Most likely**: The IR is being queued/loaded, but `processBlock()` never checks if a new one is available. So the audio thread keeps using the old/no IR.

**Possible scenario**:
1. Plugin starts → no IR found → `irLoaded = false`
2. You select IR from dropdown → `processor.loadImpulseResponse()` called on UI thread
3. IR loads successfully → `irLoaded = true`
4. But `processBlock()` doesn't re-check ...OR the load failed silently

---

## The Fix

Can_damonium should:

1. **Use `BufferTransfer` pattern** (like JUCE demo):
   - UI thread queues IR buffer (fast, non-blocking)
   - Audio thread checks with try-lock in `processBlock()`
   - Only load if new buffer available

2. **Move file reading to background thread**:
   - UI thread doesn't block
   - Background thread reads WAV, queues buffer
   - Audio thread loads from queue

3. **Add per-block IR check**:
   - Every `processBlock()` call, check if new IR is queued
   - Use spinlock (non-blocking try-lock)
   - Load if available

---

## Detailed Comparison Table

| Aspect | JUCE Demo | Can_damonium | Issue |
|--------|-----------|-------------|-------|
| **IR Loading Thread** | Audio thread (via queue) | UI thread (direct) | ❌ Blocks UI |
| **Thread Safety** | SpinLock try-lock | None | ❌ Race risk |
| **Per-Block Check** | YES - every process() | NO - only on init | ❌ New IR ignored |
| **File I/O** | Background (separate) | Direct in loadImpulseResponse() | ❌ Blocks |
| **Bypass Support** | `context.isBypassed` | Manual check | ⚠️ OK but different |

---

## Next Steps

I can implement the JUCE Demo pattern in Can_damonium:

1. Add `BufferTransfer` class to `ConvolutionEngine`
2. Update `processBlock()` to check for new IR every frame (with try-lock)
3. Update `loadImpulseResponse()` to queue IR buffer instead of loading directly
4. Optionally: move file reading to background thread

**Want me to implement this?** It's a medium-sized refactor (~150-200 lines), but it'll fix the issue properly.
