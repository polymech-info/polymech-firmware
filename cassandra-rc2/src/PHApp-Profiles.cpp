#include "PHApp.h"

#include <xmath.h>
#include <ArduinoLog.h>
#include <enums.h>
#include <json.h>

#include "features.h"
#include "config.h"

#if defined(ENABLE_AMPERAGE_BUDGET_MANAGER) && defined(ENABLE_PROFILE_TEMPERATURE)
bool PHApp_canUsePID(Component *owner, OmronE5 *device)
{
    PHApp *app = static_cast<PHApp *>(owner);
    if (!app || !device)
    {
        return false;
    }

    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        TemperatureProfile *profile = app->tempProfiles[i];
        if (profile && (profile->getCurrentStatus() == PlotStatus::RUNNING || profile->getCurrentStatus() == PlotStatus::INITIALIZING))
        {
            const auto &targetRegisters = profile->getTargetRegisters();
            for (uint16_t targetReg : targetRegisters)
            {
                if (app->findOmronByTcpAddress(targetReg) == device)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
#endif

short PHApp::load(short val0, short val1)
{
    if (!LittleFS.begin(true))
    {
        L_ERROR(F("PHApp::load() - Failed to mount LittleFS. Cannot load profiles."));
        return E_INVALID_PARAMETER; // Use invalid parameter as fallback
    }

#ifdef ENABLE_PROFILE_TEMPERATURE
    loadTempProfiles();
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    loadSignalPlots();
#endif

#ifdef ENABLE_PROFILE_PRESSURE
    loadPressureProfiles();
#endif

    // --- Link Temperature Profiles with their Signal Plots ---
#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_PROFILE_SIGNAL_PLOT)
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        if (tempProfiles[i])
        {
            short signalPlotSlotId = tempProfiles[i]->getSignalPlotSlotId();
            if (signalPlotSlotId >= 0 && signalPlotSlotId < PROFILE_SIGNAL_PLOT_COUNT)
            {
                if (signalPlots[signalPlotSlotId])
                {
                    // Set the TemperatureProfile as the parent of the SignalPlot
                    tempProfiles[i]->addPlot(signalPlots[signalPlotSlotId]);
                }
            }
        }
    }
#endif

    // --- Link Temperature Profiles with their Pressure Profiles ---
#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_PROFILE_PRESSURE)
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        if (tempProfiles[i])
        {
            short pressureProfileSlotId = tempProfiles[i]->getPressureProfileSlotId();
            if (pressureProfileSlotId >= 0 && pressureProfileSlotId < PROFILE_PRESSURE_COUNT)
            {
                if (pressureProfiles[pressureProfileSlotId])
                {
                    // Set the TemperatureProfile as the parent of the PressureProfile
                    tempProfiles[i]->addPlot(pressureProfiles[pressureProfileSlotId]);
                }
            }
        }
    }
#endif

    LittleFS.end();
    return E_OK;
}

#ifdef ENABLE_PROFILE_TEMPERATURE

short PHApp::loadTempProfiles()
{
    const char *filename = "/profile_defaults.json";
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
        L_ERROR(F("PHApp::loadTempProfiles() - Failed to open profile file: %s"), filename);
        return E_NOT_FOUND; // Use standard not found
    }

    // Replace DynamicJsonDocument with JsonDocument, letting it handle allocation.
    JsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    size_t size = file.size();
    file.close(); // Close the file ASAP

    if (JsonUtils::handleDeserializationError(error, "PHApp::loadTempProfiles", filename, [&]()
                                              {
        File errorFile = LittleFS.open(filename, "r");
        if (errorFile) {
            String contentSample;
            contentSample.reserve(512);
            while (errorFile.available() && contentSample.length() < 512) {
                contentSample += (char)errorFile.read();
            }
            errorFile.close();
            return contentSample;
        }
        return String("Could not re-open file to read sample."); }, size))
    {
        return E_INVALID_PARAMETER;
    }

    // Check if the root is a JSON array
    if (!doc.is<JsonArray>())
    {
        L_ERROR(F("PHApp::loadTempProfiles() - Profile JSON root is not an array."));
        return E_INVALID_PARAMETER; // Use invalid parameter
    }

    JsonArray profilesArray = doc.as<JsonArray>();

    uint8_t profileIndex = 0;
    for (JsonObject profileJson : profilesArray)
    {
        if (profileIndex >= PROFILE_TEMPERATURE_COUNT)
        {
            L_WARN(F("PHApp::loadTempProfiles() - Too many profiles in JSON (%d), only loading the first %d."), profilesArray.size(), PROFILE_TEMPERATURE_COUNT);
            break;
        }

        if (!tempProfiles[profileIndex])
        {
            L_ERROR(F("PHApp::loadTempProfiles() - TemperatureProfile slot %d is not initialized. Skipping JSON profile."), profileIndex);
            profileIndex++;
            continue;
        }

        if (tempProfiles[profileIndex]->load(profileJson))
        {
            //            const char *name = profileJson["name"] | "Unnamed"; // Get name for logging
        }
        else
        {
            L_ERROR(F("PHApp::loadTempProfiles() - Failed to load profile data into slot %d."), profileIndex);
        }
        profileIndex++; // Move to the next TemperatureProfile slot
    }

    if (profileIndex < profilesArray.size())
    {
        L_WARN(F("PHApp::loadTempProfiles() - Processed %d JSON profiles but only %d slots were available/initialized."), profilesArray.size(), profileIndex);
    }
    else if (profileIndex < PROFILE_TEMPERATURE_COUNT)
    {
        L_INFO(F("PHApp::loadTempProfiles() - Loaded %d profiles from JSON into %d available slots."), profileIndex, PROFILE_TEMPERATURE_COUNT);
    }
    return E_OK;
}

