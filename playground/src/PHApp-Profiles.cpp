#include "PHApp.h"
#include "config.h"
#include <ArduinoLog.h>
#include <enums.h>
#include <profiles/TemperatureProfile.h>

#ifdef ENABLE_PROFILE_TEMPERATURE
#include <LittleFS.h>
#include <ArduinoJson.h>
#endif

short PHApp::load(short val0, short val1)
{
    Log.infoln(F("PHApp::load() - Loading application data..."));

    Log.infoln(F("PHApp::load() - Attempting to load temperature profiles..."));

    if (!LittleFS.begin(true))
    { // Ensure LittleFS is mounted (true formats if necessary)
        Log.errorln(F("PHApp::load() - Failed to mount LittleFS. Cannot load profiles."));
        return E_INVALID_PARAMETER; // Use invalid parameter as fallback
    }

    const char *filename = "/profiles/defaults.json"; // Path in LittleFS
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
        Log.errorln(F("PHApp::load() - Failed to open profile file: %s"), filename);
        LittleFS.end();     // Close LittleFS
        return E_NOT_FOUND; // Use standard not found
    }

    // Increased size slightly for safety, adjust if needed
    // DynamicJsonDocument doc(JSON_ARRAY_SIZE(PROFILE_TEMPERATURE_COUNT) + PROFILE_TEMPERATURE_COUNT * JSON_OBJECT_SIZE(5 + MAX_TEMP_CONTROL_POINTS));
    // Replace DynamicJsonDocument with JsonDocument, letting it handle allocation.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    file.close();   // Close the file ASAP
    LittleFS.end(); // Close LittleFS

    if (error)
    {
        Log.errorln(F("PHApp::load() - Failed to parse profile JSON: %s"), error.c_str());
        return E_INVALID_PARAMETER; // Use invalid parameter
    }

    // Check if the root is a JSON array
    if (!doc.is<JsonArray>())
    {
        Log.errorln(F("PHApp::load() - Profile JSON root is not an array."));
        return E_INVALID_PARAMETER; // Use invalid parameter
    }

