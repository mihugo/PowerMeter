#include <TimeLib.h>
#include <TimeProvider.hpp>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LogClient.hpp>

const int   NTP_PACKET_SIZE = 48;					// Fixed size of NTP record
uint8_t     packetBuffer[NTP_PACKET_SIZE];
WiFiUDP     Udp;
IPAddress   ntpServer;							    // IP address of NTP_TIMESERVER from DNS Query
String      NTP_TIMESERVER;
int         NTP_TIMEZONES;
bool        NTP_TIMEZONEDSTEnabled;

TimeProvider timeProvider;

// ----------------------------------------------------------------------------
// Send the request packet to the NTP server.
//
// ----------------------------------------------------------------------------
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:                 
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}



//int dstOffset (unsigned long unixTime)
int dstOffset (time_t t)
{
  //borrowed from: https://forum.arduino.cc/index.php?topic=197637.0
  //this only works in the US but could be made to work WW with a bit of work
  //because it only updates during NTP sync time will not switch at the correct time.
  //if syncs are frequntly time will not be wrong for long

  //Receives unix epoch time and returns seconds of offset for local DST
  //Valid thru 2099 for US only, Calculations from "http://www.webexhibits.org/daylightsaving/i.html"
  //Code idea from jm_wsb @ "http://forum.arduino.cc/index.php/topic,40286.0.html"
  //Get epoch times @ "http://www.epochconverter.com/" for testing
  //DST update wont be reflected until the next time sync
  //Begin DST: Sunday March 14 - (1 + y*5/4) mod 7
  //End DST: Sunday November 7 - (1 + y*5/4) mod 7;
  

  if ( !NTP_TIMEZONEDSTEnabled ) {
    return (0);    //no DST for our timezone
  }

  //time_t t = unixTime;
  int beginDSTDay = (14 - (1 + year(t) * 5 / 4) % 7); 
  int beginDSTMonth=3;
  int endDSTDay = (7 - (1 + year(t) * 5 / 4) % 7);
  int endDSTMonth=11;
  if (((month(t) > beginDSTMonth) && (month(t) < endDSTMonth))
    || ((month(t) == beginDSTMonth) && (day(t) > beginDSTDay))
    || ((month(t) == beginDSTMonth) && (day(t) == beginDSTDay) && (hour(t) >= 2))
    || ((month(t) == endDSTMonth) && (day(t) < endDSTDay))
    || ((month(t) == endDSTMonth) && (day(t) == endDSTDay) && (hour(t) < 1)))
    return (3600);  //Add back in one hours worth of seconds - DST in effect
  else
    return (0);  //NonDST
}


int TimeProvider::utcOffset (time_t t)
{
    return ( NTP_TIMEZONES * SECS_PER_HOUR + dstOffset(t) );
}


time_t getNtpTime()
{
    while (Udp.parsePacket() > 0) ; // discard any previously received packets

    WiFi.hostByName(NTP_TIMESERVER.c_str() , ntpServer);

    Log.I("NTP Request: " + NTP_TIMESERVER + ": " + ntpServer.toString());
    sendNTPpacket(ntpServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            Log.I("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            unsigned long unixTime = secsSince1900 - 2208988800UL;
            return unixTime;
           // return unixTime + NTP_TIMEZONES * SECS_PER_HOUR + dstOffset(unixTime);
        }
    }
    Log.E("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

void TimeProvider::setNTPServer(String server , unsigned long syncinterval ) {
    NTPServer = server;
    NTPSync   = syncinterval;
}

void TimeProvider::setTimeZone( int tz ) {
    NTPTZ = tz;
}

void TimeProvider::setup() {
    if ( !initialized ) {

        // Initialize a local UDP port otherwise the NTP request won't work.
        Udp.begin(57334);

        NTP_TIMESERVER = NTPServer;
        NTP_TIMEZONES = NTPTZ;
        NTP_TIMEZONEDSTEnabled = NTPTZDSTEnabled;
        setSyncProvider( getNtpTime );
        setSyncInterval( NTPSync );
        initialized = true;
    }
}

void TimeProvider::logTime() {
    const char * Days [] ={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    //String datetime = String(Days[weekday()-1]) + ", " + day() +"/"+ month() + "/" + year() + " " + hour() + ":" + minute() + ":" + second();
    //this is barbaric as there must be a simpler way to format these strings with leading zeros ... in one line 
    char m[3];
    snprintf(m, 3, "%02i", minute() );
    char s[3];
    snprintf(s, 3, "%02i", second() );
    String datetime = String( Days[weekday()-1]) + ", " + month() +"/"+ day() + "/" + year() + " " + hour() + ":" + m + ":" + s;
    Log.I( datetime);
}
