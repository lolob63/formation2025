//programme permettant de simuler le compteur 
//transfety sur l'UART1
//ESP32S2 SAOLA 1M

String trame="SMAXSN1	H250207002004	00120	W";
HardwareSerial SerialLinky(1);
void setup() {
  // put your setup code here, to run once:
   SerialLinky.begin(9600,SERIAL_7E1,18,17);
   Serial.begin(9600);
   while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(trame);
  SerialLinky.println(trame);
  delay(1000);
}