#ifdef ENABLE_PROFILE_TEMPERATURE

    JsonArray profilesArray = doc.as<JsonArray>();
    Log.infoln(F("PHApp::load() - Found %d profiles in JSON file."), profilesArray.size());

    uint8_t profileIndex = 0;
    for (JsonObject profileJson : profilesArray)
    {
        if (profileIndex >= PROFILE_TEMPERATURE_COUNT)
        {
            Log.warningln(F("PHApp::load() - Too many profiles in JSON (%d), only loading the first %d."), profilesArray.size(), PROFILE_TEMPERATURE_COUNT);
            break;
        }

        if (!tempProfiles[profileIndex])
        {
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
        if (tempProfiles[profileIndex]->load(profileJson))
        {                                                       // returns bool
            const char *name = profileJson["name"] | "Unnamed"; // Get name for logging
            Log.infoln(F("PHApp::load() - Successfully loaded profile '%s' into slot %d."), name, profileIndex);
        }
        else
        {
            Log.errorln(F("PHApp::load() - Failed to load profile data into slot %d."), profileIndex);
            // Decide if we should return an error or just continue loading others
            // return E_INVALID_PARAMETER; // Option: Stop loading on first failure
        }
        Log.infoln(F("PHApp::load() - Loaded %d profiles from JSON into %d available slots and %d target registers."), profileIndex, PROFILE_TEMPERATURE_COUNT, tempProfiles[profileIndex]->getTargetRegisters().size());
        profileIndex++; // Move to the next TemperatureProfile slot
    }

    // Handle case where JSON has fewer profiles than allocated slots
    if (profileIndex < profilesArray.size())
    {
        Log.warningln(F("PHApp::load() - Processed %d JSON profiles but only %d slots were available/initialized."), profilesArray.size(), profileIndex);
    }
    else if (profileIndex < PROFILE_TEMPERATURE_COUNT)
    {
        Log.infoln(F("PHApp::load() - Loaded %d profiles from JSON into %d available slots."), profileIndex, PROFILE_TEMPERATURE_COUNT);
    }
#endif // ENABLE_PROFILE_TEMPERATURE
    // --- Load Signal Plot Profiles ---
#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    if (!LittleFS.begin(true)) { // Ensure LittleFS is mounted, or re-mount if it was closed
        // Decide on return strategy: return E_INVALID_PARAMETER or continue?
        // For now, let's log and continue, as temp profiles might have loaded.
    } else {
        const char *signalPlotFilename = "/profiles/signal_plots.json";
        File signalPlotFile = LittleFS.open(signalPlotFilename, "r");
        if (!signalPlotFile) {
            Log.errorln(F("PHApp::load() - Failed to open signal plot profile file: %s. This might be normal if it doesn't exist yet."), signalPlotFilename);
        } else {
            JsonDocument signalPlotDoc; // Use a new document for signal plots
            DeserializationError spError = deserializeJson(signalPlotDoc, signalPlotFile);
            signalPlotFile.close();

            if (spError) {
                Log.errorln(F("PHApp::load() - Failed to parse signal plot JSON: %s"), spError.c_str());
            } else if (!signalPlotDoc.is<JsonArray>()) {
                Log.errorln(F("PHApp::load() - Signal plot JSON root is not an array."));
            } else {
                JsonArray spArray = signalPlotDoc.as<JsonArray>();
                uint8_t spIndex = 0;
                for (JsonObject spJson : spArray) {
                    if (spIndex >= PROFILE_SIGNAL_PLOT_COUNT) {
                        Log.warningln(F("PHApp::load() - Too many signal plot profiles in JSON (%d), only loading the first %d."), spArray.size(), PROFILE_SIGNAL_PLOT_COUNT);
                        break;
                    }
                    if (signalPlots[spIndex]) {
                        if (signalPlots[spIndex]->load(spJson)) {
                            const char *spName = spJson["name"] | "Unnamed Signal Plot";
                        } else {
                            Log.errorln(F("PHApp::load() - Failed to load signal plot profile data into slot %d."), spIndex);
                        }
                    } else {
                        Log.errorln(F("PHApp::load() - SignalPlot slot %d is not initialized. Skipping JSON profile."), spIndex);
                    }
                    spIndex++;
                }
            }
        }
        LittleFS.end(); // Close LittleFS after signal plots are done
    }

    // --- Link Temperature Profiles with their Signal Plots ---
    Log.infoln(F("PHApp::load() - Linking Temperature Profiles to Signal Plots..."));
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
        if (tempProfiles[i]) {
            short signalPlotSlotId = tempProfiles[i]->getSignalPlotSlotId();
            if (signalPlotSlotId >= 0 && signalPlotSlotId < PROFILE_SIGNAL_PLOT_COUNT) {
                if (signalPlots[signalPlotSlotId]) {
                    // Set the TemperatureProfile as the parent of the SignalPlot
                    tempProfiles[i]->addPlot(signalPlots[signalPlotSlotId]);
                    Log.infoln(F("PHApp::load() - Linked TempProfile in slot %d to SignalPlot in slot %d."), i, signalPlotSlotId);
                } else {
                    Log.warningln(F("PHApp::load() - TempProfile in slot %d has signal plot ID %d, but that SignalPlot slot is empty."), i, signalPlotSlotId);
                }
            }
        }
    }
#endif // ENABLE_PROFILE_SIGNAL_PLOT
    return E_OK;
}

#ifdef ENABLE_PROFILE_TEMPERATURE

/**
 * @brief Handles GET requests to /api/v1/profiles
 * Returns a list of available temperature profile slots.
 */
