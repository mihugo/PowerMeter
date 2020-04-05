#ifndef __TIME_PROVIDER_H__
#define __TIME_PROVIDER_H__

#include <Arduino.h>
#include <TimeLib.h>

class TimeProvider
{
private:
    bool initialized      = false ;
    //String  NTPServer     = "pt.pool.ntp.org";
    String  NTPServer     = "time.cloudflare.com";
    unsigned long NTPSync = 3600;   
    int    NTPTZ   = -8 ;
    bool   NTPTZDSTEnabled = true ;


public:
    void setNTPServer( String, unsigned long );
    void setTimeZone( int );
    void logTime();
    void setup ();
    //void handle();
};

extern TimeProvider timeProvider;

#endif