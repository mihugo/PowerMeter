#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MQTT.h>               // For MQTT support
#include <ArduinoOTA.h>         // For OTA firmware update support
#include <ESP8266mDNS.h>
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
//#include <PZEM004T.h>
#include <PZEM004Tv30.h>
#include <SimpleTimer.h>
#include <WebServer.hpp>
#include <LogClient.hpp>
#include <TimeProvider.hpp>
#include <AppData.hpp>


#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C

#include <JC_Button.h>

#include "secrets.h"


#define  FW_Version "1.0.1"

//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>

SSD1306AsciiWire display;
TickerState displayState5;
TickerState displayState4;
TickerState displayState2;
TickerState displayState3;

char* displayTickerText5[2];
char* displayTickerText4[2];
char* displayTickerText2[2];
char* displayTickerText3[2];
int displayTickerTextI5 = 0;
int displayTickerTextI4 = 0;
int displayTickerTextI2 = 0;
int displayTickerTextI3 = 0;
uint32_t displayTickTime = 0;
#define RTN_CHECK 0

unsigned long   LCDRefreshRate = 500; 

// PZEM004T Configuration.
// Change the pins if using something other than the Wemos D1 mini and D5/D6 for UART communication.
#define         PZEM_TIMEOUT  3500
PZEM004Tv30        pzem( D5, D0);  // RX,TX
#define pzemAddress       0x42

unsigned long   pzemDataOK  = 0;
unsigned long   pzemDataNOK = 0;

int             mState = 0;                       // Current state for the state machine controlling the connection to the PZEM
                                                  // 0 - Disconnected
                                                  // 1 - Connecting
                                                  // 2 - Reading data loop
                                                  // 3 - Waiting for next read 
int unsigned mRetries = 0;                        // number of failures without sucessful read


const unsigned long   PZEM_SLEEP_TIME    = 60 * 1000;        // Sleep time between reads of PZEM004T values 
const unsigned long   PZEM_RETRY_TIME    = 250;              // Sleep time after read failure

int             MONITOR_LED   = 2;                // Monitoring led to send some visual indication to the user.
//unsigned long   tick;                             // Used for blinking the MONITOR_LED: ON -> OTA , Blink FAST: Connecting, Blink SLOW: Working
unsigned long   ledBlink = 200;                   // Led blink interval: Fast - Connecting to PZEM, slow - Connected
int             monitor_led_state = LOW; 


#define PUMP_ON D8
#define PUMP_DISABLED D6
int pumpState = 0;    //pump state 0 = normal; -1=forced off; 1 = on
time_t pumpOffTime = 0;  //time pump should be turned off ; 
time_t RelaySendTelemetryTime = 0;   // Next time pump telemetry data should be sent
#define RELAYSENDTELEMETRYNORMAL 60 * 58;  //send pump status once an hour if not active  
#define RELAYSENDTELEMETRYACTIVE 58; //send pump status once a minute if active

const byte
  LEFT_BUTTON(D7),
  RIGHT_BUTTON(D3);

Button leftBtn(LEFT_BUTTON, 25, false, true);
Button rightBtn(RIGHT_BUTTON, 25, false, true);
int leftBtnState = 0;
int rightBtnState = 0;

WiFiClient      WIFIClient;
IPAddress       thisDevice;
String          Wifi_ssid;
char            hostname[32];

SimpleTimer     timer;

MQTTClient      MQTT_client(512);
char            MQTT_AttributesTopic[256];
char            MQTT_TelemetryTopic[256];
char            SensorAttributes[512];
//char            SensorTelemetry[512];

//unsigned long   previousMillis = 0;
unsigned long   pingMqtt = 60 * 60 * 1000;  // Ping the MQTT broker every hour by sending the IOT Atributes message.
unsigned long   previousPing = 0;

unsigned int syslogEnabled = 0;

void sendPumpStatus();

void setupLCD() {

  displayTickerText5[0] = (char*)malloc(100*sizeof(char));
  displayTickerText5[1] = (char*)malloc(100*sizeof(char));
  sprintf(displayTickerText5[0], "blank");
  sprintf(displayTickerText5[1], "empty");


  displayTickerText4[0] = (char*)malloc(100*sizeof(char));
  displayTickerText4[1] = (char*)malloc(100*sizeof(char));
  sprintf(displayTickerText4[0], "blank");
  sprintf(displayTickerText4[1], "empty");

  displayTickerText2[0] = (char*)malloc(100*sizeof(char));
  displayTickerText2[1] = (char*)malloc(100*sizeof(char));
  sprintf(displayTickerText2[0], "blank");
  sprintf(displayTickerText2[1], "empty");

  displayTickerText3[0] = (char*)malloc(100*sizeof(char));
  displayTickerText3[1] = (char*)malloc(100*sizeof(char));
  sprintf(displayTickerText3[0], "blank");
  sprintf(displayTickerText3[1], "empty");

  Wire.begin();
  Wire.setClock(400000L);
  display.begin(&MicroOLED64x48, I2C_ADDRESS);
  display.setFont(lcd5x7);
   #if INCLUDE_SCROLLING == 0
  #error INCLUDE_SCROLLING must be non-zero.  Edit SSD1306Ascii.h
  #endif //  INCLUDE_SCROLLING

  display.setScrollMode(SCROLL_MODE_AUTO);
  display.clear();
  display.set1X();
    
}