void PHApp::getProfilesHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray profilesArray = doc["profiles"].to<JsonArray>();

    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        TemperatureProfile *profile = this->tempProfiles[i];
        if (profile)
        {
            Log.verboseln("    Processing Profile Slot %d: %s", i, profile->name.c_str());
            JsonObject profileObj = profilesArray.add<JsonObject>();
            profileObj["slot"] = i;
            profileObj["duration"] = profile->getDuration();    
            profileObj["status"] = (int)profile->getCurrentStatus();
            profileObj["currentTemp"] = profile->getTemperature(profile->getElapsedMs());
            profileObj["name"] = profile->name;
            profileObj["max"] = profile->max;
            profileObj["enabled"] = profile->enabled();
            profileObj["elapsed"] = profile->getElapsedMs();
            profileObj["remaining"] = profile->getRemainingTime();
            profileObj["signalPlot"] = profile->getSignalPlotSlotId();
            profileObj["description"] = profile->getDescription();
            JsonArray pointsArray = profileObj["controlPoints"].to<JsonArray>();
            const TempControlPoint *points = profile->getTempControlPoints();
            uint8_t numPoints = profile->getNumTempControlPoints();
            for (uint8_t j = 0; j < numPoints; ++j)
            {
                JsonObject pointObj = pointsArray.add<JsonObject>();
                pointObj["x"] = points[j].x;
                pointObj["y"] = points[j].y; 
            }
            JsonArray targetRegistersArray = profileObj["targetRegisters"].to<JsonArray>();
            const std::vector<uint16_t> &targets = profile->getTargetRegisters();
            for (uint16_t targetReg : targets)
            {
                targetRegistersArray.add(targetReg);
            }
        }
        else
        {
            Log.warningln("  Profile slot %d is null", i);
        }
    }

    serializeJson(doc, *response);
    request->send(response);
}

/**
 * @brief Handles POST requests to /api/v1/profiles/{slot}
 * Updates the specified temperature profile using the provided JSON data.
 *
 * @param request The incoming web request.
 * @param json The parsed JSON body from the request.
 * @param slot The profile slot number extracted from the URL.
 */
void PHApp::setProfilesHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot)
{

    if (slot < 0 || slot >= PROFILE_TEMPERATURE_COUNT)
    {
        Log.warningln("REST: setProfileHandler - Invalid slot number %d provided.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid profile slot number\"}");
        return;
    }

    // Check if the profile object exists for this slot
    TemperatureProfile *targetProfile = this->tempProfiles[slot];
    if (!targetProfile)
    {
        Log.warningln("REST: setProfileHandler - No profile found for slot %d.", slot);
        request->send(404, "application/json", "{\"success\":false,\"error\":\"Profile slot not found or not initialized\"}");
        return;
    }

    // Check if the JSON is an object
    if (!json.is<JsonObject>())
    {
        Log.warningln("REST: setProfileHandler - Invalid JSON payload (not an object) for slot %d.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: must be an object.\"}");
        return;
    }
    JsonObject jsonObj = json.as<JsonObject>();

    // Attempt to load the configuration into the profile object
    bool success = targetProfile->load(jsonObj);

    if (success)
    {
        Log.infoln("REST: Profile slot %d updated successfully.", slot);
        // Attempt to save all profiles back to JSON
        if (saveProfilesToJson()) {
            Log.infoln("REST: All profiles saved to JSON successfully after update.");
            request->send(200, "application/json", "{\"success\":true, \"message\":\"Profile updated and saved.\"}");
        } else {
            Log.errorln("REST: Profile slot %d updated, but failed to save all profiles to JSON.", slot);
            request->send(500, "application/json", "{\"success\":true, \"message\":\"Profile updated but failed to save configuration.\"}"); // Send 200 as profile was updated, but indicate save error
        }
    }
    else
    {
        Log.errorln("REST: Failed to update profile slot %d from JSON.", slot);
        // Provide a more specific error if `load` can indicate the reason
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Failed to load profile data. Check format and values.\"}");
    }
}

