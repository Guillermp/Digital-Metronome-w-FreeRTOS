#pragma once
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

    .measure-setting {
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      margin: 24px 0;
      font-size: 14px;
    }

    #measure {
      padding: 10px 12px;
      border: 1px solid currentColor;
      border-radius: 8px;
      background: var(--surface, #1d2533);
      color: var(--ink, #f0f4ff);
      font: inherit;
      cursor: pointer;
    }

    #measure:disabled {
      opacity: 0.5;
      cursor: wait;
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

  <div class="measure-setting">
    <label for="measure">Beats per measure</label>
    <select id="measure" disabled>
      <option value="2">2 beats</option>
      <option value="3">3 beats</option>
      <option value="4" selected>4 beats</option>
    </select>
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
  const measureSelect = document.getElementById("measure");
  const connection = document.getElementById("connection");

  let learning = false;
  let busy = false;
  const pendingActions = [];

  function render(data) {
    learning = data.learning;

    document.getElementById("bpm").textContent = data.bpm;
    document.getElementById("practice").hidden = !learning;
    document.getElementById("normal").hidden = learning;

    modeButton.textContent = learning
      ? "Disable riff learning"
      : "Enable riff learning";

    const measurePending = pendingActions.some(
      action => action.url === "/api/measure"
    );
    if (!measurePending && document.activeElement !== measureSelect) {
      measureSelect.value = String(data.beatsPerMeasure);
    }

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

  async function refresh(action = null) {
    if (busy) {
      if (action !== null) pendingActions.push(action);
      return;
    }

    busy = true;

    // Polling leaves controls enabled to avoid flickering.
    if (action !== null) {
      modeButton.disabled = true;
      measureSelect.disabled = true;
    }

    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 5000);

    try {
      let url = "/api/state";
      const options = {
        cache: "no-store",
        signal: controller.signal
      };

      if (action !== null) {
        url = action.url;
        options.method = "POST";
        options.headers = {
          "Content-Type": "application/x-www-form-urlencoded"
        };

        options.body = action.body;
      }

      const response = await fetch(url, options);

      if (!response.ok) {
        throw new Error("Request failed");
      }

      render(await response.json());

      connection.textContent = "Connected";
      modeButton.disabled = false;
      measureSelect.disabled = false;
    } catch (error) {
      connection.textContent =
        "Could not confirm device state. Checking again…";
      modeButton.disabled = true;
      measureSelect.disabled = true;
      pendingActions.length = 0;
    } finally {
      clearTimeout(timeout);
      busy = false;
      if (pendingActions.length > 0) {
        refresh(pendingActions.shift());
      }
    }
  }

  modeButton.addEventListener("click", () => {
    refresh({
      url: "/api/mode",
      body: `enabled=${learning ? 0 : 1}`
    });
  });

  measureSelect.addEventListener("change", () => {
    refresh({
      url: "/api/measure",
      body: `beats=${encodeURIComponent(measureSelect.value)}`
    });
  });

  refresh();
  setInterval(() => refresh(), 1000);
</script>
</body>
</html>
)HTML";