void updateLCD(int screen=-1, const char miscText[]="") {
//Handles output to LCD screen
//miscText is a bit of a hack for passing in variables that are not globals
static int prevScreen = -1;
time_t delta;  // delta time since pump on or off
//static uint32_t ScreenChangeTime;   //time that screen should automatically be changed back to "default"


//if screen was not passed in then redraw the previous screen
if (screen==-1) screen=prevScreen;
if (screen==INT_MAX) 
  {
    screen=prevScreen+1;
    if (screen > 3) 
      screen = 0;
  }
if (screen==INT_MIN) 
  { 
    screen=prevScreen-1;
    if (screen < 0 )
      screen = 3;
  }

 switch (screen)
    {
        case 0:   // Initial screen
          displayTickTime = ULONG_MAX;  //disable scroll
          display.setCursor(0,0);
          display.print(StartUPMsg);
        break;

        case 1: //Status screen

          
          if (WiFi.status() ==  WL_CONNECTED) {
            thisDevice = WiFi.localIP();
            const char * Days [] ={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
            
            //this is barbaric as there must be a simpler way to format these strings with leading zeros 
            char m[3];
            snprintf(m, 3, "%02i", minute() );
            char s[3];
            snprintf(s, 3, "%02i", second() );
            String datetime = String( Days[weekday()-1]) + ", " + month() +"/"+ day() + "/" + year() + " " + hour() + ":" + m + ":" + s;


            if (prevScreen != 1 ) 
            {
              display.clear();
              display.println(WiFi.SSID());

              //Wifi info on line 2 
              //display.println(thisDevice.toString());  
              display.tickerInit(&displayState2, System5x7, 1, false);
              displayTickerTextI2++;
              snprintf(displayTickerText2[displayTickerTextI2%2], 100,  "%s %s", thisDevice.toString().c_str(), String(WiFi.RSSI()).c_str() );
              display.tickerText(&displayState2, displayTickerText2[displayTickerTextI2%2]);
              int i;
              for (i=64; i>0; i--)
              {
                display.tickerTick(&displayState2);
              }

              //Scrolling Time on line 3
              displayTickerTextI3++;
              display.tickerInit(&displayState3, System5x7, 2, false);
              snprintf(displayTickerText3[displayTickerTextI3%2], 100,  "%s", datetime.c_str() );   // probalby a much better way  to do this
              display.tickerText(&displayState3, displayTickerText3[displayTickerTextI3%2]);
              //int i;
              for (i=64; i>0; i--)   //just some silly style look by moving it quickly at first....
              {
                display.tickerTick(&displayState3);
              }

              display.setCursor(0,3);
              if (MQTT_client.connected() == true) display.println("MQTT: OK   ");
              else                                 display.println("MQTT: Init ");




              //PZEM status on line 5 
              displayTickerTextI4++;
              display.tickerInit(&displayState4, System5x7, 4, false);
              if (mRetries == 0)
              {
                sprintf(displayTickerText4[displayTickerTextI4%2], "PZEM004T: %s", appData.getPZEMState().c_str() );
              } else {
                sprintf(displayTickerText4[displayTickerTextI4%2], "PZEM004T: %s Errors: %i", appData.getPZEMState().c_str(), mRetries );
              }
              for (i=64; i>0; i--)   //just some silly style look by moving it quickly at first....
              {
                display.tickerTick(&displayState4);
              }


              //HostName line 6 
              displayTickerTextI5++;
              display.tickerInit(&displayState5, System5x7, 5, false);
              sprintf(displayTickerText5[displayTickerTextI5%2], "Hostname: %s", hostname );
              for (i=64; i>0; i--)   //just some silly style look by moving it quickly at first....
              {
                display.tickerTick(&displayState5);
              }
            

              if (displayTickTime == ULONG_MAX) displayTickTime = millis() + 30;  // re-enable scroll

          } else  //update same screen
            {
              display.setCursor(0,0);
              display.println(WiFi.SSID());

              
              displayTickerTextI2++;
              snprintf(displayTickerText2[displayTickerTextI2%2], 100,  "%s %s", thisDevice.toString().c_str(), String(WiFi.RSSI()).c_str() );

              displayTickerTextI3++;
              snprintf(displayTickerText3[displayTickerTextI3%2], 100,  "%s", datetime.c_str() );

              display.setCursor(0,3);
              if (MQTT_client.connected() == true) display.println("MQTT: OK   ");
              else                                 display.println("MQTT: Init ");
                     
              displayTickerTextI4++;
              if (mRetries == 0)
              {
                sprintf(displayTickerText4[displayTickerTextI4%2], "PZEM004T: %s", appData.getPZEMState().c_str() );
              } else {
                sprintf(displayTickerText4[displayTickerTextI4%2], "PZEM004T: %s Errors: %i", appData.getPZEMState().c_str(), mRetries );
              }

          }
            
          } else {  //status with no WIFI connected
            displayTickTime = ULONG_MAX; //disable scroll
            display.clear();
            display.println("WiFi...");
            display.println(miscText);
            screen = -1; //force an update next time
          }
          break;

        case 2: //Power Display Screen

          displayTickTime = ULONG_MAX; //disable scroll
          display.setCursor(0,0);

          char o[13];
          snprintf(o,13,"%6s V   ", appData.getVoltage().c_str()  );
          display.println( o );
          snprintf(o,13,"%6s A   ", appData.getCurrent().c_str()  );
          display.println( o );
          snprintf(o,13,"%6s W   ", appData.getPower().c_str()  );
          display.println( o );
          snprintf(o,13,"%6s KWh ", appData.getEnergy().c_str()  );
          display.println( o );
          snprintf(o,13,"%6s ~PF ", appData.getPf().c_str()  );
          display.println( o );
          snprintf(o,13,"%6s Hz  ", appData.getFrequency().c_str()  );
          display.print( o );
          break;

        case 3:   // Pump Relay status
          displayTickTime = ULONG_MAX;  //disable scroll
          display.setCursor(0,0);
          display.println("Pump:       ");
          switch (pumpState)
            {
              case -1:
                  display.println("Disabled   ");
                  display.println("           ");
                  delta=now() - pumpOffTime;
                  display.setInvertMode(true);
                  if (delta >= 0) // should always be less than 0
                  {
                    int days = delta / 86400;
                    if (days > 0) {
                    //display.println("   " + String(days)+ " days ");
                      snprintf(o,13,"%4i days ",days);
                      display.println( o );
                      snprintf(o,13," %2i:%02i:%02i ", hour(delta), minute(delta), second(delta) );
                      display.println( o );
                      display.setInvertMode(false);
                    } else {
                      snprintf(o,13," %2i:%02i:%02i ", hour(delta), minute(delta), second(delta) );
                      display.println( o );
                      display.setInvertMode(false);
                      display.println("           ");
                    }
                  }
                  //display.println("           ");
                  break;
              case 0:
                  display.println("Normal     ");
                  display.println("           ");
                  display.println("           ");
                  display.println("           ");
                  break;
              case 1: {
                  display.println("Override   ");
                  display.println("           ");
                  delta=pumpOffTime - now();
                  if (delta >= 0) // should always be greater than 0
                  {
                    snprintf(o,13,"%4i:%02i:%02i ", hour(delta), minute(delta), second(delta) );
                    display.println( o );
                  }
                  }
                  display.println("           ");
                  break;

              default:
                  display.println("Unknown   ");
                  display.println("           ");
                  display.println("           ");
                  display.println("           ");
            }
            display.print(  "           ");
            break;

        case -1: //init mode

          displayTickTime = ULONG_MAX; //disable scroll
          display.clear();
          display.println("No data");
          break;
       
    }
    prevScreen = screen;
}

void updateLCDTask() {
  updateLCD();
}

//setup output pins for pump
void pumpIOsetup()  {
  pinMode( PUMP_DISABLED, OUTPUT);
  pinMode( PUMP_ON, OUTPUT);
}

//sets pump relay to normal mode -- both relays off
void pumpNormal() {
  digitalWrite( PUMP_DISABLED, LOW);
  digitalWrite( PUMP_ON, LOW);
}

//sets pump relay to override -- pump override on--disable off
void pumpOverride() {
  digitalWrite( PUMP_DISABLED, LOW);
  digitalWrite( PUMP_ON, HIGH);
}

//sets pump relay to disable mode -- pump override off-- disable on
void pumpDisable() {
  digitalWrite( PUMP_DISABLED, HIGH);
  digitalWrite( PUMP_ON, LOW);
}

/*
  check if it time to turn off the pump override

*/
void pumpCheck() {

  if (pumpState == 1)
  {  //pump is on already
    if (now() > pumpOffTime) 
    {
      pumpNormal();
      pumpOffTime = 0;
      pumpState = 0;
      sendPumpStatus();
    }
  }
} 

/*
  force the pump on.  Each call adds 2 hours of time.
  if disabled cancles disabled time

*/
void pumpOverrideMode() {
  switch (pumpState)
  {
    case -1:
        //Serial.println("Pump no longer disabled"); 
        pumpState = 0;
        pumpOffTime = 0;
        pumpNormal();
        break;
    case 0:
        //Serial.println("Pump override for 1 hours"); 
        pumpState = 1;
        pumpOffTime = now() + 1 * 3600;
        pumpOverride();
        break;
    case 1:
        //Serial.println("Pump Adding 1 hours to override time"); 
        pumpState = 1;
        pumpOffTime =  pumpOffTime + 1 * 3600;
        if (pumpOffTime - now() > 3 * 3600 )   // set max pump on time to 3 hours.
          pumpOffTime = now() + 3 * 3600;
        pumpOverride();    // not really necesary since already in this mode
        break;
  }
  sendPumpStatus(); 

}



/*
  force the pump disable.  
  if  pump is forced on- put into normal mode

*/
void pumpDisableMode() {
  switch (pumpState)
  {
    case -1:
        Log.I("Pump already disabled"); 
        //pumpState = -1;
        pumpDisable();
        break;
    case 0:
        Log.I("Pump Disabled"); 
        pumpState = -1;
        pumpOffTime = now();
        pumpDisable();
        break;
    case 1:
        Log.I("Pump Override canceled"); 
        pumpState = 0;
        pumpOffTime =  0;
        pumpNormal();
        break;
  }
  sendPumpStatus();

}


/*
 * Hostname:
 * 
 * Sets the device hostname for OTA and MDNS.
 * 
 * */
void setHostname() {
 
  // Set Hostname for OTA and network mDNS (add only 2 last bytes of last MAC Address)
  //sprintf_P( hostname, PSTR("ESP-PWRMETER-%04X"), ESP.getChipId() & 0xFFFF);
  snprintf_P( hostname, 32, PSTR("%S-%04X"), HOSTNAME, ESP.getChipId() & 0xFFFF);
}

/*
 * MQTT Support
 * 
 * Static atributes like, IP, SSID, and so on are set to the MQTT atributes topic.
 * Telemetry data, data that changes through time, are sent to the MQTT telemetry topic.
 * 
 */

//* Supporting functions:
void calcAttributesTopic() {
    String s = "iot/device/" + String(MQTT_ClientID) + "/attributes";
 //   String s = MQTT_TelemetryTopic_Root + String(MQTT_ClientID) + "/attributes";
    s.toCharArray(MQTT_AttributesTopic,256,0);
}

void calcTelemetryTopic() {
    //String s = "iot/device/" + String(MQTT_ClientID) + "/telemetry";
    String s = MQTT_TelemetryTopic_Root + String(MQTT_ClientID) + "/telemetry";
    s.toCharArray(MQTT_TelemetryTopic,256,0);

    Log.I("MQTT_TelemetryTopic:");
    Log.I(MQTT_TelemetryTopic);
}


/*
 * IOT Support:
 * 
 * Functions that using MQTT send data to the IOT server by publishing data on specific topics.
 * 
 */
void IOT_setAttributes() {
    String s = "[{\"type\":\"PowerMeter8266\"}," \
                 "{\"hostname\":\"" + String(hostname) + "\"}," \
                 "{\"ssid\":\""+ WiFi.SSID() + "\"}," \
                 "{\"rssi\":\""+ String(WiFi.RSSI()) + "\"}," \
                 "{\"ip\":\"" + thisDevice.toString() + "\"}," \
                 "{\"dataok\":" + String(pzemDataOK) + "}," \
                 "{\"datanok\":" + String(pzemDataNOK) + "}" \
                 "]";
    
    s.toCharArray( SensorAttributes, 512,0);
    Log.I("PowerMeter Attributes:");
    Log.I(SensorAttributes);
    MQTT_client.publish( MQTT_AttributesTopic, SensorAttributes);

}

void IOT_setTelemetry(String SensorTelemetry) {
    //s.toCharArray(SensorAttributes, 512,0);
    MQTT_client.publish( MQTT_TelemetryTopic, SensorTelemetry);
}

void IOT_setsubscribedTelemetry(String s, String SerialNo) {
  String topic = String(MQTT_TelemetryTopic_Root) + String(MQTT_ClientID) + "/rpc/response/" + SerialNo;
  //String topic = "v1/devices/me/rpc/response/" + SerialNo;
  Log.I(topic + "  " + s);
  MQTT_client.publish( topic, s);
}


//returns a string with pump telemetyr
String buildPumpTelem() {

String s = "";  
time_t delta = now();  // sloppy coding -- should really use another variable for now but better than calling now twice
char o[15];
int days;

   //time should be possitive for correct output
   if (pumpOffTime >= delta)
   {
     delta = pumpOffTime - delta;
     days = delta / 86400;
     if (days > 0) {
       snprintf(o,15,"%i %2i:%02i:%02i", days, hour(delta), minute(delta), second(delta) );
     } else {
       snprintf(o,15,"%2i:%02i:%02i", hour(delta), minute(delta), second(delta) );
     }
   } else {
     delta = delta - pumpOffTime;
     days = delta / 86400;
     if (days > 0) {
       snprintf(o,15,"-%i %2i:%02i:%02i", days, hour(delta), minute(delta), second(delta) );
     } else {
       snprintf(o,15,"-%2i:%02i:%02i", hour(delta), minute(delta), second(delta) );
     }
   }

   //Log.I("delta: " + String(delta));

    switch (pumpState)
    { 
      case -1:
        s = s + "\"pumpState\":\"Disabled\"" + \
                ",\"PumpStateTime\":\"" + o +"\"" + \
                ",\"PumpOffTime\":" + String(pumpOffTime); //horrible hack--NTPZ needs to be used here and properly initilazied  to return utc date
        break;
      case 0:
        s = s + "\"pumpState\":\"Normal\"" + \
                ",\"PumpStateTime\":\"---\"" + \
               ",\"PumpOffTime\":" + String(pumpOffTime);
        break;
      case 1:
        s = s + "\"pumpState\":\"Override\"" + \
                ",\"PumpStateTime\":\"" + o + "\"" + \
                ",\"PumpOffTime\":" + String(pumpOffTime);
        break;
    }

  return s;
}


  void sendPumpStatus() {

    // Build the MQTT message:
    String s = "{" + buildPumpTelem() + "}";
    RelaySendTelemetryTime = 0; //send pump status for next normal telemtry try
    Log.I("-> Power Telemetry data: ");
    Log.I( s );

    IOT_setTelemetry(s);

}

// MQTT Calback function for receiving subscribed messages.

constexpr unsigned int hash(const char *s, int off = 0) {                        
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];                           
} 


