#include "config_cmd_handler.h"
#include "ssc_client.h"
#define LED_CMD_PIN 2
void ConfigCmdHandler::begin(ConfigStore* store, Config* config, SSCClient* client) {
    store_ = store;
    config_ = config;
    client_ = client;
    pinMode(LED_CMD_PIN, OUTPUT);
    digitalWrite(LED_CMD_PIN, LOW); 
    Serial.println("[CfgCmd] Handler initialized");
}

void ConfigCmdHandler::handleMessage(const char* topic, uint8_t* payload, unsigned int length) {
    Serial.printf("[CfgCmd] Message received (%u bytes)\n", length);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
        Serial.printf("[CfgCmd] JSON parse error: %s\n", err.c_str());
        return;
    }

    const char* command = doc["command"];
    if (!command) {
        Serial.println("[CfgCmd] No 'command' field");
        return;
    }

    if (strcmp(command, "config:set_interval") == 0) {
        handleSetInterval(doc);
    } else if (strcmp(command, "config:set_power_interval") == 0) {
        handleSetPowerInterval(doc);
    } else if (strcmp(command, "config:set_led") == 0) {
        handleSetLed(doc);
    } else {
        Serial.printf("[CfgCmd] Unknown command: %s\n", command);
    }

}

void ConfigCmdHandler::handleSetInterval(JsonDocument& doc) {
    const char* cmdId = doc["commandId"] | "unknown";
    JsonVariant param = doc["parameters"]["publish_interval_ms"];

    if (param.isNull() || !param.is<uint32_t>()) {
        sendAck(cmdId, "config:set_interval", "failed",
                "Missing or invalid publish_interval_ms");
        return;
    }

    uint32_t newInterval = param.as<uint32_t>();
    if (newInterval < MIN_INTERVAL_MS || newInterval > MAX_INTERVAL_MS) {
        char errMsg[80];
        snprintf(errMsg, sizeof(errMsg),
                 "publish_interval_ms out of range (%u-%u)",
                 MIN_INTERVAL_MS, MAX_INTERVAL_MS);
        sendAck(cmdId, "config:set_interval", "failed", errMsg);
        return;
    }

    config_->publish_interval_ms = newInterval;

    if (!store_->save(*config_)) {
        sendAck(cmdId, "config:set_interval", "failed", "NVS save failed");
        return;
    }

    Serial.printf("[CfgCmd] publish_interval_ms = %u (saved)\n", newInterval);
    sendAck(cmdId, "config:set_interval", "ok");
}

void ConfigCmdHandler::handleSetPowerInterval(JsonDocument& doc) {
    const char* cmdId = doc["commandId"] | "unknown";
    JsonVariant param = doc["parameters"]["power_publish_interval_ms"];

    if (param.isNull() || !param.is<uint32_t>()) {
        sendAck(cmdId, "config:set_power_interval", "failed",
                "Missing or invalid power_publish_interval_ms");
        return;
    }

    uint32_t newInterval = param.as<uint32_t>();
    if (newInterval < MIN_INTERVAL_MS || newInterval > MAX_INTERVAL_MS) {
        char errMsg[80];
        snprintf(errMsg, sizeof(errMsg),
                 "power_publish_interval_ms out of range (%u-%u)",
                 MIN_INTERVAL_MS, MAX_INTERVAL_MS);
        sendAck(cmdId, "config:set_power_interval", "failed", errMsg);
        return;
    }

    config_->power_publish_interval_ms = newInterval;

    if (!store_->save(*config_)) {
        sendAck(cmdId, "config:set_power_interval", "failed", "NVS save failed");
        return;
    }

    Serial.printf("[CfgCmd] power_publish_interval_ms = %u (saved)\n", newInterval);
    sendAck(cmdId, "config:set_power_interval", "ok");
}

void ConfigCmdHandler::handleSetLed(JsonDocument& doc) {
    const char* cmdId = doc["commandId"] | "unknown";
    JsonVariant param = doc["parameters"]["state"];

    if (param.isNull() || !param.is<int>()) {
        sendAck(cmdId, "config:set_led", "failed", "Missing or invalid state (0 or 1)");
        return;
    }

    int state = param.as<int>();
    digitalWrite(LED_CMD_PIN, state ? HIGH : LOW);
    Serial.printf("[CfgCmd] LED -> %d\n", state);
    sendAck(cmdId, "config:set_led", "ok");
}

void ConfigCmdHandler::sendAck(const char* commandId, const char* command,
                                const char* status, const char* error) {
    JsonDocument doc;
    doc["commandId"] = commandId;
    doc["command"] = command;
    doc["status"] = status;
    if (error) doc["error"] = error;

    char buffer[256];
    serializeJson(doc, buffer, sizeof(buffer));

    if (client_) {
        client_->publishAck(buffer);
    }
    Serial.printf("[CfgCmd] ACK: %s\n", buffer);
}
