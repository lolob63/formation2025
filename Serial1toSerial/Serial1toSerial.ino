/*
Serial 1 to Serial
test sur carte arduino Mkr du module GPS

https://docs.arduino.cc/hardware/mkr-wifi-1010/
https://docs.arduino.cc/hardware/mkr-connector-carrier/ 
https://wiki.seeedstudio.com/Grove-GPS/

Analyse des données sur la liaison uart du GPS
trame NMEA 

*/


void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte);
  }
}
