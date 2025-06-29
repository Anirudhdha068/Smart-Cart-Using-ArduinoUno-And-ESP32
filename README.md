# Smart Cart Using Arduino Uno And ESP32
<br>


<h3>Conponets </h3>
Arduino Uno <br>
ESP32 <br>
Thermal Printer <br>
Jumper Wires <br>
LCD Screen 16*4 <br>
USB Barcode Scanner <br>
USB Host Shilde <br>
<br>
<br>
## Hardware Connections  🛠️

| Module / Part | Signal | Arduino UNO Pin(s) | Notes / Tips |
|---------------|--------|-------------------|--------------|
| **LCD 16×4 (I2C backpack)** | SDA | **A4** | I²C bus (pull-ups already on backpack) |
| | SCL | **A5** |  ― |
| | VCC | 5 V | From UNO (≈20 mA) |
| | GND | GND |  ― |
| **USB Host Shield** | — | SPI pins **D10-D13** (plus 5 V & GND) | Sits on top of UNO; provides USB-A port |
| **Barcode Scanner (USB)** | USB-A plug | USB Host Shield port | Powered from shield; no extra wiring |
| **Thermal Printer (CSN-A1X, TTL-serial)** | TX (printer ➜ Arduino) | **D2** (SoftwareSerial RX) | Level-compatible at 5 V |
| | RX (printer ⬅ Arduino) | **D3** (SoftwareSerial TX) |  ― |
| | VCC | **External 5 V / 2 A** | Printer peaks at 1.5 A when heating |
| | GND | Common GND | Tie supply ground to UNO ground |
| **RTC DS3231** | SDA | **A4** | Shares I²C bus with LCD |
| | SCL | **A5** |  ― |
| | VCC | 5 V | CR2032 keeps time offline |
| | GND | GND |  ― |
| **Buzzer / Status LED** (optional) | + | **D6** (PWM) | Audible/visual feedback |
| | – | GND |  ― |
| **Wi-Fi Module (ESP-01 / ESP8266)** *(for cloud logging)* | TX | **D8** (SoftwareSerial RX) | 3 V-only; use level shifter |
| | RX | **D7** (SoftwareSerial TX) |  ― |
| | VCC | 3.3 V (≦300 mA) | Separate 3 V reg recommended |
| | GND | Common GND |  ― |

### Power Guidelines
- **Printer** needs a dedicated 5 V ≥ 2 A source.  
- Tie **all grounds together** to avoid weird resets.  
- If the LCD backlight flickers when the printer fires, add a 470 µF capacitor across the printer’s 5 V & GND.

### I²C Address Reference
| Device | Default Addr |
|--------|--------------|
| LCD backpack | `0x27` or `0x3F` |
| DS3231 RTC | `0x68` |

```cpp
// Example SoftwareSerial mappings (grabs printer + Wi-Fi on the fly)
#include <SoftwareSerial.h>
SoftwareSerial Printer (2, 3);   // RX, TX  ➜ Thermal printer
SoftwareSerial WiFi    (8, 7);   // RX, TX  ➜ ESP-01
