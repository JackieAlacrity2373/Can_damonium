#include "IRLibrary.h"

IRLibrary::IRLibrary()
{
}

IRLibrary::~IRLibrary()
{
}

void IRLibrary::loadPresetProfiles()
{
    // TODO: Load embedded preset IRs from resources
}

void IRLibrary::scanLibraryFolder()
{
    // TODO: Scan user library folder and populate IR list
}

void IRLibrary::saveIRToLibrary (const juce::File& sourceFile, const juce::String& name)
{
    // TODO: Copy IR file to library and add to list
}

const std::vector<CanDamonium::IRMetadata>& IRLibrary::getAvailableIRs() const
{
    return library;
}

const CanDamonium::IRMetadata* IRLibrary::findIRByName (const juce::String& name) const
{
    for (const auto& ir : library)
    {
        if (ir.name == name.toStdString())
            return &ir;
    }
    return nullptr;
}
