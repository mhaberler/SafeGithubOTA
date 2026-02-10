/*
 * SafeGithubOTA - Auto-Check Example
 *
 * Demonstrates:
 * - Automatic periodic update checks (configurable interval)
 * - Custom validation callback for rollback protection
 * - Progress reporting during download
 *
 * The device checks for updates at a regular interval while running.
 * After an OTA update, the validation callback runs to verify the
 * new firmware is working correctly before confirming it.
 */

#include <WiFi.h>
#include <SafeGithubOTA.h>

// ---- Configuration ----
const char* FW_VERSION = "1.0.0";
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

SafeGithubOTA ota;

// Validation callback - runs on first boot after OTA update.
// Return true if the new firmware is working correctly.
// Return false to rollback to the previous firmware.
bool validateFirmware() {
    Serial.println("Running post-OTA validation...");

    // Example checks you might perform:

    // 1. Verify WiFi still works
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("FAIL: WiFi not connected");
        return false;
    }

    // 2. Verify a sensor is responding
    // if (!mySensor.begin()) {
    //     Serial.println("FAIL: Sensor not responding");
    //     return false;
    // }

    // 3. Verify critical data in NVS is intact
    // Preferences prefs;
    // prefs.begin("myapp", true);
    // String val = prefs.getString("key", "");
    // prefs.end();
    // if (val.length() == 0) {
    //     Serial.println("FAIL: NVS data missing");
    //     return false;
    // }

    Serial.println("Validation passed!");
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println();
    Serial.printf("=== SafeGithubOTA Auto-Check Example (v%s) ===\n", FW_VERSION);

    // Configure OTA
    ota.setVersion(FW_VERSION);

    // Set the validation callback - this is critical for rollback protection.
    // If the new firmware fails validation, the ESP32 will automatically
    // revert to the previous working firmware on the next reboot.
    ota.onValidation(validateFirmware);

    // Set progress callback for download status
    ota.onProgress([](uint32_t written, uint32_t total) {
        if (total > 0) {
            uint32_t pct = (written * 100) / total;
            Serial.printf("Download: %u%% (%u / %u bytes)\n", pct, written, total);
        } else {
            Serial.printf("Download: %u bytes\n", written);
        }
    });

    // Auto-check for updates every 6 hours
    ota.setAutoCheckInterval(6 * 60 * 60);

    // Provisioning (developer-only, before shipping)
    if (!ota.isProvisioned()) {
        Serial.println("Starting provisioning portal...");
        ota.startProvisioningPortal("MyDevice-Setup");
    }

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected: %s\n", WiFi.localIP().toString().c_str());

    // Initialize OTA
    // If this is the first boot after an OTA update, the validation callback
    // will run here. If it returns false, the device rolls back immediately.
    SGO_Error err = ota.begin();
    if (err == SGO_Error::VALIDATION_FAILED) {
        // This code actually runs on the PREVIOUS firmware after rollback
        Serial.println("Previous OTA update failed validation and was rolled back.");
    }

    Serial.println("Setup complete. Auto-check is running in the background.");
}

void loop() {
    // IMPORTANT: Call ota.loop() to process the auto-check timer.
    ota.loop();

    // Your application code goes here
    delay(10);
}