/**
 * @brief Handles GET requests to /api/v1/profiles
 * Returns a list of available temperature profile slots.
 */
void PHApp::getProfilesHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray profilesArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; i++)
    {
        TemperatureProfile *profile = tempProfiles[i];
        if (profile)
        {
            JsonObject profileObj = profilesArray.add<JsonObject>();
            profileObj["id"] = profile->id;
            profileObj["type"] = static_cast<int>(profile->getPlotType());
            profileObj["name"] = profile->name;
            profileObj["description"] = profile->getDescription();
            profileObj["duration"] = profile->getDuration();
            profileObj["max"] = profile->max;
            profileObj["slot"] = profile->slot;
            profileObj["enabled"] = profile->enabled();
            profileObj["signalPlot"] = profile->getSignalPlotSlotId();
            profileObj["pressureProfile"] = profile->getPressureProfileSlotId();

            JsonArray pointsArray = profileObj["controlPoints"].to<JsonArray>();
            const ControlPoint *points = profile->getControlPoints();
            uint8_t numPoints = profile->getNumControlPoints();
            for (uint8_t j = 0; j < numPoints; j++)
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

            // Serialize Overrides
            JsonObject overridesObj = profileObj["overrides"].to<JsonObject>();
            JsonArray spOverrides = overridesObj["sp"].to<JsonArray>();
            // We iterate over the target registers to check for offsets
            for (uint16_t targetReg : targets)
            {
                if (targetReg == 0)
                {
                    continue;
                }
                int16_t offset = profile->getTargetOffset(targetReg);
                if (offset != 0)
                {
                    JsonObject offsetEntry = spOverrides.add<JsonObject>();
                    offsetEntry["targetRegister"] = targetReg;
                    offsetEntry["offset"] = offset;
                }
            }

            JsonArray childrenArray = profileObj["children"].to<JsonArray>();
            const ushort *children = profile->getChildren();
            for (uint8_t j = 0; j < MAX_PLOTS; j++)
            {
                childrenArray.add(children[j]);
            }
        }
        else
        {
            L_WARN("  Profile slot %d is null", i);
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
    if (!json.is<JsonObject>())
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: must be an object.\"}");
        return;
    }
    JsonObject jsonObj = json.as<JsonObject>();
    if (updateProfile(jsonObj, slot))
    {
        if (saveProfilesToJson())
        {
            L_INFO("REST: Profile updated and all profiles saved successfully.");
            request->send(200, "application/json", "{\"success\":true, \"message\":\"Profile updated and saved.\"}");
        }
        else
        {
            L_ERROR("REST: Profile slot %d updated, but failed to save all profiles to JSON.", slot);
            request->send(500, "application/json", "{\"success\":true, \"message\":\"Profile updated but failed to save configuration.\"}");
        }
    }
    else
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Failed to update profile. Check format and values.\"}");
    }
}

bool PHApp::updateProfile(JsonObject &json, int slot)
{
    if (slot < 0 || slot >= PROFILE_TEMPERATURE_COUNT)
    {
        L_WARN("REST: updateProfile - Invalid slot number %d provided.", slot);
        return false;
    }
    TemperatureProfile *targetProfile = this->tempProfiles[slot];
    if (!targetProfile)
    {
        L_WARN("REST: updateProfile - No profile found for slot %d.", slot);
        return false;
    }
    if (!targetProfile->load(json))
    {
        L_ERROR("REST: Failed to update profile slot %d from JSON.", slot);
        return false;
    }

#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_PROFILE_SIGNAL_PLOT)
    targetProfile->clear(); // Always clear existing plot first
    short signalPlotSlotId = targetProfile->getSignalPlotSlotId();
    if (signalPlotSlotId >= 0 && signalPlotSlotId < PROFILE_SIGNAL_PLOT_COUNT)
    {
        if (signalPlots[signalPlotSlotId])
        {
            targetProfile->addPlot(signalPlots[signalPlotSlotId]);
        }
        else
        {
            L_WARN(F("REST: updateProfile - TempProfile in slot %d has signal plot ID %d, but that SignalPlot slot is empty."), slot, signalPlotSlotId);
        }
    }
    else
    {
        L_INFO(F("REST: updateProfile - Unlinked SignalPlot from TempProfile in slot %d due to invalid slot ID %d."), slot, signalPlotSlotId);
    }
#endif

#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_PROFILE_PRESSURE)
    short pressureProfileSlotId = targetProfile->getPressureProfileSlotId();
    if (pressureProfileSlotId >= 0 && pressureProfileSlotId < PROFILE_PRESSURE_COUNT)
    {
        if (pressureProfiles[pressureProfileSlotId])
        {
            targetProfile->addPlot(pressureProfiles[pressureProfileSlotId]);
        }
        else
        {
            L_WARN(F("REST: updateProfile - TempProfile in slot %d has pressure profile ID %d, but that PressureProfile slot is empty."), slot, pressureProfileSlotId);
        }
    }
    else
    {
        L_INFO(F("REST: updateProfile - Unlinked PressureProfile from TempProfile in slot %d due to invalid slot ID %d."), slot, pressureProfileSlotId);
    }
#endif
    return true;
}

