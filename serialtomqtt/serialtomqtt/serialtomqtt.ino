//https://icircuit.net/arduino-getting-started-mqtt-using-esp32/2138
// Carte ESP32S2 SAOALA
//  Serial0 débouche sur GPIO43 et 44
//  GPIO18 (RX1) et GPIO17 (TX1)

#define RXD1 14
#define TXD1 12
#include <WiFi.h>
#include <PubSubClient.h>

// Paramétre réseau
const char* ssid = "ORBI50";
const char* password = "modernwater884";

const char* mqtt_server = "192.168.1.82";
#define mqtt_port 1883
#define MQTT_USER "homeassistant"
#define MQTT_PASSWORD "ijun1Shai8aef6Iecho0techaegai9hah1iedoh0ongoTahng2eivie6wailahc4"
#define MQTT_SERIAL_PUBLISH_CH "Linky/serialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "Linky/serialdata/rx"

WiFiClient wifiClient;

PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connecté");
    Serial.println("Adresse IP: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attente de la connection MQTT...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connecté");
      //Once connected, publish an announcement...
      client.publish("Linky/ESP32/", "Compteur Linky");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
}

void setup() {
  //Serial0.begin(9600,SERIAL_7E1);
  Serial1.begin(9600,SERIAL_7E1,14,12);
  delay(1000);

  Serial.begin(9600);

  Serial1.setTimeout(500);// Set time out for 

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  reconnect();
}


void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

void loop() {
   client.loop();
   if (Serial1.available() > 0) {
     char bfr[501];
     memset(bfr,0, 501);
     Serial1.readBytesUntil( '\n',bfr,500);
     
     publishSerialData(bfr);
     Serial.print("ok \n");
   }
 }
