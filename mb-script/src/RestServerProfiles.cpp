#include "config.h"

#ifdef ENABLE_PROFILE_TEMPERATURE // Guard the whole file

#include <components/RestServer.h>
#include "PHApp.h" // Needed for appInstance and tempProfiles
#include "profiles/TemperatureProfile.h" // Needed for TemperatureProfile type
#include <ArduinoJson.h>
#include "Logger.h"

/**
 * @brief Handles GET requests to /api/v1/profiles
 * Returns a list of available temperature profile slots.
 */
void RESTServer::getProfilesHandler(AsyncWebServerRequest *request)
{
    Log.verboseln("REST: getProfilesHandler called");
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;

    // Use modern syntax: doc[key].to<JsonArray>()
    JsonArray profilesArray = doc["profiles"].to<JsonArray>();

#ifdef ENABLE_PROFILE_TEMPERATURE
        Log.verboseln("  Found TempProfileManager");
        for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
            TemperatureProfile *profile = appInstance->tempProfiles[i];
            if (profile) {
                Log.verboseln("    Processing Profile Slot %d: %s", i, profile->name.c_str());
                JsonObject profileObj = profilesArray.add<JsonObject>();
                profileObj["slot"] = i;
                profileObj["duration"] = profile->getDuration(); // Assuming getDuration returns ms
                profileObj["status"] = (int)profile->getCurrentStatus();                 
                profileObj["currentTemp"] = profile->getTemperature(-1); // Get current interpolated temp
                // Use modern syntax: profileObj[key].to<JsonArray>()
                JsonArray pointsArray = profileObj["controlPoints"].to<JsonArray>();
                const TempControlPoint* points = profile->getTempControlPoints();
                uint8_t numPoints = profile->getNumTempControlPoints();
                 Log.verboseln("      Adding %d control points to JSON", numPoints);
                for (uint8_t j = 0; j < numPoints; ++j) {
                    JsonObject pointObj = pointsArray.add<JsonObject>();
                    pointObj["time"] = points[j].x; // Assuming x is time (scaled 0-1000)
                    pointObj["temperature"] = points[j].y; // Assuming y is temp (scaled)
                }

                // Use modern syntax: profileObj[key].to<JsonArray>()
                JsonArray targetRegistersArray = profileObj["targetRegisters"].to<JsonArray>();
                const std::vector<uint16_t>& targets = profile->getTargetRegisters();
                Log.verboseln("      Adding %d target registers to JSON", targets.size());
                for(uint16_t targetReg : targets) {
                    targetRegistersArray.add(targetReg);
                }

            } else {
                Log.warningln("  Profile slot %d is null", i);
            }
        }
   
#else
    doc["error"] = "Temperature profiles feature not enabled.";
#endif

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
void RESTServer::setProfileHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot)
{
     
    if (slot < 0 || slot >= PROFILE_TEMPERATURE_COUNT) {
        Log.warningln("REST: setProfileHandler - Invalid slot number %d provided.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid profile slot number\"}");
        return;
    }

    // Check if the profile object exists for this slot
    TemperatureProfile* targetProfile = appInstance->tempProfiles[slot];
    if (!targetProfile) {
        Log.warningln("REST: setProfileHandler - No profile found for slot %d.", slot);
        request->send(404, "application/json", "{\"success\":false,\"error\":\"Profile slot not found or not initialized\"}");
        return;
    }

    // Check if the JSON is an object
     if (!json.is<JsonObject>()) {
        Log.warningln("REST: setProfileHandler - Invalid JSON payload (not an object) for slot %d.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: must be an object.\"}");
        return;
    }
    JsonObject jsonObj = json.as<JsonObject>();

    // Attempt to load the configuration into the profile object
    bool success = targetProfile->load(jsonObj);

    if (success) {
        Log.infoln("REST: Profile slot %d updated successfully.", slot);
        request->send(200, "application/json", "{\"success\":true}");
    } else {
        Log.errorln("REST: Failed to update profile slot %d from JSON.", slot);
        // Provide a more specific error if `load` can indicate the reason
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Failed to load profile data. Check format and values.\"}");
    }
}

#endif // ENABLE_PROFILE_TEMPERATURE 