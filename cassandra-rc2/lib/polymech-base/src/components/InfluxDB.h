#ifndef INFLUXDB_H
#define INFLUXDB_H

#include "config.h"
#include <Component.h>
#include <ArduinoLog.h>

#ifdef ENABLE_INFLUXDB
#include <InfluxDbClient.h>
class InfluxDB : public Component
{
private:
    InfluxDBClient _client;
    bool _isConnected = false;
    unsigned long _lastConnectionAttempt = 0;
    unsigned long _reconnectInterval = 1000; // Initial interval 1s
    static const unsigned long _maxReconnectInterval = 25000; // Max 25s

    void _connect();

public:
    InfluxDB(Component *owner, short _id);

    short setup() override;
    short info(short flags = 0, short val = 0) override;
    short debug() override;
    short loop() override;
    short write(Point &influxDbPoint);
};
#endif
#endif // ENABLE_INFLUXDB
