#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "DCIV-MqjQicVwnR54IO819UBTPWVeslt";
char ssid[] = "BELL971";
char pass[] = "Nasnajnay3";

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
}

