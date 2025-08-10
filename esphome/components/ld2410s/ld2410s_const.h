#include "ld2410s.h"

#pragma once

#include "esphome/core/application.h"
// #include "esphome/core/defines.h"
// #include <functional>

namespace esphome {
namespace ld2410s {

static const uint8_t SHORT_DATA_FRAME_HEADER = 0x6E;
static const uint8_t SHORT_DATA_FRAME_FOOTER = 0x62;

static const uint32_t STD_DATA_FRAME_HEADER = 0xF1F2F3F4;
static const uint32_t STD_DATA_FRAME_FOOTER = 0xF5F6F7F8;

static const uint32_t CMD_FRAME_HEADER = 0xFAFBFCFD;
static const uint32_t CMD_FRAME_FOOTER = 0x01020304;

static const uint16_t CONFIG_MODE_START_CMD = 0x00FF;
static const uint16_t CONFIG_MODE_START_REPLY = 0x01FF;
static const uint8_t CONFIG_MODE_START_VALUE[] = {0x01, 0x00};

static const uint16_t CONFIG_MODE_END_CMD = 0x00FE;
static const uint16_t CONFIG_MODE_END_REPLY = 0x01FE;

static const uint16_t OUTPUT_MODE_SWITCH_CMD = 0x007A;
static const uint16_t OUTPUT_MODE_SWITCH_REPLY = 0x017A;
static const uint8_t OUTPUT_MODE_VALUE_STD[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
static const uint8_t OUTPUT_MODE_VALUE_MIN[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const uint16_t FW_READ_CMD = 0x0000;
static const uint16_t FW_READ_REPLY = 0x0100;

static const uint16_t SN_READ_CMD = 0x0011;
static const uint16_t SN_READ_REPLY = 0x0111;
static const uint16_t SN_WRITE_CMD = 0x0010;
static const uint16_t SN_WRITE_REPLY = 0x0110;

static const uint16_t PARAMS_READ_CMD = 0x0071;
static const uint16_t PARAMS_READ_REPLY = 0x0171;
static const uint16_t PARAMS_WRITE_CMD = 0x0070;
static const uint16_t PARAMS_WRITE_REPLY = 0x0170;
static const uint16_t CFG_MAX_DETECTION_VALUE = 0x0005;
static const uint16_t CFG_MIN_DETECTION_VALUE = 0x000A;
static const uint16_t CFG_NO_DELAY_VALUE = 0x0006;
static const uint16_t CFG_STATUS_FREQ_VALUE = 0x0002;
static const uint16_t CFG_DISTANCE_FREQ_VALUE = 0x000C;
static const uint16_t CFG_RESPONSE_SPEED_VALUE = 0x000B;
static const std::string RESPONSE_SPEED_NORMAL = "Normal";
static const std::string RESPONSE_SPEED_FAST = "Fast";

static const uint16_t CALIBRATION_CMD = 0x0009;
static const uint16_t CALIBRATION_TRIGGER_VALUE = 0x0002;
static const uint16_t CALIBRATION_RETENTION_VALUE = 0x0001;
static const uint16_t CALIBRATION_TIME_VALUE = 0x0078;

static const uint16_t GATE_THRESHOLD_TRIGGER_READ_CMD = 0x0073;
static const uint16_t GATE_THRESHOLD_TRIGGER_READ_REPLY = 0x0173;
static const uint16_t GATE_THRESHOLD_TRIGGER_WRITE_CMD = 0x0072;
static const uint16_t GATE_THRESHOLD_TRIGGER_WRITE_REPLY = 0x0172;
static const uint32_t GATE_THRESHOLD_TRIGGER_WRITE_DATA[] = {

    48, 42, 36, 34, 32, 31, 31, 31, 31,
    31, 31, 31, 31, 31, 31, 31
    // 10~95 dB

    // Factory defaults:
    // tool - reset
    //  https://github.com/MrUndead1996/ld2410s-esphome/issues/4
    //   48,42,36,34,32,31,31,31,31,31,31,31,31,31,31,31
    // tool default
    //  https://drive.google.com/drive/folders/1wC8KC-DaNavNbpeVouZ1HdiBzZ9YrAcg
    //   50,46,34,32,32,32,32,32,25,25,25,25,25,25,25,25
};

static const uint16_t GATE_THRESHOLD_HOLD_READ_CMD = 0x0077;
static const uint16_t GATE_THRESHOLD_HOLD_READ_REPLY = 0x0177;
static const uint16_t GATE_THRESHOLD_HOLD_WRITE_CMD = 0x0076;
static const uint16_t GATE_THRESHOLD_HOLD_WRITE_REPLY = 0x0176;
static const uint32_t GATE_THRESHOLD_HOLD_WRITE_DATA[] = {

    45, 42, 33, 32, 28, 28, 28, 28, 28,
    28, 28, 28, 28, 28, 28, 28
    // 10~95 dB

    // Factory defaults:
    // tool - reset
    //  https://github.com/MrUndead1996/ld2410s-esphome/issues/4
    //   45,42,33,32,28,28,28,28,28,28,28,28,28,28,28,28
    // tool default
    //   52,49,26,25,25,21,22,24,23,22,21,21,20,21,21,20
};

static const uint16_t GATE_THRESHOLD_SNR_READ_CMD = 0x0075;
static const uint16_t GATE_THRESHOLD_SNR_READ_REPLY = 0x0175;
static const uint16_t GATE_THRESHOLD_SNR_WRITE_CMD = 0x0074;
static const uint16_t GATE_THRESHOLD_SNR_WRITE_REPLY = 0x0174;
static const uint32_t GATE_THRESHOLD_SNR_WRITE_DATA[] = {

    34, 34, 34, 34, 34, 34, 34, 34, 34,
    34, 34, 34, 34, 34, 34, 34
    // 5~63 dB

    // Factory defaults:
    // Not available... and probably need improvement ToDo
    // It would be good to get it from virgin ld2410s, before any calibration.
};

}  // namespace ld2410s
}  // namespace esphome
