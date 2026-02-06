#include "IRLibraryManager.h"

IRLibraryManager::IRLibraryManager()
{
    DBG("=== IRLibraryManager CONSTRUCTOR START ===");
    scanForIRs();
    DBG("=== IRLibraryManager CONSTRUCTOR END - Found " + juce::String(availableIRs.size()) + " IRs ===");
}

void IRLibraryManager::scanForIRs()
{
    DBG("IRLibraryManager::scanForIRs() called");
    availableIRs.clear();
    addKnownIRs();
    DBG("IRLibraryManager::scanForIRs() complete - found " + juce::String(availableIRs.size()) + " IRs");
}

void IRLibraryManager::addKnownIRs()
{
    juce::Array<juce::File> candidates;
    
    auto exeFile = juce::File::getSpecialLocation (juce::File::currentApplicationFile);
    auto exeDir = exeFile.getParentDirectory();
    auto cwd = juce::File::getCurrentWorkingDirectory();
    
    DBG("IRLibraryManager scanning for IRs:");
    DBG("  Exe File: " + exeFile.getFullPathName());
    DBG("  Exe Dir: " + exeDir.getFullPathName());
    DBG("  CWD: " + cwd.getFullPathName());

    auto alreadyAdded = [this](const juce::File& file)
    {
        for (const auto& ir : availableIRs)
        {
            if (ir.file == file)
                return true;
        }
        return false;
    };

    auto addIfValid = [this, &alreadyAdded](const juce::File& file)
    {
        if (file.existsAsFile() && file.getFileExtension().toLowerCase() == ".wav")
        {
            if (!alreadyAdded(file))
            {
                auto name = extractNameFromFile (file);
                availableIRs.add (IREntry (name, file, 48000.0));
                DBG("    -> Added IR: " + name + " (" + file.getFullPathName() + ")");
            }
        }
    };

    auto addWavsFromDir = [this, &addIfValid](const juce::File& dir)
    {
        if (!dir.isDirectory())
            return;

        for (juce::DirectoryIterator it (dir, false, "*.wav", juce::File::findFiles); it.next();)
        {
            addIfValid (it.getFile());
        }
    };
    
    // Navigate up from executable to find project root
    // Keep going until we find pring_Reg or reach filesystem root
    auto projectRoot = exeDir;
    for (int i = 0; i < 20; ++i)  // Increased from 5 to 20 levels
    {
        auto potentialPringReg = projectRoot.getChildFile("pring_Reg");
        DBG("  Checking level " + juce::String(i) + ": " + projectRoot.getFullPathName());
        
        if (potentialPringReg.isDirectory())
        {
            DBG("    -> Found pring_Reg directory!");
            break;
        }
        
        auto parent = projectRoot.getParentDirectory();
        if (parent == projectRoot) break; // Reached filesystem root
        projectRoot = parent;
    }
    
    // Common IR locations (in priority order)
    candidates.add (projectRoot.getChildFile ("pring_Reg").getChildFile ("RegularPringlesCan48k.wav"));
    candidates.add (exeDir.getChildFile ("pring_Reg").getChildFile ("RegularPringlesCan48k.wav"));
    candidates.add (exeDir.getChildFile ("Resources").getChildFile ("RegularPringlesCan48k.wav"));
    candidates.add (cwd.getChildFile ("pring_Reg").getChildFile ("RegularPringlesCan48k.wav"));
    candidates.add (cwd.getChildFile ("src").getChildFile ("plugin").getChildFile ("Resources").getChildFile ("RegularPringlesCan48k.wav"));

    for (const auto& file : candidates)
    {
        DBG("  Checking: " + file.getFullPathName() + " (exists: " + juce::String(file.existsAsFile() ? "true" : "false") + ")");
        addIfValid (file);
    }

    // Scan folders for any WAV IRs
    addWavsFromDir (projectRoot.getChildFile ("pring_Reg"));
    addWavsFromDir (exeDir.getChildFile ("pring_Reg"));
    addWavsFromDir (cwd.getChildFile ("pring_Reg"));

    auto userIRDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                        .getChildFile ("Can_damonium")
                        .getChildFile ("IRs");
    addWavsFromDir (userIRDir);
    
    DBG("IRLibraryManager scan complete. Found " + juce::String(availableIRs.size()) + " IR(s)");
    if (availableIRs.isEmpty())
    {
        DBG("WARNING: No IRs found in scan");
    }
}

const IRLibraryManager::IREntry* IRLibraryManager::findIRByName (const juce::String& name) const
{
    for (const auto& ir : availableIRs)
    {
        if (ir.name == name)
            return &ir;
    }
    return nullptr;
}

const IRLibraryManager::IREntry* IRLibraryManager::getDefaultIR() const
{
    // Return first IR or nullptr
    return availableIRs.isEmpty() ? nullptr : &availableIRs[0];
}

juce::Array<IRLibraryManager::IREntry> IRLibraryManager::getIRsByCanSize (const juce::String& canSize) const
{
    juce::Array<IREntry> filtered;
    
    // Map can size selector names to IR filename patterns
    juce::String pattern;
    if (canSize.containsIgnoreCase("Small"))
        pattern = "Small";
    else if (canSize.containsIgnoreCase("Regular"))
        pattern = "Regular";
    else if (canSize.containsIgnoreCase("Large") || canSize.containsIgnoreCase("Grande"))
        pattern = "Grande";
    
    // Filter IRs that match the can size pattern in their filename
    for (const auto& ir : availableIRs)
    {
        if (ir.name.containsIgnoreCase(pattern))
        {
            filtered.add(ir);
        }
    }
    
    // If no IRs found for this size, return all IRs (fallback)
    if (filtered.isEmpty())
    {
        DBG("No IRs found for can size: " + canSize + ", returning all IRs");
        return availableIRs;
    }
    
    return filtered;
}

void IRLibraryManager::addCustomIR (const juce::File& irFile)
{
    if (irFile.existsAsFile() && irFile.getFileExtension().toLowerCase() == ".wav")
    {
        auto name = extractNameFromFile (irFile);
        availableIRs.add (IREntry (name, irFile, 48000.0));
        DBG("Added custom IR: " + name);
    }
}

juce::String IRLibraryManager::extractNameFromFile (const juce::File& file)
{
    // Remove .wav extension and use filename
    auto name = file.getFileNameWithoutExtension();
    
    // Make it prettier (e.g., "RegularPringlesCan48k" -> "Regular Pringles Can 48k")
    // For now, just use the filename as-is
    return name;
}