#include <ArduinoJson.h>
void MQTT_callback(String &topic, String &payload) {
  /* Just a standard callback. */
  Log.I("Message arrived in topic: " + topic + "  " + payload);

  //try to parse it
  int last = topic.lastIndexOf('/');
  String serialNo = "";
  if (last <= -1) {
    Log.I("Failed to find serial no");
    return;    
  }
  serialNo = topic.substring(last+1);
  //Log.I("SerialNo: " + serialNo);
 
  DynamicJsonDocument doc(200);
  deserializeJson(doc, payload);

  const char* method = doc["method"];
  int pin = -1;
  int pinEnabled = 0;
   
  switch( hash(method) ){
   // case hash("pumpDisabled") : 
   // Log.I("Pump disabled detected");


    case hash("getPumpRelayStatus") :
      //pin 1 Override
      //pin 2 Disabled
      switch (pumpState)
      {
      case -1:
        IOT_setsubscribedTelemetry("{\"1\":false,\"2\":true}",serialNo);
        break;
      case 0:
        IOT_setsubscribedTelemetry("{\"1\":false,\"2\":false}",serialNo);
        break;
      case 1:
        IOT_setsubscribedTelemetry("{\"1\":true,\"2\":false}",serialNo);
        break;
      }
    break;

    case hash("setPumpRelayStatus") :

      if(doc.containsKey("params")) 
      {
        pin = doc["params"]["pin"];
        pinEnabled = (doc["params"]["enabled"]); 
      } 

      switch (pin) 
      {
        case 1 :  Log.I("Pump enable");
          if (pinEnabled)
          {
            
            IOT_setsubscribedTelemetry("{\"1\":false}",serialNo);
            pumpOverrideMode();
          } else {
            IOT_setsubscribedTelemetry("{\"1\":false}",serialNo);
            pumpState = 0;
            pumpOffTime = 0;
            pumpNormal();
            sendPumpStatus();
          }
          break;
        case 2 : Log.I("Pump disable");
          if (pinEnabled)
          {
            IOT_setsubscribedTelemetry("{\"2\":false}",serialNo);
            pumpDisableMode();
          } else {
            IOT_setsubscribedTelemetry("{\"2\":false}",serialNo);
            pumpState = 0;
            pumpOffTime = 0;
            pumpNormal();
            sendPumpStatus();
          }
          break;
        default:
          Log.I("Unknown pin:" + pin);
       }
    break;


//----remote admin commands
    case hash("getPumpAdminCommand") :
      //pin 10  -- enable syslog
      //pin 20  -- Reset KWH
      if (syslogEnabled==0) {
        IOT_setsubscribedTelemetry("{\"10\":false,\"20\":false}",serialNo);
      } else {
        IOT_setsubscribedTelemetry("{\"10\":true,\"20\":false}",serialNo);
      }
      break;

    case hash("setPumpAdminCommand") :

      if(doc.containsKey("params")) 
      {
        pin = doc["params"]["pin"];
        pinEnabled = (doc["params"]["enabled"]); 
      } 

      switch (pin) 
      {
        case 10 :  Log.I("syslog debug");
          if (pinEnabled)
          {
            IOT_setsubscribedTelemetry("{\"10\":true}",serialNo);
            Log.setUdp( true );
            syslogEnabled = 1;
            Log.I("Enabled syslog output to: " + String(UDPLOG_Server) );
          } else {
            IOT_setsubscribedTelemetry("{\"10\":false}",serialNo);
            syslogEnabled = 0;
            Log.I("Disabled syslog output to: " + String(UDPLOG_Server) );
            Log.setUdp( false );
          }
        break;

        case 20 :  Log.I("Reset KWH");
          if (pinEnabled)
          {
            IOT_setsubscribedTelemetry("{\"20\":false}",serialNo);
            //ResetKWH();
            if ( (mState==3) && pzem.resetEnergy() )
            {
              Log.I("PZEM Energy cleared");
              mState = 2;   // forcce a read now
            } else {
              Log.I("Failed to clear PZEM Energy");
            }
          } else {
            IOT_setsubscribedTelemetry("{\"20\":false}",serialNo);
          }
          break;
        
        default:
          Log.I("Unknown pin:" + pin);
       }
    break;







    default:
      Log.I("Unknown request");
      Log.I(method); 
  }

}

