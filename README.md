# Polymech FW Apps

Monorepo for the Polymech Cassandra control system — ESP32-S3 firmware, web interface, maintenance tooling, and supporting libraries.

> **WIP / Roadmap**
> - Adopt new widget system for the web interface
> - Migrate firmware build to ESP-IDF native (drop PlatformIO)

## Overview

The firmware is a general-purpose embedded control platform for industrial recycling and forming machinery. **Cassandra** is the reference application built on top of it — not the framework itself. Any application is composed by enabling features in `config.h` and dropping local overrides into `config-user.h`, with no changes required to the core codebase.

**Composable components** (toggled via `#define`):

- **VFDs** — Sako, Delta, HY (RS-485 Modbus RTU)
- **Temperature control** — Omron E5 series PID controllers (up to N devices, configurable slave IDs)
- **Pressure / force** — load cells, press cylinders, solenoids
- **Motion** — stepper axes, joystick, operator switches
- **Profiles** — temperature warm-up sequences, pressure profiles, signal plots
- **Connectivity** — Modbus TCP, RS-485, REST API, WebSocket, LittleFS web UI
- **Budgeting** — amperage budget manager for shared power rails

The web interface (`modbus-ui`) is equally composable: pages and widgets map to the Modbus/REST surface exposed by whatever components are enabled in firmware.

**Supported machine types:** injection moulders, sheet presses, shredders, extruders, vending machines, and any custom application that combines VFDs, PID zones, and relay/solenoid outputs.

> **Status:** Battle-tested over many months of continuous operation on real production hardware.

## Repository Layout

| Path | Description |
|---|---|
| [`cassandra-rc2/`](./cassandra-rc2/README.md) | Main firmware (PlatformIO / ESP32-S3). Exposes Serial, Modbus TCP, and a REST/Web UI. |
| [`web/packages/modbus-ui/`](./web/packages/modbus-ui/) | React operator interface for the device, served over WiFi. |
| [`web/packages/client/`](./web/packages/client/) | Shared TypeScript client library used by the web packages. |
| [`cli-ts/`](./cli-ts/) | Node.js / TypeScript maintenance CLI — configuration management, network setup, signal plotting, and device scripting. |
| [`mb-script/`](./mb-script/) | Scriptable Modbus sequencer firmware for automated register-level testing and commissioning. |
| [`playground/`](./playground/) | Isolated PlatformIO sandbox for prototyping firmware features before merging into the main build. |
| [`tools/`](./tools/) | Pre-built Windows binaries and upload scripts (`esptool`, `espflash`, Modbus Poll/Slave, flash download tool, `zadig`, etc.). |
| [`vendor/`](./vendor/) | Third-party datasheets and reference material (Delta, Omron, Waveshare, HY, Kincony, and others). |

## Requirements

### Firmware (`cassandra-rc2/`)

The firmware supports two build backends. **ESP-IDF native** is the target going forward; PlatformIO remains available for the existing Waveshare/ESP32-S3 targets.

| Requirement | Notes |
|---|---|
| **PlatformIO Core** (`pio`) | `pip install platformio`. Used for `waveshare` / `waveshare-release` / `esp32-p4-evboard` environments. |
| **ESP-IDF v5.3.1** | Install via the [Espressif installer](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/). Path assumed at `C:\Espressif\frameworks\esp-idf-v5.3.1`. Used by `idf:build` / `idf:flash` / `idf:monitor` scripts (via `scripts/idf-*.sh`). |
| **CMake ≥ 3.24** | Bundled with ESP-IDF; ensure it is on `PATH` for native IDF builds. |
| **Python ≥ 3.10** | Required by PlatformIO, `esptool`, Modbus test scripts, and the nanopb generator (`scripts/run_nanopb_generator.py`). Run `npm run idf:setup` to install the IDF Python venv. |
| **Node.js ≥ 20 + npm** | Drives all `npm run …` wrapper scripts. |
| **VSCode** | Recommended editor. Install either: |
| &nbsp;&nbsp;↳ **ms-vscode.cpptools** | Microsoft C/C++ extension — works out of the box with the PlatformIO-generated `compile_commands.json` (`npm run index`). |
| &nbsp;&nbsp;↳ **llvm-vs-code-extensions.vscode-clangd** | clangd — faster indexing; requires the same `compile_commands.json` and disabling ms-cpptools IntelliSense. |
| **Bash** | Required for `idf:*` and `web:*` shell scripts. On Windows use Git Bash or WSL. |

### Web interface (`web/packages/modbus-ui/`)

| Requirement | Notes |
|---|---|
| **Node.js ≥ 20 + npm** | `npm install` in `web/packages/modbus-ui/` and `web/packages/client/`. |

### Maintenance CLI (`cli-ts/`)

| Requirement | Notes |
|---|---|
| **Node.js ≥ 20 + npm** | `npm install` in `cli-ts/`. |

## References

- [OSR - Plastic Legacy Firmware (Shredders, Printhead)](https://git.polymech.info/osr-plastic/osr-firmware.git)
- [Cassandra 65cm — PolyMech Service](https://service.polymech.info/user/3bb4cfbf-318b-44d3-a9d3-35680e738421/pages/cassandra-65cm)
- [Elena - Injection Machine (Motorized) — PolyMech Service](https://service.polymech.info/user/3bb4cfbf-318b-44d3-a9d3-35680e738421/pages/elena-injection-machine-motorized)

