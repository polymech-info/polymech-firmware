#include "InfluxDB.h"
#include "config-extensions.h"

#ifdef ENABLE_INFLUXDB

InfluxDB::InfluxDB(Component *owner, short _id)
    : Component("InfluxDB", _id, Component::COMPONENT_DEFAULT, owner)
{
    setFlag(OBJECT_RUN_FLAGS::E_OF_BROKER);
}

void InfluxDB::_connect()
{
    _lastConnectionAttempt = millis();
    Log.infoln("InfluxDB: connecting...");
    if (_client.validateConnection())
    {
        Log.infoln("InfluxDB: connected!");
        _isConnected = true;
        _reconnectInterval = 1000; 
        Log.infoln("InfluxDB: connected to %s", INFLUXDB_URL);
    }
    else
    {
        Log.warningln("InfluxDB: connection failed: %s", _client.getLastErrorMessage().c_str());
        _isConnected = false;
        // Exponential backoff
        _reconnectInterval *= 2;
        if (_reconnectInterval > _maxReconnectInterval)
        {
            _reconnectInterval = _maxReconnectInterval;
        }
        Log.warningln("InfluxDB: next reconnect attempt in %lu ms", _reconnectInterval);
    }
}

short InfluxDB::setup()
{
    Component::setup();
    Log.verboseln(F("InfluxDB::setup - ID %d"), id);
    _client.setConnectionParams(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
    _client.setInsecure(true);
    _connect();
    return E_OK;
}

short InfluxDB::loop()
{
    if (!_isConnected)
    {
        if (millis() - _lastConnectionAttempt > _reconnectInterval)
        {
            _connect();
        }
    }
    return E_OK;
}

short InfluxDB::info(short flags, short val)
{
    Log.verboseln("InfluxDB::info - ID: %d, Connected: %s", id, _isConnected ? "true" : "false");
    return E_OK;
}

short InfluxDB::debug()
{
    return info(0, 0);
}

short InfluxDB::write(Point &influxDbPoint)
{
    if (!_isConnected)
    {
        Log.warningln(F("InfluxDB: not connected, cannot write point."));
        return 1; // Not connected error
    }

    if (!_client.writePoint(influxDbPoint))
    {
        Log.warningln(F("InfluxDB: failed to write point: %s"), _client.getLastErrorMessage().c_str());
        if (_client.isBufferFull())
        {
            Log.warningln(F("InfluxDB: buffer is full. Flushing..."));
            _client.flushBuffer();
        }
        return 2; // Write error
    }
    return E_OK;
} 
#endif