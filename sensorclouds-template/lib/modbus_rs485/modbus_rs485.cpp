#include "modbus_rs485.h"

ModbusRS485::ModbusRS485(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin,
                         uint8_t dePin)
    : serial_(serial), rxPin_(rxPin), txPin_(txPin), dePin_(dePin) {}

void ModbusRS485::begin(uint32_t baud) {
    pinMode(dePin_, OUTPUT);
    digitalWrite(dePin_, LOW);
    serial_.begin(baud, SERIAL_8N1, rxPin_, txPin_);
}

bool ModbusRS485::readHolding(uint8_t id, uint16_t startReg, uint8_t count,
                               uint16_t* out) {
    for (uint8_t attempt = 0; attempt < MAX_RETRIES; attempt++) {
        sendRequest(id, startReg, count);
        if (recvResponse(id, count, out)) return true;
        delay(20);
    }
    return false;
}

void ModbusRS485::sendRequest(uint8_t id, uint16_t startReg, uint8_t count) {
    uint8_t req[8];
    req[0] = id;
    req[1] = 0x03;              // FC03 Read Holding Registers
    req[2] = startReg >> 8;
    req[3] = startReg & 0xFF;
    req[4] = 0x00;
    req[5] = count;
    uint16_t crc = crc16(req, 6);
    req[6] = crc & 0xFF;
    req[7] = crc >> 8;

    // flush stale bytes before transmitting
    while (serial_.available()) serial_.read();

    digitalWrite(dePin_, HIGH);
    delayMicroseconds(100);
    serial_.write(req, 8);
    serial_.flush();
    delayMicroseconds(100);
    digitalWrite(dePin_, LOW);
}

bool ModbusRS485::recvResponse(uint8_t id, uint8_t count, uint16_t* out) {
    // Expected: id(1) + FC(1) + byteCount(1) + data(count*2) + CRC(2)
    uint8_t expectedLen = 5 + count * 2;
    uint8_t buf[64];
    uint8_t received = 0;

    uint32_t deadline = millis() + TIMEOUT_MS;
    while (millis() < deadline && received < expectedLen) {
        if (serial_.available()) {
            buf[received++] = (uint8_t)serial_.read();
        }
    }

    if (received < expectedLen) return false;
    if (buf[0] != id || buf[1] != 0x03 || buf[2] != count * 2) return false;

    uint16_t crc = crc16(buf, expectedLen - 2);
    uint16_t rxCrc = (uint16_t)buf[expectedLen - 1] << 8 | buf[expectedLen - 2];
    if (crc != rxCrc) return false;

    for (uint8_t i = 0; i < count; i++) {
        out[i] = (uint16_t)buf[3 + i * 2] << 8 | buf[4 + i * 2];
    }
    return true;
}

uint16_t ModbusRS485::crc16(const uint8_t* buf, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else              crc >>= 1;
        }
    }
    return crc;
}
