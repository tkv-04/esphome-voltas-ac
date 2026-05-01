# Voltas AC IoT Remote Control

IR remote control system for **Voltas AC (Model: RG52E1/BGEF)**, built with ESPHome + ESP32 for Home Assistant integration.

## Features

- 🌡️ **Temperature**: 17–30°C
- ❄️ **Modes**: Cool, Heat, Dry, Fan, Auto
- 💨 **Fan Speed**: Auto, Low, Medium, High
- 🔄 **Swing**: On/Off (vertical)
- 💡 **LED Display**: On/Off
- 📡 **Dynamic Protocol**: Signals generated on-the-fly — changing one parameter no longer resets others

## Project Structure

```
voltas/
├── README.md
├── .gitignore
├── ACvoltas.txt                        # Captured raw IR signals (reference)
├── voltas.ino                          # Arduino Uno test sketch
└── esphome/
    ├── voltas_ac.yaml                  # ESPHome configuration
    └── custom_components/
        └── voltas_ac/
            ├── __init__.py             # Component registration
            ├── climate.py              # Climate platform definition
            └── voltas_ac.h             # Dynamic IR signal generator
```

## Hardware

### Components
| Part | Purpose |
|------|---------|
| ESP32 DevKit | Main controller |
| IR LED (940nm) | Transmits IR signals |
| PN2222A / 2N2222 NPN transistor | Drives IR LED for range |
| 1.5kΩ resistor | Base current limiter |

### Wiring
```
                        3.3V
                          │
                    IR LED (+)
                          │
                    IR LED (-)
                          │
                    Collector (C)
                          │
ESP32 GPIO4 ──[1.5kΩ]── Base (B)     ← PN2222A
                          │
                    Emitter (E)
                          │
                         GND
```

### PN2222A Pinout (flat side facing you)
```
 E  B  C
```

## ESPHome Setup

### 1. Copy files to ESPHome config directory
```
config/
├── voltas-ac.yaml
└── custom_components/
    └── voltas_ac/
        ├── __init__.py
        ├── climate.py
        └── voltas_ac.h
```

### 2. Configure WiFi
Create or update `secrets.yaml`:
```yaml
wifi_ssid: "YourWiFiName"
wifi_password: "YourWiFiPassword"
```

### 3. Flash
```bash
esphome run voltas-ac.yaml
```

### 4. Add to Home Assistant
The device auto-discovers via the ESPHome integration.

## Home Assistant Entities

| Entity | Type | Controls |
|--------|------|----------|
| **Voltas AC** | Climate | Temperature, Mode, Fan, Swing |
| **AC Display** | Switch | LED display on/off |
| **Status** | Binary Sensor | Online/offline |
| **WiFi Signal** | Sensor | Signal strength |
| **Uptime** | Sensor | Device uptime |
| **Restart** | Button | Reboot device |

## Protocol (Decoded)

The Voltas remote sends a **6-byte frame** with 38kHz carrier:

```
[B2] [4D] [fan_byte] [~fan_byte] [temp_mode] [~temp_mode]
```

| Field | Encoding |
|-------|----------|
| Bytes 0-1 | `0xB2 0x4D` — Device ID (constant) |
| Byte 2 | Fan speed: `BF`=Auto, `9F`=Low, `5F`=Med, `3F`=High |
| Byte 3 | Bitwise complement of Byte 2 |
| Byte 4 | `(temp_code << 4) \| mode_nibble` |
| Byte 5 | Bitwise complement of Byte 4 |

Temperature uses a Gray-code-like lookup (offset from 17°C).
Mode nibbles: `0`=Cool, `4`=Dry, `8`=Auto, `C`=Heat.

## Arduino Uno (Testing)

The `voltas.ino` sketch provides a serial command interface (115200 baud) for testing with an Arduino Uno + IR LED on pin 4:

```
POWER_ON / POWER_OFF       TEMP <17-30> / TEMP_UP / TEMP_DOWN
MODE <COOL|DRY|HEAT|FAN|AUTO>   FAN <AUTO|LOW|MED|HIGH>
SWING_ON / SWING_OFF       LED_ON / LED_OFF
STATUS / HELP
```

> **Note**: The Arduino sketch uses static signal arrays (pre-decoded protocol). For production use, prefer the ESPHome component which generates signals dynamically.

## License

Personal IoT project. Use freely.
