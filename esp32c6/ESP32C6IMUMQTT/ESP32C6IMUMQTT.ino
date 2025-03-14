/*
Carte xiao esp32c6 + IMU
transfert d'une trame JSON des valeurs des accélérations en MQTT
{"ax":7.93,"ay":0.15,"az":3.73,"unit":"m.s-2"}
Choix de l'unité
Topic IMU/unit valeur en RAW g ou m.s-2

https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/
https://wiki.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-embedded-battery-management-chip/
https://wiki.seeedstudio.com/Seeeduino-XIAO-Expansion-Board/
https://wiki.seeedstudio.com/Grove-3-Axis_Digital_Accelerometer-16g/

*/
#include <ADXL345.h>
#include <WiFi.h>
#include <MQTT.h>



#define BROKER_IP    "192.168.1.87"
#define DEV_NAME     "mqttdevice"
#define MQTT_USER    "lafayette"
#define MQTT_PW      "lafayette"
#define TOPIC          "IMU"
#define TOPICrecep    "IMU/unit"
ADXL345 adxl;  //variable adxl is an instance of the ADXL345 library

double ax, ay, az;
String trameJSON;
String unit="m.s-2";


const char ssid[] = "ORBI50";
const char pass[] = "modernwater884";
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void read_sensor_data() {
    // Lecture du capteur
    double xyz[3];
    String AX,AY,AZ;
    adxl.getAcceleration(xyz);
    ax = xyz[0];
    ay = xyz[1];
    az = xyz[2];
    //Construction de la trame au format JSON
    if (unit == "g"){
      AX = String(ax);
      AY = String(ay);
      AZ = String(az);
    }
    if (unit =="m.s-2"){
      AX = String(ax*9.8);
      AY = String(ay*9.8);
      AZ = String(az*9.8);
    }

    trameJSON = "{\"ax\":";
    trameJSON += AX;
    trameJSON += ",";
    trameJSON += "\"ay\":";
    trameJSON += AY;
    trameJSON += ",";
    trameJSON += "\"az\":";
    trameJSON += AZ;
    trameJSON += ",";
    trameJSON += "\"unit\":\"";
    trameJSON += unit;
    trameJSON += "\"}";
    //Serial.print(trameJSON);
    //Serial.println();

}
void connect() {
  Serial.print("Test du  wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nWifi Connecté...");
  Serial.println("Adresse IP: ");
  Serial.println(WiFi.localIP());
  while (!client.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nBroker MQTT connecté!");

  client.subscribe(TOPICrecep);
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("reception: " + topic + " - " + payload);

  
  if(payload =="m.s-2") 
    unit="m.s-2";
  if(payload == "g"){
    unit="g";
    }

    


  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);

 
  WiFi.begin(ssid, pass);

 
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin(BROKER_IP,1883, net);
  client.onMessage(messageReceived);
  adxl.powerOn();
  /*
  //set activity/ inactivity thresholds (0-255)
  adxl.setActivityThreshold(75);    //62.5mg per increment
  adxl.setInactivityThreshold(75);  //62.5mg per increment
  adxl.setTimeInactivity(10);       // how many seconds of no activity is inactive?

  //look of activity movement on this axes - 1 == on; 0 == off
  adxl.setActivityX(1);
  adxl.setActivityY(1);
  adxl.setActivityZ(1);

  //look of inactivity movement on this axes - 1 == on; 0 == off
  adxl.setInactivityX(1);
  adxl.setInactivityY(1);
  adxl.setInactivityZ(1);

  //look of tap movement on this axes - 1 == on; 0 == off
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(0);
  adxl.setTapDetectionOnZ(1);

  //set values for what is a tap, and what is a double tap (0-255)
  adxl.setTapThreshold(50);      //62.5mg per increment
  adxl.setTapDuration(15);       //625us per increment
  adxl.setDoubleTapLatency(80);  //1.25ms per increment
  adxl.setDoubleTapWindow(200);  //1.25ms per increment

  //set values for what is considered freefall (0-255)
  adxl.setFreeFallThreshold(7);  //(5 - 9) recommended - 62.5mg per increment
  adxl.setFreeFallDuration(45);  //(20 - 70) recommended - 5ms per increment

  //setting all interrupts to take place on int pin 1
  //I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping(ADXL345_INT_SINGLE_TAP_BIT, ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_DOUBLE_TAP_BIT, ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_FREE_FALL_BIT, ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_ACTIVITY_BIT, ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_INACTIVITY_BIT, ADXL345_INT1_PIN);

  //register interrupt actions - 1 == on; 0 == off
  adxl.setInterrupt(ADXL345_INT_SINGLE_TAP_BIT, 1);
  adxl.setInterrupt(ADXL345_INT_DOUBLE_TAP_BIT, 1);
  adxl.setInterrupt(ADXL345_INT_FREE_FALL_BIT, 1);
  adxl.setInterrupt(ADXL345_INT_ACTIVITY_BIT, 1);
  adxl.setInterrupt(ADXL345_INT_INACTIVITY_BIT, 1);
  */
  connect();
}

void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    read_sensor_data();
    //Serial.println("transfert MQTT");
    client.publish(TOPIC,trameJSON);
    
  }
}
