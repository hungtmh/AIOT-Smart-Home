# NodeMCU ESP8266 LED MQTT

Use this sketch for the NodeMCU ESP8266 board shown in the photo.

## Wiring

Use an external LED on `D1`:

```text
NodeMCU D1 -> 220 ohm resistor -> LED long leg/anode
NodeMCU G  -> LED short leg/cathode
```

The code uses:

```cpp
const int LED_PIN = D1;
```

## Arduino IDE

Install:

- ESP8266 board package
- PubSubClient library

Select board:

```text
NodeMCU 1.0 (ESP-12E Module)
```

Serial Monitor baud:

```text
115200
```
