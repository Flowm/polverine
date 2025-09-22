# Polverine Environmental Monitoring System

## Hardware Overview

The **[Polverine board](https://www.blackiot.swiss/polverine)** is a mikroBUS™ sensor board developed by [BlackIoT](https://www.blackiot.swiss/) featuring the **[Bosch BMV080](https://www.bosch-sensortec.com/products/environmental-sensors/particulate-matter-sensor/bmv080/)** particulate matter sensor and the **[Bosch BME690](https://www.bosch-sensortec.com/products/environmental-sensors/gas-sensors/bme690/)** gas sensor connected to a ESP32-S3 microcontroller.

## Firmware Overview

This repository contains a firmware that transforms the Polverine board into a complete IoT environmental monitoring solution with web-based configuration, MQTT connectivity, and Home Assistant integration.

### Key Features

#### 🌐 WiFi Provisioning

- **Zero-code setup:** No need to hardcode WiFi credentials
- **Web-based configuration:** Intuitive browser interface for setup
- **Automatic fallback:** Creates WiFi hotspot if connection fails
- **Persistent storage:** Credentials stored in NVS flash memory

#### 📊 Real-time Sensor Monitoring

- **Dual sensor support:** BME690 + BMV080 integration
- **High-frequency sampling:** Continuous environmental monitoring
- **Data validation:** Built-in sensor error detection and reporting
- **Sensor dashboard:** Live web interface showing real-time values

#### 🏠 Home Assistant Integration

- **Auto-discovery:** Automatically publishes MQTT discovery messages to Home Assistant's `homeassistant/` topic prefix
- **Device registration:** Creates a unified device "Polverine XXXXXX" with manufacturer, model, and unique identifier
- **Rich entities:** 15+ sensor entities including environmental, air quality, and system health metrics
- **Entity classification:** Proper device classes (temperature, humidity, pm25, etc.) with units and icons
- **Status monitoring:** Device availability topic, sensor health states, and system diagnostics

#### ⚙️ Advanced Configuration

- **Web interface:** Complete configuration through browser
- **MQTT settings:** Broker, authentication, topic customization
- **Network management:** WiFi scanning, connection monitoring
- **Factory reset:** Hardware button for configuration reset

### Supported Measurements

| Sensor | Measurement     | Unit    | Home Assistant Entity                    |
| ------ | --------------- | ------- | ---------------------------------------- |
| BME690 | Temperature     | °C      | `sensor.polverine_xxxxx_temperature`     |
| BME690 | Humidity        | %       | `sensor.polverine_xxxxx_humidity`        |
| BME690 | Pressure        | hPa     | `sensor.polverine_xxxxx_pressure`        |
| BME690 | IAQ Index       | 0-500   | `sensor.polverine_xxxxx_iaq`             |
| BME690 | CO2 Equivalent  | ppm     | `sensor.polverine_xxxxx_co2`             |
| BME690 | VOC Equivalent  | ppm     | `sensor.polverine_xxxxx_voc`             |
| BME690 | Gas Resistance  | %       | `sensor.polverine_xxxxx_gas_percentage`  |
| BMV080 | PM10            | µg/m³   | `sensor.polverine_xxxxx_pm10`            |
| BMV080 | PM2.5           | µg/m³   | `sensor.polverine_xxxxx_pm25`            |
| BMV080 | PM1             | µg/m³   | `sensor.polverine_xxxxx_pm1`             |
| System | WiFi RSSI       | dBm     | `sensor.polverine_xxxxx_wifi_rssi`       |
| System | Free Heap       | bytes   | `sensor.polverine_xxxxx_free_heap`       |
| System | Uptime          | seconds | `sensor.polverine_xxxxx_uptime`          |
| System | CPU Temperature | °C      | `sensor.polverine_xxxxx_cpu_temperature` |

---

## Quick Start Guide

### First Time Setup (WiFi Provisioning)

1. **Power on** the Polverine device
2. **Connect** to WiFi network: `Polverine-XXXXXX` (password: `12345678`)
3. **Open browser** and navigate to: `http://192.168.4.1`
4. **Configure** your WiFi and MQTT settings
5. **Save configuration** - device will restart and connect automatically

### Configuration Reset

- **Hold BOOT button** for 5 seconds to clear all settings
- Device will restart in provisioning mode

### Web Dashboard Access

Once connected to your network:

- **Configuration:** `http://[device-ip]/config`
- **Live Dashboard:** `http://[device-ip]/dashboard`
- **Device IP** shown in router's DHCP table or Home Assistant discovery

## Development Setup

### 🔧 Prerequisites

#### System Requirements

- **Python 3.7+** (for PlatformIO)
- **Git** (for version control)

#### 📥 Initial Setup

```bash
# Clone the repository
git clone <repository-url>
cd polverine

# Install PlatformIO CLI
pip install platformio

# Verify installation
pio --version
```

### 📦 SDK Dependencies

> **⚠️ Important:** Due to Bosch Sensortec licensing restrictions, SDKs must be downloaded separately after agreeing to their terms.

1. **BMV080 Particulate Matter Sensor SDK v11.2.0**
    - 🔗 **Download:**
        [BMV080 Software](https://www.bosch-sensortec.com/software-tools/software/previous-sdk-bmv-080-versions/)
        [SDK Download](https://www.bosch-sensortec.com/software-tools/double-opt-in-forms/sdk-v11-2.html)
    - 📁 **Extract to:** `deps/bosch-sensortec/BMV080-SDK/`
    - ✅ **Expected structure:**
        ```
        deps/bosch-sensortec/BMV080-SDK/
        ├── api/
        ├── api_examples/
        ├── CHANGELOG.md
        ├── LICENSE.md
        └── README.md
        ```

2. **BME690 BSEC Gas Sensor Library v3.2.0**
    - 🔗 **Download:**
        [BME690 Software](https://www.bosch-sensortec.com/software-tools/software/bme688-and-bme690-software/)
        [BSEC Download](https://www.bosch-sensortec.com/software-tools/double-opt-in-forms/bsec-software-3-2-1-0-form.html)
    - 📁 **Extract to:** `deps/bosch-sensortec/bsec_library_v3.x/`
    - ✅ **Expected structure:**
        ```
        deps/bosch-sensortec/bsec_library_v3.x/
        ├── algo/
        ├── examples/
        └── README.md
        ```

### 🔨 Build & Flash

#### Quick Start (Recommended)

The fastest way to get started is using the provided Makefile:

```bash
# All-in-one: build, upload, and monitor
make

# Or step by step:
make build    # Build firmware only
make upload   # Flash to device
make monitor  # View serial output
```

#### Available Make Targets

| Command        | Description              | Use Case               |
| -------------- | ------------------------ | ---------------------- |
| `make`         | Build + upload + monitor | Deploy and test        |
| `make build`   | Compile firmware only    | Testing compilation    |
| `make upload`  | Flash to device          | Deploy new build       |
| `make monitor` | Start serial monitor     | Debug & testing        |
| `make deploy`  | Build + upload only      | Production deployment  |
| `make clean`   | Remove build files       | Fix build issues       |
| `make rebuild` | Clean + build            | Force complete rebuild |
| `make help`    | Show all targets         | Reference              |

#### Manual PlatformIO Commands

If you prefer direct PlatformIO commands:

```bash
# Build for Polverine board
pio run -e polverine

# Upload to connected device
pio run -e polverine -t upload

# Monitor serial output (Ctrl+C to exit)
pio device monitor --baud 115200

# Clean build artifacts
pio run -t clean
```

### ⚙️ Configuration Options

#### Runtime Configuration (Recommended)

The device creates a WiFi hotspot for first-time setup - no pre-configuration needed!

1. Connect to `Polverine-XXXXXX` network (password: `12345678`)
2. Navigate to `http://192.168.4.1`
3. Configure WiFi and MQTT settings via web interface

#### Build-time Configuration (Optional)

For development convenience, create `credentials.ini` in the project root:

```ini
[credentials]
wifi_ssid = "YourWiFiNetwork"
wifi_pass = "YourWiFiPassword"
mqtt_uri = "mqtt://your-broker.local:1883"
mqtt_user = "your-mqtt-username"
mqtt_pass = "your-mqtt-password"
```

> **Note:** Build-time credentials are used as defaults but can be overridden via the web interface.

## Support

- **Documentation:** [BlackIoT Polverine FAQ](https://www.blackiot.swiss/polverine)
- **Hardware Purchase:** [Mouser Electronics](https://www.mouser.com/ProductDetail/BlackIoT/BLK-POL-0001?qs=efUn273yAhc9eiJd1kLRxQ%3D%3D)

## Acknowledgments

- **BlackIoT Swiss** for the Polverine hardware platform and the [POLVERINE_HOMEASSISTANT_DEMO](https://github.com/BlackIoT/Polverine/tree/main/POLVERINE_HOMEASSISTANT_DEMO) example that this firmware is originally based on
- **Bosch Sensortec** for BME690 and BMV080 sensors
- **Espressif** for ESP32-S3 microcontroller platform
- **Home Assistant** community for integration standards
