#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "button.h"
#include "metronome.h"
#include "website.h"

// Defined in main.cpp.
extern SemaphoreHandle_t metronome_mutex;
extern QueueHandle_t beatsPerMeasure_queue;

namespace RiffWeb {

constexpr char WIFI_SSID[] = "iPhone de Guillermo";
constexpr char WIFI_PASSWORD[] = "465hyffghfx";
constexpr char HOSTNAME[] = "metronome";

constexpr uint8_t STREAK_TARGET = 3;

bool learningMode = false;
uint8_t successStreak = 0;
uint8_t failStreak = 0;
uint32_t selectedBeatsPerMeasure = Measures::fourBeatsPerMeasure;

WebServer server(80);

void resetStreaks() {
    successStreak = 0;
    failStreak = 0;
}

// These callbacks run inside readButtons_task(), which already
// holds metronome_mutex. Do not take the mutex again here.
void onSuccessButton() {
    if (!learningMode) {
        increaseMetronomeRate();
        return;
    }

    failStreak = 0;
    successStreak++;

    if (successStreak >= STREAK_TARGET) {
        increaseMetronomeRate();
        resetStreaks();
    }
}

void onFailureButton() {
    if (!learningMode) {
        decreaseMetronomeRate();
        return;
    }

    successStreak = 0;
    failStreak++;

    if (failStreak >= STREAK_TARGET) {
        decreaseMetronomeRate();
        resetStreaks();
    }
}

// Take a consistent snapshot, then release the mutex before
// constructing and sending the HTTP response.
void sendState() {
    xSemaphoreTake(metronome_mutex, portMAX_DELAY);

    const bool mode = learningMode;
    const uint8_t successes = successStreak;
    const uint8_t failures = failStreak;
    const int bpm = static_cast<int>(getMetronomeBPM());

    xSemaphoreGive(metronome_mutex);

    String json;
    json.reserve(128);

    json = "{\"learning\":";
    json += mode ? "true" : "false";
    json += ",\"bpm\":";
    json += String(bpm);
    json += ",\"successStreak\":";
    json += String(successes);
    json += ",\"failStreak\":";
    json += String(failures);
    json += ",\"lives\":";
    json += String(STREAK_TARGET - failures);
    json += ",\"beatsPerMeasure\":";
    json += String(selectedBeatsPerMeasure);
    json += "}";

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
}


void begin() {
    // Call before starting the button task.
    button_up.fptr = &onSuccessButton;
    button_down.fptr = &onFailureButton;

    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html; charset=utf-8", PAGE);
    });

    server.on("/api/state", HTTP_GET, sendState);

    server.on("/api/mode", HTTP_POST, []() {
        if (!server.hasArg("enabled") ||
            (server.arg("enabled") != "0" &&
             server.arg("enabled") != "1")) {
            server.send(
                400,
                "application/json",
                "{\"error\":\"enabled must be 0 or 1\"}"
            );
            return;
        }

        const bool requestedMode = server.arg("enabled") == "1";

        xSemaphoreTake(metronome_mutex, portMAX_DELAY);

        if (learningMode != requestedMode) {
            learningMode = requestedMode;
            resetStreaks();
        }

        xSemaphoreGive(metronome_mutex);

        sendState();
    });

    server.on("/api/measure", HTTP_POST, []() {
        const String value = server.arg("beats");
        if (value != "2" && value != "3" && value != "4") {
            server.send(400, "application/json",
                        "{\"error\":\"beats must be 2, 3 or 4\"}");
            return;
        }

        const uint32_t requestedBeats = static_cast<uint32_t>(value.toInt());
        if (requestedBeats != selectedBeatsPerMeasure) {
            if (beatsPerMeasure_queue == nullptr ||
                xQueueOverwrite(beatsPerMeasure_queue, &requestedBeats) != pdTRUE) {
                server.send(503, "application/json",
                            "{\"error\":\"Could not update beats per measure\"}");
                return;
            }
            // Requested setting; the buzzer consumes it on its next iteration.
            selectedBeatsPerMeasure = requestedBeats;
        }

        sendState();
    });

    server.onNotFound([]() {
        server.send(404, "text/plain", "Not found");
    });

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("Connecting to Wi-Fi...");
}

// Called from Arduino's loop(), independently of the existing tasks.
void service() {
    static bool online = false;
    static unsigned long lastRetry = 0;

    if (WiFi.status() != WL_CONNECTED) {
        if (online) {
            server.stop();
            MDNS.end();
            online = false;
            Serial.println("Wi-Fi disconnected.");
        }

        if (millis() - lastRetry >= 15000) {
            lastRetry = millis();
            Serial.println("Retrying Wi-Fi...");
            WiFi.reconnect();
        }

        return;
    }

    if (!online) {
        online = true;

        Serial.print("Website: http://");
        Serial.println(WiFi.localIP());

        if (MDNS.begin(HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
            Serial.println("Website: http://metronome.local");
        }

        server.begin();
    }

    server.handleClient();
}

} // namespace RiffWeb
