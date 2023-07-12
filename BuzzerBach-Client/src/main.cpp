#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>


//WiFi-Config
#define WIFI_SSID "BuzzerBach-Robin"
#define WIFI_PASS "BuzzerBach-Robin"
#define UDP_PORT 4210

//Button-Konfig
#define BUZZER D6
#define LED_STRIP D4

WiFiUDP UDP;
char packet[255];
char reply[] = "BUZZED_WHITE";
char reply_CORRECT[] = "GREEN";
char reply_WRONG[] = "RED";
char reply_PAUSED[] = "PAUSED";
char reply_RESET[] = "RESET";

uint8_t con_retries = 0;
bool standalone_mode = false;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, LED_STRIP, NEO_GRB + NEO_KHZ800);
void LED_setColor(int led, int redValue, int greenValue, int blueValue)
{
  pixels.setPixelColor(led, pixels.Color(redValue, greenValue, blueValue)); 
  pixels.show();
}

void LED_reset()
{
  Serial.println("Reset");
  for (int led = 0; led <= 8; led++)
    LED_setColor(led,1,1,1); //OFF.. mostly
}

void LED_correct()
{
  Serial.println("Correct");
  for (int led = 0; led <= 8; led++)
  {
    LED_setColor(led,1,255,1); //GREEN
  }
}

void LED_wrong()
{
  Serial.println("Wrong");
  for (int led = 0; led <= 8; led++)
    LED_setColor(led,255,1,1); //RED
}

void LED_buzzed()
{
  Serial.println("Buzzed");
  for (int led = 0; led <= 8; led++)
    LED_setColor(led,255,255,255); //WHITE
}

void LED_paused()
{
  Serial.println("Paused");
  for (int i = 0; i <= 10; i++)
  {
    delay(200);
    for (int led = 0; led <= 8; led++)
      LED_setColor(led,255,165,1); //ORANGE
  }
}

void LED_initiatlizing()
{
  Serial.println("INIT");
  for (int led = 0; led <= 8; led++)
  {
    LED_setColor(led,1,1,255); //BLUE
    delay(50);
    LED_setColor(led,1,1,1); //OFF.. mostly
  }
  LED_reset();
}

void LED_standalonemode()
{
  Serial.println("Standalone");
  for (int i = 0; i <= 10; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,255,1,255); //FUCHSIA
      delay(50);
      LED_setColor(led,1,1,1); //OFF.. mostly
    }
  }
  LED_reset();
}

void LED_initiatlized()
{
  Serial.println("IINITED");
  for (int i = 1; i <= 255; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,i,1,1);
      delay(1);
    }
  }
  for (int i = 1; i <= 255; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,1,i,1);
      delay(1);

    }
  }
  for (int i = 1; i <= 255; i++)
  {
    for (int led = 0; led <= 8; led++)
    {
      LED_setColor(led,1,1,i);
      delay(1);

    }
  }
  LED_reset();
}

void setup() {
  Serial.begin(9600);
  Serial.println("BuzzerBach-Client-Node");
  pixels.begin();
  //Hardware
  pinMode(BUZZER, INPUT);
  if (digitalRead(BUZZER) == LOW)
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
      LED_initiatlized();
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
      if (strstr(packet,reply))
        LED_buzzed();
      if (strstr(packet,reply_CORRECT))
        LED_correct();
      if (strstr(packet,reply_WRONG))
        LED_wrong();
      if (strstr(packet,reply_PAUSED))
        LED_paused();
      if (strstr(packet,reply_RESET))
        LED_reset();
    }
    if (digitalRead(BUZZER) == LOW)
    {
      UDP.beginPacket(WiFi.gatewayIP(), UDP_PORT);
      UDP.write(reply);
      UDP.endPacket();
    }
  }
  else
  {
    if (digitalRead(BUZZER) == LOW)
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