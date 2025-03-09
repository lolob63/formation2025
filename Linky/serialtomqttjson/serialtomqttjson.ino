//https://icircuit.net/arduino-getting-started-mqtt-using-esp32/2138
// Carte ESP32S2 SAOALA
//  Serial0 débouche sur GPIO43 et 44
//  GPIO18 (RX1) et GPIO17 (TX1)

//  Compteur Linky sortie téléinfo standart
//  transfert format json

#include <WiFi.h>
#include <PubSubClient.h>

// Paramétre réseau

const char* ssid = "ORBI50";
const char* password = "modernwater884";
/*const char* ssid = "ciscoTSSN2";
const char* password = "btssn@dm1n";*/

const char* mqtt_server = "192.168.1.81";
#define mqtt_port 1883
#define MQTT_USER "lafayette"
#define MQTT_PASSWORD "lafayette"
#define MQTT_SERIAL_PUBLISH_CH "Linky/serialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "Linky/serialdata/rx"
String trame, trame1[10];
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
     
     publishSerialData(bfr);

     Serial.println(bfr);
     //Decoupage de la chaine de carcatéres */
	   char *token = strtok(bfr, "\t");
     trame ="";
	   int i=0;
	   while (token != NULL){
       trame1[i]=token;
       i++;
		   token = strtok(NULL, "\t");
	    }
     int imax=i;
     trame="";
	   trame += "{\"";
	   trame += trame1[0];
	   trame += "\": \"";
 	   for (i = 1 ; i <imax-1 ; i= i + 1) {
	     trame += trame1[i];
	     if (i!=imax-2) trame += " ";
	    }
     trame += "\"}";

     //string vers char
     for (int i = 0; i < trame.length(); i++) { 
       bfr[i] = trame[i]; 
      } 
      Serial.println(bfr);
    }
   
 }
