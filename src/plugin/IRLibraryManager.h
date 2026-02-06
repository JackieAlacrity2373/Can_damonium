#pragma once

#include <JuceHeader.h>

/**
 * Manages available IR files and metadata.
 * Scans common IR directories and provides a list of available IRs.
 */
class IRLibraryManager
{
public:
    struct IREntry
    {
        juce::String name;        // Display name (e.g., "Regular Pringles Can")
        juce::File file;          // Full file path
        double sampleRate = 48000.0;
        
        IREntry() = default;
        IREntry(const juce::String& n, const juce::File& f, double sr = 48000.0)
            : name(n), file(f), sampleRate(sr) {}
    };

    IRLibraryManager();
    
    // Scan for available IRs in known locations
    void scanForIRs();
    
    // Get list of available IRs
    const juce::Array<IREntry>& getAvailableIRs() const noexcept { return availableIRs; }
    
    // Find IR by name
    const IREntry* findIRByName (const juce::String& name) const;
    
    // Get IRs matching a specific can size
    juce::Array<IREntry> getIRsByCanSize (const juce::String& canSize) const;
    
    // Get default IR
    const IREntry* getDefaultIR() const;
    
    // Add custom IR to library
    void addCustomIR (const juce::File& irFile);

private:
    juce::Array<IREntry> availableIRs;
    
    void addKnownIRs();
    juce::String extractNameFromFile (const juce::File& file);
};
