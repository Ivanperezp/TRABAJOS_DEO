#include <WiFi.h>
#include <TFT_eSPI.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configuración WiFi y MQTT
const char* ssid = "Lasso";
const char* password = "24825207";
const char* mqtt_server = "192.168.1.20";

WiFiClient espClient;
PubSubClient client(espClient);

// Configuración TFT
TFT_eSPI tft = TFT_eSPI();

// Configuración del sensor DHT22
#define DHTPIN 4        // Pin GPIO donde está conectado el DHT22
#define DHTTYPE DHT22   // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

float temp = 0;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup_wifi() {
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en ");
  Serial.print(topic);
  Serial.print(": ");

  // Convertir el mensaje a String
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Si el mensaje proviene de esp32/led1 (control del LED verde)
  if (String(topic) == "esp32/led1") {
    if (message == "true") {
      Serial.println("LED VERDE ENCENDIDO");
      tft.fillCircle(120, 80, 30, TFT_GREEN);
    } else if (message == "false") {
      Serial.println("LED VERDE APAGADO");
      tft.fillCircle(120, 80, 30, TFT_DARKGREY);
    }
  }

  // Si el mensaje proviene de esp32/sw (control del LED rojo)
  if (String(topic) == "esp32/sw") {
    if (message == "true") {
      Serial.println("SW ENCENDIDO - DIBUJANDO ROJO");
      tft.fillCircle(200, 80, 30, TFT_RED);
    } else if (message == "false") {
      Serial.println("SW APAGADO - DIBUJANDO GRIS");
      tft.fillCircle(200, 80, 30, TFT_DARKGREY);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar a MQTT...");
    if (client.connect("ESP32client")) {
      Serial.println("Conectado!");
      client.subscribe("esp32/led1"); // Suscripción al LED verde
      client.subscribe("esp32/sw");   // Suscripción al LED rojo
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(". Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin(); // Inicializar sensor DHT22

  tft.init();
  tft.fillScreen(TFT_YELLOW);
  tft.setRotation(1);

  // Inicialmente los círculos están apagados (gris)
  tft.fillCircle(120, 80, 30, TFT_DARKGREY); // LED verde
  tft.fillCircle(200, 80, 30, TFT_DARKGREY); // LED rojo
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Leer temperatura del DHT22
    temp = dht.readTemperature();
    if (isnan(temp)) {
      Serial.println("Error al leer el sensor DHT22");
      return;
    }

    char tempstr[10];
    dtostrf(temp, 5, 2, tempstr);
    Serial.print("Publicando temperatura: ");
    Serial.println(tempstr);
    
    client.publish("esp32/hola", tempstr);
  }
}
