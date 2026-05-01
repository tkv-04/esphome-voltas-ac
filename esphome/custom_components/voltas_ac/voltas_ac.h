#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/remote_transmitter/remote_transmitter.h"

namespace esphome {
namespace voltas_ac {

static const char *const TAG = "voltas_ac";

// =============================================================
//  Voltas AC - Dynamic IR Signal Generator
//  Remote Model: RG52E1/BGEF  |  Carrier: 38 kHz
//
//  PROTOCOL (decoded from raw captures):
//    6 bytes: [B2] [4D] [fan_byte] [~fan_byte] [temp_mode] [~temp_mode]
//    Signal:  HEADER + 48 bits + GAP + HEADER + 48 bits (repeat)
//    Header:  mark=4400, space=4300
//    Bit 1:   mark=550, space=1600
//    Bit 0:   mark=550, space=500
//    Gap:     space=5200
// =============================================================

// Temperature lookup: index = temp - 17, value = upper nibble of byte4
static const uint8_t TEMP_CODES[] = {
  0x0, 0x1, 0x3, 0x2, 0x6, 0x7, 0x5, 0x4,  // 17-24
  0xC, 0xD, 0x9, 0x8, 0xA, 0xB              // 25-30
};

// Fan byte2 values
static const uint8_t FAN_AUTO_BYTE = 0xBF;
static const uint8_t FAN_LOW_BYTE  = 0x9F;
static const uint8_t FAN_MED_BYTE  = 0x5F;
static const uint8_t FAN_HIGH_BYTE = 0x3F;
static const uint8_t FAN_FIXED_BYTE = 0x1F;  // Dry/Auto modes (no fan control)

// Mode nibble values (lower nibble of byte4)
static const uint8_t MODE_COOL_NIB = 0x0;
static const uint8_t MODE_DRY_NIB  = 0x4;
static const uint8_t MODE_AUTO_NIB = 0x8;
static const uint8_t MODE_HEAT_NIB = 0xC;
static const uint8_t MODE_FAN_NIB  = 0x4;  // Same as Dry, differentiated by byte2

// Swing command (toggle - same signal for on/off)
static const uint16_t SWING_CMD[] = {
  4350, 4300, 600, 1600, 550, 500, 600, 1600, 550, 1600, 600, 500, 550, 500,
  550, 1650, 550, 500, 600, 500, 600, 1550, 600, 500, 550, 500, 600, 1600,
  550, 1600, 600, 450, 600, 1600, 550, 500, 600, 500, 600, 500, 600, 450,
  600, 1600, 550, 1600, 600, 1600, 550, 1600, 550, 1600, 550, 1600, 600, 1550,
  600, 1600, 550, 500, 600, 500, 550, 500, 600, 500, 600, 1600, 550, 1600,
  550, 1600, 550, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 500,
  600, 450, 600, 500, 600, 1600, 550, 1600, 550, 1600, 550, 1600, 600, 1550, 600
};

// LED display signals (different protocol structure, kept as raw)
static const uint16_t LED_ON_CMD[] = {
  4400, 4300, 600, 1550, 650, 450, 550, 1600, 600, 1600, 550, 500, 550, 1650,
  500, 550, 550, 1650, 500, 550, 550, 1600, 550, 550, 550, 550, 550, 1600,
  550, 550, 550, 1600, 550, 550, 550, 1600, 550, 1600, 550, 1600, 550, 1600,
  550, 550, 550, 1600, 550, 550, 550, 100, 200, 1300, 550, 550, 550, 550,
  550, 500, 550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 1600,
  550, 550, 550, 1600, 550, 550, 550, 550, 500, 1650, 550, 500, 550, 1650,
  550, 550, 500, 1650, 550, 500, 550, 1650, 500, 1650, 550, 550, 500, 1650,
  550, 550, 500, 5200, 4350, 4350, 550, 1600, 550, 550, 500, 1650, 550, 1600,
  550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550,
  500, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 1600, 600, 1550,
  600, 1550, 600, 1600, 600, 500, 600, 1550, 650, 450, 600, 1550, 600, 500,
  600, 450, 650, 450, 600, 500, 600, 1550, 600, 500, 600, 1550, 600, 500,
  600, 1550, 600, 500, 550, 1600, 600, 450, 600, 500, 600, 1550, 600, 500,
  550, 1600, 600, 500, 550, 1600, 600, 500, 550, 1600, 600, 1550, 600, 500,
  550, 1600, 600
};

static const uint16_t LED_OFF_CMD[] = {
  4350, 4350, 550, 1600, 550, 550, 550, 1600, 550, 1650, 500, 550, 550, 1650,
  500, 500, 600, 1650, 500, 550, 550, 1600, 550, 550, 550, 500, 600, 1600,
  550, 500, 550, 1650, 550, 550, 500, 1650, 550, 1600, 550, 1600, 550, 1600,
  550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 550, 550, 500,
  550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550,
  550, 1600, 550, 550, 550, 500, 550, 1650, 500, 550, 550, 1650, 500, 550,
  550, 1600, 550, 550, 550, 1600, 550, 1600, 550, 550, 550, 1600, 550, 550,
  550, 5150, 4400, 4300, 550, 1600, 550, 550, 550, 1600, 550, 1600, 600, 500,
  550, 1600, 550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 500,
  600, 1600, 550, 500, 550, 1650, 550, 500, 550, 1650, 550, 1600, 550, 1600,
  550, 1600, 550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 550,
  550, 500, 550, 550, 550, 1600, 550, 550, 550, 1600, 550, 550, 550, 1600,
  550, 550, 550, 1600, 550, 550, 550, 550, 500, 1650, 500, 550, 550, 1650,
  500, 550, 550, 1650, 500, 550, 550, 1650, 500, 1650, 500, 550, 550, 1650,
  500, 550, 600
};

// Power OFF (special: byte2=0x7F, byte4=0x4E for cool@24 off)
static const uint16_t POWER_OFF_CMD[] = {
  4400, 4300, 550, 1650, 550, 500, 550, 1650, 550, 1600, 550, 500, 550, 550,
  550, 1600, 550, 550, 550, 500, 600, 1600, 550, 500, 600, 450, 600, 1650,
  500, 1650, 550, 450, 600, 1650, 550, 500, 550, 1650, 550, 1600, 550, 1600,
  550, 1600, 550, 500, 600, 1600, 550, 1600, 550, 1650, 550, 500, 550, 550,
  550, 500, 600, 500, 550, 1650, 500, 550, 550, 550, 550, 1600, 550, 1600,
  550, 1650, 500, 550, 550, 550, 550, 500, 600, 500, 550, 550, 550, 500,
  600, 500, 550, 550, 550, 1600, 550, 1600, 550, 1650, 550, 1550, 600, 1600,
  550, 5150, 4400, 4300, 600, 1600, 550, 500, 550, 1600, 600, 1600, 550, 500,
  600, 500, 550, 1600, 550, 550, 550, 550, 550, 1600, 550, 550, 550, 500,
  600, 1550, 600, 1600, 550, 500, 600, 1600, 550, 500, 600, 1550, 600, 1600,
  550, 1600, 550, 1600, 550, 550, 550, 1600, 550, 1600, 550, 1650, 550, 500,
  550, 550, 550, 550, 550, 500, 550, 1650, 550, 500, 550, 550, 550, 1600,
  550, 1650, 500, 1650, 550, 550, 500, 550, 550, 550, 550, 550, 500, 550,
  550, 550, 550, 550, 500, 550, 550, 1650, 500, 1650, 500, 1650, 550, 1650,
  500, 1650, 500
};


class VoltasAC : public Component, public climate::Climate {
 public:
  void set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter) {
    this->transmitter_ = transmitter;
  }

