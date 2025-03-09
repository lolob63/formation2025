//https://icircuit.net/arduino-getting-started-mqtt-using-esp32/2138
// Carte ESP32S2 SAOALA
//  Serial0 débouche sur GPIO43 et 44
//  GPIO18 (RX1) et GPIO17 (TX1)


#include <WiFi.h>
#include <PubSubClient.h>

// Paramétre réseau

const char* ssid = "ORBI50";
const char* password = "modernwater884";
ialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "Linky/serialdata/rx"
//String trame, trame1[10];
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
HardwareSerial SerialLinky(1);
void setup() {

  SerialLinky.begin(9600,SERIAL_7E1,18,17);
  delay(1000);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

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
   if (SerialLinky.available() > 0) {
     char bfr[501];
     memset(bfr,0, 501);
     SerialLinky.readBytesUntil( '\n',bfr,500);
     checksum(bfr);
     publishSerialData(bfr);

     Serial.println(bfr);
   }
 }

 char checksum(String trame2){
  char sum;
  sum=0;
  for (byte i=0;i<(trame2.length()-2);i++){
            sum = trame2[i] +sum;
  }  
  sum = (sum & 0x3F) + 0x20;
  Serial.println("Calcul du checksum pour vérifier la valeur de trame reçue.");
  Serial.print("Checksum = ");Serial.println(sum);
  Serial.println("");
  return sum;
 }
