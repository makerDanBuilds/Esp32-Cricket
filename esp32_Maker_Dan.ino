#include <WiFi.h>
#include <WebServer.h>

#define BUZZER_PIN 4

// SoftAP credentials
const char* ssid = "Maker Dan ESP32 Cricket";
const char* password = "annoying";

WebServer server(80);

// Sound modes
enum SoundMode { OFF, CRICKET, NORMAL, BEEP };
SoundMode currentMode = OFF;

// Timing variables
unsigned long lastSoundTime = 0;
bool soundActive = false;
unsigned int cricketInterval = 2000; // Default 2 seconds
unsigned int beepInterval = 500;     // Default 0.5 seconds

void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Cricket</title>

  <style>
    body { font-family: Arial, sans-serif; max-width: 400px; margin: 0 auto; padding: 20px; }
    h1 { text-align: center; color: #333; }
    .switch { position: relative; display: inline-block; width: 60px; height: 34px; margin: 10px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }
    .slider:before { position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }
    input:checked + .slider { background-color: #2196F3; }
    input:checked + .slider:before { transform: translateX(26px); }
    .switch-container { display: flex; align-items: center; justify-content: space-between; margin: 15px 0; }
    .label { margin-right: 10px; width: 100px; }
    .interval-control { margin-top: 10px; display: none; }
    .interval-btn { margin: 5px; padding: 5px 10px; border-radius: 5px; border: 1px solid #ddd; }
    .interval-btn.active { background-color: #2196F3; color: white; }
  </style>
</head>
<body>
  <h1>ESP32 Cricket</h1>
  <div class="switch-container">
    <span class="label">Cricket Sound</span>
    <label class="switch">
      <input type="checkbox" id="cricket" onclick="toggleSound(1, this)">
      <span class="slider"></span>
    </label>
  </div>
  <div id="cricket-interval" class="interval-control">
    <span>Chirp Interval:</span>
    <button class="interval-btn" onclick="setInterval(1, 500)">0.5s</button>
    <button class="interval-btn" onclick="setInterval(1, 1000)">1s</button>
    <button class="interval-btn" onclick="setInterval(1, 2000)">2s</button>
    <button class="interval-btn" onclick="setInterval(1, 3000)">3s</button>
  </div>
  
  <div class="switch-container">
    <span class="label">Normal Buzzer</span>
    <label class="switch">
      <input type="checkbox" id="normal" onclick="toggleSound(2, this)">
      <span class="slider"></span>
    </label>
  </div>
  
  <div class="switch-container">
    <span class="label">Beeping Sound</span>
    <label class="switch">
      <input type="checkbox" id="beep" onclick="toggleSound(3, this)">
      <span class="slider"></span>
    </label>
  </div>
  <div id="beep-interval" class="interval-control">
    <span>Beep Interval:</span>
    <button class="interval-btn" onclick="setInterval(3, 500)">0.5s</button>
    <button class="interval-btn" onclick="setInterval(3, 1000)">1s</button>
    <button class="interval-btn" onclick="setInterval(3, 2000)">2s</button>
    <button class="interval-btn" onclick="setInterval(3, 3000)">3s</button>
  </div>

  <script>
    function toggleSound(mode, element) {
      // If this switch is being turned off, just send OFF command
      if(!element.checked) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/setmode?mode=0", true);
        xhr.send();
        hideAllIntervals();
        return;
      }
      
      // Uncheck other switches
      document.getElementById("cricket").checked = (mode == 1);
      document.getElementById("normal").checked = (mode == 2);
      document.getElementById("beep").checked = (mode == 3);
      
      // Show/hide interval controls
      hideAllIntervals();
      if(mode == 1) document.getElementById("cricket-interval").style.display = "block";
      if(mode == 3) document.getElementById("beep-interval").style.display = "block";
      
      // Send request to ESP32
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setmode?mode=" + mode, true);
      xhr.send();
    }
    
    function setInterval(mode, interval) {
      // Highlight active button
      var container = (mode == 1) ? "cricket-interval" : "beep-interval";
      var buttons = document.getElementById(container).getElementsByClassName("interval-btn");
      for(var i = 0; i < buttons.length; i++) {
        buttons[i].classList.remove("active");
        if(parseInt(buttons[i].getAttribute("onclick").split(",")[1].trim()) == interval) {
          buttons[i].classList.add("active");
        }
      }
      
      // Send interval to ESP32
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setinterval?mode=" + mode + "&interval=" + interval, true);
      xhr.send();
    }
    
    function hideAllIntervals() {
      document.getElementById("cricket-interval").style.display = "none";
      document.getElementById("beep-interval").style.display = "none";
    }
    
    // Update switch states on page load
    window.onload = function() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/getmode", true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          var mode = parseInt(xhr.responseText);
          document.getElementById("cricket").checked = (mode == 1);
          document.getElementById("normal").checked = (mode == 2);
          document.getElementById("beep").checked = (mode == 3);
          
          if(mode == 1) document.getElementById("cricket-interval").style.display = "block";
          if(mode == 3) document.getElementById("beep-interval").style.display = "block";
        }
      };
      xhr.send();
      
      // Get current intervals
      getCurrentInterval(1); // Cricket
      getCurrentInterval(3); // Beep
    };
    
    function getCurrentInterval(mode) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/getinterval?mode=" + mode, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          var interval = parseInt(xhr.responseText);
          var container = (mode == 1) ? "cricket-interval" : "beep-interval";
          var buttons = document.getElementById(container).getElementsByClassName("interval-btn");
          for(var i = 0; i < buttons.length; i++) {
            buttons[i].classList.remove("active");
            if(parseInt(buttons[i].getAttribute("onclick").split(",")[1].trim()) == interval) {
              buttons[i].classList.add("active");
            }
          }
        }
      };
      xhr.send();
    }
  </script>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}