  void setup() override {
    this->mode = climate::CLIMATE_MODE_OFF;
    this->target_temperature = 24;
    this->fan_mode = climate::CLIMATE_FAN_AUTO;
    this->swing_mode = climate::CLIMATE_SWING_OFF;
    this->publish_state();
    ESP_LOGI(TAG, "Voltas AC controller initialized (dynamic protocol)");
  }

  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(false);
    traits.set_visual_min_temperature(17);
    traits.set_visual_max_temperature(30);
    traits.set_visual_temperature_step(1);
    traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_DRY,
      climate::CLIMATE_MODE_FAN_ONLY,
      climate::CLIMATE_MODE_AUTO,
    });
    traits.set_supported_fan_modes({
      climate::CLIMATE_FAN_AUTO,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
    });
    traits.set_supported_swing_modes({
      climate::CLIMATE_SWING_OFF,
      climate::CLIMATE_SWING_VERTICAL,
    });
    return traits;
  }

  void control(const climate::ClimateCall &call) override {
    // --- MODE CHANGE ---
    if (call.get_mode().has_value()) {
      climate::ClimateMode new_mode = *call.get_mode();
      if (new_mode == climate::CLIMATE_MODE_OFF) {
        transmit_raw(POWER_OFF_CMD, sizeof(POWER_OFF_CMD) / sizeof(uint16_t));
        this->mode = new_mode;
        this->publish_state();
        ESP_LOGI(TAG, "Power OFF");
        return;
      }
      this->mode = new_mode;
    }

    // --- TEMPERATURE CHANGE ---
    if (call.get_target_temperature().has_value()) {
      int temp = (int)*call.get_target_temperature();
      if (temp >= 17 && temp <= 30)
        this->target_temperature = temp;
    }

    // --- FAN MODE CHANGE ---
    if (call.get_fan_mode().has_value()) {
      this->fan_mode = *call.get_fan_mode();
    }

    // --- SWING MODE CHANGE ---
    if (call.get_swing_mode().has_value()) {
      climate::ClimateSwingMode new_swing = *call.get_swing_mode();
      if (new_swing != this->swing_mode) {
        // Swing is a toggle command
        transmit_raw(SWING_CMD, sizeof(SWING_CMD) / sizeof(uint16_t));
        this->swing_mode = new_swing;
        ESP_LOGI(TAG, "Swing toggled: %s", new_swing == climate::CLIMATE_SWING_VERTICAL ? "ON" : "OFF");
      }
    }

    // If AC is on, send the combined state
    if (this->mode != climate::CLIMATE_MODE_OFF) {
      send_state();
    }

    this->publish_state();
  }

  void send_led(bool on) {
    if (on) {
      transmit_raw(LED_ON_CMD, sizeof(LED_ON_CMD) / sizeof(uint16_t));
    } else {
      transmit_raw(LED_OFF_CMD, sizeof(LED_OFF_CMD) / sizeof(uint16_t));
    }
    ESP_LOGI(TAG, "LED Display: %s", on ? "ON" : "OFF");
  }

 private:
  remote_transmitter::RemoteTransmitterComponent *transmitter_{nullptr};

  // Build and transmit the full AC state as a single IR command
  void send_state() {
    int temp = (int)this->target_temperature;
    if (temp < 17) temp = 17;
    if (temp > 30) temp = 30;

    // --- Encode byte2 (fan speed) ---
    uint8_t byte2;
    if (this->mode == climate::CLIMATE_MODE_DRY || this->mode == climate::CLIMATE_MODE_AUTO) {
      byte2 = FAN_FIXED_BYTE;  // 0x1F - these modes override fan
    } else {
      switch (*this->fan_mode) {
        case climate::CLIMATE_FAN_LOW:    byte2 = FAN_LOW_BYTE;  break;
        case climate::CLIMATE_FAN_MEDIUM: byte2 = FAN_MED_BYTE;  break;
        case climate::CLIMATE_FAN_HIGH:   byte2 = FAN_HIGH_BYTE; break;
        default:                          byte2 = FAN_AUTO_BYTE; break;
      }
    }

    // --- Encode byte4 (temp + mode) ---
    uint8_t temp_code;
    if (this->mode == climate::CLIMATE_MODE_FAN_ONLY) {
      temp_code = 0xE;  // Special "no temp" code for fan mode
    } else {
      temp_code = TEMP_CODES[temp - 17];
    }

    uint8_t mode_nib;
    switch (this->mode) {
      case climate::CLIMATE_MODE_COOL:     mode_nib = MODE_COOL_NIB; break;
      case climate::CLIMATE_MODE_DRY:      mode_nib = MODE_DRY_NIB;  break;
      case climate::CLIMATE_MODE_HEAT:     mode_nib = MODE_HEAT_NIB; break;
      case climate::CLIMATE_MODE_FAN_ONLY: mode_nib = MODE_FAN_NIB;  break;
      case climate::CLIMATE_MODE_AUTO:     mode_nib = MODE_AUTO_NIB; break;
      default:                             mode_nib = MODE_COOL_NIB; break;
    }

    uint8_t byte4 = (temp_code << 4) | mode_nib;

    // Build 6-byte frame
    uint8_t frame[6] = {
      0xB2,           // byte 0: device ID
      0x4D,           // byte 1: device ID
      byte2,          // byte 2: fan speed
      (uint8_t)~byte2, // byte 3: complement
      byte4,          // byte 4: temp + mode
      (uint8_t)~byte4  // byte 5: complement
    };

    ESP_LOGI(TAG, "TX: %02X %02X %02X %02X %02X %02X (temp=%d mode=%d fan=%d)",
             frame[0], frame[1], frame[2], frame[3], frame[4], frame[5],
             temp, (int)this->mode, (int)*this->fan_mode);

    transmit_frame(frame);
  }

  // Encode 6 bytes into raw IR timing and transmit
  void transmit_frame(const uint8_t *frame) {
    auto transmit = this->transmitter_->transmit();
    auto *d = transmit.get_data();
    d->set_carrier_frequency(38000);

    // Two identical frames with gap between
    for (int repeat = 0; repeat < 2; repeat++) {
      // Header
      d->mark(4400);
      d->space(4300);

      // 48 data bits (6 bytes, MSB first)
      for (int byte_idx = 0; byte_idx < 6; byte_idx++) {
        for (int bit = 7; bit >= 0; bit--) {
          d->mark(550);
          if (frame[byte_idx] & (1 << bit)) {
            d->space(1600);  // Bit 1
          } else {
            d->space(500);   // Bit 0
          }
        }
      }

      // Gap between frames (or final mark)
      if (repeat == 0) {
        d->mark(550);
        d->space(5200);
      }
    }

    // Final mark
    d->mark(550);

    transmit.perform();
  }

  // Send pre-captured raw signal (for swing, LED, power off)
  void transmit_raw(const uint16_t *data, uint16_t len) {
    auto transmit = this->transmitter_->transmit();
    auto *d = transmit.get_data();
    d->set_carrier_frequency(38000);
    d->reserve(len);
    for (uint16_t i = 0; i < len; i++) {
      if (i % 2 == 0) {
        d->mark(data[i]);
      } else {
        d->space(data[i]);
      }
    }
    transmit.perform();
  }
};

}  // namespace voltas_ac
}  // namespace esphome
