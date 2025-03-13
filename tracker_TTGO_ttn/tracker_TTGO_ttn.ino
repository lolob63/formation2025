/*******************************************************************************
 * !!!!!!!!!!!!!!!!!!!!!!!!!prendre le compilateur esp32 v1.2 dans ide arduino!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!carte: Tbeam!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                            LB 01/12/2024
 * TTN V3 -Systeme mesure de position GPS 
 * IOT GPS ESP32 sur TTN
 *  Alimentation par batterie ou par usb 
 *  Transfert de la localisation des données de la carte vers le site ttn
 *  trame:{ "lat":45.767708,"lon":3.018599,"alt":704,"vit":0.1 }
 *  transfert toutes les 30 secondes
 *  https://www.thethingsnetwork.org/forum/t/big-esp32-sx127x-topic-part-3/18436
 *  For TTGO LoRa32 V2.1.6 and TTGO T-Beam versions V0.x, V1.0 and V1.1:
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/BluetoothSerial
 
 * https://github.com/mcci-catena/arduino-lmic
 * Compilation avec carte: ttgo Lora32-oled V1 
 * Attention Fréquence Lora
 * Compilateur ESP32 V2 V3
 * > #define hal_init LMICHAL_init
 *  dans le fichier \Arduino\libraries\MCCI_LoRaWAN_LMIC_library\project_config\lmic_project_config.h 
 * C:\Users\Laurent\Qsync\Donnees\cours\Lafayette\libraries\MCCI_LoRaWAN_LMIC_library\project_config

 https://github.com/Xinyuan-LilyGO
 *******************************************************************************/
#define LMIC_LORAWAN_SPEC_VERSION    LMIC_LORAWAN_SPEC_VERSION_1_0_2

#include <TinyGPS++.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include "key.h"
#include <axp20x.h>

AXP20X_Class axp;

#include "BluetoothSerial.h"

String device_name = "ESP32-GPS";

BluetoothSerial SerialBT;

static uint8_t mydata[] = "";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

//Configuration du module GPS
char dataGPS[100];
unsigned char trame[100];

TinyGPSPlus gps;
HardwareSerial gps1(1);

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
      } else {
        while (gps1.available() > 0){         
          if (gps.encode(gps1.read())){                   
            } 
        }
        displayInfo() ;
        // Prepare upstream data transmission at the next possible time.

        //Calcul de la longueur de la trame
        int j=0;
        for (int i=0; i<= sizeof(trame)-1;i++){
          if (trame[i] == '\n') j=i;
        }
        //LMIC_setTxData2(1, trame, sizeof(trame)-1, 0);
        LMIC_setTxData2(1, trame, j, 0);
        Serial.println(F("Packet queued"));
        
        // Prepare upstream data transmission at the next possible time.
        //LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        //Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void displayInfo(){

  float temp; float hum;
  // Affichage dans le terminal
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat());
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng());
  Serial.println("   ");
  // Construction de la trame qui sera transmisse
  sprintf(dataGPS,"{ \"name\":\"IOT1\",\"lat\":%f,\"lon\":%f,\"alt\":%f,\"vit\":%f } \n",gps.location.lat(),gps.location.lng(),gps.altitude.meters(), gps.speed.kmph());
  //sprintf(dataGPS,"{ \"name\":\"IOT1\",\"lat\":%f,\"lon\":%f} \n",gps.location.lat(),gps.location.lng());
  Serial.println(dataGPS);
  SerialBT.println(dataGPS);

  for (int i=0; i<= sizeof(dataGPS)-1;i++){
    trame[i]= byte(dataGPS[i]);
  }
  //Serial.print("température: ");
  //Serial.println(axp.getTSTemp());

}

void setup() {
  gps1.begin(9600, SERIAL_8N1, 34, 12);   //Port uart GPS 34-TX 12-RX
  Serial.begin(115200);                   //Port uart pc

  SerialBT.begin(device_name);  //Bluetooth device name

  Serial.println(F("####################################################################"));
  Serial.println(F("IOT capteur de température sur une carte TTGO t-beam avec module GPS"));
  Serial.println(F("Lycée La Fayette Clermont Ferrand ")); 
  Serial.println(F("LB 2025"));
  Serial.println(F("####################################################################"));
  Serial.println();
  Wire.begin(21, 22);
    if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
      Serial.println("AXP192 OK");
      } 
      else {
      Serial.println("AXP192 ERREUR");
      }
 
    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);

    axp.setChgLEDMode(AXP20X_LED_OFF); // LED off
  #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);

    delay(1000);
  #endif
    // LMIC init
    os_init();
    
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
