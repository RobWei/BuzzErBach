#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


//WiFi-Config
#define WIFI_SSID "BuzzerBach-Robin"
#define WIFI_PASS "BuzzerBach-Robin"
#define UDP_PORT 4210

//Button-Konfig
#define RESET_ROUND D1
#define PAUSE_ROUND D8
#define CORRECT_ROUND D2
#define WRONG_ROUND D3

WiFiUDP UDP;
char packet[255];
char reply[] = "BUZZED_WHITE";
char reply_CORRECT[] = "GREEN";
char reply_WRONG[] = "RED";
char reply_PAUSED[] = "PAUSED";
char reply_RESET[] = "RESET";

void setup() {
  Serial.begin(9600);
  Serial.println("BuzzerBach-Master-Node");
  Serial.print("Initialize Soft-AP.");
  while(!WiFi.softAP(WIFI_SSID, WIFI_PASS))
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("");
  Serial.print("Local-Address: ");
  Serial.println(WiFi.localIP());

  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port ");
  Serial.println(UDP_PORT);

  //Hardware Buttons
  pinMode(RESET_ROUND, INPUT);
  pinMode(PAUSE_ROUND, INPUT);
  pinMode(CORRECT_ROUND, INPUT);
  pinMode(WRONG_ROUND, INPUT);
}

bool paused = false;
int buzzed = 0;
IPAddress buzzed_ip;
IPAddress buzzed_ip_dummy;

void loop() {
  int packetSize = UDP.parsePacket();
  if (packetSize && paused) 
  {
    UDP.beginPacket(UDP.remoteIP(), UDP_PORT);
    UDP.write(reply_PAUSED);
    UDP.endPacket();
  }
  if (packetSize && buzzed == 0) 
  {

      buzzed = UDP.remoteIP()[3];
      buzzed_ip = UDP.remoteIP();
      buzzed_ip_dummy = UDP.remoteIP();
      Serial.print("Buzzed-ID: ");
      Serial.println(buzzed); 
      Serial.print("Received packet! Size: ");
      Serial.println(packetSize); 
      int len = UDP.read(packet, 255);
      if (len > 0)
      {
        packet[len] = '\0';
      }
      Serial.print("Packet received: ");
      Serial.println(packet);
      UDP.beginPacket(UDP.remoteIP(), UDP_PORT);
      UDP.write(reply);
      UDP.endPacket();

  }
  if (digitalRead(CORRECT_ROUND) == LOW && buzzed != 0)
  {
    Serial.println("Correct");
    UDP.beginPacket(buzzed_ip, UDP_PORT);
    UDP.write(reply_CORRECT);
    UDP.endPacket();
    delay(5000);
    UDP.beginPacket(buzzed_ip, UDP_PORT);
    UDP.write(reply_RESET);
    UDP.endPacket();
    buzzed = 0;
  }
  if (digitalRead(WRONG_ROUND) == LOW && buzzed != 0)
  {
    Serial.println("Wrong");
    UDP.beginPacket(buzzed_ip, UDP_PORT);
    UDP.write(reply_WRONG);
    UDP.endPacket();
    delay(5000);
    UDP.beginPacket(buzzed_ip, UDP_PORT);
    UDP.write(reply_RESET);
    UDP.endPacket();
    buzzed = 0;
  }
  if (digitalRead(RESET_ROUND) == LOW)
  {
    Serial.println("RESET");
    UDP.beginPacket(buzzed_ip, UDP_PORT);
    UDP.write(reply_RESET);
    UDP.endPacket();
    buzzed = 0;
    delay(1000);
  }
}
