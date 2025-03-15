/*
Accélérométre 3axes
Carte XIAO ESP32C6 + Shield 

API REST 
curl GET 'http://192.168.1.76/data'

https://wiki.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-embedded-battery-management-chip/
https://wiki.seeedstudio.com/Seeeduino-XIAO-Expansion-Board/
https://wiki.seeedstudio.com/Grove-3-Axis_Digital_Accelerometer-16g/
https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32s2/api-reference/system/freertos.html#

https://www.printables.com/model/281276-seeed-xiao-expansion-board-base-caver/files
*/

#include <U8x8lib.h>
#include <PCF8563.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <ADXL345.h>
//#include <FreeRTOS.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Time.h>

//ESP32Time rtc et NTP;
ESP32Time rtc(3600);      // offset in seconds GMT+1
const int decalage = 0;  // la valeur dépend de votre fuseau horaire.  

const char *SSID = "ORBI50";
const char *PWD = "modernwater884";

WebServer server(80);

StaticJsonDocument<250> jsonDocument;
char buffer[350];

PCF8563 pcf;

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);  // OLEDs without Reset of the Display

ADXL345 adxl;  //instancie l'objet adxl à partir de la librairie ADXL345 
double ax, ay, az;
int x, y, z;

void setup_routing() {     
  server.on("/aX", getaX);     
  server.on("/aY", getaY);     
  server.on("/aZ", getaZ); 
  server.on("/data", getData);    
  //server.on("/led", HTTP_POST, handlePost);    
          
  server.begin();    
}
 
void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["valeur"] = value;
  jsonDocument["unite"] = unit;
  serializeJson(jsonDocument, buffer);
}
 
void read_sensor_data(void * parameter) {
   for (;;) {
    //Boring accelerometer stuff
    
    adxl.readXYZ(&x, &y, &z);  //read the accelerometer values and store them in variables  x,y,z
    // Output x,y,z values
    Serial.print("values of X , Y , Z: ");
    Serial.print(x);
    Serial.print(" , ");
    Serial.print(y);
    Serial.print(" , ");
    Serial.println(z);

    double xyz[3];

    adxl.getAcceleration(xyz);
    ax = xyz[0];
    ay = xyz[1];
    az = xyz[2];
    Serial.print("X=");
    Serial.print(ax);
    Serial.println(" g");
    Serial.print("Y=");
    Serial.print(ay);
    Serial.println(" g");
    Serial.print("Z=");
    Serial.print(az);
    Serial.println(" g");
    Serial.println("**********************");
    String AX = "ax = " + String(ax*9.8);
    String AY = "ax = " + String(ay*9.8);
    String AZ = "ay = " + String(az*9.8);
    u8x8.setCursor(0, 3);
    u8x8.print(AX);
    u8x8.setCursor(0, 4);
    u8x8.print(AY);
    u8x8.setCursor(0, 5);
    u8x8.print(AZ);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
}
void afficheHeureDate() {
  Serial.print("Heure:   ");
  Serial.print(rtc.getTime() );  
  Serial.print("        Date:    ");
  Serial.print(rtc.getDate()); 
  Serial.println("");
  Serial.print(rtc.getLocalEpoch());
  Serial.println("");
}
void getaX() {
  //Serial.println("Lecture de la temperature");
  create_json("aX", ax, "g");
  server.send(200, "application/json", buffer);
}
void getaY() {
  //Serial.println("Lecture de la temperature");
  create_json("aY", ay, "g");
  server.send(200, "application/json", buffer);
}
void getaZ() {
  //Serial.println("Lecture de la temperature");
  create_json("aZ", az, "g");
  server.send(200, "application/json", buffer);
}
void getData() {
  afficheHeureDate();
  Serial.println("Lecture des valeurs du capteur ");
  jsonDocument.clear();
  add_json_object("aX", ax, "g");
  add_json_object("aY", ay, "g");
  add_json_object("aZ", az, "g");
  add_json_object("temps", rtc.getLocalEpoch(), "Epoch");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["valeur"] = value;
  obj["unite"] = unit; 
}
void setup_task() {    
  xTaskCreate(     
  read_sensor_data,      
  "Read sensor data",      
  2048,      
  NULL,      
  1,     
  NULL     
  );     
}

void setup() {
  Serial.begin(115200);

  Serial.print("Se connecte à la borne Wi-Fi");
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
 /*---------Mies à l'heure via NTP---------------*/
  configTime(decalage * 3600, 0, "fr.pool.ntp.org");  //
 // en Europe, essayez europe.pool.ntp.org ou fr.pool.ntp.org
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
    rtc.setTimeStruct(timeinfo); 
  }

  afficheHeureDate();
    
  Serial.print("Carte connectée! Adresse IP: ");
  Serial.println(WiFi.localIP());

  // ecran
  u8x8.begin();
  u8x8.setFlipMode(2);  // set number from 1 to 3, the screen word will rotary 180
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 0);
  u8x8.print("Lycee La Fayette");
  u8x8.setCursor(0, 1);
  u8x8.print(WiFi.localIP());
  //
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
  setup_task();    
  setup_routing(); 
}

void loop() {
   server.handleClient();
}