//* Connects to the MQTT Broker
void MQTT_Connect() {
    MQTT_client.begin( MQTT_Server, MQTT_Port , WIFIClient );
    MQTT_client.onMessage( MQTT_callback );
    MQTT_client.setOptions( 120, true, 500 );

    unsigned int i=0;
    while (! MQTT_client.connect( MQTT_ClientID, MQTT_UserID, MQTT_Password ) ) {
        i++;
        Log.E("MQTT Connection failed. " + String(i));
        //need to buid state machine and make it partof loop---this is happening too often
        //delay here is bad and counter is just a hack if things go really wrong
        delay(1000);
        if (i>900)
        {
          Log.E("Start from scratch.... Rebooting");
          ESP.restart();
        }

    }

    calcAttributesTopic();
    calcTelemetryTopic();

    //MQTT_client.subscribe("pump/override");

    static int subscribeOK = 0;
    
    if (subscribeOK == 0) {

    String subtopic = String(MQTT_TelemetryTopic_Root) + String(MQTT_ClientID) + "/rpc/request/+";
   
    int err;
    //err =  MQTT_client.subscribe("v1/devices/me/rpc/request/+",1);
    err =  MQTT_client.subscribe(subtopic,1);
    if (! err ) {
      Log.E("failed to subscribe to pump/override.");
      Serial.print("failed, err=");
      Serial.print(err);
      Serial.print(" rc=");
      Serial.print(MQTT_client.returnCode());
      Serial.print(" last error=");
      Serial.println(MQTT_client.lastError());
      //const char lwmqtt_err_text[][28] = { "SUCCESS", "BUFFER_TOO_SHORT", "VARNUM_OVERFLOW", "NETWORK_FAILED_CONNECT", "NETWORK_TIMEOUT", "NETWORK_FAILED_READ", "NETWORK_FAILED_WRITE", "REMAINING_LENGTH_OVERFLOW", "REMAINING_LENGTH_MISMATCH", "MISSING_OR_WRONG_PACKET", "CONNECTION_DENIED", "FAILED_SUBSCRIPTION", "SUBACK_ARRAY_OVERFLOW", "PONG_TIMEOUT", "UNKNOWN" };
      //const char lwmqtt_return_code_text[][25] = { "CONNECTION_ACCEPTED", "UNACCEPTABLE_PROTOCOL", "IDENTIFIER_REJECTED", "SERVER_UNAVAILABLE", "BAD_USERNAME_OR_PASSWORD", "NOT_AUTHORIZED", "UNKNOWN_RETURN_CODE", "UNKNOWN" };

//lwmqtt_err_t lastError();
//lwmqtt_return_code_t returnCode();
        delay(1000);
        subscribeOK++;
    } 
    } else {   //every other failure try again
      subscribeOK = 0;
    }



    Log.I("Connected to MQTT!");
}

