# Smart Cart Using Arduino Uno And ESP32
<br>

<h3> 🔌 Components And Connections </h3>

### 🧠 Arduino UNO Connections

| Component / Module           | Signal           | Arduino UNO Pin | Notes |
|-----------------------------|------------------|------------------|-------|
| **I2C LCD (16x4)**          | SDA              | A4               | I2C |
|                             | SCL              | A5               |  ―   |
|                             | VCC              | 5V               |  ―   |
|                             | GND              | GND              |  ―   |
| **USB Host Shield**         | SPI              | D10–D13          | Mounts on UNO |
|                             | 5V, GND          | 5V, GND          | Required for barcode scanner |
| **Barcode Scanner**         | USB Plug         | USB Host Shield  | Plug directly |
| **Thermal Printer (TTL)**   | TX (Printer ➜ Uno)| D5 (Software RX) | Uses SoftwareSerial |
|                             | RX (Uno ➜ Printer)| D6 (Software TX) | Baud: 19200 |
|                             | VCC              | External 5V (2A) | Do **not** use Arduino 5V |
|                             | GND              | GND              | Common GND with Arduino |
| **RTC Module (DS3231)**     | SDA              | A4               | I2C shared with LCD |
|                             | SCL              | A5               |  ―   |
|                             | VCC              | 5V               |  ―   |
|                             | GND              | GND              |  ―   |
| **ESP32 (for cloud link)**  | TX (ESP32)       | D7 (Uno RX)      | Use voltage divider (5V ➜ 3.3V) |
|                             | RX (ESP32)       | D8 (Uno TX)      | Use level shifter |
|                             | GND              | GND              | Must share GND |
|                             | VCC              | External 3.3V    | Use separate LDO regulator if needed |

---

### 📶 ESP32 Connections

| Component       | Signal | ESP32 Pin | Notes |
|----------------|--------|-----------|-------|
| **Serial Comm to Arduino UNO** | TX | GPIO17 (or any free TX) | Send product data |
|                             | RX | GPIO16 (or any free RX) | Receive barcode |
|                             | GND | GND       | Common GND |
|                             | VCC | 3.3V      | Supply from USB or LDO |

> **Note**: Update `Serial.begin()` or `Serial1.begin()` in your ESP32 code based on pins used.

---

### ⚠️ Power Supply Notes

- **Thermal Printer**: Needs **5V 2A** supply (use adapter or Li-ion + boost converter).
- Do **not power printer from Arduino UNO's 5V pin** — it can't supply enough current.
- ESP32 needs **3.3V**, not 5V! Always use a **level shifter** or resistor divider on `RX` line.
- Make sure all **GNDs are connected together** (UNO, printer, ESP32, external supplies).

---

### 🧾 Optional Enhancements

- **Checkout barcode** value (`3366`) triggers bill print.
- Use **Google Sheets** via ESP32 to fetch product details using barcode.
- Connect **status LED or buzzer** to UNO D9/D10 if needed for feedback.

---

### 🔁 Communication Summary

- **Barcode scanner** ➜ Arduino UNO via USB Host Shield  
- **UNO sends barcode to ESP32** via Serial (D7-D8)  
- **ESP32 searches barcode in Google Sheets**  
- **ESP32 sends product name + price back to UNO**, which updates LCD & cart  
- **At checkout (3366)** ➜ Thermal printer prints invoice