void handleSetMode() {
  if (server.hasArg("mode")) {
    int mode = server.arg("mode").toInt();
    switch(mode) {
      case 1: currentMode = CRICKET; break;
      case 2: currentMode = NORMAL; break;
      case 3: currentMode = BEEP; break;
      default: currentMode = OFF; break;
    }
    lastSoundTime = millis();
    soundActive = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
  server.send(200, "text/plain", "OK");
}

void handleSetInterval() {
  if (server.hasArg("mode") && server.hasArg("interval")) {
    int mode = server.arg("mode").toInt();
    unsigned int interval = server.arg("interval").toInt();
    
    if(mode == 1) cricketInterval = interval;
    else if(mode == 3) beepInterval = interval;
    
    lastSoundTime = millis(); // Reset timing
  }
  server.send(200, "text/plain", "OK");
}

void handleGetMode() {
  server.send(200, "text/plain", String((int)currentMode));
}

void handleGetInterval() {
  if (server.hasArg("mode")) {
    int mode = server.arg("mode").toInt();
    if(mode == 1) server.send(200, "text/plain", String(cricketInterval));
    else if(mode == 3) server.send(200, "text/plain", String(beepInterval));
    else server.send(200, "text/plain", "0");
  }
}

void playCricketSound() {
  if (!soundActive) {
    // Louder cricket chirp with more pulses
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 20; j++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(400);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(400);
      }
      delay(20);
    }
    soundActive = true;
  }
}

void playNormalSound() {
  if (!soundActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    soundActive = true;
  }
}

void playBeepSound() {
  if (millis() - lastSoundTime >= beepInterval) {
    digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN)); // Toggle buzzer state
    lastSoundTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Set up SoftAP
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Set up web server
  server.on("/", handleRoot);
  server.on("/setmode", handleSetMode);
  server.on("/setinterval", handleSetInterval);
  server.on("/getmode", handleGetMode);
  server.on("/getinterval", handleGetInterval);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  switch(currentMode) {
    case CRICKET:
      if (millis() - lastSoundTime >= cricketInterval) {
        soundActive = false;
        lastSoundTime = millis();
      }
      playCricketSound();
      break;
      
    case NORMAL:
      playNormalSound();
      break;
      
    case BEEP:
      playBeepSound();
      break;
      
    case OFF:
    default:
      digitalWrite(BUZZER_PIN, LOW);
      soundActive = false;
      break;
  }
  
  delay(10);
}