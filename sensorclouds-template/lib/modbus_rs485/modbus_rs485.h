#pragma once

#include <Arduino.h>
#include <stdint.h>

// Modbus RTU master over half-duplex RS485.
// Supports FC03 (Read Holding Registers) only.
class ModbusRS485 {
public:
    // rxPin/txPin: UART2 GPIO. dePin: DE/RE line (HIGH = TX, LOW = RX).
    ModbusRS485(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin, uint8_t dePin);

    void begin(uint32_t baud);

    // Read `count` holding registers starting at `startReg` from slave `id`.
    // Returns true and fills `out` (length `count`) on success.
    // Retries up to MAX_RETRIES times with TIMEOUT_MS per attempt.
    bool readHolding(uint8_t id, uint16_t startReg, uint8_t count,
                     uint16_t* out);

private:
    void     sendRequest(uint8_t id, uint16_t startReg, uint8_t count);
    bool     recvResponse(uint8_t id, uint8_t count, uint16_t* out);
    uint16_t crc16(const uint8_t* buf, uint8_t len);

    HardwareSerial& serial_;
    uint8_t         rxPin_;
    uint8_t         txPin_;
    uint8_t         dePin_;

    static constexpr uint32_t TIMEOUT_MS  = 200;
    static constexpr uint8_t  MAX_RETRIES = 3;
};
