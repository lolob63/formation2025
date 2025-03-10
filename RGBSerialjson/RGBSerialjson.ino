// pilotage simple d'une barre de 10 led RGB
// envoyer la trame format json
/*
{
    "led": 1,
    "R": 255,
    "G": 255,
    "B": 0
}
*/

#include <Adafruit_NeoPixel.h>
#include <Arduino_JSON.h>

#define PIN        2 
#define NUMPIXELS 10 // Nombre de Led

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500 // Temps de pause entre 2 pixels

void setup() {

  pixels.begin(); // init 
  Serial.begin(9600);
  while (!Serial) {
        ; // Attente de la connxion soit active
      }
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();
}

byte R,G,B;

void loop() {
  uint16_t led;
  Serial.println("Saisir la commande au format json:");
  while (Serial.available() == 0) {}     // Attente de la reception d'une chaine

  String teststr = Serial.readString();  // Lecture de la chaine saisie
  teststr.trim();                        // supprime dans la chaine \r \n 
                                        //https://docs.arduino.cc/language-reference/en/variables/data-types/stringObject/Functions/trim/

  Serial.println(teststr);

  JSONVar myObject = JSON.parse(teststr);
    if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject)); // prints: object
if (myObject.hasOwnProperty("led")) {
    Serial.print("myObject[\"led\"] = ");
    Serial.println((byte) myObject["led"]);
    led=(int) myObject["led"];
  }
if (myObject.hasOwnProperty("R")) {
    Serial.print("myObject[\"R\"] = ");
    Serial.println((byte) myObject["R"]);
    R=(byte) myObject["R"];
  }
if (myObject.hasOwnProperty("G")) {
    Serial.print("myObject[\"G\"] = ");
    Serial.println((byte)myObject["G"]);
    G=(byte) myObject["G"];
  }
  if (myObject.hasOwnProperty("B")) {
    Serial.print("myObject[\"B\"] = ");
    Serial.println((byte)myObject["B"]);
    B=(byte) myObject["B"];
  }

  pixels.setPixelColor(led, pixels.Color(R, G, B));
 
  pixels.show();   // Send the updated pixel colors to the hardware.*/
}