/*
 * OTA support
 * 
 */
void OTA_Setup() {

    Log.I("Setting up OTA...");
    ArduinoOTA.setHostname( hostname );
    ArduinoOTA.setPassword( OTAPassword );
    ArduinoOTA.begin();

    // OTA callbacks
    ArduinoOTA.onStart([]() {
      Log.I(F("\r\nOTA Starting"));
      digitalWrite( MONITOR_LED, HIGH);
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      uint8_t percent = progress/( total/100 );
      
      if (percent % 10 == 0) {
        Log.I( String(percent));
        digitalWrite( MONITOR_LED, !digitalRead( MONITOR_LED));    
      }
    });

    ArduinoOTA.onEnd([]() {

      Log.I(F("OTA Done\nRebooting..."));
      digitalWrite( MONITOR_LED, LOW);
    });

    ArduinoOTA.onError([](ota_error_t error) {
      Log.E("OTA Error: " + String(error));

      if (error == OTA_AUTH_ERROR) {
        Log.E("OTA Auth Failed");
      } else
      if (error == OTA_BEGIN_ERROR) {
        Log.E("OTA Begin Failed");
      } else
      if (error == OTA_CONNECT_ERROR) {
        Log.E("OTA Connect Failed");
      } else
      if (error == OTA_RECEIVE_ERROR) {
        Log.E("OTA Receive Failed");
      } else
      if (error == OTA_END_ERROR) {
        Log.E("OTA End Failed");
      }

      ESP.restart();
    });

    Log.I("OTA setup done!");
}

