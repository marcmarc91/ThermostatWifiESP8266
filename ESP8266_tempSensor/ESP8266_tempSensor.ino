#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "DHT.h"
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define DHTPIN 2
#define DHTTYPE DHT11

#ifndef STASSID
#define STASSID "Net"
#define STAPSK  "secret123"
#endif

DHT dht(DHTPIN, DHTTYPE);
IPAddress SendIP (192, 168, 0, 200);
WiFiUDP udp;
ESP8266WebServer server(80);

const char *ssid = STASSID;
const char *pass = STAPSK;

long previousMillis = 0;
long interval = 6000;

float valueReadTemp;
float valueSetTemp;

unsigned int localPort = 2000;
char packetBuffer[1];

const char HTTP_HTML[] PROGMEM = "<!DOCTYPE html>\
<html>\
<head>\
 <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
  <script>\
    window.setInterval(\"update()\", 2000);\
    function update(){\
      var xHR=new XMLHttpRequest();\
      xHR.open(\"GET\", \"/temp\", true);\
      xHR.onreadystatechange = function() {\
        if (xHR.readyState != XMLHttpRequest.DONE || xHR.status != 200) return;\
        document.getElementById('temp').innerHTML=xHR.responseText;\
      };\
      xHR.send();\
    }\
  </script>\
</head>\
<body style=\"text-align:center\">\
    <h1>My Thermostat</h1>\
    <font size=\"7\"><span id=\"temp\">{0}</span>&deg;</font>\
    <p>\
    <form method=\"post\">\
        Set to: <input type=\"text\" name=\"sp\" value=\"{1}\" style=\"width:50px\"><br/><br/>\
        <input type=\"submit\" style=\"width:100px\">\
    <form>\
    </p>\
</body>\
</html>";

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);   //Connect to access point
  WiFi.hostname("WemosDHT11");

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(400);
  }

  Serial.begin(9600);

  EEPROM.begin(512);
  dht.begin();
  udp.begin(localPort);
  server.on("/", handleRoot);
  server.on("/temp", handleGetTemp);
  server.begin();

  Serial.println("HTTP server started");

  if (isnan(EEPROM.get(1, valueSetTemp))) {
    EEPROM.put(1, 0);
    EEPROM.commit();
  }

  if (isnan(EEPROM.get(0, valueReadTemp))) {
    EEPROM.put(0, 0);
    EEPROM.commit();
  }

  EEPROM.put(1, 0);
  EEPROM.put(0, 0);

}

void loop()
{
  //
  //  unsigned long currentMillis = millis();
  //  if (currentMillis - previousMillis > interval) {
  //    previousMillis = currentMillis;
  //    readTemp();
  //    Serial.print("Temperatura citita: ");
  //    Serial.println(valueReadTemp);
  //  }
  //
  //
  //  int cb = udp.parsePacket();
  //  if (!cb) {
  //    delay(20);
  //    if (checkTemp(valueSetTemp, valueReadTemp) == 0) {
  //      sendCommand('0');
  //    } else if (checkTemp(valueSetTemp, valueReadTemp) == 1) {
  //      sendCommand('1');
  //    }
  //  }
  //  else {
  //
  //    udp.read(packetBuffer, 1);
  //    Serial.print(packetBuffer);
  //    delay(20);
  //  }
  //
  //  server.handleClient();

}


/////////
int checkTemp(float tempSet, float tempRead)
{

  if (tempSet == tempRead) {
    return 0;
  } else {
    return 1;
  }
}

/////////
void readTemp() {
  float temperature = dht.readTemperature();
  EEPROM.put(0, temperature);
  EEPROM.get(0, valueReadTemp);
  EEPROM.commit();

}

/////////
void setTemp(float valueTemperature) {
  EEPROM.put(1, valueTemperature);
  EEPROM.get(1, valueSetTemp);
  EEPROM.commit();
}

/////////
float sendCommand(int pos) {
  udp.beginPacket(SendIP, 2000);
  char a[1];
  a[0] = char(pos);
  udp.write(a, 1); //Send one byte to ESP8266
  udp.endPacket();
}

/////////
void handleRoot() {
  if (server.method() == HTTP_POST) {
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "sp") {
        float   sp = server.arg(i).toFloat();
        setTemp(sp);
        Serial.print(sp);
      }
    }
  }

  String page = FPSTR(HTTP_HTML);
  page.replace("{0}", String((float)valueReadTemp));
  page.replace("{1}", String((float)valueSetTemp));
  server.send(200, "text/html", page);

}


void handleGetTemp() {
  server.send(200, "text/plain", String((float)valueReadTemp));
}
