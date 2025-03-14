/*
Programme de transfert d'une coordonnées GPS en utilisant le 
protocole MQTT sur une liaison wifi
carte MKR1010 + carte shield grove+ 
https://docs.arduino.cc/hardware/mkr-wifi-1010/
https://docs.arduino.cc/hardware/mkr-connector-carrier/ 
https://wiki.seeedstudio.com/Grove-GPS/
Gestion de la batterie Lithium
la LED de la carte s'allume si la batterie est en dessous 70% charge
*/

#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
#endif

#include "arduino_secrets.h"
#include "BQ24195.h"
#include <TinyGPS++.h>
#include <string> 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// Pour se connecter avec SSL/TLS :
// 1) Remplacez WiFiClient par WiFiSSLClient.
// 2) Modifiez la valeur du port de 1883 à 8883.
// 3) Remplacez la valeur du courtier par un serveur avec un certificat racine SSL/TLS connu
// flashé dans le module WiFi.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

TinyGPSPlus gps;
char buffer[64] = "xx\n"; // buffer array for data receive over serial port
int count = 0;          // counter for buffer array
double lat, lon;

const long interval = 1000;
unsigned long previousMillis = 0;

//gestion de labatterie
float rawADC;           //valeur du convertisseur
float voltADC;          //ADC convertion en volte
float voltBat;          //ctension de la batterie
//Valeur des résistances sur le pont diviseur
int R1 =  330000;       // resistance antre la batterie et la SAMD pin PB09
int R2 = 1000000;       // resistance entre la SAMD pin PB09 et la masse
int max_Source_voltage; // tension de source supérieure pour la batterie
//définir la tension à laquelle la batterie est pleine/vide
float batteryFullVoltage = 4.2;   //limite de tension supérieure pour la batterie
float batteryEmptyVoltage = 3.5;  //limite de tension basse pour la batterie
float batteryCapacity = 4;            //Capacitée de la batterie an Ah