void display_WIFIInfo() {
    if (WiFi.status() ==  WL_CONNECTED) {
      Log.I("Connected to WIFI: " + WiFi.SSID() );

      //Log.I(  );
      /*Serial.print("MAC: ");
      Serial.println(MAC_char);*/

      thisDevice = WiFi.localIP();
      Log.I("  IP: " + thisDevice.toString() );

    } else {
      Log.I("Wifi not connected: ");
    }


}

/*
 * back_tasks:  Executes the background tasks
 * 
 * Calls the functions necessary to keep everything running smoothly while waiting or looping
 * Such tasks include mantaining the MQTT connection, checking OTA status and updating the timers.
 * 
 */

void back_tasks() {
    MQTT_client.loop();           // Handle MQTT
    timer.run();                  // Handle SimpleTimer

}



/*
 * WIFI_Setup: Setup the WIFI connection.
 * 
 * It cycles over the configured access points until a sucessufull connection is done
 * 
 */
void WIFI_Setup() {
    bool   connected = false;
    char   *ssid;
    char   *pwd;
    int    cntAP = 0;
    int    tries = 0;
    String out;

    Log.I("Connecting to WIFI...");
    
    // Station mode.
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.hostname(hostname);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

   

    while ( !connected ) {
        ssid = (char *)APs[cntAP][0];
        pwd = (char *)APs[cntAP][1];
        Log.I("Connecting to: ");
        Log.I(ssid);
        updateLCD(1,ssid);

        WiFi.begin(ssid, pwd );

        if (WiFi.waitForConnectResult() != WL_CONNECTED) {
                Log.E("Connection Failed! Trying next AP...");
                Log.E("Number of tries: " + String(tries));
                
                cntAP++;
                tries++;
                // Circle the array one entry after another
                if (cntAP == NUMAPS )
                cntAP = 0;
                
                // Set Monitor led on:  Light up the led to show that something is working
                digitalWrite( MONITOR_LED, LOW); 
                delay(1000);
                digitalWrite( MONITOR_LED, HIGH); 

                yield();
        } else
                connected = true;
    }

/*    WiFi.macAddress(MAC_address);
    for (unsigned int i = 0; i < sizeof(MAC_address); ++i){
      sprintf(MAC_char,"%s%02x:",MAC_char,MAC_address[i]);
    }
*/  
    display_WIFIInfo();
    updateLCD(1);
}

/*
 * check_Connectivity:  Checks the connectivity. 
 * 
 * Checks the connectivity namely if we are connected to WIFI and to the MQTT broker.
 * If not, we try to reconnect.
 * 
 */

void check_Connectivity() {

        /* Check WIFI connection first. */        
        if ( WiFi.status() != WL_CONNECTED ) {
            Log.I("calling WIFI_Setup");
            WIFI_Setup();
            MQTT_Connect();
        } else {
            /* Check MQTT connectivity: */
            if ( !MQTT_client.connected() ) {
                MQTT_Connect();
                // Send the IOT device attributes at MQTT connection
                IOT_setAttributes();
            }
        }
}



