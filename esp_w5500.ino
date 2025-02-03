/***** Blynk and Wi-Fi Credentials *****/
#define BLYNK_TEMPLATE_ID   "TMPL2zKGp-Sin"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN    "DCIV-MqjQicVwnR54IO819UBTPWVeslt"

/***** Library Includes *****/
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>   // Blynk over Wi-Fi
#include <SPI.h>
#include <Ethernet2.h>           // W5500-compatible Ethernet library
#include <EthernetUdp.h>         // UDP support for Ethernet

const char* ssid     = "BELL971";
const char* password = "Nasnajnay3";

/***** Ethernet (W5500) Settings *****/
// MAC for the W5500 (ensure it’s unique on your network)
byte deviceMac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Optional: use a static IP for the Ethernet module if required.
IPAddress localIp(192, 168, 1, 177);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 1, 1);

// Broadcast IP & port for sending WoL packet.
IPAddress broadcastIp(192, 168, 1, 255);
const uint16_t broadcastPort = 9; // Port often used for WoL

// Target device MAC address (the device to wake)
byte targetMac[6] = { 0xd0,0x11,0xe5,0x82,0xff,0xa7 };

// W5500 Chip Select pin, set according to your wiring.
const int W5500_CS_PIN = D1;

/***** Global Instances *****/
EthernetUDP Udp;    // Used for sending the WoL packet
BlynkTimer timer;   // For scheduled tasks

/***** Blynk Virtual Pin Handgit ler *****/
// Attach this to a button on Virtual Pin V1.
// When the button is pressed via Blynk, the WoL packet is sent.
BLYNK_WRITE(V1) {
  int buttonState = param.asInt();
  if (buttonState) {  // When button is pressed on the app.
    Serial.println("Blynk triggered: Sending WoL packet...");
    sendWOL();
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  //----------------------------
  // 1. Initialize Wi-Fi & Blynk
  //----------------------------

  // Connect to Wi-Fi for Blynk operations.
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize Blynk on Wi-Fi.
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("Blynk started over Wi-Fi.");

  //--------------------------------
  // 2. Initialize Ethernet (W5500)
  //--------------------------------

  // Set the CS pin for W5500.
  pinMode(W5500_CS_PIN, OUTPUT);
  digitalWrite(W5500_CS_PIN, HIGH);

  // Assign CS to the Ethernet library.
  Ethernet.init(W5500_CS_PIN);

  // Try DHCP first.
  if (Ethernet.begin(deviceMac) == 0) {
    Serial.println("Ethernet DHCP failed. Using static IP.");
    Ethernet.begin(deviceMac, localIp, gateway, subnet);
  }
  delay(1000);
  Serial.print("W5500 Ethernet IP: ");
  Serial.println(Ethernet.localIP());

  // Start UDP service on Ethernet.
  if (Udp.begin(8888) == 1) {
    Serial.println("UDP (W5500) started");
  } else {
    Serial.println("UDP (W5500) start failed");
  }

  // Optional Timer: Check Blynk connection status every 10 seconds.
  timer.setInterval(10000L, []() {
    Serial.print("Blynk connected: ");
    Serial.println(Blynk.connected() ? "Yes" : "No");
  });
}

void loop() {
  // Run Blynk tasks over Wi-Fi.
  Blynk.run();

  // Run scheduled timers.
  timer.run();

  // You could perform additional tasks here if needed.
}

/***** Function to Send the WoL Magic Packet ******/
void sendWOL() {
  // The magic packet is 6 bytes of 0xFF, then 16 repetitions of the target MAC.
  const int packetSize = 6 + 16 * 6;
  byte packet[packetSize];

  // Build the packet.
  for (int i = 0; i < 6; i++) {
    packet[i] = 0xFF;
  }
  for (int i = 1; i <= 16; i++) {
    memcpy(packet + i * 6, targetMac, 6);
  }

  Serial.println("Sending Wake-on-LAN packet via Ethernet (W5500)...");
  Udp.beginPacket(broadcastIp, broadcastPort);
  Udp.write(packet, packetSize);
  Udp.endPacket();
  Serial.println("Magic packet sent!");
}
