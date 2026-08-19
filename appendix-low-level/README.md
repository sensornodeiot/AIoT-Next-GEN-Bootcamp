# Appendix — Low-Level Connectivity (optional)

These are the **hand-coded** versions of Modules 5–10. They drive the low-level `SSCClient` library directly — you connect WiFi yourself, build an `SSCConfig` from build flags, and call `begin()` / `update()` by hand (env `ssn32`).

**This is not the main course path.** For Modules 5–10 the course uses the ready-made template in [`../sensorclouds-template/`](../sensorclouds-template/) (flash + configure in a browser). These folders exist only to show what that template does *inside*, for anyone curious.

| Folder | Low-level topic |
|--------|-----------------|
| `module-05-ssc-client` | connect WiFi + `SSCClient` + `begin()`/`update()` |
| `module-06-mqtt-registration` | the MQTT model, observed on the low-level client |
| `module-07-publish-telemetry` | hand-encode + `publishTelemetry()` |
| `module-09-receive-commands` | `setCommandCallback` + parse raw bytes + self-built ACK |
| `module-10-ota-bonus` | `OtaHandler` wired by hand |

Each is a self-contained PlatformIO project (env `ssn32`): open the folder and `pio run -e ssn32`.
