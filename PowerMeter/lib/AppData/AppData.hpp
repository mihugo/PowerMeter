#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include <Arduino.h>

#define PZEM_DISCONNECTED   0
#define PZEM_CONNECTING     1
#define PZEM_CONNECTED      2
#define PZEM_CONNECTFAIL    3

class AppData
{
private:
    String  m_FWVersion;
    String  m_logServerIP;
    float   m_V = 0;
    float   m_I = 0;
    float   m_P = 0;
    float   m_E = 0;
    float   m_hz = 0;
    float   m_pf = 0;

    unsigned long m_samplesOK = 0;
    unsigned long m_samplesNOK= 0;

    uint8_t    m_PZEMState = 0;    // 0 - Not Connected, 1 - Connecting, 2 - Connected.
    int     m_pumpState = 0;  
    int     m_pumpOffTime = 0;

public:

    void setFWVersion( String);
    void setLogServerIPInfo( String );

    void setVoltage( float );
    void setCurrent( float );
    void setPower( float );
    void setEnergy( float );
    void setFrequency( float );
    void setPf( float );
    void setSamplesOK();
    void setSamplesNOK();

    void setPZEMState( uint8_t );
    String getPZEMState();

    String getFWVersion();
    String getLogServerIPInfo();
    String getDevIP();
    String getGWIP();
    String getSSID();
    String getRSSI();
    String getHeap() ;

    String getVoltage();
    String getCurrent();
    String getPower();
    String getEnergy();
    String getFrequency();
    String getPf();

    String getSamplesOK();
    String getSamplesNOK();

    String getRelayState();
    void   setRelayState(int);
    String getPumpOffTime();
    void   setPumpOffTime(int);
};

extern AppData appData;

#endif