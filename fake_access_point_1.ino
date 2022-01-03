/*
 *
 *When the data indicator is orange (on Kit kat ; grey on older versions),
 *this means that the device is unable to receive a response from GCM (Google Cloud Messaging,
 *the framework that handles push notifications). This traffic is sent through ports 5228, 5229, and 5230.
 *If the AP is blocking or interfering with traffic on those ports, 
 *push notifications won't work and the data indicator will be orange.
 *
 * http://clients3.google.com/generate_204 (must return 204)
 * http://www.gstatic.com/generate_204 (must return 204)
 * http://connectivitycheck.gstatic.com/generate_204 (must return 204)
 * 
  IOS
  Connectivity spoofing is now also working in iOS. The URL that needs to be replicated is captive.apple.com/hotspot-detect.html,
  and it needs to respond with a 200 and exactly the following content: "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>"
 */



/* Create a WiFi access point and provide a web server on it. 
** For more details see http://42bots.com.
*/

#include <ESP8266WiFi.h>
#include <DNSServer_multi.h> // lol, identical to DNSServer but with hackery in there to bypass Android/iOS connectivity checks
#include <ESP8266WebServer.h>

IPAddress    apIP(42, 42, 42, 42);  // Defining a static IP address: local & gateway
                                    // Default IP in AP mode is 192.168.4.1

/* This are the WiFi access point settings. Update them to your likin */
const char *ssid = "Chuo Line Free Internet";
// const char *password = "ESP8266Test";

// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);

const byte DNS_PORT = 53;
DNSServer dnsServer;

/*
void handleRoot() {
  char html[1000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hour = min / 60;

// Build an HTML page to display on the web-server root address
  snprintf ( html, 1000,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>ESP8266 WiFi Network</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 1.5em; Color: #000000; }\
      h1 { Color: #AA0000; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 Wi-Fi Access Point and Web Server Demo</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>This page refreshes every 10 seconds. Click <a href=\"javascript:window.location.reload();\">here</a> to refresh the page now.</p>\
  </body>\
</html>",

    hour, min % 60, sec % 60
  );
  server.send ( 200, "text/html", html );
}
*/

/*
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST"; // TODO: get this out of there
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}
*/


void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  //set-up the custom IP address
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  
  /* You can remove the password parameter if you want the AP to be open. */
  // WiFi.softAP(ssid, password);
  WiFi.softAP(ssid);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);


  // modify TTL associated  with the domain name (in seconds)
  // default is 60 seconds
  dnsServer.setTTL(300);
  // set which return code will be used for all other domains (e.g. sending
  // ServerFailure instead of NonExistentDomain will reduce number of queries
  // sent by clients)
  // default is DNSReplyCode::NonExistentDomain
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);

  // start DNS server for a specific domain name
  // dnsServer.start(DNS_PORT, "www.example.com", apIP);

  // Catch all and redirect all to the same location
  // Since we are using our hacky version of the library, this does not capture the 3 addresses used to check connectivity
  dnsServer.start(DNS_PORT, "*", apIP);
  // dnsServer.start(DNS_PORT, "clients3.google.com/generate_204", apIP);

  /*
  server.on("/", handleRoot );
  server.on("/led=1", handleRoot);
  server.on("/led=0", handleRoot);
  server.on("/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  */

  // Android-specific URL for checking to see if there is an internet connection
  server.on("/generate_204", []() {
    server.send ( 204, "text/plain", "" );  
  });
  
  // iOS-specific (untested)
  server.on("/hotspot-detect.html", []() {
    server.send ( 200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>" );
  });

  // unused
  // server.onNotFound ( handleNotFound );
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