void PWRMeter_Connect() {
    //uint8_t tries = 0;
    bool    pzemOK = false;
    
    Log.I("Connecting to PZEM004T...");
    appData.setPZEMState(PZEM_CONNECTING);

    if ( (pzemOK=pzem.setAddress(pzemAddress)) == false)  {
        Log.E("Failed to connect to PZEM004T...");
        appData.setPZEMState(PZEM_CONNECTFAIL);
        mState = 0;       // Return to the NOT Connected State.
    }

    if (pzemOK) {
        Log.I("Connection to PZEM004T OK!");
        appData.setPZEMState(PZEM_CONNECTED);
        mState = 2;        // Move forward to the Connected State.
        updateLCD(2);      //change display to show power setting
    } else {
      appData.setPZEMState(PZEM_DISCONNECTED);
      mState = 0;          // Return to the NOT Connected State.
    }


}

/*
 * PWRMeter_getData:
 * 
 * Gets data from the Power meter and sends it to the backend through MQTT
 * 
 */

int PWRMeter_getData() {
    float volts = 0;
 
    // Set Monitor led on:
    digitalWrite( MONITOR_LED, LOW); 

    Log.I("Getting PowerMeter data...");

    // Get the PZEM004T Power Meter data
    volts = pzem.voltage();
    if ( isnan(volts) ) {
      Log.E("No valid data obtained from the PowerMeter: V=nan");
      //failed  so fall into next state for retry
      return 1;
    } else if ( volts == -1 ) {
      Log.E("No valid data obtained from the PowerMeter: V=-1");
      //failed  so fall into next state for retry
      return 1;
    } else {
      Log.I("PowerMeter Data OK!");
    }

    // Execute the back ground tasks otherwise we may loose conectivity to the MQTT broker.
    back_tasks();
 

    float amps = pzem.current();
    float watts = pzem.power();
    float kwh = pzem.energy();
    float pf = pzem.pf();
    back_tasks();
    float hz = pzem.frequency();


    // Turn led off:
    digitalWrite( MONITOR_LED, HIGH); 

    // Build the MQTT message:
    
    String s = "{\"Volts\":" + String(volts) + \
               ",\"Amps\":" + String(amps) + \
               ",\"Watts\":" + String(watts) + \
               ",\"KWh\":" + String(kwh) + \
               ",\"PF\":" + String(pf) + \
               ",\"Freq\":" + String(hz);

     if ( now() - RelaySendTelemetryTime > 0)
     {
       s  = s + "," + buildPumpTelem();
       if (pumpState == 0)
       {
         RelaySendTelemetryTime = now() + RELAYSENDTELEMETRYNORMAL;
       } else {
         RelaySendTelemetryTime = now() + RELAYSENDTELEMETRYACTIVE;
       }
     }       
     s = s + "}";

//Log.I("next data time:" + String(RelaySendTelemetryTime) );
//Log.I(" current time:" + String(now()));
//Log.I("   delta time:" + String(RelaySendTelemetryTime-now()));
    Log.I("-> Power Meter data: ");
    Log.I( s );

    appData.setVoltage( volts );
    appData.setCurrent( amps );
    appData.setPower( watts ); 
    appData.setEnergy( kwh );
    appData.setPf( pf );
    appData.setFrequency( hz );
    
    // Send the data through MQTT to the backend
    if ( volts >= 0 ) {
        IOT_setTelemetry(s);
        appData.setSamplesOK();
        pzemDataOK++;
    } else {
        Log.W("Data not sent due to invalid read.");
        appData.setSamplesNOK();
        pzemDataNOK++;
    }

    mState = 2;   // Move back to the Read data state to trigger another (future) read.

    return 0;
}

void PWRMeter_ReadState() {
  mState = 2;
}

// Just blink the onboard led according to the defined period
void Blink_MonitorLed() {
  digitalWrite( MONITOR_LED , monitor_led_state );
  if ( monitor_led_state == LOW ) 
    monitor_led_state = HIGH;
  else 
    monitor_led_state = LOW;

  timer.setTimeout( ledBlink , Blink_MonitorLed ); // With this trick we can change the blink rate of the led
}

void IOT_SendAttributes() {
  IOT_setAttributes();
}

void printTime() {
  timeProvider.logTime();
}


/*
 * MAIN CODE
 * 
 */

