#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

enum class OtaHandlerState : uint8_t {
    Idle        = 0,
    Downloading = 1,
    Verifying   = 2,
    Installing  = 3,
    PendingAck  = 4
};

struct OtaState {
    char commandId[65];
    char targetVersion[20];
    uint8_t state;  // 0=idle, 2=pending_ack
    bool isPending() const { return state != 0; }
    void clear() { memset(this, 0, sizeof(OtaState)); }
};

class OtaHandler {
public:
    void begin(PubSubClient* mqtt, const char* ackTopic);

    // Called from MQTT command callback for ota:* commands
    void handleMessage(const char* topic, uint8_t* payload, unsigned int length);

    // Drive non-blocking state machine — call from loop()
    void update();

    // Check NVS for pending post-reboot ACK — call once after first MQTT connect
    void checkPendingAck();

    bool isUpdating() const { return state_ != OtaHandlerState::Idle; }
    OtaHandlerState state() const { return state_; }

private:
    void handleOtaUpdate(JsonDocument& doc);
    void handleOtaRollback(JsonDocument& doc);

    bool downloadAndInstall(const char* url, const char* checksum, size_t fileSize);
    bool verifyChecksum(const uint8_t* computed, const char* expected);

    void sendAck(const char* commandId, const char* command,
                 const char* status, int progress = -1,
                 const char* version = nullptr, const char* error = nullptr);

    bool saveOtaState(const OtaState& s);
    bool loadOtaState(OtaState& s);
    void clearOtaState();

    PubSubClient* mqtt_ = nullptr;
    const char* ackTopic_ = nullptr;
    OtaHandlerState state_ = OtaHandlerState::Idle;

    char commandId_[65];
    char targetVersion_[20];
    char downloadUrl_[256];
    char checksum_[65];
    size_t fileSize_ = 0;

    static constexpr const char* OTA_NAMESPACE = "ota_state";
};
