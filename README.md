# PowerMeter
This fork has been updated from the original code to include:
  support for V3 of PZEM modules
  Added LCD display
  Added relay control to both override and disble
  Added 2 push buttons for local control to cylce through various display screens and enable and disable relays
  

This repository holds a PZEM-004T and ESP8266 based power/energy meter. More information at: https://primalcortex.wordpress.com/2019/07/06/measuring-home-energy-consumption-with-the-pzem004t-and-esp8266/

The repository structure is as follows:
The *PowerMeter* repository/folder contains the firmware for an ESP8266 and PZEM-004T based power/energy meter. This is the firmware that should be used to have a full functional ESP8266 based powermeter.
The PowerMeter is designed to use Wemos D1 ESP8266 boards.

The *PZEM004T-Test* repository contains a simple program for testing the ESP8266 to PZEM004 communications and the retrieving of data from the PZEM004T.

The *Node-Red* folder contains a flow that receives published telemetry data from the Power Meter, saves it on an InfluxDB database and creates a Node-Red Dashboard UI to show the latest collected data.

The *Grafana* folder contains a very simple grafana dashboard that retrieves data from the InfluxDB database and displays it. It's a no frills dashboard...

# Screenshots:

Some screenshots of the PowerMeter working:

*ESP8266 based Power Meter Web Interface:*
![](/images/web_page.png?raw=true)

*The Node-Red UI:*
![](/images/node_red.png?raw=true)

*The simple Grafana dashboard:*
![](/images/grafana.png?raw=true)
