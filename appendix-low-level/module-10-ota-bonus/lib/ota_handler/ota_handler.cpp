#include "ota_handler.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>

void OtaHandler::begin(PubSubClient* mqtt, const char* ackTopic) {
    mqtt_ = mqtt;
    ackTopic_ = ackTopic;
    state_ = OtaHandlerState::Idle;
    memset(commandId_, 0, sizeof(commandId_));
    memset(targetVersion_, 0, sizeof(targetVersion_));
    memset(downloadUrl_, 0, sizeof(downloadUrl_));
    memset(checksum_, 0, sizeof(checksum_));
    fileSize_ = 0;
    Serial.println("[OTA] Handler initialized");
}

void OtaHandler::handleMessage(const char* topic, uint8_t* payload, unsigned int length) {
    Serial.printf("[OTA] Message received (%u bytes)\n", length);

    JsonDocument doc;
    if (deserializeJson(doc, payload, length)) {
        Serial.println("[OTA] JSON parse error");
        return;
    }

    const char* command = doc["command"];
    if (!command) return;

    if (strcmp(command, "ota:update") == 0) {
        handleOtaUpdate(doc);
    } else if (strcmp(command, "ota:rollback") == 0) {
        handleOtaRollback(doc);
    }
}

void OtaHandler::update() {
    // Reserved for future non-blocking expansion
}

void OtaHandler::handleOtaUpdate(JsonDocument& doc) {
    if (state_ != OtaHandlerState::Idle) {
        const char* cmdId = doc["commandId"] | "unknown";
        sendAck(cmdId, "ota:update", "failed", -1, nullptr, "Update already in progress");
        return;
    }

    const char* cmdId = doc["commandId"];
    JsonObject params = doc["parameters"];
    const char* version = params["version"];
    const char* url = params["downloadUrl"];
    const char* chk = params["checksum"];
    size_t fSize = params["fileSize"] | 0;

    if (!cmdId || !version || !url || !chk || fSize == 0) {
        sendAck(cmdId ? cmdId : "unknown", "ota:update", "failed",
                -1, nullptr, "Missing required fields");
        return;
    }

    strncpy(commandId_, cmdId, sizeof(commandId_) - 1);
    strncpy(targetVersion_, version, sizeof(targetVersion_) - 1);
    strncpy(downloadUrl_, url, sizeof(downloadUrl_) - 1);
    strncpy(checksum_, chk, sizeof(checksum_) - 1);
    fileSize_ = fSize;

    Serial.printf("[OTA] Update command: v%s, %u bytes\n", targetVersion_, fileSize_);

    state_ = OtaHandlerState::Downloading;
    sendAck(commandId_, "ota:update", "downloading", 0);

    bool success = downloadAndInstall(downloadUrl_, checksum_, fileSize_);

    if (success) {
        state_ = OtaHandlerState::Installing;
        sendAck(commandId_, "ota:update", "installing", 100);

        OtaState otaState;
        otaState.clear();
        strncpy(otaState.commandId, commandId_, sizeof(otaState.commandId) - 1);
        strncpy(otaState.targetVersion, targetVersion_, sizeof(otaState.targetVersion) - 1);
        otaState.state = 2;
        saveOtaState(otaState);

        Serial.println("[OTA] Rebooting to apply update...");
        delay(500);
        ESP.restart();
    } else {
        state_ = OtaHandlerState::Idle;
    }
}

void OtaHandler::handleOtaRollback(JsonDocument& doc) {
    const char* cmdId = doc["commandId"] | "unknown";
    Serial.println("[OTA] Rollback command received");

    const esp_partition_t* previous = esp_ota_get_next_update_partition(nullptr);
    if (!previous) {
        sendAck(cmdId, "ota:rollback", "failed", -1, nullptr, "No previous partition");
        return;
    }

    if (esp_ota_set_boot_partition(previous) != ESP_OK) {
        sendAck(cmdId, "ota:rollback", "failed", -1, nullptr, "Failed to set boot partition");
        return;
    }

    sendAck(cmdId, "ota:rollback", "ok");
    Serial.println("[OTA] Rollback set, rebooting...");
    delay(500);
    ESP.restart();
}

