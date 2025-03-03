# ESP32 MQTT Temperature and LED Control
## Author
Developed by Ivanperezp
## Overview
This project uses an ESP32 to connect to a WiFi network and communicate with an MQTT broker. The ESP32 reads temperature data from a DHT22 sensor and publishes it to an MQTT topic. Additionally, it listens for commands to control LED indicators on a TFT display.

## Components Used
- **ESP32**
- **DHT22 Temperature Sensor**
- **TFT Display (TFT_eSPI library required)**
- **MQTT Broker**

## Dependencies
Ensure you have the following libraries installed in your Arduino IDE:
- `WiFi.h` (for WiFi connectivity)
- `TFT_eSPI.h` (for TFT display control)
- `PubSubClient.h` (for MQTT communication)
- `DHT.h` (for DHT22 sensor readings)

## Circuit Connections
- **DHT22 Sensor:**
  - VCC → 3.3V
  - GND → GND
  - DATA → GPIO 4 (DHTPIN)

- **TFT Display:** (Connections may vary depending on the model)
  - Connect according to the TFT_eSPI library configuration.

## Configuration
Modify the following credentials before uploading the code:
```cpp
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";
const char* mqtt_server = "Your_MQTT_Broker_IP";
```

## Code Explanation
### WiFi and MQTT Setup
```cpp
void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
```
This function connects the ESP32 to a WiFi network.

### MQTT Message Handling
```cpp
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on ");
  Serial.print(topic);
  Serial.print(": ");

  // Convert the message to a String
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
```
This function processes received MQTT messages and updates the TFT display accordingly.

### Reconnection to MQTT Broker
```cpp
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32client")) {
      Serial.println("Connected!");
      client.subscribe("esp32/led1"); // Subscription for the green LED
      client.subscribe("esp32/sw");   // Subscription for the red LED
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(". Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
```
This function attempts to reconnect to the MQTT broker if the connection is lost.

### Setup Function
```cpp
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin(); // Initialize DHT22 sensor

  tft.init();
  tft.fillScreen(TFT_YELLOW);
  tft.setRotation(1);

  // Initially, the circles are off (gray)
  tft.fillCircle(120, 80, 30, TFT_DARKGREY); // Green LED
  tft.fillCircle(200, 80, 30, TFT_DARKGREY); // Red LED
}
```
This function initializes WiFi, MQTT, the DHT22 sensor, and the TFT display.

### Loop Function
```cpp
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Read temperature from the DHT22 sensor
    temp = dht.readTemperature();
    if (isnan(temp)) {
      Serial.println("Error reading the DHT22 sensor");
      return;
    }

    char tempstr[10];
    dtostrf(temp, 5, 2, tempstr);
    Serial.print("Publishing temperature: ");
    Serial.println(tempstr);
    
    client.publish("esp32/hola", tempstr);
  }
}
```
This function ensures that the ESP32 stays connected to the MQTT broker and publishes temperature readings every 2 seconds.

## MQTT Topics
- **Publish:**
  - `esp32/hola` → Sends temperature data
- **Subscribe:**
  - `esp32/led1` → Controls the green LED on the TFT display
  - `esp32/sw` → Controls the red LED on the TFT display

## How to Use
1. Upload the code to the ESP32 using the Arduino IDE.
2. Ensure that your MQTT broker is running.
3. The ESP32 will connect to WiFi and subscribe to the defined MQTT topics.
4. Send `true` or `false` messages to `esp32/led1` or `esp32/sw` to control the LED indicators on the TFT display.
5. The ESP32 will publish temperature data to `esp32/hola` every 2 seconds.

## Troubleshooting
- **ESP32 Not Connecting to WiFi:** Check SSID and password.
- **MQTT Connection Fails:** Ensure the broker is running and reachable.
- **No Temperature Data Published:** Verify DHT22 sensor connections.