bool PHApp::saveProfilesToJson()
{
    if (!LittleFS.begin(true))
    {
        L_ERROR(F("PHApp::saveProfilesToJson() - Failed to mount LittleFS. Cannot save profiles."));
        return false;
    }

    const char *filename = "/profile_defaults.json"; // Path in LittleFS
    JsonDocument doc;
    JsonArray profilesArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; i++)
    {
        TemperatureProfile *profile = tempProfiles[i];
        if (profile)
        {
            JsonObject profileObj = profilesArray.add<JsonObject>();
            profileObj["id"] = profile->id;
            profileObj["type"] = static_cast<int>(profile->getPlotType());
            profileObj["name"] = profile->name;
            profileObj["description"] = profile->getDescription();
            profileObj["duration"] = profile->getDuration();
            profileObj["max"] = profile->max;
            profileObj["signalPlot"] = profile->getSignalPlotSlotId();
            profileObj["pressureProfile"] = profile->getPressureProfileSlotId();
            profileObj["enabled"] = profile->enabled();

            JsonArray pointsArray = profileObj["controlPoints"].to<JsonArray>();
            const ControlPoint *points = profile->getControlPoints();
            uint8_t numPoints = profile->getNumControlPoints();
            for (uint8_t j = 0; j < numPoints; j++)
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

            // Serialize Overrides
            JsonObject overridesObj = profileObj["overrides"].to<JsonObject>();
            JsonArray spOverrides = overridesObj["sp"].to<JsonArray>();
            // We iterate over the target registers to check for offsets
            for (uint16_t targetReg : targets)
            {
                if (targetReg == 0)
                {
                    continue;
                }
                int16_t offset = profile->getTargetOffset(targetReg);
                if (offset != 0)
                {
                    JsonObject offsetEntry = spOverrides.add<JsonObject>();
                    offsetEntry["targetRegister"] = targetReg;
                    offsetEntry["offset"] = offset;
                }
            }

            JsonArray childrenArray = profileObj["children"].to<JsonArray>();
            const ushort *children = profile->getChildren();
            for (uint8_t j = 0; j < MAX_PLOTS; j++)
            {
                childrenArray.add(children[j]);
            }
        }
        else
        {
            L_WARN(F("PHApp::saveProfilesToJson() - Profile slot %d is null, skipping."), i);
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);

    if (LittleFS.exists(filename))
    {
        LittleFS.remove(filename);
    }

    File file = LittleFS.open(filename, "w");
    if (!file)
    {
        L_ERROR(F("PHApp::saveProfilesToJson() - Failed to open profile file for writing: %s"), filename);
        LittleFS.end();
        return false;
    }

    size_t bytesWritten = file.print(jsonString);
    file.close();
    LittleFS.end();

    if (bytesWritten > 0)
    {
        return true;
    }
    else
    {
        L_ERROR(F("PHApp::saveProfilesToJson() - Failed to serialize JSON or write to file: %s"), filename);
        return false;
    }
}

#endif // ENABLE_PROFILE_TEMPERATURE

