#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "button.h"
#include "metronome.h"

// Defined in main.cpp.
extern SemaphoreHandle_t metronome_mutex;

namespace RiffWeb {

constexpr char WIFI_SSID[] = "iPhone de Guillermo";
constexpr char WIFI_PASSWORD[] = "465hyffghfx";
constexpr char HOSTNAME[] = "metronome";

constexpr uint8_t STREAK_TARGET = 3;

bool learningMode = false;
uint8_t successStreak = 0;
uint8_t failStreak = 0;

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
    json += "}";

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
}

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Riff learning</title>

  <style>
    body {
      margin: 0;
      padding: 40px 20px;
      background: #10141c;
      color: #f0f4ff;
      font-family: system-ui, sans-serif;
      text-align: center;
    }

    main {
      max-width: 440px;
      margin: auto;
    }

    .card {
      background: #1d2533;
      padding: 24px;
      border-radius: 16px;
      margin: 20px 0;
    }

    #bpm {
      font-size: 64px;
      font-weight: bold;
    }

    button {
      width: 100%;
      padding: 16px;
      border: 0;
      border-radius: 10px;
      background: #a6f3c0;
      color: #102017;
      font-size: 18px;
      cursor: pointer;
    }

    button:disabled {
      opacity: 0.5;
      cursor: wait;
    }

    #lives {
      color: #ff8397;
      font-size: 36px;
      letter-spacing: 8px;
    }

    #success { color: #a6f3c0; }
    #failure { color: #ff8397; }

    .hint, #connection {
      color: #b7c3d8;
    }
  </style>
</head>

<body>
<main>
  <h1>Riff learning</h1>

  <div class="card">
    <div id="bpm">—</div>
    <div>BPM</div>
  </div>

  <button id="mode" disabled>Connecting…</button>

  <section id="practice" class="card" hidden>
    <div id="lives" role="img" aria-label="Lives"></div>

    <p>
      Success streak:
      <strong id="success">0 / 3</strong>
    </p>

    <p>
      Mistake streak:
      <strong id="failure">0 / 3</strong>
    </p>

    <p class="hint">
      UP button: played correctly<br>
      DOWN button: made a mistake
    </p>

    <p class="hint">
      Three successes: +10 BPM<br>
      Three mistakes: −10 BPM<br>
      Counters reset after each group of three.
    </p>
  </section>

  <p id="normal" class="hint">
    Normal mode: buttons adjust BPM.
  </p>

  <p id="connection" role="status">Connecting…</p>
</main>

<script>
  const modeButton = document.getElementById("mode");
  const connection = document.getElementById("connection");

  let learning = false;
  let busy = false;

  function render(data) {
    learning = data.learning;

    document.getElementById("bpm").textContent = data.bpm;
    document.getElementById("practice").hidden = !learning;
    document.getElementById("normal").hidden = learning;

    modeButton.textContent = learning
      ? "Disable riff learning"
      : "Enable riff learning";

    document.getElementById("success").textContent =
      `${data.successStreak} / 3`;

    document.getElementById("failure").textContent =
      `${data.failStreak} / 3`;

    const lives = document.getElementById("lives");

    lives.textContent =
      "♥".repeat(data.lives) + "♡".repeat(3 - data.lives);

    lives.setAttribute(
      "aria-label",
      `${data.lives} of 3 lives remaining`
    );
  }

  async function refresh(changeMode = false) {
    if (busy) return;

    busy = true;

    if (changeMode) {
    modeButton.disabled = true;
    }

    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 5000);

    try {
      let url = "/api/state";
      const options = {
        cache: "no-store",
        signal: controller.signal
      };

      if (changeMode) {
        url = "/api/mode";
        options.method = "POST";
        options.headers = {
          "Content-Type": "application/x-www-form-urlencoded"
        };

        // Send the desired state instead of a blind toggle.
        options.body = `enabled=${learning ? 0 : 1}`;
      }

      const response = await fetch(url, options);

      if (!response.ok) {
        throw new Error("Request failed");
      }

      render(await response.json());

      connection.textContent = "Connected";
      modeButton.disabled = false;
    } catch (error) {
      connection.textContent =
        "Connection lost. Display may be stale. Retrying…";
    } finally {
      clearTimeout(timeout);
      busy = false;
    }
  }

  modeButton.addEventListener("click", () => refresh(true));

  refresh();
  setInterval(() => refresh(), 1000);
</script>
</body>
</html>
)HTML";

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