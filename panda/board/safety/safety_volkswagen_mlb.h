#pragma once

#include "safety_declarations.h"
#include "safety_volkswagen_common.h"

#define MSG_ESP_03      0x0B3   // RX from ABS, for wheel speeds
#define MSG_LH_EPS_03   0x09F   // RX from EPS, for driver steering torque
#define MSG_ESP_05      0x106   // RX from ABS, for brake switch state
#define MSG_TSK_06      0x120   // RX from ECU, for ACC status from drivetrain coordinator
#define MSG_MOTOR_20    0x121   // RX from ECU, for driver throttle input
#define MSG_HCA_01      0x126   // TX by OP, Heading Control Assist steering torque
#define MSG_LDW_02      0x397   // TX by OP, Lane line recognition and text alerts

static uint8_t mlb_crc8_lut_8h2f[256]; // Static lookup table for CRC8 poly 0x2F, aka 8H2F/AUTOSAR
static bool mlb_brake_pedal_switch = false;
static bool mlb_brake_pressure_detected = false;

static uint32_t mlb_get_checksum(const CANPacket_t *to_push) {
  return (uint8_t)GET_BYTE(to_push, 0);
}

static uint8_t mlb_get_counter(const CANPacket_t *to_push) {
  return (uint8_t)GET_BYTE(to_push, 1) & 0xFU;
}

static uint32_t mlb_compute_crc(const CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  int len = GET_LEN(to_push);

  uint8_t crc = 0xFFU;
  for (int i = 1; i < len; i++) {
    crc ^= (uint8_t)GET_BYTE(to_push, i);
    crc = mlb_crc8_lut_8h2f[crc];
  }

  uint8_t counter = mlb_get_counter(to_push);
  if (addr == MSG_LH_EPS_03) {
    crc ^= (uint8_t[]){0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5}[counter];
  } else if (addr == MSG_ESP_05) {
    crc ^= (uint8_t[]){0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07}[counter];
  } else if (addr == MSG_TSK_06) {
    crc ^= (uint8_t[]){0xC4,0xE2,0x4F,0xE4,0xF8,0x2F,0x56,0x81,0x9F,0xE5,0x83,0x44,0x05,0x3F,0x97,0xDF}[counter];
  } else if (addr == MSG_MOTOR_20) {
    crc ^= (uint8_t[]){0xE9,0x65,0xAE,0x6B,0x7B,0x35,0xE5,0x5F,0x4E,0xC7,0x86,0xA2,0xBB,0xDD,0xEB,0xB4}[counter];
  } else {
    // Undefined CAN message, CRC check expected to fail
  }
  crc = mlb_crc8_lut_8h2f[crc];

  return (uint8_t)(crc ^ 0xFFU);
}

static safety_config mlb_init(uint16_t param) {
  static const CanMsg MLB_STOCK_TX_MSGS[] = {{MSG_HCA_01, 0, 8}, {MSG_LDW_02, 0, 8}, {MSG_LH_EPS_03, 2, 8}};

  static RxCheck mlb_rx_checks[] = {
    {.msg = {{MSG_ESP_03, 0, 8, .check_checksum = false, .max_counter = 0U, .frequency = 100U}, { 0 }, { 0 }}},
    {.msg = {{MSG_LH_EPS_03, 0, 8, .check_checksum = true, .max_counter = 15U, .frequency = 100U}, { 0 }, { 0 }}},
    {.msg = {{MSG_ESP_05, 0, 8, .check_checksum = true, .max_counter = 15U, .frequency = 50U}, { 0 }, { 0 }}},
    {.msg = {{MSG_TSK_06, 0, 8, .check_checksum = true, .max_counter = 15U, .frequency = 50U}, { 0 }, { 0 }}},
    {.msg = {{MSG_MOTOR_20, 0, 8, .check_checksum = true, .max_counter = 15U, .frequency = 50U}, { 0 }, { 0 }}},
  };

  UNUSED(param);

  mlb_brake_pedal_switch = false;
  mlb_brake_pressure_detected = false;

  gen_crc_lookup_table_8(0x2F, mlb_crc8_lut_8h2f);
  return BUILD_SAFETY_CFG(mlb_rx_checks, MLB_STOCK_TX_MSGS);
}

static void mlb_rx_hook(const CANPacket_t *to_push) {
  if (GET_BUS(to_push) == 0U) {
    int addr = GET_ADDR(to_push);

    if (addr == MSG_LH_EPS_03) {
      int torque_driver_new = GET_BYTE(to_push, 5) | ((GET_BYTE(to_push, 6) & 0x1FU) << 8);
      int sign = (GET_BYTE(to_push, 6) & 0x80U) >> 7;
      if (sign == 1) {
        torque_driver_new *= -1;
      }
      update_sample(&torque_driver, torque_driver_new);
    }

    if (addr == MSG_TSK_06) {
      int acc_status = (GET_BYTE(to_push, 3) & 0x7U);
      bool cruise_engaged = (acc_status == 3) || (acc_status == 4) || (acc_status == 5);
      acc_main_on = cruise_engaged || (acc_status == 2);
      
      if (!acc_main_on) {
        controls_allowed = false;
      }
    }

    if (addr == MSG_MOTOR_20) {
      gas_pressed = ((GET_BYTES(to_push, 0, 4) >> 12) & 0xFFU) != 0U;
    }

    if (addr == MSG_ESP_05) {
      mlb_brake_pressure_detected = (GET_BYTE(to_push, 3) & 0x4U) >> 2;
    }

    brake_pressed = mlb_brake_pedal_switch || mlb_brake_pressure_detected;

    generic_rx_checks((addr == MSG_HCA_01));
  }
}

static bool mlb_tx_hook(const CANPacket_t *to_send) {
  const SteeringLimits MLB_STEERING_LIMITS = {
    .max_steer = 300,
    .max_rt_delta = 75,
    .max_rt_interval = 250000,
    .max_rate_up = 4,
    .max_rate_down = 10,
    .driver_torque_allowance = 80,
    .driver_torque_factor = 3,
    .type = TorqueDriverLimited,
  };

  int addr = GET_ADDR(to_send);
  bool tx = true;

  if (addr == MSG_HCA_01) {
    int desired_torque = GET_BYTE(to_send, 2) | ((GET_BYTE(to_send, 3) & 0x1U) << 8);
    bool sign = GET_BIT(to_send, 31U);
    if (sign) {
      desired_torque *= -1;
    }

    bool steer_req = GET_BIT(to_send, 30U);

    if (steer_torque_cmd_checks(desired_torque, steer_req, MLB_STEERING_LIMITS)) {
      tx = false;
    }
  }

  return tx;
}

static int mlb_fwd_hook(int bus_num, int addr) {
  int bus_fwd = -1;

  switch (bus_num) {
    case 0:
      bus_fwd = 2; // Forward all remaining traffic from Extended CAN onward
      break;
    case 2:
      if ((addr == MSG_HCA_01) || (addr == MSG_LDW_02)) {
        bus_fwd = -1; // openpilot takes over LKAS steering control
      } else {
        bus_fwd = 0; // Forward all remaining traffic from Extended CAN devices to J533 gateway
      }
      break;
    default:
      bus_fwd = -1; // No forwarding for other buses
      break;
  }

  return bus_fwd;
}

static void mlb_legacy_hook(const CANPacket_t *to_push) {
  // For legacy commands from the original code
}

const safety_hooks mlb_hooks = {
  .init = mlb_init,
  .rx = mlb_rx_hook,
  .tx = mlb_tx_hook,
  .fwd = mlb_fwd_hook,
  .legacy = mlb_legacy_hook,
};

