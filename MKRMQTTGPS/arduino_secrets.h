
#define SECRET_SSID "ORBI50"
#define SECRET_PASS "modernwater884"
#define MQTT_LOGIN "lafayette"
#define MQTT_PASS "lafayette"
/*
#define SECRET_SSID "wifilntab"
#define SECRET_PASS "@dm1nwifi"
#define MQTT_LOGIN ""
#define MQTT_PASS ""
*/
//const char broker[]    = "172.21.28.65";

const char broker[]    = "192.168.1.87";
int        port        = 1883;
const char willTopic[] = "GPS/will";
const char inTopic[]   = "GPS/led";          //topic in
const char outTopic[]  = "GPS/GPS";     //topic out