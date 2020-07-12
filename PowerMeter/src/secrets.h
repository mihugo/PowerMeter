  #ifndef _SECRETS_H
#define _SECRETS_H

#define NUMAPS 4
static char const *APs[NUMAPS][2] = {
  {"AAAAAA","asdfljkasdfqweui34u"},
  {"BBBBB","açsdlkfj29338fnree2"},
  {"CCCCCCC","dfewererr"},
  {"ZZZZZZZ","herer3ra"}
};

// For connecting to the MQTT Broker
#define MQTT_Server    "192.168.1.17"
#define MQTT_Port      1883
#define MQTT_ClientID  "me"
// #define MQTT_TelemetryTopic_Root "iot/device/""
#define MQTT_TelemetryTopic_Root "v1/devices/"
//#define MQTT_UserID    "ESP8266_PowerMeter"
//#define MQTT_ClientID  "ESP8266_PowerMeter"
#define MQTT_ClientID  "ESP8266_PowerMeter"
#define MQTT_UserID    "ESP8266_PowerMeter"
#define MQTT_Password  "password1"
#define MQTT_Password  ""

// For the UDP Log Server
#define UDPLOG_Server  "192.168.1.251"     // My local PC, but should be a server"
#define UDPLOG_Port    514

#define HOSTNAME "PumpPowerMeter"
//#define StartupMsg "Hello      \nWorld      \n           \n           \n           \n           "
//#define OTAPassword "donttellanyone"

#endif