void PHApp::setPIDs(short temperatureProfileSlot, uint8_t sp)
{
#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_OMRON_E5)
    L_INFO(F("PHApp::startPids() - Starting PIDs for temperature profile slot: %d"), temperatureProfileSlot);
    if (temperatureProfileSlot < 0 || temperatureProfileSlot >= PROFILE_TEMPERATURE_COUNT)
    {
        L_WARN(F("PHApp::startPids() - Invalid temperature profile slot: %d"), temperatureProfileSlot);
        return;
    }

    TemperatureProfile *profile = tempProfiles[temperatureProfileSlot];
    if (!profile)
    {
        L_WARN(F("PHApp::startPids() - Temperature profile in slot %d is null."), temperatureProfileSlot);
        return;
    }

    const std::vector<uint16_t> &targetRegisters = profile->getTargetRegisters();
    if (targetRegisters.empty())
    {
        L_INFO(F("PHApp::startPids() - Profile in slot %d has no target registers to start."), temperatureProfileSlot);
        return;
    }

    if (!rs485)
    {
        L_ERROR(F("PHApp::startPids() - RS485 manager is not initialized. Cannot start physical PIDs."));
        return;
    }

    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevices = rs485->deviceManager.getMaxDevices();
    int startedCount = 0;

    for (uint16_t targetReg : targetRegisters)
    {
        for (int i = 0; i < numDevices; ++i)
        {
            Component *comp = devices[i];
            if (comp && comp->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
            {
                OmronE5 *omron = static_cast<OmronE5 *>(comp);
                uint16_t spCmdAddr = omron->mb_tcp_base_address() + static_cast<uint16_t>(E_OmronTcpOffset::CMD_SP);

                if (spCmdAddr == targetReg)
                {
                    int16_t offset = profile->getTargetOffset(targetReg);
                    int16_t finalSp = (int16_t)sp + offset;
                    if (finalSp < 0)
                        finalSp = 0;

                    omron->setSP((uint16_t)finalSp);
                    startedCount++;
                    break;
                }
            }
        }
    }
#endif // ENABLE_PROFILE_TEMPERATURE && ENABLE_OMRON_E5
}

void PHApp::startPids(short temperatureProfileSlot)
{
#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_OMRON_E5)
    L_INFO(F("PHApp::startPids() - Starting PIDs for temperature profile slot: %d"), temperatureProfileSlot);
    if (temperatureProfileSlot < 0 || temperatureProfileSlot >= PROFILE_TEMPERATURE_COUNT)
    {
        L_WARN(F("PHApp::startPids() - Invalid temperature profile slot: %d"), temperatureProfileSlot);
        return;
    }

    TemperatureProfile *profile = tempProfiles[temperatureProfileSlot];
    if (!profile)
    {
        L_WARN(F("PHApp::startPids() - Temperature profile in slot %d is null."), temperatureProfileSlot);
        return;
    }

    const std::vector<uint16_t> &targetRegisters = profile->getTargetRegisters();
    if (targetRegisters.empty())
    {
        L_INFO(F("PHApp::startPids() - Profile in slot %d has no target registers to start."), temperatureProfileSlot);
        return;
    }

    int startedCount = 0;
    for (uint16_t targetReg : targetRegisters)
    {
        if (targetReg == 0)
        {
            continue;
        }

        OmronE5 *omron = findOmronByTcpAddress(targetReg);
        if (omron)
        {
            L_INFO(F("PHApp::startPids() - Starting Omron %d for target register %d"), omron->slaveId, targetReg);
            omron->run();
            startedCount++;
        }
        else
        {
            L_WARN(F("PHApp::startPids() - No Omron found for target register %d"), targetReg);
        }
    }
#endif // ENABLE_PROFILE_TEMPERATURE
}

void PHApp::stopPids(short temperatureProfileSlot)
{
#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_OMRON_E5)
    if (temperatureProfileSlot < 0 || temperatureProfileSlot >= PROFILE_TEMPERATURE_COUNT)
    {
        L_WARN(F("PHApp::stopPids() - Invalid temperature profile slot: %d"), temperatureProfileSlot);
        return;
    }

    TemperatureProfile *profile = tempProfiles[temperatureProfileSlot];
    if (!profile)
    {
        L_WARN(F("PHApp::stopPids() - Temperature profile in slot %d is null."), temperatureProfileSlot);
        return;
    }

    const std::vector<uint16_t> &targetRegisters = profile->getTargetRegisters();
    if (targetRegisters.empty())
    {
        L_INFO(F("PHApp::stopPids() - Profile in slot %d has no target registers. Stopping profile only."), temperatureProfileSlot);
        profile->stop();
        return;
    }

    if (!rs485)
    {
        L_ERROR(F("PHApp::stopPids() - RS485 manager is not initialized. Cannot stop physical PIDs."));
        profile->stop(); // Still stop the software profile
        return;
    }

    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevices = rs485->deviceManager.getMaxDevices();
    int stoppedCount = 0;

    for (uint16_t targetReg : targetRegisters)
    {
        for (int i = 0; i < numDevices; ++i)
        {
            Component *comp = devices[i];
            if (comp && comp->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
            {
                OmronE5 *omron = static_cast<OmronE5 *>(comp);
                uint16_t spCmdAddr = omron->mb_tcp_base_address() + static_cast<uint16_t>(E_OmronTcpOffset::CMD_SP);

                if (spCmdAddr == targetReg)
                {
                    omron->stop();
                    stoppedCount++;
                    break;
                }
            }
        }
    }
#endif // ENABLE_PROFILE_TEMPERATURE
}

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
bool PHApp::updateSignalPlot(JsonObject &json, int slot)
{
    if (slot < 0 || slot >= PROFILE_SIGNAL_PLOT_COUNT)
    {
        L_WARN("REST: updateSignalPlot - Invalid slot number %d provided.", slot);
        return false;
    }

    SignalPlot *targetProfile = this->signalPlots[slot];
    if (!targetProfile)
    {
        L_WARN("REST: updateSignalPlot - No profile found for slot %d.", slot);
        return false;
    }

    bool success = targetProfile->load(json);

    if (success)
    {
        L_INFO("REST: SignalPlot slot %d staged for update.", slot);
    }
    else
    {
        L_ERROR("REST: Failed to update SignalPlot slot %d from JSON.", slot);
    }
    return success;
}

short PHApp::loadSignalPlots()
{
    const char *signalPlotFilename = "/signal_plots.json";
    File signalPlotFile = LittleFS.open(signalPlotFilename, "r");
    if (!signalPlotFile)
    {
        L_ERROR(F("PHApp::loadSignalPlots() - Failed to open signal plot profile file: %s. This might be normal if it doesn't exist yet."), signalPlotFilename);
        return E_NOT_FOUND;
    }

    JsonDocument signalPlotDoc; // Use a new document for signal plots
    DeserializationError spError = deserializeJson(signalPlotDoc, signalPlotFile);
    signalPlotFile.close();

    if (JsonUtils::handleDeserializationError(spError, "PHApp::loadSignalPlots", signalPlotFilename))
    {
        return E_INVALID_PARAMETER;
    }
    if (!signalPlotDoc.is<JsonArray>())
    {
        L_ERROR(F("PHApp::loadSignalPlots() - Signal plot JSON root is not an array."));
        return E_INVALID_PARAMETER;
    }

    JsonArray spArray = signalPlotDoc.as<JsonArray>();
    uint8_t spIndex = 0;
    for (JsonObject spJson : spArray)
    {
        if (spIndex >= PROFILE_SIGNAL_PLOT_COUNT)
        {
            L_WARN(F("PHApp::loadSignalPlots() - Too many signal plot profiles in JSON (%d), only loading the first %d."), spArray.size(), PROFILE_SIGNAL_PLOT_COUNT);
            break;
        }
        if (signalPlots[spIndex])
        {
            if (signalPlots[spIndex]->load(spJson))
            {
                const char *spName = spJson["name"] | "Unnamed Signal Plot";
            }
            else
            {
                L_ERROR(F("PHApp::loadSignalPlots() - Failed to load signal plot profile data into slot %d."), spIndex);
            }
        }
        else
        {
            L_ERROR(F("PHApp::loadSignalPlots() - SignalPlot slot %d is not initialized. Skipping JSON profile."), spIndex);
        }
        spIndex++;
    }
    return E_OK;
}

bool PHApp::saveSignalPlotsToJson()
{

    if (!LittleFS.begin(true))
    {
        L_ERROR(F("PHApp::saveSignalPlotsToJson() - Failed to mount LittleFS. Cannot save profiles."));
        return false;
    }

    const char *filename = "/signal_plots.json"; // Corrected filename
    JsonDocument doc;
    JsonArray profilesArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; ++i)
    {
        SignalPlot *profile = this->signalPlots[i];
        if (profile)
        {
            JsonObject profileJson = profilesArray.add<JsonObject>();
            profileJson["type"] = static_cast<int>(profile->getPlotType());
            profileJson["slot"] = i;
            profileJson["name"] = profile->name;
            profileJson["duration"] = profile->getDuration();

            JsonArray pointsArray = profileJson["controlPoints"].to<JsonArray>();
            const S_SignalControlPoint *points = profile->getControlPoints();
            uint8_t numPoints = profile->getNumControlPoints();
            for (uint8_t j = 0; j < numPoints; ++j)
            {
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

            JsonArray childrenArray = profileJson["children"].to<JsonArray>();
            const ushort *children = profile->getChildren();
            for (uint8_t j = 0; j < MAX_PLOTS; j++)
            {
                childrenArray.add(children[j]);
            }
        }
        else
        {
            L_WARN(F("PHApp::saveSignalPlotsToJson() - SignalPlot slot %d is null, skipping."), i);
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);

    if (LittleFS.exists(filename))
    {
        LittleFS.remove(filename);
    }

    File file = LittleFS.open(filename, "w");
    if (!file)
    {
        L_ERROR(F("PHApp::saveSignalPlotsToJson() - Failed to open profile file for writing: %s"), filename);
        LittleFS.end();
        return false;
    }

    size_t bytesWritten = file.print(jsonString);
    file.close();
    LittleFS.end();

    if (bytesWritten > 0)
    {
        return true;
    }
    else
    {
        L_ERROR(F("PHApp::saveSignalPlotsToJson() - Failed to serialize JSON or write to file: %s"), filename);
        return false;
    }
}
#endif // ENABLE_PROFILE_SIGNAL_PLOT

#ifdef ENABLE_PROFILE_TEMPERATURE
void PHApp::loopProfiles()
{
    static uint32_t lastProfileLoop = 0;
    uint32_t now = millis();
    uint32_t loopDelta = now - lastProfileLoop;
    if (lastProfileLoop == 0)
    { // First run init
        loopDelta = 0;
    }
    lastProfileLoop = now;

    if (this->_delayed_omron_write_pending && (now - this->_delayed_omron_write_ts > OMRON_WRITE_DELAY_MS))
    {
        setAllOmronComWrite(true, false);
        this->_delayed_omron_write_pending = false;
    }
    if (!appSettings->get("ALWAYS_WARMUP", false))
    {
        return;
    }
    // Check for profiles that are warming up
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        TemperatureProfile *profile = tempProfiles[i];
        if (profile != nullptr && profile->enabled())
        {
            if (profile->getCurrentStatus() == PlotStatus::INITIALIZING)
            {
                bool allPidsReady = true;

                // Only check PIDs if warmup is enabled
                if (appSettings->get("ALWAYS_WARMUP", true))
                {
                    const auto &targetRegisters = profile->getTargetRegisters();
                    if (!targetRegisters.empty())
                    {
                        for (uint16_t targetReg : targetRegisters)
                        {
                            if (targetReg == 0)
                            {
                                continue;
                            }
                            OmronE5 *omron = findOmronByTcpAddress(targetReg);
                            if (omron && omron->enabled())
                            {
                                if (omron->hasError())
                                {
                                    //    continue;
                                }

                                uint16_t pv, sp;
                                if (omron->getPV(pv) && omron->getSP(sp))
                                {
                                    uint32_t min_temp = appSettings->get("MIN_TEMPERATURE", (uint32_t)0);
                                    uint32_t max_temp = appSettings->get("MAX_TEMPERATURE", (uint32_t)280);
                                    if (pv >= max_temp || pv <= min_temp)
                                    {
                                        // L_ERROR(F("PHApp::loopProfiles() - PV %u for Omron %d is outside the allowed range of %ld-%ld"), pv, omron->slaveId, min_temp, max_temp);
                                        allPidsReady = false;
                                        break;
                                    }

                                    // A PID is considered "ready" for the profile to start if:
                                    // 1. It has reached the setpoint within the allowed deadband (e.g., SP-5).
                                    // 2. It has overshot the setpoint, but not by more than the maximum allowed overshoot (e.g., SP+50).
                                    bool isReady = (pv >= (sp - HEATUP_DEADBAND)) && (pv < (sp + MAX_OVERSHOOT));
                                    if (!isReady)
                                    {
                                        allPidsReady = false;
                                        break;
                                    }
                                }
                                else
                                {
                                    allPidsReady = false;
                                    break;
                                }
                            }
                            else
                            {
                                allPidsReady = false;
                                break;
                            }
                        }
                    }
                }
                // ... (Warmup logic above is fine)
                if (allPidsReady)
                {
                    onTemperatureProfileReady(profile);
                }
            }
            else
            {
                PlotStatus status = profile->getCurrentStatus();
                if (status == PlotStatus::RUNNING || status == PlotStatus::WAITING)
                {
                    checkProfileLag(profile, now, loopDelta);
                }
            }
        }
    }
}

void PHApp::checkProfileLag(TemperatureProfile *profile, uint32_t now, uint32_t loopDelta)
{
    if (!profile || !getAppPidLag())
        return;

    const std::vector<uint16_t> &targetRegisters = profile->getTargetRegisters();
    for (uint16_t targetReg : targetRegisters)
    {
        if (targetReg == 0)
            continue;
        OmronE5 *omron = findOmronByTcpAddress(targetReg);
        if (omron)
        {
            uint16_t pv;
            if (omron->getPV(pv))
            {
                int16_t currentTarget = profile->getValue(profile->getElapsedMs());
                int16_t offset = profile->getTargetOffset(targetReg);
                // Apply offset to target to match what PID sees
                int32_t diff = ((int32_t)currentTarget + offset) - (int32_t)pv;
                uint32_t deadband = appSettings->get("SP_DEADBAND", (uint32_t)5);

                if (diff > (int32_t)deadband)
                {
                    // We are lagging.
                    profile->setWaiting(true); // Set status to WAITING
                    if (profile->getLagDuration() < TEMP_MAX_LAG)
                    {
                        // Use the loopDelta passed from loopProfiles which accurately tracks time since last check
                        if (loopDelta > 0)
                        {
                            profile->slipTime(loopDelta);
                            profile->setLagDuration(profile->getLagDuration() + loopDelta);

                            if (now - profile->getLastLogMs() > 5000)
                            {
                                L_INFO(F("Lag Compensation: Slipping. Target %d, PV %d, Diff %d > DB %d. Duration: %lu ms"), currentTarget + offset, pv, diff, deadband, profile->getLagDuration());
                                profile->setLastLogMs(now);
                            }
                        }
                    }
                    else
                    {
                        // Timeout reached
                        if (now - profile->getLastLogMs() > 5000)
                        {
                            L_INFO(F("Lag Compensation: Timeout reached (%lu ms). Forcing advance despite lag."), profile->getLagDuration());
                            profile->setLastLogMs(now);
                        }
                    }
                    // Break after first lagging PID found - one lagging PID stalls the whole profile.
                    return;
                }
            }
        }
    }
    // If we reach here, no PIDs are lagging.
    profile->setWaiting(false); // Clear waiting status
    profile->setLagDuration(0);
}

void PHApp::onTemperatureProfileStarted(PlotBase *profile)
{
    this->_delayed_omron_write_pending = true;
    this->_delayed_omron_write_ts = millis();
    TemperatureProfile *tempProfile = static_cast<TemperatureProfile *>(profile);
    if (tempProfile->getCurrentStatus() == PlotStatus::RUNNING)
    {
        if (appSettings->get("ALWAYS_WARMUP", true))
        {
            uint8_t temp = tempProfile->getValue(0);
            L_INFO(F("PHApp::onTemperatureProfileStarted() - Starting temperature profile %d with temperature %d."), profile->slot, temp);
            tempProfile->setStatus(PlotStatus::INITIALIZING);
            setPIDs(tempProfile->slot, temp);
        }

        if (appSettings->get("ALWAYS_USE_SEQUENTIAL_HEATING", false))
        {
            stopPids(tempProfile->slot);
#if ENABLED(ENABLE_AMPERAGE_BUDGET_MANAGER)
            pidManagerAmperage->reset();
#endif
        }
        else
        {
            startPids(tempProfile->slot);
        }
    }

#ifdef ENABLE_FEEDBACK_3C
    if (feedback3C_0)
    {
        feedback3C_0->setMode(Feedback3C::MODE_RUNNING);
    }
#endif

#ifdef ENABLE_FEEDBACK_BUZZER
    if (feedbackBuzzer_0)
    {
        feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_FAST_BLINK, 1000);
    }
#endif

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
    if (pidManagerAmperage->enabled())
    {
        pidManagerAmperage->reset();
    }
#endif
}
void PHApp::onTemperatureProfileStopped(PlotBase *profile)
{
    L_INFO(F("PHApp::onTemperatureProfileStopped() - Stopping temperature profile %d."), profile->slot);
    if (appSettings->get("ALWAYS_STOP_PIDS_ON_PROFILE_FINISHED", true))
    {
#if defined(ENABLE_OMRON_E5)
        stopPids(profile->slot);
#endif
    }

#ifdef ENABLE_FEEDBACK_3C
    if (feedback3C_0)
    {
        feedback3C_0->setMode(Feedback3C::MODE_STANDBY);
    }
#endif
}
void PHApp::onTemperatureProfileFinished(PlotBase *profile)
{
#ifdef ENABLE_FEEDBACK_3C
    if (feedback3C_0)
    {
        feedback3C_0->setMode(Feedback3C::MODE_FINISHED);
    }
#endif
#ifdef ENABLE_FEEDBACK_BUZZER
    if (feedbackBuzzer_0)
    {
        feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_LONG_BEEP_SHORT_PAUSE, 2000);
    }
#endif
}
void PHApp::onTemperatureProfilePaused(PlotBase *profile)
{
#ifdef ENABLE_FEEDBACK_3C
    if (feedback3C_0)
    {
        feedback3C_0->setMode(Feedback3C::MODE_WAITING);
    }
#endif
#ifdef ENABLE_FEEDBACK_BUZZER
    if (feedbackBuzzer_0)
    {
        feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_FAST_BLINK, 1000);
    }
