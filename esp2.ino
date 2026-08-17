#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "...";
const char* password = "...";

WiFiUDP udp;
const int udpPort = 4210;
const int ledPin = 26;
const int buzzerPin = 25;

unsigned long lastPacketTime = 0;
const long connectionTimeout = 5000;
bool connectionLost = false;

unsigned long lastBlink = 0;
bool ledState = false;
const long blinkInterval = 300;

String lastStatus = "NORMAL";

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Gateway IP address: ");
  Serial.println(WiFi.localIP());

  udp.begin(udpPort);
  Serial.println("Listening for UDP packets...");
}

void loop() {
  unsigned long now = millis();

  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) incomingPacket[len] = 0;
    String data = String(incomingPacket);
    lastPacketTime = now;
    connectionLost = false;

    int statusIndex = data.indexOf("STATUS=");
    if (statusIndex != -1) lastStatus = data.substring(statusIndex + 7);

    Serial.println("-------------------------");
    Serial.println(data);
    Serial.println("-------------------------");
  }

  if (!connectionLost && lastPacketTime != 0 && (now - lastPacketTime > connectionTimeout)) {
    connectionLost = true;
    Serial.println("DEVICE ESP32_01");
    Serial.println("CONNECTION LOST");
  }

  // LED: solid on during CRITICAL, blinking during WARNING, off during NORMAL
  if (lastStatus == "CRITICAL") {
    digitalWrite(ledPin, HIGH);
  } else if (lastStatus == "WARNING") {
    if (now - lastBlink >= blinkInterval) { lastBlink = now; ledState = !ledState; digitalWrite(ledPin, ledState); }
  } else {
    digitalWrite(ledPin, LOW);
  }

  // Buzzer: continuous during CRITICAL, intermittent during WARNING, silent during NORMAL
  if (lastStatus == "CRITICAL") digitalWrite(buzzerPin, HIGH);
  else if (lastStatus == "WARNING") digitalWrite(buzzerPin, (now / 500) % 2);
  else digitalWrite(buzzerPin, LOW);
}