#include "PHApp.h"
#include "config.h"
#include <ArduinoLog.h>           // For logging
#include <enums.h>                // For error codes like E_INVALID_PARAMETER
#ifdef ENABLE_PROFILE_TEMPERATURE
#include <LittleFS.h>             // For file system access
#include <ArduinoJson.h>          // For JSON parsing
#include "profiles/TemperatureProfile.h" // For TemperatureProfile
#endif // ENABLE_PROFILE_TEMPERATURE

short PHApp::load(short val0, short val1)
{
    Log.infoln(F("PHApp::load() - Loading application data..."));

#ifdef ENABLE_PROFILE_TEMPERATURE
    Log.infoln(F("PHApp::load() - Attempting to load temperature profiles..."));

    if (!LittleFS.begin(true)) { // Ensure LittleFS is mounted (true formats if necessary)
        Log.errorln(F("PHApp::load() - Failed to mount LittleFS. Cannot load profiles."));
        return E_INVALID_PARAMETER; // Use invalid parameter as fallback
    }

    const char* filename = "/profiles/defaults.json"; // Path in LittleFS
    File file = LittleFS.open(filename, "r");
    if (!file) {
        Log.errorln(F("PHApp::load() - Failed to open profile file: %s"), filename);
        LittleFS.end(); // Close LittleFS
        return E_NOT_FOUND; // Use standard not found
    }

    // Increased size slightly for safety, adjust if needed
    // DynamicJsonDocument doc(JSON_ARRAY_SIZE(PROFILE_TEMPERATURE_COUNT) + PROFILE_TEMPERATURE_COUNT * JSON_OBJECT_SIZE(5 + MAX_TEMP_CONTROL_POINTS));
    // Replace DynamicJsonDocument with JsonDocument, letting it handle allocation.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    file.close(); // Close the file ASAP
    LittleFS.end(); // Close LittleFS

    if (error) {
        Log.errorln(F("PHApp::load() - Failed to parse profile JSON: %s"), error.c_str());
        return E_INVALID_PARAMETER; // Use invalid parameter
    }

    // Check if the root is a JSON array
    if (!doc.is<JsonArray>()) {
        Log.errorln(F("PHApp::load() - Profile JSON root is not an array."));
        return E_INVALID_PARAMETER; // Use invalid parameter
    }

    JsonArray profilesArray = doc.as<JsonArray>();
    Log.infoln(F("PHApp::load() - Found %d profiles in JSON file."), profilesArray.size());

    uint8_t profileIndex = 0;
    for (JsonObject profileJson : profilesArray) {
        if (profileIndex >= PROFILE_TEMPERATURE_COUNT) {
            Log.warningln(F("PHApp::load() - Too many profiles in JSON (%d), only loading the first %d."), profilesArray.size(), PROFILE_TEMPERATURE_COUNT);
            break;
        }

        if (!tempProfiles[profileIndex]) {
             Log.errorln(F("PHApp::load() - TemperatureProfile slot %d is not initialized. Skipping JSON profile."), profileIndex);
             // Don't increment profileIndex here, try to load next JSON into same slot if possible?
             // Or increment profileIndex to align JSON index with slot index? Let's align.
             profileIndex++;
             continue;
        }

        // Assuming TemperatureProfile (or its base PlotBase) has a public method
        // like loadFromJson that takes the JsonObject and calls the protected virtual load.
        // We also assume it returns bool or short (E_OK for success).
        Log.infoln(F("PHApp::load() - Loading JSON data into TemperatureProfile slot %d..."), profileIndex);
        // Now call the protected load() directly, as PHApp is a friend
        if (tempProfiles[profileIndex]->load(profileJson)) { // returns bool
            const char* name = profileJson["name"] | "Unnamed"; // Get name for logging
            Log.infoln(F("PHApp::load() - Successfully loaded profile '%s' into slot %d."), name, profileIndex);
        } else {
            Log.errorln(F("PHApp::load() - Failed to load profile data into slot %d."), profileIndex);
            // Decide if we should return an error or just continue loading others
            // return E_INVALID_PARAMETER; // Option: Stop loading on first failure
        }

        profileIndex++; // Move to the next TemperatureProfile slot
    }

    // Handle case where JSON has fewer profiles than allocated slots
    if (profileIndex < profilesArray.size()) {
         Log.warningln(F("PHApp::load() - Processed %d JSON profiles but only %d slots were available/initialized."), profilesArray.size(), profileIndex);
    } else if (profileIndex < PROFILE_TEMPERATURE_COUNT) {
         Log.infoln(F("PHApp::load() - Loaded %d profiles from JSON into %d available slots."), profileIndex, PROFILE_TEMPERATURE_COUNT);
    }

    Log.infoln(F("PHApp::load() - Finished loading temperature profiles."));
#endif // ENABLE_PROFILE_TEMPERATURE

    Log.infoln(F("PHApp::load() - Application data loading complete."));
    return E_OK;
}
