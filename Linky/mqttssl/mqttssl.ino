//Mqtt ssl
//carte Mkr1010
//24/02/2025
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ESP_SSLClient.h>
// https://github.com/arduino-libraries/ArduinoMqttClient
#include <ArduinoMqttClient.h>
//Configuration wifi
#define WIFI_SSID "ORBI50"
#define WIFI_PASSWORD "modernwater884"

ESP_SSLClient ssl_client;
WiFiClient basic_client;
MqttClient mqttClient(ssl_client);

unsigned long lastMillis = 0;
int count = 0;
//Configuration Broker MQTT
const char broker[] = "ba9bddf75efe49e19063a5c0b7eb015d.s1.eu.hivemq.cloud";
int port = 8883;
#define MQTT_USER "btsciel"
#define MQTT_PASSWORD "BTSciel63!"
const char topic[] = "arduino/test";

const long interval = 3000;
unsigned long previousMillis = 0;
bool mqttReady = false;

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
        ; // Attente de la connxion soit active
      }
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connexion avec l'adresse IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // ignore server ssl certificate verification
  ssl_client.setInsecure();

  // Set the receive and transmit buffers size in bytes for memory allocation (512 to 16384).
  ssl_client.setBufferSizes(1024 /* rx */, 512 /* tx */);

  /** Call setDebugLevel(level) to set the debug
   * esp_ssl_debug_none = 0
   * esp_ssl_debug_error = 1
   * esp_ssl_debug_warn = 2
   * esp_ssl_debug_info = 3
   * esp_ssl_debug_dump = 4
   */
  ssl_client.setDebugLevel(1);

  // In case ESP32 WiFiClient, the session timeout should be set,
  // if the TCP session was kept alive because it was unable to detect the server disconnection.
  // Assign the basic client
  // Due to the basic_client pointer is assigned, to avoid dangling pointer, basic_client should be existed
  // as long as it was used by ssl_client for transportation.

  ssl_client.setClient(&basic_client);

  Serial.print("Tentative de connexion au broker MQTT sur ssl: ");
  Serial.println(broker);
  mqttClient.setUsernamePassword(MQTT_USER,MQTT_PASSWORD);
  if (!mqttClient.connect(broker, port)) {
    Serial.print("Erreur connexion MQTT! Code erreur = ");
    Serial.println(mqttClient.connectError());
    return;
  }

  Serial.println("Vous êtes connecté au broker MQTT!");
  Serial.println();

  Serial.print("Souscription au topic: ");
  Serial.println(topic);
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(topic);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Attente d'un message sur le topic: ");
  Serial.println(topic);
  Serial.println();
  mqttReady = true;
}

void loop()
{
  if (!mqttReady)
    return;
  int messageSize = mqttClient.parseMessage();
  if (messageSize){
    // we received a message, print out the topic and contents
    Serial.print("Message reçu sur le topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', longueur ");
    Serial.print(messageSize);
    Serial.println(" octets:");

    // use the Stream interface to print the contents
    while (mqttClient.available())
    {
      Serial.print((char)mqttClient.read());
    }
    Serial.println();
    Serial.println();
  }

  mqttClient.poll();

  if (millis() - lastMillis > interval)
  {
    lastMillis = millis();

    Serial.print("Envoie d'un message sur le topic: ");

    Serial.println(topic);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);

    mqttClient.print(count);

    mqttClient.endMessage();
    count++;
  }
}