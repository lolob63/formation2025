/* Lycée La Fayette 
Pilotage de barre de led RGB
05/12/23
Commande json sur mqtt
topic: hello
{
    "name": "led",
    "status": "on",
    "colorRED": 125,
    "colorGREEN": 125,
    "colorBLUE": 125
  }
  Allumage d'une led
{
    "name": "led",
    "status": "off",
    "colorRED": 0,
    "colorGREEN": 0,
    "colorBLUE": 0
  }
https://www.arduino.cc/reference/en/libraries/arduino_json/
https://github.com/256dpi/arduino-mqtt
https://wiki.seeedstudio.com/Grove-RGB_LED_Stick-10-WS2813_Mini/

*/
#define BROKER_IP    "192.168.1.81"
#define DEV_NAME     "mqttdevice"
#define MQTT_USER    "lafayette"
#define MQTT_PW      "lafayette"
const char ssid[] = "ORBI50";
const char pass[] = "modernwater884";

#include <MQTT.h>

#ifdef ARDUINO_SAMD_MKRWIFI1010
#include <WiFiNINA.h>
#elif ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#else
#error unknown board
#endif

WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;

#include <Arduino_JSON.h>
#include <Adafruit_NeoPixel.h>

// pin où est connecté la chaine de led RGB
#define PIN        1 

String var;
int j=0;

// Nombre de LED
#define NUMPIXELS 10 
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 50 // Time (in milliseconds) to pause between pixels

byte colorRED,colorGREEN,colorBLUE;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("");
    delay(1000);
  }
  while (!client.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnecté au serveur MQTT!");
  client.subscribe("hello"); //SUBSCRIBE TO TOPIC hello
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Reception: " + topic + " - " + payload);
  JSONVar myObject = JSON.parse(payload);
  //Serial.println(JSON.typeof(myObject));
  if (myObject.hasOwnProperty("name")) {
    Serial.print("myObject[\"name\"] = ");
    Serial.println((const char*) myObject["name"]);
  }
  if (myObject.hasOwnProperty("status")) {
    Serial.print("myObject[\"status\"] = ");

    Serial.println((const char*) myObject["status"]);
    //   Serial.println(myObject["status"]);
  }
  if (myObject.hasOwnProperty("pin")) {
    Serial.print("myObject[\"pin\"] = ");
    Serial.println((int) myObject["pin"]);
  }
  if (myObject.hasOwnProperty("colorRED")) {
    Serial.print("myObject[\"colorRED\"] = ");
    colorRED =(byte) myObject["colorRED"];
    Serial.println((byte) myObject["colorRED"]);
  }
  if (myObject.hasOwnProperty("colorGREEN")) {
    Serial.print("myObject[\"colorGREEN\"] = ");
    colorGREEN =(byte) myObject["colorGREEN"];
    Serial.println((byte) myObject["colorGREEN"]);
  }

  if (myObject.hasOwnProperty("colorBLUE")) {
    Serial.print("myObject[\"colorBLUE\"] = ");
    colorBLUE =(byte) myObject["colorBLUE"];
    Serial.println((byte) myObject["colorBLUE"]);
  }
  if (topic == "hello") {
    if (String((const char*)myObject["status"]) == "on") {
      Serial.println("on");
      digitalWrite(LED_BUILTIN, HIGH);
    } 
    if (String((const char*)myObject["status"]) == "off") {
      Serial.println("off");
      digitalWrite(LED_BUILTIN, LOW);
    }
  } 
}

void ledRGB(){
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    pixels.setPixelColor(i, pixels.Color(colorRED,colorGREEN,colorBLUE));
    pixels.show();   // Envoie la valeur sur les led.
    delay(DELAYVAL); // Pause
  }
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();   // Envoie la valeur sur les led.
    delay(DELAYVAL); // Pause
  }
}

   
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // wait for serial port to connect. Needed for native USB port only
    // attempt to connect to WiFi network:
  }
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Connecté au wifi");
  pinMode(LED_BUILTIN, OUTPUT);
  pixels.begin(); // initialise l'objet NeoPixel
  pixels.clear(); // Eteint tous les pixels
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  //
  // MQTT brokers usually use port 8883 for secure connections.
  client.begin(BROKER_IP, 1883, net);
  client.onMessage(messageReceived);
  connect();
}
void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }
  ledRGB();
}

