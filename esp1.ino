#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "...";
const char* password = "...";

WiFiUDP udp;
const int udpPort = 4210;
IPAddress gatewayIP(172, 31, 236, 191); // Gateway's most recent IP address

const int trigPin = 5;
const int echoPin = 18;
const int rainPin = 35;
const int gasPin = 34;
const int pirPin = 19;

unsigned long lastDistanceRead = 0, lastPirRead = 0, lastGasRead = 0, lastRainRead = 0, lastSend = 0;
const long distanceInterval = 100, pirInterval = 100, gasInterval = 500, rainInterval = 1000, sendInterval = 1000;

float distance = 0;
int gasValue = 0, rainValue = 0, pirValue = 0;

const float DISTANCE_WARNING = 8;
const float DISTANCE_CRITICAL = 4;
const int GAS_WARNING = 3200;
const int GAS_CRITICAL = 3800;
const int RAIN_WARNING = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Sensor Node IP address: ");
  Serial.println(WiFi.localIP());
}

// Reads distance from the HC-SR04 ultrasonic sensor
float readDistance() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.0343 / 2;
}

// Determines overall system status based on all sensor readings
String calculateStatus() {
  if (distance > 0 && distance <= DISTANCE_CRITICAL) return "CRITICAL";
  if (gasValue >= GAS_CRITICAL) return "CRITICAL";
  if (distance > 0 && distance <= DISTANCE_WARNING) return "WARNING";
  if (gasValue >= GAS_WARNING) return "WARNING";
  if (rainValue <= RAIN_WARNING) return "WARNING";
  if (pirValue == 1) return "WARNING";
  return "NORMAL";
}

void loop() {
  unsigned long now = millis();

  if (now - lastDistanceRead >= distanceInterval) { lastDistanceRead = now; distance = readDistance(); }
  if (now - lastPirRead >= pirInterval) { lastPirRead = now; pirValue = digitalRead(pirPin); }
  if (now - lastGasRead >= gasInterval) { lastGasRead = now; gasValue = analogRead(gasPin); }
  if (now - lastRainRead >= rainInterval) { lastRainRead = now; rainValue = analogRead(rainPin); }

  if (now - lastSend >= sendInterval) {
    lastSend = now;
    String status = calculateStatus();
    String packet = "DEVICE=ESP32_01 DISTANCE=" + String(distance, 1) +
                     " RAIN=" + String(rainValue) + " GAS=" + String(gasValue) +
                     " PIR=" + String(pirValue) + " STATUS=" + status;
    udp.beginPacket(gatewayIP, udpPort);
    udp.print(packet);
    udp.endPacket();
  }
}