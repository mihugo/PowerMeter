const char TEMPLATE_HEADER[] =
  "<!DOCTYPE html>"
	"<html><head>"
	//"<title>ESP8266 PZEM004T Power Meter IoT Device</title>"
  "<title>Pumphouse Power Meter</title>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\">"
  "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css\" integrity=\"sha512-dTfge/zgoMYpP7QbHy4gWMEGsbsdZeCXz7irItjcC3sPUFtf0kuFbDz/ixG7ArTxmDjLXDmezHubeNikyKGVyQ==\" crossorigin=\"anonymous\">"
  "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap-theme.min.css\" integrity=\"sha384-aUGj/X2zp5rLCbBxumKTCw2Z50WgIr1vs/PFN4praOTvYXWlVyh2UtNUU0KAUhAX\" crossorigin=\"anonymous\">"
  "<script src=\"//code.jquery.com/jquery-1.11.3.min.js\"></script>"
  "<script src=\"//code.jquery.com/jquery-migrate-1.2.1.min.js\"></script>"
  //"<script src=\"trianglify.min.js\"></script>"
  "</head>"
	"<body id=\"homepage\">"
  "<div class=\"container\">"
// 	"  <h1><center><b>ESP8266 Power Meter</b></center></h1>"
  "  <h1><center><b>Pumphouse Power Meter</b></center></h1>"
  "  <div class=\"row\">"
  "    <div class=\"col-md-4\">"
  "    <div class=\"panel panel-primary\">"
  "    <div class=\"panel-heading\">"
  "      <h3 class=\"panel-title\"><b>ESP8266</b></h3>"
  "    </div>"
  "    <table class=\"table table-striped\"><thead><tr><th></th><th>Parameter</th><th>Value</th></tr></thead><tbody>"
  "    <tr><td><i class=\"glyphicon glyphicon-link\"></i></td><td><b> Firmware Version: </b></td><td>"
  // getFWV()
  "%s"
   "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-link\"></i></td><td><b> ESP8266 IP: </b></td><td>"
  // getIPAddress()
  "%s"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-cloud\"></i></td><td><b> ESP8266 GW: </b></td><td>"
  //getGW()
  "%s"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-grain\"></i></td><td><b> Wifi SSID: </b></td><td>"
  //getSSID()
  "%s"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-signal\"></i></td><td><b> Wifi RSSI: </b></td><td>"
  //String(WiFi.RSSI())
  "%s"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-wrench\"></i></td><td><b> Log Server: </b></td><td>"
  //Log Server IP
  "%s"
  "</td></tr>"

  "<tr><td><i class=\"glyphicon glyphicon-flash\"></i></td><td><b> ESP Heap: </b></td><td>"
  //ESP.getFreeHeap()
  "%s"
  "</td></tr></tbody></table>"
  "</td></tr>"
  "</div>"
  "</div>"

  "<div class=\"col-md-4\">"
  "<div class=\"panel panel-primary\">"
  "<div class=\"panel-heading\">"
  "<h3 class=\"panel-title\"><b>PZEM004T Data</b></h3>"
  "</div>"
  "<table class=\"table table-striped\"><thead><tr><th></th><th>Parameter</th><th>Value</th></tr></thead><tbody>"
  "<tr><td><i class=\"glyphicon glyphicon-scale\"></i></td><td><b>Voltage: </b></td><td>"
  // getVoltage()
  "%s"
  "V"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-scale\"></i></td><td><b>Current: </b></td><td>"
  // getCurrent()
  "%s"
  "A"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-scale\"></i></td><td><b>Power: </b></td><td>"
  // getPower()
  "%s"
  "W"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-flash\"></i></td><td><b>Energy: </b></td><td>"
  //getEnergy() 
  "%s"
  "W"
  "</td></tr>"
"<tr><td><i class=\"glyphicon glyphicon-music\"></i></td><td><b>Frequency: </b></td><td>"
  // getFrequency()
  "%s"
  "hz"
  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-scale\"></i></td><td><b>Power Factor: </b></td><td>"
  // getPf()
  "%s"
  "</td></tr>"
  "</tbody></table>"
  "</div>"
  "</div>"


  "<div class=\"col-md-4\">"
  "<div class=\"panel panel-primary\">"
  "<div class=\"panel-heading\">"
  "<h3 class=\"panel-title\"><b>Samples</b></h3>"
  "</div>"
  "<table class=\"table table-striped\"><thead><tr><th></th><th>Parameter</th><th>Value</th></tr></thead><tbody>"
  "<tr><td><i class=\"glyphicon glyphicon-scale\"></i></td><td><b>OK: </b></td><td>"
  "%s"

  "</td></tr>"
  "<tr><td><i class=\"glyphicon glyphicon-scale\"></i></td><td><b>NOK:</b></td><td>"
  "%s"

  "</td></tr>"
    "<tr><td><i class=\"glyphicon glyphicon-scale\"></i></td><td><b>PZEM State:</b></td><td>"
  "%s"

  "</td></tr>"
  "</tr></tbody></table>"
  "</div>"
  "</div>"


"<div class=\"col-md-4\">"
  "<div class=\"panel panel-primary\">"
  "<div class=\"panel-heading\">"
  "<h3 class=\"panel-title\"><b>Relay Control</b></h3>"
  "</div>"
  "<table class=\"table table-striped\"><thead><tr><th></th><th>Parameter</th><th>Value</th></tr></thead><tbody>"
  "<tr><td><i class=\"glyphicon glyphicon-alert\"></i></td><td><b>Pump State: </b></td><td>"
  "%s"
  //getPumpState()

 
  "</td></tr>"
    "<tr><td><i class=\"glyphicon glyphicon-hourglass\"></i></td><td><b>Time:</b></td><td>"
  "%s"
//getPumpOffTime() {


  "</td></tr>"
  "</tr></tbody></table>"
  "</div>"
  "</div>"


;

const char TEMPLATE_FOOTER[] =
  "</div>"
  "</div>"
  "<script>"
  "   function addTriangleTo(target) {"
  "       var dimensions = target.getClientRects()[0];"
  "       var pattern = Trianglify({"
  "               width: 1920,"
  "               height: 1080"
  "       });"
  "       target.style['background-image'] = 'url(' + pattern.png() + ')';"
  "       target.style['background-size'] = 'cover';"
  "       target.style['-webkit-background-size'] = 'cover';"
  "       target.style['-moz-background-size'] = 'cover';"
  "       target.style['-o-background-size'] = 'cover';"
  "    }"
  "    var resizeTimer;"
  "    $(window).on('resize', function(e) {"
  "       clearTimeout(resizeTimer);"
  "       resizeTimer = setTimeout(function() {"
  "           addTriangleTo(homepage);"
  "       }, 400);"
  "    });"
  "    addTriangleTo(homepage);"
  " </script>"
  "</body>"
  "</html>";
