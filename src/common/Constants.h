#pragma once

#include <string>
#include <vector>

/**
 * Shared constants and utilities for Can Damonium project
 */

namespace CanDamonium {

// IR Constants
constexpr const char* PROFILE_SMALL = "Small";
constexpr const char* PROFILE_REGULAR = "Regular";
constexpr const char* PROFILE_GRANDE = "Grande";

constexpr const char* PRESET_PROFILES[] = {PROFILE_SMALL, PROFILE_REGULAR, PROFILE_GRANDE};
constexpr int NUM_PRESET_PROFILES = 3;

// IR File format specifications
constexpr int IR_SAMPLE_RATE = 48000; // Will refine as needed
constexpr int IR_BIT_DEPTH = 24;

// Library paths
constexpr const char* DEFAULT_LIBRARY_FOLDER = "Can_damonium/IRs";

struct IRMetadata {
    std::string name;
    std::string profile;
    std::string filePath;
    int sampleRate = 0;
    int duration = 0; // in milliseconds
    bool isPreset = false;
};

} // namespace CanDamonium
