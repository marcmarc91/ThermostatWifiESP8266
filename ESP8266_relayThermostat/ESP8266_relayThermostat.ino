#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "Net"
#define STAPSK  "secret123"
#endif

IPAddress SendIP(192, 168, 0, 100);
WiFiUDP udp;

const char *ssid = STASSID;
const char *pass = STAPSK;

const int relayPin = D1;

unsigned int localPort = 2000;
char packetBuffer[1];

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayPin, OUTPUT);


  WiFi.begin(ssid, pass);   //Connect to access point
  WiFi.hostname("WemosRelay");
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(400);
  }

  Serial.begin(9600);
  udp.begin(localPort);
}

void loop()
{
  int cb = udp.parsePacket();
  if (!cb)
  {
    udp.beginPacket(SendIP, localPort);
    char a[1];
    a[0] = char(Serial.read()); //Serial Byte Read
    udp.write(a, 1); //Send one byte to ESP8266
    udp.endPacket();

  }
  else {
    // We've received a UDP packet, send it to serial
    udp.read(packetBuffer, 1); // read the packet into the buffer, we are reading only one byte


    if (packetBuffer[0] == '1') {
      digitalWrite(relayPin, HIGH);
      udp.beginPacket(SendIP, 2000);
      udp.write('2');
      udp.endPacket();
    }
    else if (packetBuffer[0] == '0') {
      digitalWrite(relayPin, LOW);
    }
    // Serial.print(packetBuffer);
    delay(20);
  }


}
