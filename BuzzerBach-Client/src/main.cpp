#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>


//WiFi-Config
#define WIFI_SSID "BuzzerBach-Robin"
#define WIFI_PASS "BuzzerBach-Robin"
#define UDP_PORT 4210

//Button-Konfig
#define BUZZER D1
#define LED_STRIP D2

WiFiUDP UDP;
char packet[255];
char reply[] = "BUZZED_WHITE";
char reply_CORRECT[] = "GREEN";
char reply_WRONG[] = "RED";
char reply_PAUSED[] = "PAUSED";

uint8_t con_retries = 0;
bool standalone_mode = false;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, LED_STRIP, NEO_GRB + NEO_KHZ800);
void LED_setColor(int led, int redValue, int greenValue, int blueValue)
{
  pixels.setPixelColor(led, pixels.Color(redValue, greenValue, blueValue)); 
  pixels.show();
}

void LED_correct()
{
  for (int led = 0; led <= 8; led++)
    LED_setColor(led,1,255,1); //GREEN
}

void LED_wrong()
{
  for (int led = 0; led <= 8; led++)
    LED_setColor(led,255,1,1); //RED
}

void LED_buzzed()
{
  for (int led = 0; led <= 8; led++)
    LED_setColor(led,255,255,255); //WHITE
}

void LED_paused()
{
  for (int i = 0; i <= 10; i++)
  {
    delay(200);
    for (int led = 0; led <= 8; led++)
      LED_setColor(led,255,165,1); //ORANGE
  }
}

void LED_reset()
{
  for (int led = 0; led <= 8; led++)
    LED_setColor(led,1,1,1); //OFF.. mostly
}

void LED_initiatlizing()
{
  for (int led = 0; led <= 8; led++)
  {
    LED_setColor(led,1,1,255); //BLUE
    delay(200);
    LED_setColor(led,1,1,1); //OFF.. mostly
  }
  LED_reset();
}

void LED_standalonemode()
{
  for (int i = 0; i <= 10; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,255,1,255); //FUCHSIA
      delay(200);
      LED_setColor(led,1,1,1); //OFF.. mostly
    }
  }
  LED_reset();
}


void LED_initiatlized()
{
  for (int i = 1; i <= 255; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,i,1,1);
      delay(10);
    }
  }
  for (int i = 1; i <= 255; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,255,i,1);
      delay(10);

    }
  }
  for (int i = 1; i <= 255; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,255,255,i);
      delay(10);

    }
  }
  LED_reset();
}

void setup() {
  Serial.begin(115200);
  Serial.println("BuzzerBach-Client-Node");
  pixels.begin();
  //Hardware
  pinMode(BUZZER, INPUT);
  if (digitalRead(BUZZER) == HIGH)
  {
    Serial.println("Buzzer im Standalone-Mode gebootet!");
    standalone_mode = true;
    LED_standalonemode();
  }
  if(!standalone_mode)
  {
    Serial.print("Initialize Station.");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while(WiFi.status()!=WL_CONNECTED && con_retries<20)
    {
      Serial.print(".");
      con_retries++;
      LED_initiatlizing();
    }
    Serial.println("");
    if (con_retries == 20)
    {
      Serial.println("Master-Node nicht gefunden!");
      Serial.println("Buzzer im Standalone-Mode gebootet!");
      standalone_mode = true;
    }
    if(WiFi.status()==WL_CONNECTED)
    {
      Serial.print("Local-Address: ");
      Serial.println(WiFi.localIP());

      UDP.begin(UDP_PORT);
      Serial.print("Listening on UDP port ");
      Serial.println(UDP_PORT);
    }
  }
}

long unsigned buzzed_millis = millis();

void loop() {
  if (!standalone_mode)
  {
    int packetSize = UDP.parsePacket();
    if (packetSize) 
    {
      Serial.print("Received packet! Size: ");
      Serial.println(packetSize); 
      int len = UDP.read(packet, 255);
      if (len > 0)
      {
        packet[len] = '\0';
      }
      Serial.print("Packet received: ");
      Serial.println(packet);
      if (packet == reply)
        LED_buzzed();
      if (packet == reply_CORRECT)
        LED_correct();
      if (packet == reply_WRONG)
        LED_wrong();
      if (packet == reply_PAUSED)
        LED_paused();
    }
    if (digitalRead(BUZZER) == HIGH)
    {
      UDP.beginPacket(WiFi.gatewayIP(), UDP_PORT);
      UDP.write(reply);
      UDP.endPacket();
    }
  }
  else
  {
    if (digitalRead(BUZZER) == HIGH)
    {
      buzzed_millis = millis()+(1000*10);
      LED_buzzed();
    }
    if(millis() > buzzed_millis)
    {
      LED_reset();
    }
  }
}