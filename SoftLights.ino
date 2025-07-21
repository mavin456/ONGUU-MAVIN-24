
// SoftLights.ino
// Smart Lighting Control Dashboard for ESP32
//Group: MECH GIANTS GROUP 7

                                                                                                                    
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "MECH";
const char* password = "12345678";

// LED pin definitions
const int LED1_PIN = 5;  // Living Room LED
const int LED2_PIN = 18;  // Kitchen LED
const int LED3_PIN = 19;  // Bedroom LED

// LED states
bool led1_state = false;
bool led2_state = false;
bool led3_state = false;

// Web server on port 80
WebServer server(80);

// HTML for the dashboard
const char* dashboard_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Soft Lights - Smart Lighting Dashboard</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
        }
        .container { max-width: 1200px; margin: auto; padding: 20px; }
        .header, .dashboard, .footer {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 30px;
            margin-bottom: 30px;
            text-align: center;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
        }
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            background: linear-gradient(45deg, #3498db, #9b59b6);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .lights-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        .light-card {
            background: linear-gradient(135deg, #ff7675, #d63031);
            padding: 30px;
            border-radius: 20px;
            color: white;
        }
        .light-card.on {
            background: linear-gradient(135deg, #00b894, #00a085);
        }
        .light-status {
            font-size: 1.2em;
            margin: 10px 0;
        }
        .status-indicator {
            display: inline-block;
            width: 15px;
            height: 15px;
            border-radius: 50%;
            margin-right: 10px;
            background: #ff4757;
            animation: pulse 2s infinite;
        }
        .status-indicator.on {
            background: #2ed573;
        }
        .light-button, .refresh-button {
            background: white;
            color: #333;
            padding: 10px 20px;
            border-radius: 20px;
            border: none;
            cursor: pointer;
            margin-top: 10px;
        }
        .light-button:hover, .refresh-button:hover {
            transform: scale(1.05);
        }
        .team-info {
            font-size: 0.9em;
            opacity: 0.8;
        }
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Soft Lights</h1>
            <p>Smart Lighting Control Dashboard</p>
        </div>
        
        <div class="dashboard">
            <h2>Lighting Control Center</h2>
            <button class="refresh-button" onclick="refreshStatus()">Refresh Status</button>
            
            <div class="lights-grid">
                <div class="light-card" id="light1">
                    <h3>Living Room</h3>
                    <div class="light-status">
                        <span class="status-indicator" id="indicator1"></span>
                        <span id="status1">OFF</span>
                    </div>
                    <button class="light-button" onclick="toggleLight(1)">Toggle Light</button>
                </div>
                <div class="light-card" id="light2">
                    <h3>Kitchen</h3>
                    <div class="light-status">
                        <span class="status-indicator" id="indicator2"></span>
                        <span id="status2">OFF</span>
                    </div>
                    <button class="light-button" onclick="toggleLight(2)">Toggle Light</button>
                </div>
                <div class="light-card" id="light3">
                    <h3>Bedroom</h3>
                    <div class="light-status">
                        <span class="status-indicator" id="indicator3"></span>
                        <span id="status3">OFF</span>
                    </div>
                    <button class="light-button" onclick="toggleLight(3)">Toggle Light</button>
                </div>
            </div>
        </div>
        
        <div class="footer">
            <h3>Soft Lights</h3>
            <p>Intelligent Lighting Solutions</p>
            <div class="team-info">
                Team: MECH GIANTS
            </div>
        </div>
    </div>

    <script>
        function toggleLight(lightNum) {
            fetch('/toggle?light=' + lightNum)
                .then(response => response.json())
                .then(data => {
                    updateLightStatus(lightNum, data.state);
                });
        }

        function updateLightStatus(lightNum, state) {
            const card = document.getElementById('light' + lightNum);
            const indicator = document.getElementById('indicator' + lightNum);
            const status = document.getElementById('status' + lightNum);

            if (state) {
                card.classList.add('on');
                indicator.classList.add('on');
                status.textContent = 'ON';
            } else {
                card.classList.remove('on');
                indicator.classList.remove('on');
                status.textContent = 'OFF';
            }
        }

        function refreshStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    updateLightStatus(1, data.led1);
                    updateLightStatus(2, data.led2);
                    updateLightStatus(3, data.led3);
                });
        }

        setInterval(refreshStatus, 5000);
        refreshStatus();
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);

    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(LED3_PIN, OUTPUT);
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/toggle", handleToggle);
    server.on("/status", handleStatus);

    server.begin();
    Serial.println("Web server started!");
}

void loop() {
    server.handleClient();
}

void handleRoot() {
    server.send(200, "text/html", dashboard_html);
}

void handleToggle() {
    if (server.hasArg("light")) {
        int lightNum = server.arg("light").toInt();
        bool newState = false;

        switch (lightNum) {
            case 1:
                led1_state = !led1_state;
                digitalWrite(LED1_PIN, led1_state);
                newState = led1_state;
                break;
            case 2:
                led2_state = !led2_state;
                digitalWrite(LED2_PIN, led2_state);
                newState = led2_state;
                break;
            case 3:
                led3_state = !led3_state;
                digitalWrite(LED3_PIN, led3_state);
                newState = led3_state;
                break;
        }

        server.send(200, "application/json", "{\"state\":" + String(newState ? "true" : "false") + "}");
    } else {
        server.send(400, "text/plain", "Light parameter missing");
    }
}

void handleStatus() {
    String json = "{";
    json += "\"led1\":" + String(led1_state ? "true" : "false") + ",";
    json += "\"led2\":" + String(led2_state ? "true" : "false") + ",";
    json += "\"led3\":" + String(led3_state ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
}