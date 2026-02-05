#pragma once

#include <JuceHeader.h>
#include <vector>
#include "../common/Constants.h"

/**
 * Manages user IR library and preset profile access
 */
class IRLibrary
{
public:
    IRLibrary();
    ~IRLibrary();

    // Load preset profiles
    void loadPresetProfiles();

    // User library management
    void scanLibraryFolder();
    void saveIRToLibrary (const juce::File& sourceFile, const juce::String& name);

    // Get available IRs
    const std::vector<CanDamonium::IRMetadata>& getAvailableIRs() const;
    const CanDamonium::IRMetadata* findIRByName (const juce::String& name) const;

private:
    std::vector<CanDamonium::IRMetadata> library;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IRLibrary)
};