void setup() {
  IPAddress udpServerAddress;
  udpServerAddress.fromString(UDPLOG_Server);

  appData.setFWVersion(FW_Version);
  appData.setLogServerIPInfo(udpServerAddress.toString());

  Serial.begin(115200);
  delay (200);                           // Wait for the serial port to settle.


  setupLCD();
  updateLCD(0);  //show startup message
  timer.setInterval( LCDRefreshRate , updateLCDTask ); 
  delay(500);    //leave startup on for 1/2 second

  setHostname();
  Log.setSerial( true );                 // Log to Serial
  Log.setServer( udpServerAddress, UDPLOG_Port );
  Log.setTagName("PWM01");                // Define a tag for log lines output

  // Indicator onbord LED
  pinMode( MONITOR_LED, OUTPUT);
  digitalWrite( MONITOR_LED, LOW);

  // Button setup
   leftBtn.begin();
  rightBtn.begin(); 
  
  //relay setup
  pumpIOsetup();
  pumpNormal();
 

  // We set WIFI first...
    
  WIFI_Setup();
  
  // Setup Logging system.
  //Log.I("Enabling UDP Log Server...");
  //Log.setUdp( true );                  // Log to UDP server when connected to WIFI.
  //syslogEnabled = 1;
  Log.W("------------------------------------------------> Power Meter REBOOT");

  // Setup OTA
  OTA_Setup();

  // Set time provider to know current date and time
  timeProvider.setup();
  timeProvider.logTime();
    
  updateLCD(1);
 
  //Connect to the MQTT Broker:
  MQTT_Connect();

  // Send the IOT device attributes at MQTT connection
  IOT_setAttributes();

  // Setup WebServer so that we can have a web page while connecting to the PZEM004T
  Log.I("Setting up the embedded web server...");
  webServer.setup();
  Log.I("Web server available at port 80.");

  //delay(100);
  //display_WIFIInfo();                   // To display wifi info on the UDP socket.

  // Setup the monitor blinking led
  timer.setTimeout( ledBlink , Blink_MonitorLed ); 

  // Periodically send to the MQTT server the IOT device state
  timer.setInterval( pingMqtt , IOT_SendAttributes );

  // Periodically log the time
  timer.setInterval( 3 * 60 * 1000 , printTime );

  // Setup MDNS
  MDNS.begin( hostname );
  MDNS.addService("http", "tcp", 80);
  Log.I("Setup Done");
}

void loop() {
    // Check if we are still connected.
    check_Connectivity();

    // Power meter state machine
    switch (mState)
    {
        case 0:   // We are not connected. Trigger a connection every 3s until we connect.
                timer.setTimeout( 3000 , PWRMeter_Connect );
                ledBlink = 200;  // Blink the LED Fast
                mState = 1;
        break;
        case 1:   // Connecting. The PWRMeter_Connect function will move to next state
                  // So we do nothing here.
        break;
        case 2:   // We are connected. Trigger a Power meter read.
                if (PWRMeter_getData() == 0) {          //non-zero return means it failed
                  timer.setTimeout(PZEM_SLEEP_TIME, PWRMeter_ReadState);
                  ledBlink = 500;  // Blink the LED Slow
                  mRetries = 0;  
                } else {   //failed to read retry in shorter time span
                  mRetries++;

                  if (mRetries%10==0) {
                     Log.E("No valid data obtained from the PowerMeter after 10 tries");
                     timer.setTimeout(PZEM_SLEEP_TIME, PWRMeter_ReadState);
                     //mRetries = 0;
                     if (mRetries > 999) 
                     {  // give up and try to connect again...
                       mState=0;
                     }
                  } else {
                    timer.setTimeout(PZEM_RETRY_TIME, PWRMeter_ReadState); 
                  }           
                }
                mState = 3;
        break;

        case 3:   // We are waiting for reading the data. -- Timer wait until next Read-- The PWRMeter_ReadState will move back to the previous state.
        break;
  
        default:
           Log.E("Invalid state");
           mState=0;
        break;
    }

    back_tasks();
    //just did it in back_task
    // MQTT_client.loop();           // Handle MQTT
    //timer.run();                  // Handle SimpleTimer
    ArduinoOTA.handle();          // Handle OTA.
 
    leftBtn.read();
    rightBtn.read();

    if (leftBtn.wasPressed())
    {
      //Serial.println("Left Button pressed");
      updateLCD(INT_MIN);
    }

    if ( leftBtn.pressedFor(2000) )
    {
      if ( leftBtnState == 0)
      {
        //Serial.println("Left Button pressed for 2 second");
        pumpDisableMode();
        updateLCD(3);
      }
      leftBtnState=1;
    }

    if (leftBtnState==1)  //long pressed detected earlier
    {
      if ( leftBtn.wasReleased() )
      {
        //Serial.println("Left Button released after being presssed");
        leftBtnState=0;
      }
    }


    if (rightBtn.wasPressed())
    {
      //Serial.println("Right Button pressed");
      updateLCD(INT_MAX);
    }

    if ( rightBtn.pressedFor(2000) )
    {
      if (rightBtnState == 0)
      {
        //Serial.println("Right Button pressed for 2 second");
        pumpOverrideMode();
        updateLCD(3);
      }
      rightBtnState=1;
    }

    if (rightBtnState==1)  //long pressed detected earlier
    {
      if ( rightBtn.wasReleased() )
      {
        //Serial.println("Right Button released after being presssed");
        rightBtnState=0;
      }
    }

    pumpCheck();   // check if pump needs to be turned off

    //update display ticker
    if (displayTickTime <= millis()) 
    {
      displayTickTime = millis() + 30;

      // Should check for error. rtn < 0 indicates error.
      int8_t rtn = display.tickerTick(&displayState4);
      if (rtn <= RTN_CHECK) 
      {
        // Should check for error. Return of false indicates error.
        //display.tickerText(&displayState, text[(displayN++)%3]);
        display.tickerText(&displayState4, displayTickerText4[displayTickerTextI4%2]); 
      }

      rtn = display.tickerTick(&displayState5);
      if (rtn <= RTN_CHECK) 
      {
        display.tickerText(&displayState5, displayTickerText5[displayTickerTextI5%2]); 
      }

      rtn = display.tickerTick(&displayState2);
      if (rtn <= RTN_CHECK) 
      {
        display.tickerText(&displayState2, displayTickerText2[displayTickerTextI2%2]); 
      }

      rtn = display.tickerTick(&displayState3);
      if (rtn <= RTN_CHECK) 
      {
        display.tickerText(&displayState3, displayTickerText3[displayTickerTextI3%2]); 
      }


    }

}


