#pragma once

// Config Portal AP settings
#define OTA_AP_SSID "SSN32-Config"
#define OTA_AP_PASSWORD "ssn32config"

// Boot button to trigger config portal
#define OTA_TRIGGER_PIN 0 // GPIO0 = BOOT button

// Firmware version fallback (overridden by build flag)
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0-dev"
#endif

// ── RS485 / Modbus ──────────────────────────────────────────────────
#define RS485_RX_PIN   16   // UART2 RX
#define RS485_TX_PIN   17   // UART2 TX
#define RS485_DE_PIN    4   // DE/RE direction control (HIGH = TX)
#define RS485_BAUD   9600

// ── WTVB02-485 vibration sensor ─────────────────────────────────────
#define VIBRATION_MODBUS_ID  0x50

// ── RS485 power meter ───────────────────────────────────────────────
#define POWER_METER_ENABLED              1
#define POWER_METER_MODBUS_ID         0x01
#define POWER_METER_READ_INTERVAL_MS  5000
#define POWER_METER_WORD_SWAP            0   // set 1 if meter swaps float words