bool OtaHandler::downloadAndInstall(const char* url, const char* checksum, size_t fileSize) {
    WiFiClientSecure secureClient;
    secureClient.setInsecure();

    HTTPClient http;
    Serial.printf("[OTA] Downloading: %s\n", url);

    if (!http.begin(secureClient, url)) {
        sendAck(commandId_, "ota:update", "failed", -1, nullptr, "HTTP connection failed");
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        char errMsg[64];
        snprintf(errMsg, sizeof(errMsg), "HTTP error: %d", httpCode);
        sendAck(commandId_, "ota:update", "failed", -1, nullptr, errMsg);
        http.end();
        return false;
    }

    mbedtls_sha256_context sha256Ctx;
    mbedtls_sha256_init(&sha256Ctx);
    mbedtls_sha256_starts(&sha256Ctx, 0);

    if (!Update.begin(fileSize)) {
        sendAck(commandId_, "ota:update", "failed", -1, nullptr, "Update.begin() failed");
        mbedtls_sha256_free(&sha256Ctx);
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    uint8_t buf[2048];
    size_t totalRead = 0;
    int lastProgress = 0;

    while (totalRead < fileSize) {
        if (mqtt_) mqtt_->loop();

        size_t available = stream->available();
        if (available == 0) {
            if (!stream->connected()) break;
            delay(10);
            continue;
        }

        size_t toRead = min(available, sizeof(buf));
        size_t bytesRead = stream->readBytes(buf, toRead);
        if (bytesRead == 0) break;

        if (Update.write(buf, bytesRead) != bytesRead) {
            sendAck(commandId_, "ota:update", "failed", -1, nullptr, "Flash write error");
            Update.abort();
            mbedtls_sha256_free(&sha256Ctx);
            http.end();
            return false;
        }

        mbedtls_sha256_update(&sha256Ctx, buf, bytesRead);
        totalRead += bytesRead;

        int progress = (int)((totalRead * 100) / fileSize);
        if (progress / 25 > lastProgress / 25) {
            sendAck(commandId_, "ota:update", "downloading", progress);
            lastProgress = progress;
        }
    }

    http.end();

    if (totalRead != fileSize) {
        char errMsg[80];
        snprintf(errMsg, sizeof(errMsg), "Download incomplete: %u/%u bytes", totalRead, fileSize);
        sendAck(commandId_, "ota:update", "failed", -1, nullptr, errMsg);
        Update.abort();
        mbedtls_sha256_free(&sha256Ctx);
        return false;
    }

    state_ = OtaHandlerState::Verifying;
    uint8_t hash[32];
    mbedtls_sha256_finish(&sha256Ctx, hash);
    mbedtls_sha256_free(&sha256Ctx);

    if (!verifyChecksum(hash, checksum)) {
        sendAck(commandId_, "ota:update", "failed", -1, nullptr, "Checksum verification failed");
        Update.abort();
        return false;
    }

    if (!Update.end(true)) {
        sendAck(commandId_, "ota:update", "failed", -1, nullptr, "Update.end() failed");
        return false;
    }

    const esp_partition_t* updatePartition = esp_ota_get_next_update_partition(nullptr);
    if (updatePartition) {
        if (esp_ota_set_boot_partition(updatePartition) != ESP_OK) {
            sendAck(commandId_, "ota:update", "failed", -1, nullptr, "Failed to set boot partition");
            return false;
        }
        Serial.printf("[OTA] Boot partition set: %s\n", updatePartition->label);
    }

    Serial.println("[OTA] Download and verification complete");
    return true;
}

bool OtaHandler::verifyChecksum(const uint8_t* computed, const char* expected) {
    char computedHex[65];
    for (int i = 0; i < 32; i++) snprintf(&computedHex[i * 2], 3, "%02x", computed[i]);
    computedHex[64] = '\0';
    Serial.printf("[OTA] Checksum computed: %s\n", computedHex);
    Serial.printf("[OTA] Checksum expected: %s\n", expected);
    return strcasecmp(computedHex, expected) == 0;
}

void OtaHandler::sendAck(const char* commandId, const char* command,
                          const char* status, int progress,
                          const char* version, const char* error) {
    JsonDocument doc;
    doc["commandId"] = commandId;
    doc["command"] = command;
    doc["status"] = status;
    if (progress >= 0) doc["progress"] = progress;
    if (version) doc["firmwareVersion"] = version;
    if (error) doc["error"] = error;

    char buffer[512];
    serializeJson(doc, buffer, sizeof(buffer));

    if (mqtt_ && mqtt_->connected()) mqtt_->publish(ackTopic_, buffer);
    Serial.printf("[OTA] ACK: %s\n", buffer);
}

bool OtaHandler::saveOtaState(const OtaState& s) {
    Preferences prefs;
    prefs.begin(OTA_NAMESPACE, false);
    prefs.putString("cmd_id", s.commandId);
    prefs.putString("target_ver", s.targetVersion);
    prefs.putUChar("state", s.state);
    prefs.end();
    Serial.println("[OTA] State persisted to NVS");
    return true;
}

bool OtaHandler::loadOtaState(OtaState& s) {
    Preferences prefs;
    prefs.begin(OTA_NAMESPACE, true);
    uint8_t state = prefs.getUChar("state", 0);
    if (state == 0) {
        prefs.end();
        return false;
    }
    s.state = state;
    prefs.getString("cmd_id", s.commandId, sizeof(s.commandId));
    prefs.getString("target_ver", s.targetVersion, sizeof(s.targetVersion));
    prefs.end();
    return true;
}

void OtaHandler::clearOtaState() {
    Preferences prefs;
    prefs.begin(OTA_NAMESPACE, false);
    prefs.clear();
    prefs.end();
}

void OtaHandler::checkPendingAck() {
    OtaState s;
    s.clear();
    if (!loadOtaState(s)) return;
    if (s.state != 2) return;

    Serial.printf("[OTA] Post-reboot: pending ACK for cmd %s, target v%s\n",
                  s.commandId, s.targetVersion);

    esp_ota_mark_app_valid_cancel_rollback();

    if (strcmp(FIRMWARE_VERSION, s.targetVersion) == 0) {
        sendAck(s.commandId, "ota:update", "ok", -1, FIRMWARE_VERSION);
        Serial.println("[OTA] Update confirmed successful");
    } else {
        char errMsg[80];
        snprintf(errMsg, sizeof(errMsg), "Version mismatch: running %s, expected %s",
                 FIRMWARE_VERSION, s.targetVersion);
        sendAck(s.commandId, "ota:update", "failed", -1, nullptr, errMsg);
        Serial.println("[OTA] Update verification failed");
    }

    clearOtaState();
}
