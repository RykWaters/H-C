#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

//server config
const boolean testMode = false;
const boolean largeSign = false;

//Live Wifi Details
char* ssid = "";
char* password = "";
IPAddress staticIP(192, 168, 50, 245); //Large Sign IP
IPAddress staticIPSmall(192, 168, 50, 242); //Small Sign IP
IPAddress gateway(192, 168, 50, 254);   //IP Address of your WiFi Router (Gateway)

ESP8266WebServer server(80);

const bool use_auth = false;
const char* www_username = "hackcraft";
const char* www_password = "smartsign";

IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS

const char* deviceName = "SmashLabs-SmartSign-Large";

//whether wifi connected
boolean wifiConnected = false;
WiFiEventHandler disconnectedEventHandler;

void serverSetup() {
  if (testMode) {
    ssid = ssidTest;
    password = passwordTest;
    staticIP = staticIPTest;
    staticIPSmall = staticIPSmallTest;
    gateway = gatewayTest;
  }
  if (!largeSign) {
    staticIP = staticIPSmall;
    deviceName = "SmashLabs-SmartSign-Small";
  }
  Serial.println();
  Serial.print("Attenmpting to connect with IP:");
  Serial.println(staticIP);
  WiFi.hostname(deviceName);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.config(staticIP, subnet, gateway, dns);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed!");
    wifiConnected = false;
    return;
    //delay(1000);
    //ESP.restart();
  }
  ArduinoOTA.begin();

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event)
  {
    Serial.println("Station disconnected");
    wifiConnected = false;
  });

  server.on("/", handleColourChange);
  server.on("/set_colour", handleColourChange);
  //server.on("/cycle", handleCycle);
  
  server.begin();

  Serial.println();
  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
  wifiConnected = true;
}

void serverLoop() {
  if (wifiConnected) {
    ArduinoOTA.handle();
    server.handleClient();
  } else {
    serverSetup();
  }
}

void handleAuth() {
  if (use_auth && !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
}

void handleRoot() {
  handleAuth();
  server.send(200, "text/plain", "Welcome to the Smart Sign");
}

void handleColourChange() {
  handleAuth();
  String message = "Number of args received:";
  message += server.args();            //Get number of parameters
  message += "\n";                            //Add a new line

  //reset everything
  r = 0;
  g = 0;
  b = 0;
  w = 0;
  wipeDelay = 0;
  numberToWriteFrom = 0;
  numberToWriteTo = NUM_LEDS;
  
  for (int i = 0; i < server.args(); i++) {
    setValueFromArg(server.argName(i), server.arg(i));
  } 
  customColour = strip.Color(r, g, b, w);
  cycling = false;
  colorWipe(customColour, numberToWriteFrom, numberToWriteTo, wipeDelay);
  server.send(200, "text/plain", message);       //Response to the HTTP request
  delay(5000);
  fade(customColour, nextFadeTo, 2000, false, numberToWriteFrom, numberToWriteTo);
}

void setValueFromArg(String argNameStr, String argVal) {
  uint16_t val = argVal.toInt();
  if (val >= 0 && val <= 255) {
    if (argNameStr == "r") {
      r = val;
    }
    if (argNameStr == "g") {
      g = val;
    }
    if (argNameStr == "b") {
      b = val;
    }
    if (argNameStr == "w") {
      w = val;
    }
    if (argNameStr == "delay") {
      wipeDelay = val;
    }
    if (argNameStr == "from") {
      numberToWriteFrom = val;
    }
    if (argNameStr == "to") {
      Serial.print("TO SERVER ");
      Serial.println(val);
      numberToWriteTo = val;
    }
    if (argNameStr == "letter") {
      writeLetter(val);
    }
  }
}

//void handleCycle() {
//  cycling = !cycling;
//  server.send(200, "text/plain", "Toggled Cycle");       //Response to the HTTP request
//}