//******************************
// Fonction de décodage GPS
//******************************
void displayInfo()
{
  Serial.print(F("Coordonnées: ")); 
  if (gps.location.isValid())
  {
    lat = gps.location.lat();
    Serial.print(lat, 6);
    lon=gps.location.lng();
    Serial.print(F(","));
    Serial.print(lon, 6);
  }
  else
  {
    Serial.print(F("Valeurs non valides"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("Valeurs non valides"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("Valeurs non valides"));
  }

  Serial.println();
}
void setup() {
  // initialise  pin LED_BUILTIN en sortie.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialise le port série et attend que le port s'ouvre :
  Serial.begin(9600);
  Serial1.begin(9600);

  while (!Serial) {
      // attend que le port série se connecte. Nécessaire pour le port USB natif uniquement
      }
  while (!Serial1) {
      // attend que le port série se connecte. Nécessaire pour le port USB natif uniquement
      }
  // batterie
  analogReference(AR_DEFAULT);      // the upper value of ADC is set to 3.3V
  analogReadResolution(10);         // this will give us 4096 (2^12) levels on the ADC
  //configure BQ24195 PMIC
  PMIC.begin();                                               // start the PMIC I2C connection
  PMIC.enableBoostMode();                                     // boost battery output to 5V
  PMIC.setMinimumSystemVoltage(batteryEmptyVoltage);          // set the minimum battery output to 3.5V
  PMIC.setChargeVoltage(batteryFullVoltage);                  // set battery voltage at full charge
  PMIC.setChargeCurrent(batteryCapacity/2);                   // set battery current to C/2 in amps
  PMIC.enableCharge();                                        // enable charging of battery
  // The formula for calculating the output of a voltage divider is
  // Vout = (Vsource x R2)/(R1 + R2)
  // If we consider that 3.3V is the maximum that can be applied to Vout then the maximum source voltage is calculated as
  max_Source_voltage = (3.3 * (R1 + R2))/R2;
  //**************************************************************************///

  // tentative de connexion au réseau WiFi :
  Serial.print("Attente de connection au wifi WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // échec, réessayer
    Serial.print(".");
    delay(500);
  }

  Serial.println("Vous êtes connecté au réseau");
  Serial.println();

  // Vous pouvez fournir un ID client unique, s'il n'est pas défini, la bibliothèque utilise Arduino-millis()
  // Chaque client doit avoir un identifiant client unique
  // mqttClient.setId("clientId");

    // You can provide a username and password for authentication
    mqttClient.setUsernamePassword(MQTT_LOGIN, MQTT_PASS);

  // Par défaut, la bibliothèque se connecte avec le drapeau "clean session" défini,
  // vous pouvez désactiver ce comportement en utilisant
  // mqttClient.setCleanSession(faux);

   // définit un message will, utilisé par le broker lorsque la connexion est coupée de manière inattendue
   // vous devez connaître la taille du message au préalable, et il doit être défini avant de vous connecter
  String willPayload = "Connexion coupee!";
  bool willRetain = true;
  int willQos = 1;

  mqttClient.beginWill(willTopic, willPayload.length(), willRetain, willQos);
  mqttClient.print(willPayload);
  mqttClient.endWill();

  Serial.print("Attente de connection au broker MQTT: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection en erreur! Code erreur = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("Vous êtes connecté au broker MQTT!");
  Serial.println();

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);

  Serial.print("Souscription au topic: ");
  Serial.println(inTopic);
  Serial.println();

 // s'abonner à un sujet
 // le deuxième paramètre fixe la QoS de l'abonnement,
 // la bibliothèque prend en charge l'abonnement à QoS 0, 1 ou 2
  int subscribeQos = 1;

  mqttClient.subscribe(inTopic, subscribeQos);
 
  // les topics peuvent être désabonnés en utilisant :
  // mqttClient.unsubscribe(inTopic);

  Serial.print("Attente des messages sur le topic: ");
  Serial.println(inTopic);
  Serial.println();
}

void loop() {
  // appel poll() régulièrement pour permettre à la bibliothèque de recevoir des messages MQTT et
  // envoie des keep alives MQTT qui évite d'être déconnecté par le broker
  mqttClient.poll();

  unsigned long currentMillis = millis();
  String payload = "";
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
        displayInfo();
  if (currentMillis - previousMillis >= interval) {
    Serial.print("Transfert du message au topic: ");
    Serial.println(outTopic);
    payload += "lat=";
    payload += String(lat,6);
    payload += " lon=";
    payload += String(lon,6);
    Serial.println(payload);
    // envoie un message, l'interface d'impression peut être utilisée pour définir le contenu du message
    // dans ce cas, nous connaissons la taille à l'avance, de sorte que la charge utile du message peut être diffusée
    bool retained = false;
    int qos = 1;
    bool dup = false;

    mqttClient.beginMessage(outTopic, payload.length(), retained, qos, dup);
    mqttClient.print(payload);
    mqttClient.endMessage();

    }
  // enregistre la dernière fois qu'un message a été envoyé
  previousMillis = currentMillis;
  //batterie();
  count++;
  
}

void onMqttMessage(int messageSize) {

  // nous avons reçu un message, imprimez le sujet et le contenu
  Serial.print("Message reçu du topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', duplicate = ");
  Serial.print(mqttClient.messageDup() ? "true" : "false");
  Serial.print(", QoS = ");
  Serial.print(mqttClient.messageQoS());
  Serial.print(", retained = ");
  Serial.print(mqttClient.messageRetain() ? "true" : "false");
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" Octets:");

  // utilise l'interface Stream pour imprimer le contenu
  String valeur="";
  while (mqttClient.available()) {
    
    valeur += (char)mqttClient.read();
    
  }
  Serial.println(valeur);
 
  Serial.println();
}

//*********************************************************************************************
//Gestion de la batterie https://docs.arduino.cc/tutorials/mkr-wifi-1010/mkr-battery-app-note
//*********************************************************************************************
void batterie(){
  rawADC = analogRead(ADC_BATTERY);                     //la valeur obtenue directement à la broche d'entrée PB09
  voltADC = rawADC * (3.3/1023.0);                      //convertir la valeur ADC en tension lue sur la broche
  voltBat = voltADC * (max_Source_voltage/3.3);         //
  int new_batt = (voltBat - batteryEmptyVoltage) * (100) / (batteryFullVoltage - batteryEmptyVoltage);    //
  //report information over Serial
  Serial.print("L'ADC sur PB09 lit une valeur de ");
  Serial.println(rawADC);
  Serial.print(" ce qui équivaut à ");
  Serial.print(voltADC);
  Serial.print("V.");
  Serial.println("Cela signifie que la tension de la batterie est ");
  Serial.print(voltBat);
  Serial.print("V.");
  Serial.println("Ce qui équivaut à un niveau de charge de ");
  Serial.print(new_batt);
  Serial.println("%.");
  if (new_batt < 70) 
    digitalWrite(LED_BUILTIN, HIGH);
  else
   digitalWrite(LED_BUILTIN, LOW);;
}