#endif
}
void PHApp::onTemperatureProfileResumed(PlotBase *profile)
{
#ifdef ENABLE_FEEDBACK_3C
    if (feedback3C_0)
    {
        feedback3C_0->setMode(Feedback3C::MODE_RUNNING);
    }
#endif
}
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
void PHApp::onSignalPlotStarted(PlotBase *plot)
{
    // Implementation can be added here
}

void PHApp::onSignalPlotStopped(PlotBase *plot)
{
    // Implementation can be added here
}

void PHApp::onSignalPlotPaused(PlotBase *plot)
{
    // Implementation can be added here
}

void PHApp::onSignalPlotResumed(PlotBase *plot)
{
    // Implementation can be added here
}

void PHApp::onSignalPlotFinished(PlotBase *plot)
{
    // Implementation can be added here
}
#endif

#ifdef ENABLE_PROFILE_PRESSURE
short PHApp::loadPressureProfiles()
{
    const char *filename = "/pressure_profiles.json";
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
        L_ERROR(F("PHApp::loadPressureProfiles() - Failed to open profile file: %s"), filename);
        return E_NOT_FOUND;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        L_ERROR(F("PHApp::loadPressureProfiles() - Failed to parse JSON from %s. Error: %s"), filename, error.c_str());
        return E_INVALID_PARAMETER;
    }

    if (!doc.is<JsonArray>())
    {
        L_ERROR(F("PHApp::loadPressureProfiles() - Profile JSON root is not an array."));
        return E_INVALID_PARAMETER;
    }

    JsonArray profilesArray = doc.as<JsonArray>();

    uint8_t profileIndex = 0;
    for (JsonObject profileJson : profilesArray)
    {
        if (profileIndex >= PROFILE_PRESSURE_COUNT)
        {
            L_WARN(F("PHApp::loadPressureProfiles() - Too many profiles in JSON (%d), only loading the first %d."), profilesArray.size(), PROFILE_PRESSURE_COUNT);
            break;
        }

        if (!pressureProfiles[profileIndex])
        {
            L_ERROR(F("PHApp::loadPressureProfiles() - PressureProfile slot %d is not initialized. Skipping JSON profile."), profileIndex);
            profileIndex++;
            continue;
        }

        if (pressureProfiles[profileIndex]->load(profileJson))
        {
            const char *name = profileJson["name"] | "Unnamed"; // Get name for logging
        }
        else
        {
            L_ERROR(F("PHApp::loadPressureProfiles() - Failed to load profile data into slot %d."), profileIndex);
        }
        profileIndex++;
    }
    return E_OK;
}
void PHApp::getPressureProfilesHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray profilesArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_PRESSURE_COUNT; i++)
    {
        PressureProfile *profile = pressureProfiles[i];
        if (profile)
        {
            JsonObject profileObj = profilesArray.add<JsonObject>();
            profileObj["id"] = profile->id;
            profileObj["type"] = static_cast<int>(profile->getPlotType());
            profileObj["name"] = profile->name;
            profileObj["description"] = profile->getDescription();
            profileObj["duration"] = profile->getDuration();
            profileObj["max"] = profile->max;
            profileObj["slot"] = profile->slot;
            profileObj["enabled"] = profile->enabled();

            JsonArray pointsArray = profileObj["controlPoints"].to<JsonArray>();
            const ControlPoint *points = profile->getControlPoints();
            uint8_t numPoints = profile->getNumControlPoints();
            for (uint8_t j = 0; j < numPoints; j++)
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

            JsonArray childrenArray = profileObj["children"].to<JsonArray>();
            const ushort *children = profile->getChildren();
            for (uint8_t j = 0; j < MAX_PLOTS; j++)
            {
                childrenArray.add(children[j]);
            }
        }
    }

    serializeJson(doc, *response);
    request->send(response);
}
void PHApp::setPressureProfilesHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot)
{
    if (!json.is<JsonObject>())
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: must be an object.\"}");
        return;
    }
    JsonObject jsonObj = json.as<JsonObject>();
    if (updatePressureProfile(jsonObj, slot))
    {
        if (savePressureProfilesToJson())
        {
            request->send(200, "application/json", "{\"success\":true, \"message\":\"Profile updated and saved.\"}");
        }
        else
        {
            request->send(500, "application/json", "{\"success\":true, \"message\":\"Profile updated but failed to save configuration.\"}");
        }
    }
    else
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Failed to update profile. Check format and values.\"}");
    }
}
bool PHApp::updatePressureProfile(JsonObject &json, int slot)
{
    if (slot < 0 || slot >= PROFILE_PRESSURE_COUNT)
    {
        return false;
    }
    PressureProfile *targetProfile = this->pressureProfiles[slot];
    if (!targetProfile)
    {
        return false;
    }
    if (!targetProfile->load(json))
    {
        return false;
    }
    targetProfile->clear();
    return true;
}
bool PHApp::savePressureProfilesToJson()
{
    JsonDocument doc;
    JsonArray profilesArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_PRESSURE_COUNT; i++)
    {
        PressureProfile *profile = pressureProfiles[i];
        if (profile)
        {
            JsonObject profileObj = profilesArray.add<JsonObject>();
            profileObj["id"] = profile->id;
            profileObj["type"] = static_cast<int>(profile->getPlotType());
            profileObj["name"] = profile->name;
            profileObj["description"] = profile->getDescription();
            profileObj["duration"] = profile->getDuration();
            profileObj["max"] = profile->max;
            profileObj["enabled"] = profile->enabled();

            JsonArray pointsArray = profileObj["controlPoints"].to<JsonArray>();
            const ControlPoint *points = profile->getControlPoints();
            uint8_t numPoints = profile->getNumControlPoints();
            for (uint8_t j = 0; j < numPoints; j++)
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

            JsonArray childrenArray = profileObj["children"].to<JsonArray>();
            const ushort *children = profile->getChildren();
            for (uint8_t j = 0; j < MAX_PLOTS; j++)
            {
                childrenArray.add(children[j]);
            }
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);
    File file = LittleFS.open("/pressure_profiles.json", "w");
    if (!file)
    {
        return false;
    }

    file.print(jsonString);
    file.close();
    return true;
}
void PHApp::onPressureProfileStarted(PlotBase *profile)
{
}
void PHApp::onPressureProfileStopped(PlotBase *profile)
{
}
void PHApp::onPressureProfileFinished(PlotBase *profile)
{
}
void PHApp::onPressureProfilePaused(PlotBase *profile)
{
}
void PHApp::onPressureProfileResumed(PlotBase *profile)
{
}
#endif // ENABLE_PROFILE_PRESSURE

#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)

#include <components/RS485.h>
#include <components/OmronE5.h>

OmronE5 *PHApp::findOmronByTcpAddress(uint16_t tcpAddress)
{
    if (!rs485)
    {
        L_WARN(F("findOmronByTcpAddress called but rs485 is null"));
        return nullptr;
    }

    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevices = rs485->deviceManager.getMaxDevices();

    for (int i = 0; i < numDevices; ++i)
    {
        if (devices[i] != nullptr && devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
        {
            OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
            if (omron)
            {
                uint16_t baseAddr = omron->mb_tcp_base_address();
                if (baseAddr > 0 && tcpAddress >= baseAddr && tcpAddress < (baseAddr + OmronE5::OMRON_TCP_BLOCK_COUNT))
                {
                    return omron;
                }
            }
        }
    }

    return nullptr;
}

void PHApp::onTemperatureProfileReady(PlotBase *profile)
{
#if defined(ENABLE_FEEDBACK_BUZZER)
    if (feedbackBuzzer_0)
    {
        feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_FAST_BLINK, 2000);
    }
#endif
    profile->onReady();
}

#endif
