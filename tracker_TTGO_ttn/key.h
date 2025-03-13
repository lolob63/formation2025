
//carte eui-a8610a3434468119
//Carte tracker GPS

#ifndef _KEY_H
#define _KEY_H
// This EUI must be in little-endian format, so least-significant-byte
// first.    ###### JoinEUI de TTN ###############
static const u1_t PROGMEM APPEUI[8]={ 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
//           ###### DevEUI de TTN ###############
static const u1_t PROGMEM DEVEUI[8]={ 0x19, 0x81, 0x46, 0x34, 0x34, 0x0A, 0x61, 0xA8};
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). 
static const u1_t PROGMEM APPKEY[16] ={ 0x3B, 0xC0, 0x85, 0x32, 0x28, 0xAB, 0xCF, 0x0B, 0x6F, 0xBA, 0x56, 0xE7, 0xCE, 0x9E, 0xC6, 0xA9 };


void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

// Pin mapping pour la carte TTGO GPS LORA

const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 23,
    .dio = {26, 33, 32},
};

#endif
