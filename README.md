# ESP32 Wireless Sensor and Telematics Data Collection System

A small-scale IoT/telematics system built with two ESP32 boards. One board (Sensor Node) reads data from multiple sensors and sends it wirelessly over UDP to a second board (Gateway), which displays the data and raises a visual/audible alarm when values cross defined thresholds.

## System Architecture

```
 HC-SR04 ──────┐
 Gas Sensor ───┤
 Rain Sensor ──┤
 PIR ──────────┴──> ESP32-1 (Sensor Node) ──Wi-Fi/UDP──> ESP32-2 (Gateway) ──> Serial Monitor + LED + Buzzer
```

- **ESP32-1 (Sensor Node):** reads all sensors on independent, non-blocking timers, computes a system status, and sends a UDP packet to the Gateway once per second.
- **ESP32-2 (Gateway):** listens for UDP packets, prints the data to the Serial Monitor, detects connection loss, and drives a red LED and a buzzer based on the received status.

## Hardware Used

| Component | Notes |
|---|---|
| 2x ESP32 Dev Module | Sensor Node + Gateway |
| HC-SR04 | Ultrasonic distance sensor |
| MH-series rain sensor module | Analog output (AO) |
| MQ-series gas sensor | Analog output (AOUT) |
| PIR motion sensor (HC-SR501 type) | Digital output |
| Red LED + 220Ω resistor | Alarm indicator (Gateway) |
| Active buzzer | Alarm indicator (Gateway) |
| Resistors for voltage dividers | See wiring below |

## Wiring — ESP32-1 (Sensor Node)

| Sensor | Pin | Notes |
|---|---|---|
| HC-SR04 TRIG | D5 | |
| HC-SR04 ECHO | D18 | Through a voltage divider (two equal resistors, ~4.7kΩ each) — ECHO outputs 5V, ESP32 GPIO max is 3.3V |
| Rain sensor VCC | 3.3V pin | Powered at 3.3V (not 5V) so AO output stays within safe ADC range |
| Rain sensor AO | D35 | |
| Gas sensor VCC | 5V | Needs 5V for its heating element |
| Gas sensor AOUT | D34 | Through a voltage divider (two equal resistors, ~4.7kΩ each) |
| PIR OUT | D19 | Output is already 3.3V logic level, no divider needed |

All GND and VCC (5V) lines are shared on a common power rail.

## Wiring — ESP32-2 (Gateway)

| Component | Pin | Notes |
|---|---|---|
| LED anode (+) | D26 | Through a 220Ω resistor |
| LED cathode (−) | GND | |
| Buzzer (+) | D25 | |
| Buzzer (−) | GND | |

No sensors are connected to the Gateway board.

## UDP Packet Format

Plain key-value text format, chosen for simplicity and easy debugging over Serial Monitor without needing a parsing library:

```
DEVICE=ESP32_01 DISTANCE=135.0 RAIN=4095 GAS=420 PIR=0 STATUS=NORMAL
```

## Status Logic

The Sensor Node evaluates all readings each cycle and reports one of three states:

- **NORMAL** — all readings within safe range
- **WARNING** — one or more readings past the warning threshold
- **CRITICAL** — one or more readings past the critical threshold (takes priority over WARNING)

| Condition | Warning | Critical |
|---|---|---|
| Distance | ≤ 8 cm | ≤ 4 cm |
| Gas | ≥ 3200 | ≥ 3800 |
| Rain | ≤ 2000 | — |
| Motion (PIR) | detected | — |

On the Gateway, the LED blinks and the buzzer beeps intermittently during WARNING; the LED stays solid on and the buzzer sounds continuously during CRITICAL. Both turn off automatically once the status returns to NORMAL.

## Setup

1. Open each sketch in Arduino IDE (ESP32 board support required).
2. In both `ESP32_1_SensorNode.ino` and `ESP32_2_Gateway.ino`, set your Wi-Fi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
3. Upload the Gateway code first, open its Serial Monitor (115200 baud), and note the printed IP address.
4. In the Sensor Node code, update:
   ```cpp
   IPAddress gatewayIP(x, x, x, x); // Gateway's IP address
   ```
5. Upload the Sensor Node code.
6. Power both boards and monitor the Gateway's Serial Monitor for incoming data.

## Connection Loss Detection

If the Gateway does not receive a packet for more than 5 seconds, it prints:

```
DEVICE ESP32_01
CONNECTION LOST
```