bool PHApp::saveProfilesToJson() {
    Log.infoln(F("PHApp::saveProfilesToJson() - Saving all temperature profiles to JSON..."));

    if (!LittleFS.begin(true)) {
        Log.errorln(F("PHApp::saveProfilesToJson() - Failed to mount LittleFS. Cannot save profiles."));
        return false;
    }

    const char *filename = "/profiles/defaults.json"; // Path in LittleFS
    JsonDocument doc; // Use a single JsonDocument for the array
    JsonArray profilesArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
        TemperatureProfile *profile = this->tempProfiles[i];
        if (profile) {
            JsonObject profileJson = profilesArray.add<JsonObject>();
            profileJson["type"] = "temperature"; // Assuming this is fixed for now
            profileJson["slot"] = i;
            profileJson["name"] = profile->name;
            profileJson["duration"] = profile->getDuration(); // Duration in ms
            profileJson["max"] = profile->max;
            profileJson["description"] = profile->getDescription();
            profileJson["signalPlot"] = profile->getSignalPlotSlotId();
            // controlPoints
            JsonArray pointsArray = profileJson["controlPoints"].to<JsonArray>();
            const TempControlPoint *points = profile->getTempControlPoints();
            uint8_t numPoints = profile->getNumTempControlPoints();
            for (uint8_t j = 0; j < numPoints; ++j) {
                JsonObject pointObj = pointsArray.add<JsonObject>();
                pointObj["x"] = points[j].x;
                pointObj["y"] = points[j].y;
            }
            // targetRegisters
            JsonArray targetRegistersArray = profileJson["targetRegisters"].to<JsonArray>();
            const std::vector<uint16_t> &targets = profile->getTargetRegisters();
            for (uint16_t targetReg : targets) {
                targetRegistersArray.add(targetReg);
            }
        } else {
            Log.warningln(F("PHApp::saveProfilesToJson() - Profile slot %d is null, skipping."), i);
        }
    }

    File file = LittleFS.open(filename, "w"); // Open for writing, creates if not exists, truncates if exists
    if (!file) {
        Log.errorln(F("PHApp::saveProfilesToJson() - Failed to open profile file for writing: %s"), filename);
        LittleFS.end();
        return false;
    }

    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    LittleFS.end();

    if (bytesWritten > 0) {
        Log.infoln(F("PHApp::saveProfilesToJson() - Successfully wrote %d bytes to %s"), bytesWritten, filename);
        return true;
    } else {
        Log.errorln(F("PHApp::saveProfilesToJson() - Failed to serialize JSON or write to file: %s"), filename);
        return false;
    }
}


#endif // ENABLE_PROFILE_TEMPERATURE

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
bool PHApp::saveSignalPlotsToJson() { 
    
    Log.infoln(F("PHApp::saveSignalPlotsToJson() - Saving all SignalPlot profiles to JSON..."));

    if (!LittleFS.begin(true)) {
        Log.errorln(F("PHApp::saveSignalPlotsToJson() - Failed to mount LittleFS. Cannot save profiles."));
        return false;
    }

    const char *filename = "/profiles/signal_plots.json"; // Corrected filename
    JsonDocument doc; 
    JsonArray profilesArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; ++i) {
        SignalPlot *profile = this->signalPlots[i];
        if (profile) {
            JsonObject profileJson = profilesArray.add<JsonObject>();
            profileJson["type"] = "signal";
            profileJson["slot"] = i;
            profileJson["name"] = profile->name;
            profileJson["duration"] = profile->getDuration(); 
            
            JsonArray pointsArray = profileJson["controlPoints"].to<JsonArray>();
            const S_SignalControlPoint *points = profile->getControlPoints();
            uint8_t numPoints = profile->getNumControlPoints();
            for (uint8_t j = 0; j < numPoints; ++j) {
                JsonObject pointObj = pointsArray.add<JsonObject>();
                pointObj["id"] = points[j].id;
                pointObj["time"] = points[j].time;
                pointObj["name"] = points[j].name;
                pointObj["description"] = points[j].description;
                pointObj["state"] = (int16_t)points[j].state;
                pointObj["type"] = (int16_t)points[j].type;
                pointObj["arg_0"] = points[j].arg_0;
                pointObj["arg_1"] = points[j].arg_1;
                pointObj["arg_2"] = points[j].arg_2;
            }
        } else {
            Log.warningln(F("PHApp::saveSignalPlotsToJson() - SignalPlot slot %d is null, skipping."), i);
        }
    }

    File file = LittleFS.open(filename, "w"); 
    if (!file) {
        Log.errorln(F("PHApp::saveSignalPlotsToJson() - Failed to open profile file for writing: %s"), filename);
        LittleFS.end();
        return false;
    }

    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    LittleFS.end();

    if (bytesWritten > 0) {
        Log.infoln(F("PHApp::saveSignalPlotsToJson() - Successfully wrote %d bytes to %s"), bytesWritten, filename);
        return true;
    } else {
        Log.errorln(F("PHApp::saveSignalPlotsToJson() - Failed to serialize JSON or write to file: %s"), filename);
        return false;
    }
}
#endif // ENABLE_PROFILE_SIGNAL_PLOT
