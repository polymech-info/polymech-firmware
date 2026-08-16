# PID Controller Logging via MQTT

This document outlines the setup for logging PID controller data to an InfluxDB instance via MQTT and how to visualize this data in Grafana.

## Firmware MQTT Setup

The firmware is configured to send PID controller data (Setpoint, Process Value, and heating state) to an MQTT broker. This data is then picked up by a Telegraf instance, which forwards it to InfluxDB.

### Enabling the Feature

To enable MQTT logging, ensure the following line is present and not commented out in `cassandra-rc2/src/config.h`:

```c++
#define ENABLE_MQTT_LOGGING
```

### MQTT Configuration

The MQTT broker details are configured in `cassandra-rc2/src/PHApp-Mqtt.cpp`. You may need to change the `MQTT_BROKER` IP address to match your environment.

```c++
#define MQTT_BROKER "192.168.1.100" // Replace with your MQTT broker's IP address
#define MQTT_PORT 1883
```

### Data Format

The firmware sends data using the InfluxDB Line Protocol. The format is as follows:

-   **Measurement:** `pid_controller`
-   **Tags:**
    -   `controller_id`: (integer) The ID of the PID controller (e.g., 1, 2, ... 8).
-   **Fields:**
    -   `sp`: (integer) The Setpoint value.
    -   `pv`: (integer) The Process Value.
    -   `heating`: (boolean) The heating state (`true` or `false`).

**Example Payload:**

```
pid_controller,controller_id=1 sp=150,pv=148,heating=true
```

This message is published to the topic `pid/controller/<id>`.

---

## Grafana: Adding InfluxDB Data Source

Follow these steps to connect your Grafana instance to the InfluxDB container.

1.  **Navigate to Grafana:** Open your web browser and go to `http://localhost:3000`.

2.  **Login:** Use the default credentials `admin` for both username and password, unless you have changed them. You will be prompted to change the password on first login.

3.  **Go to Data Sources:**
    *   In the left-hand menu, hover over the **Configuration** (cog) icon.
    *   Click on **Data Sources**.

4.  **Add Data Source:**
    *   Click the **"Add new data source"** button.
    *   Select **"InfluxDB"** from the list of time series databases.

5.  **Configure the Connection:**

    *   **Name:** Give your data source a name, for example, `InfluxDB_PID`.
    *   **Query Language:** Select **Flux**.
    *   **URL:** Set the URL to `http://influxdb:8086`. Grafana and InfluxDB are in the same Docker network, so they can communicate using their service names.
    *   **Authentication:** Leave the "Basic auth" and "With credentials" toggles off.
    *   **Organization:** Enter `my-org`.
    *   **Token:** Enter `my-super-token`.
    *   **Default Bucket:** Enter `my-bucket`.
    *   **Min time interval:** `10s` (optional, but good practice to match the Telegraf flush interval).

    The credentials and settings are derived from your `docker-compose.yaml` file:

    -   `DOCKER_INFLUXDB_INIT_USERNAME`: **admin**
    -   `DOCKER_INFLUXDB_INIT_PASSWORD`: **admin123**
    -   `DOCKER_INFLUXDB_INIT_ORG`: **my-org**
    -   `DOCKER_INFLUXDB_INIT_BUCKET`: **my-bucket**
    -   `DOCKER_INFLUXDB_INIT_ADMIN_TOKEN`: **my-super-token**

6.  **Save & Test:**
    *   Click the **"Save & test"** button at the bottom of the page.
    *   You should see a green notification saying "Data source is working".

You are now ready to create dashboards and query your PID controller data from InfluxDB in Grafana. 